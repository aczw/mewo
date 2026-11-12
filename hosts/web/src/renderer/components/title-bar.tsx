import { Wordmark } from "./logo/wordmark";

import styles from "./title-bar.module.css";

const TitleBar = () => {
  return (
    <header className={styles.titleBar}>
      <nav className={styles.tab}>
        <div className={styles.project}>New project*</div>
        <span className={styles.slash}>/</span>
        <p>Pass name</p>
      </nav>

      <Wordmark className={styles.wordmark} />
    </header>
  );
};

export { TitleBar };
