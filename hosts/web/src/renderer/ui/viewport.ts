import type { Runtime } from "../../mewo/classes/runtime";
import { Surface } from "../../mewo/classes/surface";
import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  runtime: Runtime;
};

class Viewport extends Component {
  readonly surface: Surface;
  readonly #dimsElt: HTMLParagraphElement;

  constructor(parent: HTMLElement, init: Init) {
    super();

    const canvasElt = el("canvas", { id: "mewo" });
    const divElt = el("div", { class: "viewport" }, [canvasElt]);

    parent.append(divElt);
    this.defer(() => divElt.remove());

    this.surface = this.use(new Surface(init.runtime, canvasElt));

    this.#dimsElt = el("p");
    const infoElt = el("div", { class: "viewport-info" }, [this.#dimsElt]);
    divElt.append(infoElt);

    this.updateInfo();
  }

  updateInfo(): void {
    const dims = this.surface.dimensions;
    this.#dimsElt.textContent = `${dims.width}×${dims.height}`;
  }
}

export { Viewport };
