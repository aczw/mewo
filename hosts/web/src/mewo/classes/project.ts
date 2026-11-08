import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, kPassIdNone, type PassId, type Ptr } from "../types";

class Project implements Disposable {
  #handle: Ptr;

  constructor(firstPassName: string, firstPassSource: string) {
    this.#handle = Mewo.projectCreate(firstPassName, firstPassSource);
    if (this.#handle === kNull) throw new MewoError("project creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("project used after dispose");
    return this.#handle;
  }

  setPassSource(passId: PassId, source: string): void {
    if (!Mewo.projectSetPassSource(this.handle, passId, source))
      throw new MewoError("failed to set pass source");
  }

  setCommonSource(source: string): void {
    if (!Mewo.projectSetCommonSource(this.handle, source))
      throw new MewoError("failed to set common source");
  }

  /** Appends a pass to the end of the list and returns its ID. */
  addPass(name: string, source: string): PassId {
    // `>>> 0`: wasm returns a `uint32_t` as a signed i32, so reinterpret it as unsigned.
    const passId = Mewo.projectAddPass(this.handle, name, source) >>> 0;
    if (passId === kPassIdNone) throw new MewoError("failed to add pass");
    return passId;
  }

  /** Throws a `MewoError` with `MewoResult.InvalidOperation` when removing the last pass. */
  removePass(passId: PassId): void {
    if (!Mewo.projectRemovePass(this.handle, passId)) throw new MewoError("failed to remove pass");
  }

  /** `newIndex` is the pass' index after the move. */
  movePass(passId: PassId, newIndex: number): void {
    if (!Mewo.projectMovePass(this.handle, passId, newIndex))
      throw new MewoError("failed to move pass");
  }

  /** Always >= 1. */
  getPassCount(): number {
    return Mewo.projectGetPassCount(this.handle) >>> 0;
  }

  /** Returns `null` if `index` is out of range. */
  getPassId(index: number): PassId | null {
    const passId = Mewo.projectGetPassId(this.handle, index) >>> 0;
    return passId === kPassIdNone ? null : passId;
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.projectDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Project };
