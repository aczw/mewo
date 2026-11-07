import { Project } from "../mewo/classes/project";
import { Runtime } from "../mewo/classes/runtime";
import { Session } from "../mewo/classes/session";
import { Surface } from "../mewo/classes/surface";
import { MewoEvent } from "../mewo/types";

const kCanvasId = "mewo";
const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

const information = document.getElementById("info")!;
information.innerText = `Chrome (v${versions.chrome()}) / Node.js (v${versions.node()}) / Electron (v${versions.electron()})`;

const main = () => {
  const runtime = new Runtime();
  console.info("[libmewo] created runtime");

  const surface = new Surface(runtime, document.getElementById(kCanvasId) as HTMLCanvasElement);
  console.info("[libmewo] created surface");

  const project = new Project("Pass 1", kDefaultPassSource);
  console.info("[libmewo] initialized new project");

  runtime.requestDevice(surface);

  let session: Session | null = null;

  const onFrame: FrameRequestCallback = (time) => {
    runtime.tick();

    for (const event of runtime.pollEvents()) {
      if (event.type === MewoEvent.None) break;

      if (event.type === MewoEvent.DeviceReady) {
        console.info("[libmewo] device is ready");
        session = new Session(
          runtime,
          project,
          surface.dimensions.width,
          surface.dimensions.height,
        );
      } else {
        console.error("[libmewo]", event.message);
      }
    }

    if (session === null) {
      window.requestAnimationFrame(onFrame);
      return;
    }

    if (surface.checkForResize()) {
      const dims = surface.dimensions;
      if (dims.width !== 0 && dims.height !== 0) session.setRenderSize(dims.width, dims.height);
    }

    session.render(surface, time);
    window.requestAnimationFrame(onFrame);
  };

  window.requestAnimationFrame(onFrame);
};

main();
