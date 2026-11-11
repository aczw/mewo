import { useEngine } from "../engine/context";
import { PassList } from "./pass-list";

const ProjectController = () => {
  const engine = useEngine();

  return (
    <div className="project-controller">
      <PassList />
      <button className="add-pass" onClick={() => engine.addPass()}>
        Add pass
      </button>
    </div>
  );
};

export { ProjectController };
