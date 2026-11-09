import type { Runtime } from "../../mewo/classes/runtime";
import { Surface } from "../../mewo/classes/surface";
import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  runtime: Runtime;
};

type Callbacks = {
  onSetPaused: (isPaused: boolean) => void;
};

class Viewport extends Component {
  readonly surface: Surface;

  readonly #playbackBtnElt: HTMLButtonElement;

  readonly #timeElt: HTMLParagraphElement;
  readonly #dimsElt: HTMLParagraphElement;
  readonly #fpsElt: HTMLParagraphElement;
  readonly #runningFpsValues: number[] = [];

  #isPaused = false;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    const canvasElt = el("canvas", { id: "mewo" });
    const viewportElt = el("div", { class: "viewport" }, [canvasElt]);
    parent.append(viewportElt);
    this.defer(() => viewportElt.remove());

    this.surface = this.use(new Surface(init.runtime, canvasElt));

    {
      this.#playbackBtnElt = el("button", { text: "Pause" });
      this.#playbackBtnElt.addEventListener("click", () => {
        cb.onSetPaused(!this.#isPaused);
        this.#isPaused = !this.#isPaused;
        this.#playbackBtnElt.textContent = this.#isPaused ? "Resume" : "Pause";
      });

      const controlsElt = el("div", {}, [this.#playbackBtnElt]);
      viewportElt.append(controlsElt);
    }

    {
      this.#timeElt = el("p", { text: "0.00s" });

      this.#dimsElt = el("p");
      this.#fpsElt = el("p", { text: "0.0 FPS" });
      const infoRightElt = el("div", { class: "viewport-info-right" });
      infoRightElt.append(this.#dimsElt);
      infoRightElt.append(this.#fpsElt);

      const infoElt = el("div", { class: "viewport-info" }, [this.#timeElt, infoRightElt]);
      viewportElt.append(infoElt);
    }

    this.updateDimensionsInfo();
  }

  updateDimensionsInfo(): void {
    const dims = this.surface.dimensions;
    this.#dimsElt.textContent = `${dims.width}×${dims.height}`;
  }

  updateFpsInfo(deltaTimeMs: number): void {
    if (deltaTimeMs < Number.EPSILON) return;

    const fps = Math.round(1000 / deltaTimeMs);

    this.#runningFpsValues.push(fps);
    if (this.#runningFpsValues.length > 100) this.#runningFpsValues.shift();

    let sum = 0;
    for (const fpsValue of this.#runningFpsValues) sum += fpsValue;
    const averageFps = Math.round((sum / this.#runningFpsValues.length) * 10) * 0.1;

    this.#fpsElt.textContent = `${averageFps.toFixed(1)} FPS`;
  }

  updateTimeInfo(timeMs: number): void {
    this.#timeElt.textContent = `${(timeMs * 0.001).toFixed(2)}s`;
  }
}

export { Viewport };
