import { Project } from "../mewo/classes/project";
import { Runtime } from "../mewo/classes/runtime";
import { Surface } from "../mewo/classes/surface";

const kCanvasId = "mewo";
const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

const initialize = () => {
  const runtime = new Runtime();
  console.info("[libmewo] created runtime");

  const surface = new Surface(runtime, document.getElementById(kCanvasId) as HTMLCanvasElement);
  console.info("[libmewo] created surface");

  const project = new Project("Pass 1", kDefaultPassSource);
  console.info("[libmewo] initialized new project");

  runtime.requestDevice(surface);

  return { runtime, surface, project, session: null };
};

export { initialize as initializeEngine };
