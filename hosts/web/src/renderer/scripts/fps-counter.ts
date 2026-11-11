class FpsCounter {
  readonly #values: number[] = [];
  #average = 0;

  push(deltaTime: number): void {
    if (deltaTime < Number.EPSILON) return;

    const fps = Math.round(1000 / deltaTime);
    this.#values.push(fps);

    if (this.#values.length > 100) this.#values.shift();

    let sum = 0;
    for (const value of this.#values) sum += value;
    this.#average = Math.round((sum / this.#values.length) * 10) * 0.1;
  }

  get average(): number {
    return this.#average;
  }
}

export { FpsCounter };
