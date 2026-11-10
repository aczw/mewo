import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, type Ptr } from "../types";
import type { Project } from "./project";
import type { Runtime } from "./runtime";
import type { Surface } from "./surface";

class Session implements Disposable {
  #handle: Ptr;

  constructor(runtime: Runtime, project: Project, width: number, height: number) {
    this.#handle = Mewo.sessionCreate(runtime.handle, project.handle, width, height);
    if (this.#handle === kNull) throw new MewoError("session creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("session used after dispose");
    return this.#handle;
  }

  /** In milliseconds. */
  get time(): number {
    return Mewo.sessionGetTime(this.handle);
  }

  get paused(): boolean {
    return Mewo.sessionIsPaused(this.handle);
  }

  set paused(paused: boolean) {
    Mewo.sessionSetPaused(this.handle, paused);
  }

  setRenderSize(newWidth: number, newHeight: number): void {
    if (!Mewo.sessionSetRenderSize(this.handle, newWidth, newHeight))
      throw new MewoError("failed to set session render size");
  }

  render(surface: Surface, timeMs: number): void {
    if (!Mewo.sessionRender(this.handle, surface.handle, timeMs))
      throw new MewoError("session render failed");
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.sessionDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Session };
