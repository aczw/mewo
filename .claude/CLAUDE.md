# mewo

## Memory / notes for Claude
- Keep persistent notes about this project and how we work together **in this file** (or elsewhere under `.claude/`), not in the user-level memory directory (`~/.claude/projects/.../memory/`).
- `.claude/` and `CLAUDE.md` are the only places Claude may edit without asking.
- Keep this file to durable content: vision, conventions, and decisions (with the reasoning). No progress logs or session-by-session status. Remove anything that goes stale.

## How we work
- **Advise, don't edit.** The user writes the code themselves to learn. Give design guidance, explanations, reviews, and small illustrative snippets in chat. Only touch project files outside `.claude/` / `CLAUDE.md` when explicitly asked (the user has done this for mechanical config, e.g. writing `CMakePresets.json` entries).
- **Reviews:** when the user asks "how am I doing?", read the actual files (don't assume earlier findings were fixed), group findings into "will break", "builds but undermines the intent", and "polish", and explain the *why*. Running read-only checks (configure, build, ctest, `cmake --list-presets`) to verify is fine.

## User's expertise
- The `senior-project` branch reflects the user's current level, no more and no less: comfortable with modern C++ (C++23, variants/visitors, RAII), CMake and submodules, WebGPU basics (pipelines, bind groups, uniform buffers and alignment, shader modules), Dear ImGui, and SDL3.
- Less experience with: application architecture and layering, async/non-blocking design, multi-pass rendering and render graphs, web/WASM targets. Explain those from first principles; skip the basics they already know.

## Project vision
- mewo is roughly "Shadertoy, but native": a desktop app for writing and running shaders live.
- Long-term (aspirational, not current scope): run the same engine on the web as a platform.
- Design implication: keep the core portable (WebGPU-style API, platform glue confined to a host layer, no OS calls/blocking/threads assumed in the core), but don't build for the web yet.

## Prior art
- Branch `senior-project` holds the previous version (school senior design project): C++23, Dawn/WebGPU, WGSL, SDL3, Dear ImGui. **Reference only; don't reuse its code.**
- Lessons from it: the UI owned core concerns (shader prelude in `Editor`, `Project` depended on `Editor`); the model was single-pass with no passes/channels; one god event variant + god class; blocking shader compilation (`WaitAny`) wouldn't work on the web; time/frame counters were tangled with gfx and ImGui; the uniform layout was duplicated by hand between C++ and the WGSL prelude.

## Decisions
_Record each decision with a short "why"._

### Architecture
- **Ghostty-style split (tentative):** a core library, `libmewo`, plus separate frontends in any language (e.g. Swift/SwiftUI on macOS, HTML/CSS on web, something on Windows/Linux). Why: inspired by Ghostty's libghostty + apprt design, and it enforces the core/UI separation the previous version lacked.
- **libmewo exposes a C API and is written against the standard `webgpu.h`** (webgpu-native/webgpu-headers), not a specific implementation's API. The implementation (Dawn, wgpu-native, or emdawnwebgpu for the browser) is picked at build time. Why: one core for native and Wasm; no privileged frontend language.
- **libmewo internals are C++; the public API is pure C** (`mewo.h`). Why: the user knows modern C++ well, so the new territory is the architecture and C API design. Rules: no exceptions or C++ types cross the boundary, opaque handles, explicit ownership.
- **Primary render handoff: the host gives libmewo a native surface** (CAMetalLayer, HWND, Wayland surface, canvas) and libmewo creates its own `WGPUSurface`. A CPU readback path is a possible fallback, and is useful for export and thumbnails anyway.
- **Development happens on both Windows and macOS.** That's why this file lives in the repo: it syncs between machines through git. The core and the reference host must build on both.
- gpui deprioritized: it has no clean way to embed external GPU content (upstream has nothing documented; forks like gpui-ce are experimenting).

### C API object model
- **`mewo_runtime_t`**: process-wide engine. Owns the WebGPU instance/adapter/device/queue, shared caches, and the event queue.
- **`mewo_project_t`**: the document, pure data, **no GPU**. Holds everything that gets saved: metadata, the ordered pass list (name, source, output format/size, named inputs), `main_pass` flag, output pass, shared (Common) source, assets, optional authoring intent (preferred size/aspect), per-pass revision counters.
- **`mewo_session_t`**: one running instance of a project (the user chose "session" over "view"/"player"). Owns everything that exists because the project is being rendered: playback clock, input state, render size, per-pass compiled modules / last-good pipelines / status / diagnostics / output textures / bind groups, uniform buffers, GPU copies of assets, attached surface, frame stats. Never saved. Can run headless (export, thumbnails). Each session has independent simulation state.
- **`mewo_surface_t`**: a native drawing target from the host; a session optionally presents to one.
- A session **reconciles** with its project on each render using the revision counters: recompile edited passes (keep the last good pipeline until the new one compiles), reallocate textures on format/size changes, rebuild bind groups on input changes, rebuild the execution plan on add/remove/reorder.
- **Reference counting for all C API objects**, `webgpu.h`-style (`*_add_ref` / `*_release`). Why: GC'd and ARC hosts (JS, Swift) can't easily guarantee destruction order. The user is new to this; see the learning list.
- **Async results come through a pollable event queue** (`mewo_runtime_poll_event`), not callbacks. Why: no reentrancy hazards, and it fits every frontend. An optional `wakeup` callback for hosts that sleep between frames is a maybe, not a promise.
- Threading: all calls for a runtime happen on one thread; libmewo does no locking (for now).
- File I/O: libmewo never touches the filesystem. The core owns the project format (parsing/serializing); the host moves the bytes (provides referenced files, writes serialized output, watches files for hot reload).

### Pass model
- A project is an **ordered list of passes**; execution order = list order. Users can add passes before the main pass (Shadertoy-style buffers) and after it (post-processing, e.g. blur). One pass is the **output** (presented/exported; defaults to last). Why: post-processing chains are something Shadertoy lacks; a plain ordered list keeps the core simple and predictable.
- **No special Image pass in the core.** A `main_pass` flag marks the user's main pass for the UI and for Shadertoy import; the core ignores it otherwise. "Pre / main / post" grouping is a UI concept.
- **Inputs bind to sources with an explicit frame:** `current` (only valid from earlier passes) or `previous` (any pass, including itself = feedback). Reordering never silently changes meaning, and `current` edges can't form cycles. Double buffering is inferred (only passes read as `previous`). Any pass may read any earlier pass, e.g. post passes reading pre-image buffers.
- **Invalid states are allowed but reported.** Why: editors need transient invalid states (e.g. mid-reorder). The project exposes validation results (no GPU needed); the UI highlights problems; the session binds a **black fallback texture** for broken inputs (a visible signal) and emits a warning event.
- Other sources: "previous pass in list" (makes post effects reorderable/reusable), image textures, keyboard, later video/audio. Sampling (filter/wrap) is per input binding.
- Per-pass output format (rgba8 / rgba16f / rgba32f) and size (match session, scale, fixed).
- Pass IDs are stable opaque integers from the core, never list indices.
- Common code is a project-level shared source prepended to every pass, not a pass.
- **Named inputs:** each pass input has a user-chosen name used directly in WGSL (e.g. `bloom`). Names default to `in_0`, `in_1`, ... (the UI shows inputs as an ordered list so the numbering reads clearly). Smart defaults derived from the source (e.g. `blur_h`, `noise`) were considered "too magic" for now and may come later; if added, name once at creation and never auto-rename. Names must be valid, unique WGSL identifiers that don't collide with keywords or prelude names. Why: readability, especially in post chains.
- **Feedback state on shader edits (v1):** source edits just swap the pipeline, so buffer contents naturally survive. Format/size changes reallocate that pass's textures (contents reset). An explicit session `reset` clears all buffers and the clock. Deferred: preserving contents across reallocation, and a "reset on compile" option.

### Shader contract
- The contract (entry point, built-ins, input bindings, coordinate conventions) is an API: projects store a **contract version**.
- **WGSL is the only shader language** (browsers only accept WGSL). Shadertoy compatibility is not a goal.
- **Entry point:** the user writes `fn main(fragCoord: vec2f) -> vec4f`. The core owns the vertex shader (fullscreen triangle) and generates the real `@fragment` entry point, which calls the user's `main`. Generated identifiers use a reserved `mewo_` prefix.
- **Inputs in WGSL:** an input named `bloom` is exposed as `bloom_tex` (texture) and `bloom_smp` (sampler). Why: WGSL forbids textures/samplers in structs, so a `bloom.handle`-style struct is impossible; both suffixes abbreviated for consistency. The bare name stays free for user variables, e.g. `let bloom = textureSample(bloom_tex, bloom_smp, uv);`.
- **`mw` fields (v1):** `resolution: vec2f` (this pass's output size), `time: f32` (seconds), `delta_time: f32`, `frame: u32`, `frame_rate: f32`, `mouse: { position: vec2f, click: vec2f, down: bool }`. Mouse is always tracked (including hover), in this pass's pixel space, y-up (stored normalized in the uniform buffer, scaled in generated init). `date` dropped (the core can't read OS clocks; hurts deterministic export). Start minimal: adding fields later is non-breaking, removing is breaking. Possible later: `mouse.buttons`, `mouse.pressed`, `mouse.inside`.
- **Time precision:** WGSL has no `f64`/`u64`/`i64`, so shader-side time stays `f32` seconds (step size ~4 ms at 10 h, ~8 ms at a day; fine for v1). The CPU-side clock is kept as an exact 64-bit integer tick count (or double) and converted to `f32` each frame, so there's no accumulated drift. A precision-friendly split (`u32` whole seconds + `f32` fraction) can be added later if needed.
- **Built-ins are a namespaced struct named `mw` (e.g. `mw.time`, `mw.mouse.down`), not Shadertoy's `iTime`-style globals.** Why: compatibility with Shadertoy isn't a main goal (porting needs work anyway), so there's no reason to carry its baggage (e.g. `vec3` `iResolution`, `iMouse` sign-encoding). Planned approach: the user-facing struct is a `var<private>` filled by generated init code from the underlying uniform buffers (group 0 per-frame, group 1 per-pass), so its layout can be friendly (nested structs, bools) independent of the buffer layout.
- **Coordinates are y-up everywhere** (origin bottom-left, like Shadertoy). Internal textures are stored with row 0 = bottom, so `@builtin(position).xy` is already the y-up coordinate and sampling needs no flips. Flip only at presentation to the surface, readback/export, and image asset upload.
- **WGSL extensions/directives (e.g. `f16`) are a project-level setting** that the core emits at the top of the assembled module. Users don't write `enable` directives. Why: directives must precede declarations, and the core can check device support first.
- Module assembly order: user pass source first (so its line numbers need no mapping, since WGSL module-scope declarations are order-independent), then Common source, then generated prelude + entry point. A segment map translates diagnostics (Common lines, collisions with generated names).
- Uniform layout has a single source of truth in C++ (a field table) that generates both the WGSL declarations and the byte offsets.

### Repo & build
- Layout: `libmewo/` (`include/mewo/mewo.h` = the single public C header; `src/{api,project,shader,runtime,session,gpu,util}`; `tests/`), `hosts/` (`sdl/` reference host in plain C, later `macos/`, `web/`; "hosts" because the web isn't really an app), `examples/`, `docs/`, `third_party/`, `cmake/` (`webgpu.cmake` selects the implementation; `utils.cmake` has `mewo_set_target_defaults(target C|CXX)`, which sets the language standard (C11 / C++23, no extensions) and calls `mewo_set_target_warnings`). Every first-party target calls `mewo_set_target_defaults`.
- **CMake targets enforce the architecture:** `mewo_model` (project/ + shader/; links only the JSON lib; no webgpu, so GPU includes fail to compile), `mewo` (everything + `api/`; links `mewo_model`, webgpu, stb), `mewo_sdl` (links `mewo` + SDL3; only hosts see SDL).
- `api/` is the only place C meets C++ (thin `extern "C"` + try/catch forwarding). `gpu/` is the seam for implementation-specific code (surface creation, extensions).
- `tests/c_check.c` (target `mewo_c_check`, an OBJECT library) compiles `mewo.h` as C11 `-pedantic` to catch C++-isms. It must stay C11, not C23: C23 makes `f()` mean `f(void)` and makes `bool` a keyword, which would hide exactly the mistakes it exists to catch.
- **Presets** (`CMakePresets.json`): configure `debug-clang` / `release-clang` (Ninja, clang/clang++, `build/<preset>`, compile_commands on). Build presets per configuration: `<cfg>` (everything), `<cfg>-model` (`mewo_model` + `mewo_tests` only, never touches Dawn, so it's the fast loop), `<cfg>-host-sdl` (`mewo_sdl`). Test presets per configuration with `outputOnFailure` and `noTestsAction: error`. The user drives builds through VS Code's CMake Tools extension ("CMake: Select Build Preset"; "[Targets In Preset]" is the target setting, not the preset).
- **Gotchas found during setup:** Dawn registers `tint_unittests` with CTest unless `TINT_BUILD_TESTS` is OFF. After adding any `set(... CACHE ...)` dependency option to an existing build dir, reconfigure with `cmake --preset <p> --fresh`, because non-`FORCE` cache sets don't override cached values. Multi-line generator expressions break (whitespace splits or leaks into flags); use one-line `"$<...:a;b>"` forms.
- **Dawn first** as the `webgpu.h` implementation, selected through `cmake/webgpu.cmake` so wgpu-native can be added later.
- **Dependencies are git submodules only** (no FetchContent). Small single-header deps (stb_image) are vendored in `third_party/`. Note: Dawn's own build may still fetch its dependencies via its script (`DAWN_FETCH_DEPENDENCIES`).
- **Tests use Catch2 v3** (submodule, registered with CTest via `catch_discover_tests`). Why: expression-decomposing `REQUIRE`, and `SECTION`s suit the many small variations in GPU-free `mewo_model` tests; v3 is a compiled library, so no single-header compile cost.
- The macOS SwiftUI app will be an Xcode/SwiftPM project linking a libmewo static lib/xcframework, not built by CMake.

### Dev environment
- **Windows:** LLVM via winget (`LLVM.LLVM`), but winget doesn't add `C:\Program Files\LLVM\bin` to PATH; add it manually. CMake (`Kitware.CMake`) and Ninja (`Ninja-build.Ninja`) via winget. Clang targets the MSVC ABI, so it needs **Build Tools for Visual Studio 2026** (winget id `Microsoft.VisualStudio.BuildTools`, no year in the id) with the `Microsoft.VisualStudio.Workload.VCTools` workload + recommended components (MSVC toolset and Windows SDK). Use the GNU-style `clang`/`clang++` drivers, not `clang-cl`, to match the warning flags.
- **macOS:** Apple Clang (its libc++ lags upstream LLVM on C++23 features). Clone with `--recursive` or run `git submodule update --init`. Dawn's dependency fetch needs Python 3 on both machines.

## Roadmap
1. Skeleton: CMake targets, presets, submodules, placeholder sources; `mewo_sdl` prints `mewo_version_string()` and one Catch2 test passes, on both machines.
2. First vertical slice: `mewo_runtime_t` with async device init reported via the event queue, `mewo_surface_t` from SDL's native window handle (`gpu/` seam), a minimal session drawing one hardcoded shader, and the SDL host on SDL3 main callbacks (`SDL_AppInit`/`SDL_AppIterate`/...).
3. Real `mewo_model`: project data model, validation, shader assembly + segment map. GPU-free, test-driven with the `-model` presets.
4. Sessions render real projects: single pass, then multi-pass with feedback.
- Planned discussion before step 2: refcounting from first principles, and the exceptions-vs-`std::expected` question (both shape the first real `mewo.h`).

## Learning list
Topics the user wants to learn through this project (teach from first principles when they come up):
- Reference counting across a C API: add_ref/release semantics, ownership conventions, atomic vs. non-atomic counts, cycles, interplay with Swift ARC and JS GC.
- C API design (study `webgpu.h` as the reference).
- Async/non-blocking design (WebGPU futures/callbacks, `wgpuInstanceProcessEvents`, no blocking on the web).
- Render graphs / multi-pass rendering with feedback (ping-pong).

## Open questions
- Frontend order. Suggested: the SDL3 reference host first (it runs on both dev machines), then SwiftUI on macOS. The Windows frontend can wait.
- Error handling inside the C++ core: exceptions with a catch-all at the C boundary, vs. `std::expected`, which maps directly onto C error codes.
- Built-in editor vs. external editor + hot reload.
- Scope beyond Shadertoy (compute, audio, meshes).
