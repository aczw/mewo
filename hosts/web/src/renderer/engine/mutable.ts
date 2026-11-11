class Mutable<T extends Disposable> implements Disposable {
  #value: T | null = null;

  get value(): T | null {
    return this.#value;
  }

  set value(newValue: T | null) {
    if (newValue === this.#value) return;

    this.#value?.[Symbol.dispose]();
    this.#value = newValue;
  }

  [Symbol.dispose](): void {
    this.value = null;
  }
}

export { Mutable };
