import { Mewo } from "./api";
import { MewoResult } from "./types";
import { resultToString } from "./util";

class MewoError extends Error {
  readonly result: MewoResult;

  constructor(message: string) {
    const result = Mewo.lastResult();
    super(`[libmewo] ${message}: ${resultToString(result)}`);

    this.name = "MewoError";
    this.result = result;
  }
}

export { MewoError };
