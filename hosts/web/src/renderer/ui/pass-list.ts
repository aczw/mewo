import type { PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import type { EngineSnapshot } from "../system/engine";
import { PassRow, type PassRowInit } from "./pass-row";

type Callbacks = {
  onRemovePass: (removedPassId: PassId) => void;
  onSetActivePass: (newActivePassId: PassId) => void;
  onPassRename: (passId: PassId, newPassName: string) => void;
};

class PassList extends Component {
  readonly #listElt: HTMLUListElement;
  readonly #rows = new Map<PassId, PassRow>();
  readonly #cb: Callbacks;

  constructor(parent: HTMLElement, init: EngineSnapshot, cb: Callbacks) {
    super();

    this.#cb = cb;

    this.#listElt = el("ul", { class: "pass-list" });
    parent.append(this.#listElt);
    this.defer(() => this.#listElt.remove());
    this.defer(() => {
      for (const row of this.#rows.values()) row[Symbol.dispose]();
    });

    this.update(init);
  }

  update(snapshot: EngineSnapshot): void {
    // Delete and dispose any `PassRow`s whose passes have been deleted
    const aliveIds = new Set(snapshot.passes.map((pass) => pass.id));
    for (const [passId, row] of this.#rows) {
      if (aliveIds.has(passId)) continue;
      row[Symbol.dispose]();
      this.#rows.delete(passId);
    }

    let domCursor = this.#listElt.firstElementChild;
    for (const pass of snapshot.passes) {
      const state: PassRowInit = {
        name: pass.name,
        isActive: pass.id === snapshot.activePass,
        canDelete: snapshot.passes.length > 1,
      };

      let passRow = this.#rows.get(pass.id);
      if (passRow) {
        passRow.update(state);
      } else {
        // Create new `PassRow`s for newly added passes
        passRow = this.#createRow(pass.id, state);
        this.#rows.set(pass.id, passRow);
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
      onRemove: () => this.#cb.onRemovePass(passId),
      onSetActive: () => this.#cb.onSetActivePass(passId),
      onRename: (newPassName) => this.#cb.onPassRename(passId, newPassName),
    });
  }
}

export { PassList };
