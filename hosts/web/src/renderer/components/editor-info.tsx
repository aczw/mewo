import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import styles from "./editor-info.module.css";

const EditorInfo = () => {
  return (
    <div className={styles.editorInfo}>
      <div className={styles.left}>
        <ul className={styles.diagnostics}>
          <li className={styles.iconLabel}>
            <OctagonXIcon size="var(--icon)" /> 0
          </li>
          <li className={styles.iconLabel}>
            <TriangleAlertIcon size="var(--icon)" /> 0
          </li>
          <li className={styles.iconLabel}>
            <InfoIcon size="var(--icon)" /> 0
          </li>
        </ul>

        <p>
          Compiled <span className={styles.time}>in 23ms</span>
        </p>
      </div>

      <div className={styles.right}>
        <p>5234 chars</p>
        <p>Ln 2, Col 34</p>
      </div>
    </div>
  );
};

export { EditorInfo };
