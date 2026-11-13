import type { EditorState } from "@codemirror/state";

import { CompilationStatus } from "./compilation-status";
import { DiagnosticsCount } from "./diagnostics-count";

import styles from "./info.module.css";

type DocInfo = { line: number; column: number; length: number };

const calculateDocInfo = (state: EditorState): DocInfo => {
  const head = state.selection.main.head;
  const line = state.doc.lineAt(head);
  return { line: line.number, column: head - line.from + 1, length: state.doc.length };
};

const Info = ({ docInfo }: { docInfo: DocInfo }) => {
  return (
    <div className={styles.info}>
      <div className={styles.left}>
        <DiagnosticsCount />
        <CompilationStatus />
      </div>

      <div className={styles.right}>
        <p>{docInfo.length} chars</p>
        <p className={styles.lineColumn}>
          Ln {docInfo.line}, Col {docInfo.column}
        </p>
      </div>
    </div>
  );
};

export { Info, calculateDocInfo, type DocInfo };
