import { Project } from "../../mewo/classes/project";
import { Runtime } from "../../mewo/classes/runtime";

const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

// type Pass = { id: PassId; name: string };
// type EngineSnapshot = { passes: Pass[]; activePass: PassId };

class Engine implements Disposable {
  readonly #stack = new DisposableStack();
  readonly #runtime: Runtime;
  readonly #project: Project;

  #listeners = new Set<() => void>();

  constructor() {
    this.#runtime = this.#stack.use(new Runtime());
    this.#project = this.#stack.use(new Project("Pass 1", kDefaultPassSource));
  }

  get runtime() {
    return this.#runtime;
  }

  get project() {
    return this.#project;
  }

  subscribe(fn: () => void): () => boolean {
    this.#listeners.add(fn);
    return () => this.#listeners.delete(fn);
  }

  [Symbol.dispose]() {
    this.#stack.dispose();
  }
}

export { Engine };
