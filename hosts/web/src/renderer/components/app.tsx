import { Controller } from "./controller";
import { Editor } from "./editor";
import { EditorInfo } from "./editor-info";
import { Viewport } from "./viewport";

import styles from "./app.module.css";

const App = () => {
  return (
    <main className={styles.main}>
      <div className={styles.left}>
        <Editor />
        <EditorInfo />
      </div>
      <div className={styles.right}>
        <Viewport />
        <Controller />
      </div>
    </main>
  );
};

export { App };
