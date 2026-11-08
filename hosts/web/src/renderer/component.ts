abstract class Component implements Disposable {
  readonly elt: HTMLElement;
  #stack = new DisposableStack();

  constructor(elt: HTMLElement) {
    this.elt = elt;
  }

  protected add<T extends Component>(child: T): T {
    return this.#stack.use(child);
  }

  protected defer(fn: () => void): void {
    this.#stack.defer(fn);
  }

  [Symbol.dispose](): void {
    this.#stack.dispose();
    this.elt.remove();
  }
}

export { Component };
