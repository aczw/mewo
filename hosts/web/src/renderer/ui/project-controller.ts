import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import type { EngineSnapshot } from "../system/engine";
import { PassList } from "./pass-list";

type Callbacks = {
  onAddPass: () => void;
  onRemovePass: (removedPassId: PassId) => void;
  onSetActivePass: (newActivePassId: PassId) => void;
  onPassRename: (passId: PassId, newPassName: string) => void;
};

class ProjectController extends Component {
  readonly #passList: PassList;

  constructor(parent: HTMLElement, init: EngineSnapshot, cb: Callbacks) {
    super();

    const divElt = el("div", { class: "project-controller" });
    parent.append(divElt);
    this.defer(() => divElt.remove());

    this.#passList = this.use(
      new PassList(divElt, init, {
        onRemovePass: cb.onRemovePass,
        onSetActivePass: cb.onSetActivePass,
        onPassRename: cb.onPassRename,
      }),
    );

    const addPassBtnElt = el("button", { class: "add-pass", text: "Add pass" });
    addPassBtnElt.addEventListener("click", cb.onAddPass);
    divElt.append(addPassBtnElt);
  }

  update(snapshot: EngineSnapshot): void {
    this.#passList.update(snapshot);
  }
}

export { ProjectController };
