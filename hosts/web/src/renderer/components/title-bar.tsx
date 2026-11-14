import { MenuIcon } from "lucide-react";

import { useEngineSnapshot } from "../engine/context";
import { Wordmark } from "./logo/wordmark";

import styles from "./title-bar.module.css";

const TitleBar = () => {
  const { selection, passes } = useEngineSnapshot();

  let sourceName: string;
  if (selection.kind === "pass") {
    let activePassName = "<unknown pass>";
    for (const pass of passes) {
      if (pass.id === selection.id) {
        activePassName = pass.name;
        break;
      }
    }
    sourceName = activePassName;
  } else {
    sourceName = "Common";
  }

  return (
    <header className={styles.titleBar}>
      {window.platform === "win32" && (
        <button onClick={() => alert("menu!")} className={styles.menu}>
          <div className={styles.menuButtonWrapper}>
            <Wordmark className={styles.wordmark} />
            <MenuIcon width="auto" height="100%" />
          </div>
        </button>
      )}

      <nav className={styles.tab}>
        <button onClick={() => alert("rename project!")} className={styles.renameProject}>
          <h1>New project*</h1>
        </button>

        <span className={styles.slash}>/</span>
        <p>{sourceName}</p>
      </nav>

      {window.platform === "darwin" && <Wordmark className={styles.wordmark} />}
    </header>
  );
};

export { TitleBar };
