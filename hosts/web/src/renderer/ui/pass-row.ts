import { Component } from "../system/component";
import { el } from "../system/dom";

class PassRow extends Component {
  constructor(parent: HTMLElement, name: string, isActive: boolean) {
    super();

    const listItemElt = el("li", {
      text: name,
      class: isActive ? "pass-row is-active" : "pass-row",
    });

    parent.append(listItemElt);
    this.defer(() => listItemElt.remove());
  }
}

export { PassRow };
