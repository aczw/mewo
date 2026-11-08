import { Project } from "../../mewo/classes/project";
import { Runtime } from "../../mewo/classes/runtime";
import { Session } from "../../mewo/classes/session";
import { MewoEvent, type PassId } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { Mutable } from "../system/mutable";
import { Editor } from "./editor";
import { ProjectController } from "./project-controller";
import { Viewport } from "./viewport";

const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

const kNewPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return textureSample(prev_tex, prev_smp, uv);
}`;

class App extends Component {
  readonly #runtime: Runtime;
  readonly #project: Project;
  readonly #sessionHolder: Mutable<Session>;

  readonly #viewport: Viewport;
  readonly #projectController: ProjectController;
  readonly #editor: Editor;

  #rafId = 0;
  #activePassId: PassId;

  constructor(parent: HTMLElement) {
    super();

    this.#runtime = this.use(new Runtime());
    this.#project = this.use(new Project("Pass 1", kDefaultPassSource));
    this.#sessionHolder = this.use(new Mutable<Session>());

    const editorElt = el("div", { class: "left" });
    const rightElt = el("div", { class: "right" });
    const main = el("main", {}, [editorElt, rightElt]);
    parent.append(main);
    this.defer(() => main.remove());

    // First pass has to exist
    this.#activePassId = this.#project.getPassId(0)!;

    this.#viewport = this.use(new Viewport(rightElt, { runtime: this.#runtime }));
    this.#projectController = this.use(
      new ProjectController(
        rightElt,
        {
          project: this.#project,
          activePassId: this.#activePassId,
        },
        {
          onAddPass: () => {
            const passCount = this.#project.getPassCount();
            const newPassId = this.#project.addPass(`Pass ${passCount + 1}`, kNewPassSource);

            this.#editor.addState(this.#project, newPassId);
            this.#projectController.addPass(this.#project, newPassId);

            this.#setActivePassId(newPassId);
          },

          onSetActivePass: (newActivePassId) => {
            if (this.#activePassId === newActivePassId) return;
            this.#setActivePassId(newActivePassId);
          },
        },
      ),
    );

    this.#editor = this.use(
      new Editor(
        editorElt,
        { project: this.#project, activePassId: this.#activePassId },
        {
          onSourceChanged: (shownPassId, newSource) => {
            this.#project.setPassSource(shownPassId, newSource);
          },
        },
      ),
    );

    this.#runtime.requestDevice(this.#viewport.surface);
    this.#run();
  }

  #run(): void {
    const onFrame: FrameRequestCallback = (time) => {
      this.#rafId = window.requestAnimationFrame(onFrame);
      this.#runtime.tick();

      for (const event of this.#runtime.pollEvents()) {
        switch (event.type) {
          case MewoEvent.DeviceReady: {
            console.info("[libmewo] device is ready");

            const { width, height } = this.#viewport.surface.dimensions;
            this.#sessionHolder.value = new Session(this.#runtime, this.#project, width, height);

            break;
          }

          default: {
            console.error("[libmewo]", event.message);
            break;
          }
        }
      }

      const session = this.#sessionHolder.value;
      if (session === null) return;

      if (this.#viewport.surface.checkForResize()) {
        const dims = this.#viewport.surface.dimensions;
        if (dims.width !== 0 && dims.height !== 0) session.setRenderSize(dims.width, dims.height);
      }

      session.render(this.#viewport.surface, time);
    };

    this.#rafId = window.requestAnimationFrame(onFrame);
    this.defer(() => window.cancelAnimationFrame(this.#rafId));
  }

  #setActivePassId(newActivePassId: PassId) {
    this.#activePassId = newActivePassId;
    this.#editor.showState(this.#activePassId);
    this.#projectController.setActivePass(this.#activePassId);
  }
}

export { App };
