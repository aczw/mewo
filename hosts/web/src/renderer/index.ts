import type { Project } from "../mewo/classes/project";
import type { Runtime } from "../mewo/classes/runtime";
import { Session } from "../mewo/classes/session";
import type { Surface } from "../mewo/classes/surface";
import { MewoEvent } from "../mewo/types";
import { Editor } from "./components/editor";
import { initializeEngine } from "./engine";

type App = {
  runtime: Runtime;
  surface: Surface;
  project: Project;
  session: Session | null;

  editor: Editor;
};

const app: App = {
  ...initializeEngine(),
  editor: new Editor(document.getElementById("editor") as HTMLDivElement),
};

const onFrame: FrameRequestCallback = (time) => {
  const { runtime, surface, project } = app;
  runtime.tick();

  for (const event of runtime.pollEvents()) {
    if (event.type === MewoEvent.None) break;

    if (event.type === MewoEvent.DeviceReady) {
      console.info("[libmewo] device is ready");
      app.session = new Session(
        runtime,
        project,
        surface.dimensions.width,
        surface.dimensions.height,
      );
    } else {
      console.error("[libmewo]", event.message);
    }
  }

  if (app.session === null) {
    window.requestAnimationFrame(onFrame);
    return;
  }

  if (surface.checkForResize()) {
    const dims = surface.dimensions;
    if (dims.width !== 0 && dims.height !== 0) app.session.setRenderSize(dims.width, dims.height);
  }

  app.session.render(surface, time);
  window.requestAnimationFrame(onFrame);
};

window.requestAnimationFrame(onFrame);
