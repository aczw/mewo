import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, MewoEvent, type Ptr } from "../types";
import type { Surface } from "./surface";

class Runtime implements Disposable {
  #handle: Ptr;

  constructor() {
    this.#handle = Mewo.runtimeCreate();
    if (this.#handle === kNull) throw new MewoError("runtime creation failed");
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

  *pollEvents(): Generator<{ type: MewoEvent; message: string }> {
    for (let event; (event = Mewo.runtimePollEvent(this.handle)) !== MewoEvent.None;) {
      yield { type: event, message: Mewo.eventMessage() };
    }
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.runtimeDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Runtime };
