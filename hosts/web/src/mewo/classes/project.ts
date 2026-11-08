import { Mewo } from "../api";
import { MewoError } from "../error";
import { kNull, kPassIdNone, MewoResult, type PassId, type Ptr } from "../types";

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

  addPass(name: string, source: string): PassId {
    const passId = Mewo.projectAddPass(this.handle, name, source);
    if (passId === kPassIdNone) throw new MewoError("failed to add pass");
    return passId;
  }

  removePass(passId: PassId): void {
    if (!Mewo.projectRemovePass(this.handle, passId)) throw new MewoError("failed to remove pass");
  }

  movePass(passId: PassId, newIndex: number): void {
    if (!Mewo.projectMovePass(this.handle, passId, newIndex))
      throw new MewoError("failed to move pass");
  }

  getPassCount(): number {
    return Mewo.projectGetPassCount(this.handle);
  }

  getPassId(index: number): PassId | null {
    const passId = Mewo.projectGetPassId(this.handle, index);
    return passId === kPassIdNone ? null : passId;
  }

  getPassName(passId: PassId): string {
    const name = Mewo.projectGetPassName(this.handle, passId);

    // An empty string is a valid name, so we have to check the result explicitly
    if (Mewo.lastResult() !== MewoResult.Success) throw new MewoError("failed to get pass name");

    return name;
  }

  getPassSource(passId: PassId): string {
    const source = Mewo.projectGetPassSource(this.handle, passId);

    // An empty string is a valid source, so we have to check the result explicitly
    if (Mewo.lastResult() !== MewoResult.Success) throw new MewoError("failed to get pass source");

    return source;
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.projectDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Project };
