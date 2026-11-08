import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  name: string;
  isActive: boolean;
};

type Callbacks = {
  onSetActive: () => void;
};

class PassRow extends Component {
  readonly elt: HTMLLIElement;

  constructor(init: Init, cb: Callbacks) {
    super();

    const nameElt = el("button", { class: "pass-row-name", text: init.name });
    nameElt.addEventListener("click", cb.onSetActive);

    const renameElt = el("button", { class: "pass-row-rename", text: "Rename" });
    renameElt.addEventListener("click", () => prompt("New pass name"));

    this.elt = el("li", { class: init.isActive ? "pass-row is-active" : "pass-row" }, [
      nameElt,
      renameElt,
    ]);

    this.defer(() => this.elt.remove());
  }

  setActive(isActive: boolean) {
    this.elt.classList.toggle("is-active", isActive);
  }
}

export { PassRow };
