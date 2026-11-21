import type { EditorState } from "@codemirror/state";

import { MewoDiagnosticSource } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";
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
  const { passes, selection, commonDiagnostics } = useEngineSnapshot();

  const activePass =
    selection.kind === "pass" ? passes.find((pass) => pass.id === selection.id) : undefined;
  const hasDiagnostics =
    selection.kind === "pass"
      ? (activePass?.diagnostics.filter((diag) => diag.source !== MewoDiagnosticSource.Common)
          .length ?? 0) > 0
      : commonDiagnostics.length > 0;

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
          {hasDiagnostics ? (
            <ul>
              <li>Diagnostics</li>
              <li>Diagnostics</li>
              <li>Diagnostics</li>
              <li>Diagnostics</li>
              <li>Diagnostics</li>
            </ul>
          ) : (
            <p className="text-fg-subtle">No diagnostics.</p>
          )}
        </div>
      )}
    </div>
  );
};

export { StatusBar, calculateDocInfo, type DocInfo };
