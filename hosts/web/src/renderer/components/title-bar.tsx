import { MenuIcon } from "lucide-react";

import { Wordmark } from "./logo/wordmark";

import styles from "./title-bar.module.css";

const TitleBar = () => {
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
        <p>Pass name</p>
      </nav>

      {window.platform === "darwin" && <Wordmark className={styles.wordmark} />}
    </header>
  );
};

export { TitleBar };
