import type { EditorState } from "@codemirror/state";

import { DiagnosticsCount } from "./diagnostics-count";

type DocInfo = { line: number; column: number; length: number };

const calculateDocInfo = (state: EditorState): DocInfo => {
  const head = state.selection.main.head;
  const line = state.doc.lineAt(head);
  return { line: line.number, column: head - line.from + 1, length: state.doc.length };
};

const StatusBar = ({ docInfo }: { docInfo: DocInfo }) => {
  return (
    <div className="flex justify-between rounded-b-lg bg-gray-dark p-1.5">
      <DiagnosticsCount />

      <div className="flex items-center gap-x-3 *:leading-none">
        <p>{docInfo.length} chars</p>
        <p>
          Ln {docInfo.line}, Col {docInfo.column}
        </p>
      </div>
    </div>
  );
};

export { StatusBar, calculateDocInfo, type DocInfo };
