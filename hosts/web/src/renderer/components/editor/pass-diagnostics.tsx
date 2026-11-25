import { Button } from "@base-ui/react";
import { ComponentIcon, InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import { MewoDiagnosticSeverity, MewoDiagnosticSource } from "../../../mewo/types";
import { useEngine, useEngineSnapshot } from "../../engine/context";
import { kIconSize } from "../../scripts/constants";

const PassDiagnostics = () => {
  const engine = useEngine();
  const { passes, selection, commonDiagnostics } = useEngineSnapshot();

  const activePass =
    selection.kind === "pass" ? passes.find((pass) => pass.id === selection.id) : undefined;

  if (activePass === undefined)
    return <p className="p-1.5 text-fg-subtle">No active pass selected.</p>;

  const diagnostics = activePass.diagnostics.filter(
    (diag) => diag.source !== MewoDiagnosticSource.Common,
  );
  const errorCommonDiagnostics = commonDiagnostics.filter(
    (diag) => diag.severity === MewoDiagnosticSeverity.Error,
  );

  if (diagnostics.length === 0 && errorCommonDiagnostics.length === 0)
    return <p className="p-1.5 text-fg-subtle">No diagnostics found for {activePass.name}.</p>;

  return (
    <ul className="*:flex *:justify-between *:gap-x-3 *:border-b *:border-b-border *:p-1.5">
      {errorCommonDiagnostics.length > 0 && (
        <li>
          <div className="flex gap-x-1.5">
            <ComponentIcon size={kIconSize} className="h-lh shrink-0" />
            <p>
              Common has {errorCommonDiagnostics.length} error
              {errorCommonDiagnostics.length > 1 ? "s " : ""}.
            </p>
          </div>
          <Button
            onClick={() => engine.selectCommon()}
            className="btn h-fit bg-base hover:bg-overlay"
          >
            Switch
          </Button>
        </li>
      )}

      {diagnostics.map((diag, index) => (
        <li key={index} className="hover:bg-base">
          <div className="flex gap-x-1.5">
            {diag.severity === MewoDiagnosticSeverity.Error ? (
              <OctagonXIcon size={kIconSize} className="h-lh shrink-0" />
            ) : diag.severity === MewoDiagnosticSeverity.Warning ? (
              <TriangleAlertIcon size={kIconSize} className="h-lh shrink-0" />
            ) : (
              <InfoIcon size={kIconSize} className="h-lh shrink-0" />
            )}

            <p className="whitespace-pre-wrap first-letter:uppercase">{diag.message}</p>
          </div>

          <p className="shrink-0 text-fg-subtle">
            {diag.source === MewoDiagnosticSource.None ? (
              ""
            ) : diag.source === MewoDiagnosticSource.Generated ? (
              "(generated)"
            ) : (
              <span className="flex gap-x-3">
                <span>Ln {diag.line}</span>
                <span>Col {diag.column}</span>
              </span>
            )}
          </p>
        </li>
      ))}
    </ul>
  );
};

export { PassDiagnostics };
