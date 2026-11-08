import { Component } from "../system/component";
import { el } from "../system/dom";

type Init = {
  name: string;
  isActive: boolean;
};

class PassRow extends Component {
  constructor(parent: HTMLElement, init: Init) {
    super();

    const listItemElt = el("li", {
      text: init.name,
      class: init.isActive ? "pass-row is-active" : "pass-row",
    });

    parent.append(listItemElt);
    this.defer(() => listItemElt.remove());
  }
}

export { PassRow };
