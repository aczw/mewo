import { MewoError } from "./error";
import createWasmModule from "./generated/mewo";
import { MewoResult, type Ptr } from "./types";

const module = await createWasmModule();

/** Call right after a fallible operation. Throws if it failed. */
const check = <T>(value: T, what: string): T => {
  const result = module._mewo_web_last_result() as MewoResult;
  if (result !== MewoResult.Success) throw new MewoError(what, result);
  return value;
};

/** Copies JS strings into WASM memory for the duration of `fn`. */
const withCStrings = <T>(strings: string[], fn: (ptrs: Ptr[]) => T): T => {
  const ptrs = strings.map((str) => module.stringToNewUTF8(str));
  try {
    return fn(ptrs);
  } finally {
    for (const ptr of ptrs) module._free(ptr);
  }
};

const readCString = (ptr: Ptr): string => module.UTF8ToString(ptr);

export { module as wasm, check, withCStrings, readCString };
