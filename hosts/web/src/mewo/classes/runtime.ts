import { kNull, MewoEvent, type Ptr } from "../types";
import { check, readCString, wasm } from "../wasm";
import type { Surface } from "./surface";

class Runtime implements Disposable {
  #handle: Ptr;

  constructor() {
    this.#handle = check(wasm._mewo_web_runtime_create(), "runtime creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("runtime used after dispose");
    return this.#handle;
  }

  tick(): void {
    wasm._mewo_web_runtime_tick(this.handle);
  }

  requestDevice(surface: Surface): void {
    wasm._mewo_web_runtime_request_device(this.handle, surface.handle);
  }

  *pollEvents(): Generator<{ type: MewoEvent; message: string }> {
    for (
      let event;
      (event = wasm._mewo_web_runtime_poll_event(this.handle) as MewoEvent) !== MewoEvent.None;
    ) {
      yield { type: event, message: readCString(wasm._mewo_web_event_message()) };
    }
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    wasm._mewo_web_runtime_destroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Runtime };
