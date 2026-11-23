import { MewoResult } from "./types";

const resultToString = (result: MewoResult): string => {
  switch (result) {
    case MewoResult.Success:
      return "success";
    case MewoResult.NotReady:
      return "not ready";
    case MewoResult.Internal:
      return "internal";
    case MewoResult.InvalidArgument:
      return "invalid argument";
    case MewoResult.InvalidOperation:
      return "invalid operation";
    default:
      return `unknown (${result})`;
  }
};

class MewoError extends Error {
  readonly result: MewoResult;

  constructor(message: string, result: MewoResult) {
    super(`[libmewo] ${message}: ${resultToString(result)}`);
    this.name = "MewoError";
    this.result = result;
  }
}

export { MewoError };
