import { Editor } from "./components/editor";
import { ProjectController } from "./components/project-controller";
import { Viewport } from "./components/viewport";

const App = () => {
  return (
    <main>
      <div className="left">
        <Editor />
      </div>
      <div className="right">
        <Viewport />
        <ProjectController />
      </div>
    </main>
  );
};

export { App };
