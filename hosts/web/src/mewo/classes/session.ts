import { Mewo } from "../api";
import { MewoError } from "../error";
import {
  type Diagnostic,
  kNull,
  MewoPassCompileStatus,
  type PassCompilation,
  type PassId,
  type Ptr,
} from "../types";
import type { Project } from "./project";
import type { Runtime } from "./runtime";
import type { Surface } from "./surface";

class Session implements Disposable {
  #handle: Ptr;

  constructor(runtime: Runtime, project: Project, width: number, height: number) {
    this.#handle = Mewo.sessionCreate(runtime.handle, project.handle, width, height);
    if (this.#handle === kNull) throw new MewoError("session creation failed");
  }

  get handle(): Ptr {
    if (this.#handle === kNull) throw new Error("session used after dispose");
    return this.#handle;
  }

  /** In milliseconds. */
  get time(): number {
    return Mewo.sessionGetTime(this.handle);
  }

  get paused(): boolean {
    return Mewo.sessionIsPaused(this.handle);
  }

  set paused(paused: boolean) {
    Mewo.sessionSetPaused(this.handle, paused);
  }

  getPassStatusRevision(): number {
    return Mewo.sessionGetPassStatusRevision(this.handle);
  }

  readPassCompile(passId: PassId): PassCompilation | null {
    const status = Mewo.sessionGetPassCompileStatus(this.handle, passId);
    if (status === -1) return null;

    const duration = Mewo.sessionGetPassCompileDuration(this.handle, passId);
    if (duration === -1) return { status: MewoPassCompileStatus.Pending };

    return { status, duration: duration };
  }

  readPassDiagnostics(passId: PassId): Diagnostic[] | null {
    const count = Mewo.sessionGetPassDiagnosticCount(this.handle, passId);
    if (count === -1) return null;

    const diagnostics: Diagnostic[] = [];
    for (let idx = 0; idx < count; ++idx) {
      if (!Mewo.sessionSelectPassDiagnostic(this.handle, passId, idx))
        throw new MewoError("failed to read pass diagnostic");

      diagnostics.push({
        severity: Mewo.diagnosticSeverity(),
        message: Mewo.diagnosticMessage(),
        source: Mewo.diagnosticSource(),
        line: Mewo.diagnosticLine(),
        column: Mewo.diagnosticColumn(),
      });
    }

    return diagnostics;
  }

  setRenderSize(newWidth: number, newHeight: number): void {
    if (!Mewo.sessionSetRenderSize(this.handle, newWidth, newHeight))
      throw new MewoError("failed to set session render size");
  }

  render(surface: Surface, timeMs: number): void {
    if (!Mewo.sessionRender(this.handle, surface.handle, timeMs))
      throw new MewoError("session render failed");
  }

  [Symbol.dispose](): void {
    if (this.#handle === kNull) return;

    Mewo.sessionDestroy(this.#handle);
    this.#handle = kNull;
  }
}

export { Session };
