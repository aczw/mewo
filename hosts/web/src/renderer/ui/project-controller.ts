import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassList } from "./pass-list";

type Init = {
  project: Project;
  activePassId: PassId;
};

type Callbacks = {
  onPassAdded: () => void;
};

class ProjectController extends Component {
  readonly #passList: PassList;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    const divElt = el("div", { class: "project-controller" });
    parent.append(divElt);
    this.defer(() => divElt.remove());

    this.#passList = this.use(
      new PassList(divElt, { project: init.project, activePassId: init.activePassId }),
    );

    const addPassBtnElt = el("button", { class: "add-pass", text: "Add pass" });
    addPassBtnElt.addEventListener("click", cb.onPassAdded);
    divElt.append(addPassBtnElt);
  }

  addPass(project: Project, passId: PassId) {
    this.#passList.add(project, passId, false);
  }

  setActivePass(passId: PassId) {
    this.#passList.setActivePass(passId);
  }
}

export { ProjectController };
