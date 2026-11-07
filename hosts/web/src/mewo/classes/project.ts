import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, type Ptr } from "../types";

class Project implements Disposable {
  #handle: Ptr;

  constructor() {
    this.#handle = Mewo.projectCreate();
    if (this.#handle === kNull) throw new MewoError("project creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("project used after dispose");
    return this.#handle;
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.projectDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Project };
