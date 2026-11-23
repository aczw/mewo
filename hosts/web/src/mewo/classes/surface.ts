import { type Dimensions, kNull, type Ptr } from "../types";
import { check, wasm, withCStrings } from "../wasm";
import type { Runtime } from "./runtime";

class Surface implements Disposable {
  #handle: Ptr;
  #canvas: HTMLCanvasElement;
  #dimensions: Dimensions;

  constructor(runtime: Runtime, canvas: HTMLCanvasElement) {
    if (canvas.id === "") throw new Error("canvas has no id");

    this.#canvas = canvas;
    this.#dimensions = Surface.#measure(canvas);

    const { width, height } = this.#dimensions;
    const handle = withCStrings([`#${canvas.id}`], ([selector]) =>
      wasm._surface_create_canvas(runtime.handle, selector!, width, height),
    );
    this.#handle = check(handle, "surface creation failed");
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

    wasm._surface_resize(this.handle, current.width, current.height);
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

    wasm._surface_destroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Surface };
