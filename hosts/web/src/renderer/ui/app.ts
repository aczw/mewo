import { Component } from "../system/component";
import { el } from "../system/dom";
import { Engine } from "../system/engine";
import { Editor } from "./editor";
import { ProjectController } from "./project-controller";
import { Viewport } from "./viewport";

class App extends Component {
  readonly #engine: Engine;
  readonly #viewport: Viewport;
  readonly #projectController: ProjectController;
  readonly #editor: Editor;

  constructor(parent: HTMLElement) {
    super();

    this.#engine = this.use(new Engine());
    const engine = this.#engine;

    const editorElt = el("div", { class: "left" });
    const rightElt = el("div", { class: "right" });
    const main = el("main", {}, [editorElt, rightElt]);
    parent.append(main);
    this.defer(() => main.remove());

    const snapshot = engine.getSnapshot();

    this.#viewport = this.use(
      new Viewport(rightElt, snapshot, {
        onSetPaused: (paused) => engine.setPaused(paused),
      }),
    );
    this.defer(engine.createSurfaceAndAttachCanvas(this.#viewport.canvas));
    this.defer(engine.subscribeFrame((stats) => this.#viewport.updateStats(stats)));

    this.#projectController = this.use(
      new ProjectController(rightElt, snapshot, {
        onAddPass: () => engine.addPass(),
        onRemovePass: (passId) => engine.removePass(passId),
        onSetActivePass: (passId) => engine.setActivePass(passId),
        onPassRename: (passId, name) => engine.renamePass(passId, name),
      }),
    );

    this.#editor = this.use(
      new Editor(editorElt, snapshot, {
        getPassSource: (pass) => engine.getPassSource(pass),
        onSourceChanged: (pass, source) => engine.setPassSource(pass, source),
      }),
    );

    this.defer(engine.subscribe(() => this.#refresh()));
  }

  #refresh(): void {
    const snapshot = this.#engine.getSnapshot();
    this.#viewport.update(snapshot);
    this.#projectController.update(snapshot);
    this.#editor.update(snapshot);
  }
}

export { App };
