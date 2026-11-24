import { kNull, kPassIdNone, MewoNameCheck, type PassId, type Ptr } from "../types";
import { check, readCString, wasm, withCStrings } from "../wasm";

class Project implements Disposable {
  #handle: Ptr;

  constructor(firstPassName: string, firstPassSource: string) {
    const handle = withCStrings([firstPassName, firstPassSource], ([n, s]) =>
      wasm._project_create(n!, s!),
    );
    this.#handle = check(handle, "project creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("project used after dispose");
    return this.#handle;
  }

  setPassSource(passId: PassId, source: string): void {
    withCStrings([source], ([s]) => wasm._project_set_pass_source(this.handle, passId, s!));
    check(undefined, "failed to set pass source");
  }

  setPassName(passId: PassId, name: string): void {
    withCStrings([name], ([n]) => wasm._project_set_pass_name(this.handle, passId, n!));
    check(undefined, "failed to rename pass");
  }

  checkPassName(name: string, renamedPassId: PassId = kPassIdNone): MewoNameCheck {
    const result = withCStrings([name], ([n]) =>
      wasm._project_check_pass_name(this.handle, renamedPassId, n!),
    );
    return check(result as MewoNameCheck, "failed to check pass name");
  }

  setCommonSource(source: string): void {
    withCStrings([source], ([s]) => wasm._project_set_common_source(this.handle, s!));
    check(undefined, "failed to set common source");
  }

  addPass(name: string, source: string): PassId {
    const passId = withCStrings([name, source], ([n, s]) =>
      wasm._project_add_pass(this.handle, n!, s!),
    );
    return check(passId, "failed to add pass");
  }

  removePass(passId: PassId): void {
    wasm._project_remove_pass(this.handle, passId);
    check(undefined, "failed to remove pass");
  }

  movePass(passId: PassId, newIndex: number): void {
    wasm._project_move_pass(this.handle, passId, newIndex);
    check(undefined, "failed to move pass");
  }

  getPassCount(): number {
    return wasm._project_get_pass_count(this.handle);
  }

  getPassId(index: number): PassId | null {
    // Can't fail: an out-of-range index returns `MEWO_PASS_ID_NONE` instead of an error
    const passId = wasm._project_get_pass_id(this.handle, index);
    return passId === kPassIdNone ? null : passId;
  }

  getPassName(passId: PassId): string {
    const ptr = wasm._project_get_pass_name(this.handle, passId);
    return readCString(check(ptr, "failed to get pass name"));
  }

  getPassSource(passId: PassId): string {
    const ptr = wasm._project_get_pass_source(this.handle, passId);
    return readCString(check(ptr, "failed to get pass source"));
  }

  getCommonSource(): string {
    return readCString(wasm._project_get_common_source(this.handle));
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    wasm._project_destroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Project };
