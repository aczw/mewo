import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassList } from "./pass-list";

type Init = {
  project: Project;
  activePassId: PassId;
};

class ProjectController extends Component {
  readonly #passList: PassList;

  constructor(parent: HTMLElement, init: Init) {
    super();

    const divElt = el("div", { class: "project-controller" });

    parent.append(divElt);
    this.defer(() => divElt.remove());

    this.#passList = this.use(
      new PassList(divElt, { project: init.project, activePassId: init.activePassId }),
    );
  }
}

export { ProjectController };
