import type { Runtime } from "../../mewo/classes/runtime";
import { Surface } from "../../mewo/classes/surface";
import { Component } from "../system/component";
import { el } from "../system/dom";

class Viewport extends Component {
  readonly surface: Surface;

  constructor(parent: HTMLElement, runtime: Runtime) {
    super();

    const canvasElt = el("canvas", { id: "mewo" });

    parent.append(canvasElt);
    this.defer(() => canvasElt.remove());

    this.surface = this.use(new Surface(runtime, canvasElt));
  }
}

export { Viewport };
