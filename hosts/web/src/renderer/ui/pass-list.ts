import type { Project } from "../../mewo/classes/project";
import { kPassIdNone, type PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassRow } from "./pass-row";

type Init = {
  project: Project;
  activePassId: PassId;
};

type Callbacks = {
  onSetActivePass: (newActivePassId: PassId) => void;
};

class PassList extends Component {
  readonly #listElt: HTMLUListElement;
  readonly #rows = new Map<PassId, PassRow>();
  readonly #onSetActivePass: (newActivePassId: PassId) => void;

  #currActivePass: PassId = kPassIdNone;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    this.#listElt = el("ul", { class: "pass-list" });
    this.#onSetActivePass = cb.onSetActivePass;

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

    this.defer(() => {
      for (const row of this.#rows.values()) row[Symbol.dispose]();
    });
  }

  add(project: Project, passId: PassId, isActive: boolean) {
    const passRow = new PassRow(
      {
        name: project.getPassName(passId),
        isActive,
      },
      {
        onSetActive: () => this.#onSetActivePass(passId),
      },
    );

    this.#listElt.append(passRow.elt);
    this.#rows.set(passId, passRow);
  }

  setActivePass(passId: PassId) {
    this.#rows.get(this.#currActivePass)!.setActive(false);
    this.#rows.get(passId)!.setActive(true);
    this.#currActivePass = passId;
  }
}

export { PassList };
