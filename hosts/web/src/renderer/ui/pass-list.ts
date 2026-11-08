import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
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

  constructor(parent: HTMLElement, init: Init) {
    super();

    this.#listElt = el("ul", { class: "pass-list" });

    for (let idx = 0; idx < init.project.getPassCount(); ++idx) {
      const passId = init.project.getPassId(idx);

      if (!passId) continue;

      this.#rows.set(
        passId,
        new PassRow(this.#listElt, {
          name: init.project.getPassName(passId),
          isActive: init.activePassId === passId,
        }),
      );
    }

    const passesHeadingElt = el("h2", { class: "passes-heading", text: "Passes" });
    parent.append(passesHeadingElt);
    this.defer(() => passesHeadingElt.remove());

    parent.append(this.#listElt);
    this.defer(() => this.#listElt.remove());
  }
}

export { PassList };
