import { Mewo } from "../mewo/api";
import { Runtime } from "../mewo/classes";
import { kNull, MewoEvent, type Ptr } from "../mewo/types";
import { resultToString } from "../mewo/util";

const kCanvasId = "mewo" as const;

const information = document.getElementById("info")!;
information.innerText = `Chrome (v${versions.chrome()}) / Node.js (v${versions.node()}) / Electron (v${versions.electron()})`;

const main = () => {
  const runtime = new Runtime();
  console.log("[libmewo] created runtime");

  const surface = runtime.createSurface(document.getElementById(kCanvasId) as HTMLCanvasElement);
  console.log("[libmewo] created surface");

  const project = Mewo.projectCreate();

  if (project === kNull) {
    console.error("project creation failed:", resultToString(Mewo.lastResult()));
    return;
  } else {
    console.log("created project");
  }

  runtime.requestDevice(surface);

  let session: Ptr = kNull;

  const onFrame: FrameRequestCallback = (time) => {
    runtime.tick();

    for (const event of runtime.pollEvents()) {
      if (event.type === MewoEvent.None) break;

      if (event.type === MewoEvent.DeviceReady) {
        console.info("[libmewo] device is ready");
        session = Mewo.sessionCreate(
          runtime.handle,
          project,
          surface.dimensions.width,
          surface.dimensions.height,
        );

        if (session === kNull)
          console.error("[libmewo] session creation failed:", resultToString(Mewo.lastResult()));
      } else {
        console.error("[libmewo]", event.message);
      }
    }

    if (session === kNull) {
      window.requestAnimationFrame(onFrame);
      return;
    }

    if (surface.checkForResize()) {
      const dims = surface.dimensions;

      if (dims.width !== 0 && dims.height !== 0) {
        if (!Mewo.sessionSetRenderSize(session, dims.width, dims.height))
          console.error("[libmewo] failed to set render size:", resultToString(Mewo.lastResult()));
      }
    }

    if (!Mewo.sessionRender(session, surface.handle, time))
      console.error("session failed to render:", resultToString(Mewo.lastResult()));

    window.requestAnimationFrame(onFrame);
  };

  window.requestAnimationFrame(onFrame);
};

main();
