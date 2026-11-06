export interface MewoModule {
  cwrap(
    name: string,
    returnType: "string" | "number" | "boolean" | null,
    argTypes: string[],
  ): (...args: any[]) => any;
}

declare function createMewo(): Promise<MewoModule>;

export default createMewo;
