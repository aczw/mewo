type Argument = "string" | "number";

export interface WasmModule {
  cwrap(
    name: string,
    returnType: "string" | "number" | "boolean" | null,
    argTypes: Argument[],
  ): (...args: any[]) => any;
}

declare function createWasmModule(): Promise<WasmModule>;

export default createWasmModule;
