import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, MewoEvent, type Ptr } from "../types";
import { Surface } from "./surface";

class Runtime implements Disposable {
  #handle: Ptr;
  #children = new Set<Disposable>();

  constructor() {
    this.#handle = Mewo.runtimeCreate();
    if (this.#handle === kNull) throw new MewoError(Mewo.lastResult());
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("runtime used after dispose");
    return this.#handle;
  }

  tick(): void {
    Mewo.runtimeTick(this.handle);
  }

  requestDevice(surface: Surface): void {
    Mewo.runtimeRequestDevice(this.handle, surface.handle);
  }

  createSurface(canvas: HTMLCanvasElement): Surface {
    const surface = new Surface(this.handle, canvas, () => this.#children.delete(surface));
    this.#children.add(surface);
    return surface;
  }

  *pollEvents(): Generator<{ type: MewoEvent; message: string }> {
    for (let event; (event = Mewo.runtimePollEvent(this.handle)) !== MewoEvent.None;) {
      yield { type: event, message: Mewo.eventMessage() };
    }
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;
    if (this.#children.size > 0) throw new Error("dispose sessions and surfaces first");

    Mewo.runtimeDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Runtime };
