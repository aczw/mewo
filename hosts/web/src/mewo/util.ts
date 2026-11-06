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

export { resultToString };
