import { Mewo } from "../api";
import { MewoError } from "../error";
import { type Dimensions, kNull, type Ptr } from "../types";
import type { Runtime } from "./runtime";

class Surface implements Disposable {
  #handle: Ptr;
  #canvas: HTMLCanvasElement;
  #dimensions: Dimensions;

  constructor(runtime: Runtime, canvas: HTMLCanvasElement) {
    if (canvas.id === "") throw new Error("canvas has no id");

    this.#canvas = canvas;
    this.#dimensions = Surface.#measure(canvas);
    this.#handle = Mewo.surfaceCreateCanvas(
      runtime.handle,
      `#${canvas.id}`,
      this.#dimensions.width,
      this.#dimensions.height,
    );

    if (this.#handle === kNull) throw new MewoError("surface creation failed");
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
  }
}

export { Surface };
