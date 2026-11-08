abstract class Component implements Disposable {
  #stack = new DisposableStack();

  protected use<T extends Disposable>(child: T): T {
    return this.#stack.use(child);
  }

  protected defer(fn: () => void): void {
    this.#stack.defer(fn);
  }

  [Symbol.dispose](): void {
    this.#stack.dispose();
  }
}

export { Component };
