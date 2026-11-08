import { Project } from "../../mewo/classes/project";
import { Runtime } from "../../mewo/classes/runtime";
import { Session } from "../../mewo/classes/session";
import { MewoEvent } from "../../mewo/types";
import { Component } from "../system/component";
import { el } from "../system/dom";
import { Mutable } from "../system/mutable";
import { Editor } from "./editor";
import { Viewport } from "./viewport";

const kDefaultPassSource = `fn main(frag_coord: vec2f) -> vec4f {
  let uv = frag_coord / mw.resolution;
  return vec4f(0.5 + 0.5 * cos(mw.time + uv.xyx + vec3f(0.0, 2.0, 4.0)), 1.0);
}`;

class App extends Component {
  readonly runtime: Runtime;
  readonly project: Project;
  readonly sessionHolder: Mutable<Session>;

  readonly editor: Editor;
  readonly viewport: Viewport;

  constructor(parent: HTMLElement) {
    super();

    this.runtime = this.use(new Runtime());
    this.project = this.use(new Project("Pass 1", kDefaultPassSource));
    this.sessionHolder = this.use(new Mutable<Session>());

    const editorElt = el("div", { class: "left" });
    const viewportElt = el("div", { class: "viewport" });
    const passListElt = el("ul", { class: "pass-list", text: "Passes" });

    const main = el("main", {}, [
      editorElt,
      el("div", { class: "right" }, [viewportElt, passListElt]),
    ]);

    parent.append(main);
    this.defer(() => main.remove());

    this.editor = this.use(new Editor(editorElt));
    this.viewport = this.use(new Viewport(viewportElt, this.runtime));

    this.runtime.requestDevice(this.viewport.surface);
    this.#run();
  }

  #run(): void {
    const onFrame: FrameRequestCallback = (time) => {
      this.runtime.tick();

      for (const event of this.runtime.pollEvents()) {
        switch (event.type) {
          case MewoEvent.DeviceReady: {
            console.info("[libmewo] device is ready");

            const { width, height } = this.viewport.surface.dimensions;
            this.sessionHolder.value = new Session(this.runtime, this.project, width, height);

            break;
          }

          default: {
            console.error("[libmewo]", event.message);
            break;
          }
        }
      }

      if (this.sessionHolder.value === null) {
        window.requestAnimationFrame(onFrame);
        return;
      }

      const session = this.sessionHolder.value;

      if (this.viewport.surface.checkForResize()) {
        const dims = this.viewport.surface.dimensions;

        if (dims.width !== 0 && dims.height !== 0) session.setRenderSize(dims.width, dims.height);
      }

      session.render(this.viewport.surface, time);
      window.requestAnimationFrame(onFrame);
    };

    const id = window.requestAnimationFrame(onFrame);

    this.defer(() => window.cancelAnimationFrame(id));
  }
}

export { App };
