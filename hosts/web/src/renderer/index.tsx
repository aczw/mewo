import { StrictMode } from "react";
import { createRoot } from "react-dom/client";

import { App } from "./components/app";
import { EngineContext } from "./engine/context";
import { Engine } from "./engine/engine";

const engine = new Engine();
document.documentElement.dataset.platform = window.platform;

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <EngineContext value={engine}>
      <App />
    </EngineContext>
  </StrictMode>,
);
