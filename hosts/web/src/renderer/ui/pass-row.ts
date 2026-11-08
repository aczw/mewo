import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  name: string;
  isActive: boolean;
  canDelete: boolean;
};

type Callbacks = {
  onRemove: () => void;
  onSetActive: () => void;
  onRename: (newPassName: string) => void;
};

class PassRow extends Component {
  readonly elt: HTMLLIElement;
  readonly #nameElt: HTMLButtonElement;
  readonly #deleteElt: HTMLButtonElement;

  #name: string;
  #isActive: boolean;
  #canDelete: boolean;

  constructor(init: Init, cb: Callbacks) {
    super();

    this.#name = init.name;
    this.#isActive = init.isActive;
    this.#canDelete = init.canDelete;

    this.#nameElt = el("button", { class: "pass-row-name", text: init.name });
    this.#nameElt.addEventListener("click", cb.onSetActive);

    const renameElt = el("button", { class: "pass-row-rename", text: "Rename" });
    renameElt.addEventListener("click", () => cb.onRename("incredible new name"));

    this.#deleteElt = el("button", {
      class: "pass-row-delete",
      text: "Delete",
      attributes: this.#canDelete ? undefined : { disabled: "" },
    });
    this.#deleteElt.addEventListener("click", cb.onRemove);

    this.elt = el("li", { class: init.isActive ? "pass-row is-active" : "pass-row" }, [
      this.#nameElt,
      renameElt,
      this.#deleteElt,
    ]);
    this.defer(() => this.elt.remove());
  }

  update(state: Init) {
    if (this.#name !== state.name) {
      this.#nameElt.textContent = state.name;
      this.#name = state.name;
    }

    if (this.#isActive !== state.isActive) {
      this.elt.classList.toggle("is-active", state.isActive);
      this.#isActive = state.isActive;
    }

    if (this.#canDelete !== state.canDelete) {
      this.#deleteElt.disabled = !state.canDelete;
      this.#canDelete = state.canDelete;
    }
  }
}

export { PassRow, type Init as PassRowInit };
