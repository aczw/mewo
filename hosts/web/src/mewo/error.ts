import { MewoResult } from "./types";
import { resultToString } from "./util";

class MewoError extends Error {
  readonly result: MewoResult;

  constructor(result: MewoResult) {
    super(`[libmewo] call failed: ${resultToString(result)}`);
    this.name = "MewoError";
    this.result = result;
  }
}

export { MewoError };
