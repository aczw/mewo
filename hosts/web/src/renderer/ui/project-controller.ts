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
  onAddPass: () => void;
  onSetActivePass: (newActivePassId: PassId) => void;
  onPassRename: (passId: PassId, newPassName: string) => void;
};

class ProjectController extends Component {
  readonly #passList: PassList;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    const divElt = el("div", { class: "project-controller" });
    parent.append(divElt);
    this.defer(() => divElt.remove());

    this.#passList = this.use(
      new PassList(
        divElt,
        { project: init.project, activePassId: init.activePassId },
        {
          onSetActivePass: cb.onSetActivePass,
          onPassRename: cb.onPassRename,
        },
      ),
    );

    const addPassBtnElt = el("button", { class: "add-pass", text: "Add pass" });
    addPassBtnElt.addEventListener("click", cb.onAddPass);
    divElt.append(addPassBtnElt);
  }

  update(project: Project, activePassId: PassId) {
    this.#passList.update(project, activePassId);
  }
}

export { ProjectController };
