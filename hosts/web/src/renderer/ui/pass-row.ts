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

    const buttonElt = el("button", { text: init.name });
    buttonElt.addEventListener("click", cb.onSetActive);
    this.elt = el("li", { class: init.isActive ? "pass-row is-active" : "pass-row" }, [buttonElt]);

    this.defer(() => this.elt.remove());
  }

  setActive(isActive: boolean) {
    this.elt.classList.toggle("is-active", isActive);
  }
}

export { PassRow };
