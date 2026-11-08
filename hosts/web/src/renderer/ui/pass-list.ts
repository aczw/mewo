import type { Project } from "../../mewo/classes/project";
import { kPassIdNone, type PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassRow } from "./pass-row";

type Init = {
  project: Project;
  activePassId: PassId;
};

class PassList extends Component {
  readonly #listElt: HTMLUListElement;
  readonly #rows = new Map<PassId, PassRow>();

  #currActivePass: PassId = kPassIdNone;

  constructor(parent: HTMLElement, init: Init) {
    super();

    this.#listElt = el("ul", { class: "pass-list" });

    for (let idx = 0; idx < init.project.getPassCount(); ++idx) {
      const passId = init.project.getPassId(idx);

      if (!passId) continue;

      const isActive = init.activePassId === passId;
      this.add(init.project, passId, isActive);

      if (isActive) this.#currActivePass = passId;
    }

    const passesHeadingElt = el("h2", { class: "passes-heading", text: "Passes" });
    parent.append(passesHeadingElt);
    this.defer(() => passesHeadingElt.remove());

    parent.append(this.#listElt);
    this.defer(() => this.#listElt.remove());
  }

  add(project: Project, passId: PassId, isActive: boolean) {
    this.#rows.set(
      passId,
      new PassRow(this.#listElt, {
        name: project.getPassName(passId),
        isActive,
      }),
    );
  }

  setActivePass(passId: PassId) {
    this.#rows.get(this.#currActivePass)!.toggleActive();
    this.#rows.get(passId)!.toggleActive();
    this.#currActivePass = passId;
  }
}

export { PassList };
