import type { EditorState } from "@codemirror/state";
import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import styles from "./info.module.css";

type DocInfo = { line: number; column: number; length: number };

const calculateDocInfo = (state: EditorState): DocInfo => {
  const head = state.selection.main.head;
  const line = state.doc.lineAt(head);
  return { line: line.number, column: head - line.from + 1, length: state.doc.length };
};

const Info = ({ info }: { info: DocInfo }) => {
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
        <p>{info.length} chars</p>
        <p>
          Ln {info.line}, Col {info.column}
        </p>
      </div>
    </div>
  );
};

export { Info, calculateDocInfo, type DocInfo };
