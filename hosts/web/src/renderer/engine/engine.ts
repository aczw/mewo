import { Project } from "../../mewo/classes/project";
import { Runtime } from "../../mewo/classes/runtime";
import { Session } from "../../mewo/classes/session";
import { Surface } from "../../mewo/classes/surface";
import { type Dimensions, MewoEvent, type Pass, type PassId } from "../../mewo/types";
import { Mutable } from "./mutable";

const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

const kNewPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return textureSample(prev_tex, prev_smp, uv);
}`;

type EngineSnapshot = { passes: Pass[]; activePass: PassId; paused: boolean };
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

  #activePass: PassId;
  #paused = false;

  #snapshot: EngineSnapshot;
  readonly #listeners = new Set<() => void>();
  readonly #frameListeners = new Set<(stats: FrameStats) => void>();

  #rafId = 0;
  #prevFrameTime: number | null = null;

  constructor() {
    this.#runtime = this.#stack.use(new Runtime());
    this.#project = this.#stack.use(new Project("Pass 1", kDefaultPassSource));
    this.#sessionHolder = this.#stack.use(new Mutable<Session>());

    this.#activePass = this.#project.getPassId(0)!;
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
    this.#activePass = this.#project.addPass(`Pass ${passCount + 1}`, kNewPassSource);
    this.#emit();
  }

  removePass(pass: PassId): void {
    this.#project.removePass(pass);
    if (pass === this.#activePass) this.#activePass = this.#project.getPassId(0)!;
    this.#emit();
  }

  setActivePass(pass: PassId): void {
    this.#activePass = pass;
    this.#emit();
  }

  renamePass(pass: PassId, newName: string): void {
    this.#project.setPassName(pass, newName);
    this.#emit();
  }

  getPassSource(pass: PassId): string {
    return this.#project.getPassSource(pass);
  }

  setPassSource(pass: PassId, source: string): void {
    this.#project.setPassSource(pass, source);
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
    const passes: Pass[] = [];
    for (let idx = 0; idx < this.#project.getPassCount(); ++idx) {
      const passId = this.#project.getPassId(idx)!;
      passes.push({ id: passId, name: this.#project.getPassName(passId) });
    }

    return { passes, activePass: this.#activePass, paused: this.#paused };
  }

  [Symbol.dispose](): void {
    this.#surface?.[Symbol.dispose]();
    this.#surface = null;
    this.#stack.dispose();
  }
}

export { Engine };
export type { EngineSnapshot, FrameStats, Pass };
