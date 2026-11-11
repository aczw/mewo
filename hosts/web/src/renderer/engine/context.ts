import { createContext, use, useSyncExternalStore } from "react";

import { Engine, type EngineSnapshot } from "./engine";

const EngineContext = createContext<Engine | null>(null);

const useEngine = (): Engine => {
  const engine = use(EngineContext);
  if (engine === null) throw new Error("useEngine called outside <EngineContext>");
  return engine;
};

const useEngineSnapshot = (): EngineSnapshot => {
  const engine = useEngine();
  return useSyncExternalStore(engine.subscribe, engine.getSnapshot);
};

export { EngineContext, useEngine, useEngineSnapshot };
