import { Controller } from "./controller";
import { Editor } from "./editor/editor";
import { TitleBar } from "./title-bar";
import { Viewport } from "./viewport";

import styles from "./app.module.css";

const App = () => {
  return (
    <>
      <TitleBar />
      <main className={styles.main}>
        <div className={styles.left}>
          <Editor />
        </div>
        <div className={styles.right}>
          <Viewport />
          <Controller />
        </div>
      </main>
    </>
  );
};

export { App };
