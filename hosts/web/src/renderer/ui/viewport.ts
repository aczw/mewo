import { Component } from "../system/component";
import { el } from "../system/dom";
import type { EngineSnapshot, FrameStats } from "../system/engine";

type Callbacks = {
  onSetPaused: (isPaused: boolean) => void;
};

class Viewport extends Component {
  readonly canvas: HTMLCanvasElement;

  readonly #playbackBtnElt: HTMLButtonElement;
  readonly #timeElt: HTMLParagraphElement;
  readonly #dimsElt: HTMLParagraphElement;
  readonly #fpsElt: HTMLParagraphElement;
  readonly #runningFpsValues: number[] = [];

  #paused = false;

  constructor(parent: HTMLElement, init: EngineSnapshot, cb: Callbacks) {
    super();

    this.canvas = el("canvas", { id: "mewo" });
    const viewportElt = el("div", { class: "viewport" }, [this.canvas]);
    parent.append(viewportElt);
    this.defer(() => viewportElt.remove());

    {
      this.#playbackBtnElt = el("button");
      this.#playbackBtnElt.addEventListener("click", () => cb.onSetPaused(!this.#paused));

      const controlsElt = el("div", {}, [this.#playbackBtnElt]);
      viewportElt.append(controlsElt);
    }

    {
      this.#timeElt = el("p", { text: "0.00s" });
      this.#dimsElt = el("p");
      this.#fpsElt = el("p", { text: "0.0 FPS" });

      const infoRightElt = el("div", { class: "viewport-info-right" }, [
        this.#dimsElt,
        this.#fpsElt,
      ]);
      const infoElt = el("div", { class: "viewport-info" }, [this.#timeElt, infoRightElt]);
      viewportElt.append(infoElt);
    }

    this.update(init);
  }

  update(snapshot: EngineSnapshot): void {
    this.#paused = snapshot.paused;
    this.#playbackBtnElt.textContent = snapshot.paused ? "Resume" : "Pause";
  }

  updateStats(stats: FrameStats): void {
    this.#timeElt.textContent = `${(stats.sessionTime * 0.001).toFixed(2)}s`;
    this.#dimsElt.textContent = `${stats.dimensions.width}×${stats.dimensions.height}`;
    this.#updateFps(stats.deltaTime);
  }

  #updateFps(deltaTimeMs: number): void {
    if (deltaTimeMs < Number.EPSILON) return;

    const fps = Math.round(1000 / deltaTimeMs);

    this.#runningFpsValues.push(fps);
    if (this.#runningFpsValues.length > 100) this.#runningFpsValues.shift();

    let sum = 0;
    for (const fpsValue of this.#runningFpsValues) sum += fpsValue;
    const averageFps = Math.round((sum / this.#runningFpsValues.length) * 10) * 0.1;

    this.#fpsElt.textContent = `${averageFps.toFixed(1)} FPS`;
  }
}

export { Viewport };
