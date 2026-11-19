import type { EditorState } from "@codemirror/state";

import { DiagnosticsCount } from "./diagnostics-count";

type DocInfo = { line: number; column: number; length: number };

type Props = {
  docInfo: DocInfo;
  showDiagnostics: boolean;
  setShowDiagnostics: (show: boolean) => void;
};

const calculateDocInfo = (state: EditorState): DocInfo => {
  const head = state.selection.main.head;
  const line = state.doc.lineAt(head);
  return { line: line.number, column: head - line.from + 1, length: state.doc.length };
};

const StatusBar = ({ docInfo, showDiagnostics, setShowDiagnostics }: Props) => {
  return (
    <div className="@container flex h-full flex-col bg-base">
      <div className="flex shrink-0 justify-between border-b border-b-border p-1.5">
        <DiagnosticsCount
          showDiagnostics={showDiagnostics}
          setShowDiagnostics={setShowDiagnostics}
        />

        <div className="flex items-center gap-x-3 *:leading-none">
          <p className="@max-[19rem]:hidden">
            {docInfo.length} <span className="text-fg-subtle">chars</span>
          </p>
          <p>
            <span className="text-fg-subtle">Ln</span> {docInfo.line}
          </p>
          <p>
            <span className="text-fg-subtle">Col</span> {docInfo.column}
          </p>
        </div>
      </div>

      {showDiagnostics && (
        <div className="min-h-0 grow overflow-y-auto border-b border-b-border bg-surface p-1.5">
          <ul>
            <li>Diagnostics</li>
            <li>Diagnostics</li>
            <li>Diagnostics</li>
            <li>Diagnostics</li>
            <li>Diagnostics</li>
          </ul>
        </div>
      )}
    </div>
  );
};

export { StatusBar, calculateDocInfo, type DocInfo };
