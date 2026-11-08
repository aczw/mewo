import type { Project } from "../../mewo/classes/project";
import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { PassRow, type PassRowInit } from "./pass-row";

type Init = {
  project: Project;
  activePassId: PassId;
};

type Callbacks = {
  onRemovePass: (removedPassId: PassId) => void;
  onSetActivePass: (newActivePassId: PassId) => void;
  onPassRename: (passId: PassId, newPassName: string) => void;
};

class PassList extends Component {
  readonly #listElt: HTMLUListElement;
  readonly #rows = new Map<PassId, PassRow>();

  readonly #onRemovePass: (removedPassId: PassId) => void;
  readonly #onSetActivePass: (newActivePassId: PassId) => void;
  readonly #onPassRename: (passId: PassId, newPassName: string) => void;

  constructor(parent: HTMLElement, init: Init, cb: Callbacks) {
    super();

    this.#listElt = el("ul", { class: "pass-list" });
    this.#onRemovePass = cb.onRemovePass;
    this.#onSetActivePass = cb.onSetActivePass;
    this.#onPassRename = cb.onPassRename;

    this.update(init.project, init.activePassId);

    const passesHeadingElt = el("h2", { class: "passes-heading", text: "Passes" });
    parent.append(passesHeadingElt);
    this.defer(() => passesHeadingElt.remove());

    parent.append(this.#listElt);
    this.defer(() => this.#listElt.remove());

    this.defer(() => {
      for (const row of this.#rows.values()) row[Symbol.dispose]();
    });
  }

  update(project: Project, activePassId: PassId) {
    const passIds: PassId[] = [];
    for (let idx = 0; idx < project.getPassCount(); ++idx) passIds.push(project.getPassId(idx)!);

    // Delete and dispose any `PassRow`s whose passes have been deleted
    const aliveIds = new Set(passIds);
    for (const [passId, row] of this.#rows) {
      if (aliveIds.has(passId)) continue;
      row[Symbol.dispose]();
      this.#rows.delete(passId);
    }

    let domCursor = this.#listElt.firstElementChild;
    for (const passId of passIds) {
      const state: PassRowInit = {
        name: project.getPassName(passId),
        isActive: passId === activePassId,
        canDelete: passIds.length > 1,
      };

      let passRow = this.#rows.get(passId);
      if (passRow) {
        passRow.update(state);
      } else {
        // Create new `PassRow`s for newly added passes
        passRow = this.#createRow(passId, state);
        this.#rows.set(passId, passRow);
      }

      if (passRow.elt === domCursor) {
        domCursor = domCursor.nextElementSibling;
      } else {
        this.#listElt.insertBefore(passRow.elt, domCursor);
      }
    }
  }

  #createRow(passId: PassId, init: PassRowInit): PassRow {
    return new PassRow(init, {
      onRemove: () => this.#onRemovePass(passId),
      onSetActive: () => this.#onSetActivePass(passId),
      onRename: (newPassName) => this.#onPassRename(passId, newPassName),
    });
  }
}

export { PassList };
