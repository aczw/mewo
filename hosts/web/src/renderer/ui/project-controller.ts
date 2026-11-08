import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassList } from "./pass-list";

class ProjectController extends Component {
  readonly activePassId: PassId;
  // @ts-ignore
  readonly #passList: PassList;

  constructor(parent: HTMLElement, project: Project) {
    super();

    const divElt = el("div", { class: "project-controller" });

    parent.append(divElt);
    this.defer(() => divElt.remove());

    // First pass has to exist
    this.activePassId = project.getPassId(0)!;
    this.#passList = this.use(new PassList(divElt, project, this.activePassId));
  }
}

export { ProjectController };
