import { Mewo } from "./api";
import { MewoError } from "./error";
import { type Dimensions, kNull, MewoEvent, type Ptr } from "./types";

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

class Surface implements Disposable {
  #handle: Ptr;
  #canvas: HTMLCanvasElement;
  #dimensions: Dimensions;
  #onDispose: () => void;

  constructor(runtime: Ptr, canvas: HTMLCanvasElement, onDispose: () => void) {
    if (canvas.id === "") throw new Error("canvas has no id");

    this.#canvas = canvas;
    this.#dimensions = Surface.#measure(canvas);
    this.#handle = Mewo.surfaceCreateCanvas(
      runtime,
      `#${canvas.id}`,
      this.#dimensions.width,
      this.#dimensions.height,
    );

    if (this.#handle === kNull) throw new MewoError(Mewo.lastResult());

    this.#onDispose = onDispose;
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("surface used after dispose");
    return this.#handle;
  }

  get dimensions(): Dimensions {
    return this.#dimensions;
  }

  checkForResize(): boolean {
    const current = Surface.#measure(this.#canvas);

    if (current.width === this.#dimensions.width && current.height === this.#dimensions.height)
      return false;

    Mewo.surfaceResize(this.handle, current.width, current.height);
    this.#dimensions = current;
    return true;
  }

  static #measure(canvas: HTMLCanvasElement): Dimensions {
    return {
      width: Math.round(canvas.clientWidth * window.devicePixelRatio),
      height: Math.round(canvas.clientHeight * window.devicePixelRatio),
    };
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.surfaceDestroy(this.#handle);
    this.#handle = kNull;
    this.#onDispose();
  }
}

export { Runtime, type Surface };
