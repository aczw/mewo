import {
  kNull,
  type MewoDiagnosticSeverity,
  type MewoDiagnosticSource,
  MewoPassCompileStatus,
  MewoResult,
  type PassCompilation,
  type PassDiagnostic,
  type PassId,
  type Ptr,
} from "../types";
import { check, readCString, wasm } from "../wasm";
import type { Project } from "./project";
import type { Runtime } from "./runtime";
import type { Surface } from "./surface";

class Session implements Disposable {
  #handle: Ptr;

  constructor(runtime: Runtime, project: Project, width: number, height: number) {
    const handle = wasm._session_create(runtime.handle, project.handle, width, height);
    this.#handle = check(handle, "session creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("session used after dispose");
    return this.#handle;
  }

  /** In milliseconds. */
  get time(): number {
    return wasm._session_get_time(this.handle);
  }

  get paused(): boolean {
    // Wasm returns C `bool` as 0/1
    return wasm._session_is_paused(this.handle) !== 0;
  }

  set paused(paused: boolean) {
    wasm._session_set_paused(this.handle, paused ? 1 : 0);
  }

  reset(): void {
    wasm._session_reset(this.handle);
  }

  getPassStatusRevision(): number {
    return wasm._session_get_pass_status_revision(this.handle);
  }

  /** `null` for a pass ID the session doesn't know (yet). */
  readPassCompile(passId: PassId): PassCompilation | null {
    const status = wasm._session_get_pass_compile_status(
      this.handle,
      passId,
    ) as MewoPassCompileStatus;
    if (wasm._web_last_result() === MewoResult.InvalidArgument) return null;
    check(undefined, "failed to read pass compile status");

    // The duration is only available once a compile has finished
    if (status === MewoPassCompileStatus.Pending) return { status };

    const duration = wasm._session_get_pass_compile_duration(this.handle, passId);
    return { status, duration: check(duration, "failed to read pass compile duration") };
  }

  /** `null` for a pass ID the session doesn't know (yet). */
  readPassDiagnostics(passId: PassId): PassDiagnostic[] | null {
    const count = wasm._session_get_pass_diagnostic_count(this.handle, passId);
    if (wasm._web_last_result() === MewoResult.InvalidArgument) return null;
    check(undefined, "failed to read pass diagnostic count");

    const diagnostics: PassDiagnostic[] = [];
    for (let idx = 0; idx < count; ++idx) {
      wasm._session_select_pass_diagnostic(this.handle, passId, idx);
      check(undefined, "failed to read pass diagnostic");

      diagnostics.push({
        severity: wasm._diagnostic_severity() as MewoDiagnosticSeverity,
        message: readCString(wasm._diagnostic_message()),
        source: wasm._diagnostic_source() as MewoDiagnosticSource,
        line: wasm._diagnostic_line(),
        column: wasm._diagnostic_column(),
      });
    }

    return diagnostics;
  }

  setRenderSize(newWidth: number, newHeight: number): void {
    wasm._session_set_render_size(this.handle, newWidth, newHeight);
    check(undefined, "failed to set session render size");
  }

  render(surface: Surface, timeMs: number): void {
    wasm._session_render(this.handle, surface.handle, timeMs);
    check(undefined, "session render failed");
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    wasm._session_destroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Session };
