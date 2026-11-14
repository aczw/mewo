import { Project } from "../../mewo/classes/project";
import { Runtime } from "../../mewo/classes/runtime";
import { Session } from "../../mewo/classes/session";
import { Surface } from "../../mewo/classes/surface";
import {
  type CommonDiagnostic,
  type Dimensions,
  MewoDiagnosticSource,
  MewoEvent,
  type Pass,
  type PassId,
} from "../../mewo/types";
import { Mutable } from "./mutable";

const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

const kNewPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return textureSample(prev_tex, prev_smp, uv);
}`;

type Selection = { kind: "pass"; id: PassId } | { kind: "common" };
type EngineSnapshot = {
  passes: Pass[];
  selection: Selection;
  paused: boolean;
  commonDiagnostics: CommonDiagnostic[];
};
type FrameStats = { deltaTime: number; sessionTime: number; dimensions: Dimensions };

const sameDimensions = (a: Dimensions, b: Dimensions): boolean => {
  return a.width === b.width && a.height === b.height;
};

class Engine implements Disposable {
  readonly #stack = new DisposableStack();

  readonly #runtime: Runtime;
  readonly #project: Project;
  readonly #sessionHolder: Mutable<Session>;

  #surface: Surface | null = null;
  #deviceRequested = false;
  #deviceReady = false;
  #dimensions: Dimensions = { width: 0, height: 0 };

  #selection: Selection;
  #paused = false;
  #lastStatusRevision: number | null = null;

  #snapshot: EngineSnapshot;
  readonly #listeners = new Set<() => void>();
  readonly #frameListeners = new Set<(stats: FrameStats) => void>();

  #rafId = 0;
  #prevFrameTime: number | null = null;

  constructor() {
    this.#runtime = this.#stack.use(new Runtime());
    this.#project = this.#stack.use(new Project("Pass 1", kDefaultPassSource));
    this.#sessionHolder = this.#stack.use(new Mutable<Session>());

    this.#selection = { kind: "pass", id: this.#project.getPassId(0)! };
    this.#snapshot = this.#readSnapshot();

    this.#rafId = window.requestAnimationFrame(this.#onFrame);
    this.#stack.defer(() => window.cancelAnimationFrame(this.#rafId));
  }

  readonly subscribe = (listener: () => void): (() => void) => {
    this.#listeners.add(listener);
    return () => {
      this.#listeners.delete(listener);
    };
  };

  readonly getSnapshot = (): EngineSnapshot => this.#snapshot;

  readonly subscribeFrame = (listener: (stats: FrameStats) => void): (() => void) => {
    this.#frameListeners.add(listener);
    return () => {
      this.#frameListeners.delete(listener);
    };
  };

  addPass(): void {
    const passCount = this.#project.getPassCount();
    this.#selection = {
      kind: "pass",
      id: this.#project.addPass(`Pass ${passCount + 1}`, kNewPassSource),
    };
    this.#emit();
  }

  removePass(passId: PassId): void {
    this.#project.removePass(passId);

    if (this.#selection.kind === "pass" && passId === this.#selection.id)
      this.#selection = { kind: "pass", id: this.#project.getPassId(0)! };

    this.#emit();
  }

  selectPass(passId: PassId): void {
    this.#selection = { kind: "pass", id: passId };
    this.#emit();
  }

  selectCommon(): void {
    this.#selection = { kind: "common" };
    this.#emit();
  }

  renamePass(passId: PassId, newName: string): void {
    this.#project.setPassName(passId, newName);
    this.#emit();
  }

  getCommonSource(): string {
    return this.#project.getCommonSource();
  }

  setCommonSource(source: string): void {
    this.#project.setCommonSource(source);
  }

  getPassSource(passId: PassId): string {
    return this.#project.getPassSource(passId);
  }

  setPassSource(passId: PassId, source: string): void {
    this.#project.setPassSource(passId, source);
  }

  setPaused(paused: boolean): void {
    this.#paused = paused;
    if (this.#sessionHolder.value !== null) this.#sessionHolder.value.paused = paused;
    this.#emit();
  }

  createSurfaceAndAttachCanvas(canvas: HTMLCanvasElement): () => void {
    if (this.#surface !== null) throw new Error("canvas is already attached");

    const surface = new Surface(this.#runtime, canvas);
    this.#surface = surface;

    if (!this.#deviceRequested) {
      this.#runtime.requestDevice(surface);
      this.#deviceRequested = true;
    }

    return () => {
      if (this.#surface !== surface) return;
      surface[Symbol.dispose]();
      this.#surface = null;
    };
  }

  readonly #onFrame = (time: DOMHighResTimeStamp): void => {
    this.#rafId = window.requestAnimationFrame(this.#onFrame);

    const deltaTime = this.#prevFrameTime === null ? 0 : time - this.#prevFrameTime;
    this.#prevFrameTime = time;

    this.#runtime.tick();
    for (const event of this.#runtime.pollEvents()) {
      switch (event.type) {
        case MewoEvent.DeviceReady: {
          console.info("[libmewo] device is ready");
          this.#deviceReady = true;
          break;
        }

        default: {
          console.error("[libmewo]", event.message);
          break;
        }
      }
    }

    const surface = this.#surface;
    if (surface === null || !this.#deviceReady) return;

    surface.checkForResize();
    const dims = surface.dimensions;
    const hasArea = dims.width !== 0 && dims.height !== 0;

    let session = this.#sessionHolder.value;
    if (session === null) {
      if (!hasArea) return;

      session = new Session(this.#runtime, this.#project, dims.width, dims.height);
      session.paused = this.#paused;
      this.#sessionHolder.value = session;
      this.#dimensions = dims;
    } else if (hasArea && !sameDimensions(dims, this.#dimensions)) {
      session.setRenderSize(dims.width, dims.height);
      this.#dimensions = dims;
    }

    session.render(surface, time);

    const revision = session.getPassStatusRevision();
    if (revision !== this.#lastStatusRevision) {
      this.#lastStatusRevision = revision;
      this.#emit();
    }

    const stats: FrameStats = {
      deltaTime,
      sessionTime: session.time,
      dimensions: this.#dimensions,
    };

    for (const listener of this.#frameListeners) listener(stats);
  };

  #emit(): void {
    this.#snapshot = this.#readSnapshot();
    for (const listener of this.#listeners) listener();
  }

  #readSnapshot(): EngineSnapshot {
    const session = this.#sessionHolder.value;

    const passes: Pass[] = [];
    for (let idx = 0; idx < this.#project.getPassCount(); ++idx) {
      const passId = this.#project.getPassId(idx)!;
      passes.push({
        id: passId,
        name: this.#project.getPassName(passId),
        compilation: session?.readPassCompile(passId) ?? null,
        diagnostics: session?.readPassDiagnostics(passId) ?? [],
      });
    }

    const byKey = new Map<string, CommonDiagnostic>();
    for (const pass of passes) {
      for (const diagnostic of pass.diagnostics) {
        if (diagnostic.source !== MewoDiagnosticSource.Common) continue;

        const { severity, line, column, message } = diagnostic;
        const key = `${severity}:${line}:${column}:${message}`;

        const exists = byKey.get(key);
        if (exists) {
          exists.passIds.push(pass.id);
        } else {
          byKey.set(key, { ...diagnostic, passIds: [pass.id] });
        }
      }
    }

    return {
      passes,
      selection: this.#selection,
      paused: this.#paused,
      commonDiagnostics: [...byKey.values()].sort((a, b) => a.line - b.line || a.column - b.column),
    };
  }

  [Symbol.dispose](): void {
    this.#surface?.[Symbol.dispose]();
    this.#surface = null;
    this.#stack.dispose();
  }
}

export { Engine };
export type { EngineSnapshot, FrameStats, Selection };
