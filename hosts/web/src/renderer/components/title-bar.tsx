import { MenuIcon } from "lucide-react";

import { useEngineSnapshot } from "../engine/context";
import { Wordmark } from "./logo/wordmark";

import styles from "./title-bar.module.css";

const TitleBar = () => {
  const engineSnapshot = useEngineSnapshot();

  let activePassName = "<unknown pass>";
  for (const pass of engineSnapshot.passes) {
    if (pass.id === engineSnapshot.activePass) {
      activePassName = pass.name;
      break;
    }
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
        <div className={styles.project}>New project*</div>
        <span className={styles.slash}>/</span>
        <p>{activePassName}</p>
      </nav>

      {window.platform === "darwin" && <Wordmark className={styles.wordmark} />}
    </header>
  );
};

export { TitleBar };
