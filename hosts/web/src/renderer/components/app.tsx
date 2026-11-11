import { Editor } from "./editor";
import { ProjectController } from "./project-controller";
import { Viewport } from "./viewport";

import styles from "./app.module.css";

const App = () => {
  return (
    <main className={styles.main}>
      <div className={styles.left}>
        <Editor />
      </div>
      <div className={styles.right}>
        <Viewport />
        <ProjectController />
      </div>
    </main>
  );
};

export { App };
