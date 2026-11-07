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
  }

  setRenderSize(newWidth: number, newHeight: number) {
    if (!Mewo.sessionSetRenderSize(this.#handle, newWidth, newHeight))
      throw new MewoError(Mewo.lastResult());
  }

  render(surface: Surface, timeMs: number) {
    if (!Mewo.sessionRender(this.#handle, surface.handle, timeMs))
      throw new MewoError(Mewo.lastResult());
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.sessionDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Session };
