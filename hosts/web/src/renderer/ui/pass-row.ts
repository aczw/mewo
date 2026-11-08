import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  name: string;
  isActive: boolean;
};

type Callbacks = {
  onSetActive: () => void;
  onPassRename: (newPassName: string) => void;
};

class PassRow extends Component {
  readonly elt: HTMLLIElement;
  readonly #nameElt: HTMLButtonElement;

  #name: string;
  #isActive: boolean;

  constructor(init: Init, cb: Callbacks) {
    super();

    this.#name = init.name;
    this.#isActive = init.isActive;

    this.#nameElt = el("button", { class: "pass-row-name", text: init.name });
    this.#nameElt.addEventListener("click", cb.onSetActive);

    const renameElt = el("button", { class: "pass-row-rename", text: "Rename" });
    renameElt.addEventListener("click", () => cb.onPassRename("incredible new name"));

    this.elt = el("li", { class: init.isActive ? "pass-row is-active" : "pass-row" }, [
      this.#nameElt,
      renameElt,
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
  }
}

export { PassRow, type Init as PassRowInit };
