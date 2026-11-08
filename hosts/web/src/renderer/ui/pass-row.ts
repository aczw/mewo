import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  name: string;
  isActive: boolean;
};

class PassRow extends Component {
  readonly #listItemElt: HTMLLIElement;

  constructor(parent: HTMLElement, init: Init) {
    super();

    this.#listItemElt = el("li", {
      text: init.name,
      class: init.isActive ? "pass-row is-active" : "pass-row",
    });

    parent.append(this.#listItemElt);
    this.defer(() => this.#listItemElt.remove());
  }

  toggleActive() {
    this.#listItemElt.classList.toggle("is-active");
  }
}

export { PassRow };
