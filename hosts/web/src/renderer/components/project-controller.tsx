import { useEngine } from "../engine/context";
import { PassList } from "./pass-list";

import styles from "./project-controller.module.css";

const ProjectController = () => {
  const engine = useEngine();

  return (
    <div className={styles.projectController}>
      <PassList />

      <button className={styles.addPass} onClick={() => engine.addPass()}>
        Add pass
      </button>
    </div>
  );
};

export { ProjectController };
