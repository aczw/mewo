import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassRow } from "./pass-row";

class PassList extends Component {
  readonly #listElt: HTMLUListElement;
  readonly #rows = new Map<PassId, PassRow>();

  readonly activePassId: PassId;

  constructor(parent: HTMLElement, project: Project) {
    super();

    this.#listElt = el("ul", { class: "pass-list" });

    for (let idx = 0; idx < project.getPassCount(); ++idx) {
      const passId = project.getPassId(idx);

      if (!passId) continue;

      this.#rows.set(passId, new PassRow(this.#listElt, project.getPassName(passId)));
    }

    // First pass has to exist
    this.activePassId = project.getPassId(0)!;

    parent.append(this.#listElt);
    this.defer(() => this.#listElt.remove());
  }
}

export { PassList };
