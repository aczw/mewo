import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import { MewoDiagnosticSeverity, MewoDiagnosticSource } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";
import { kIconSize } from "../../scripts/constants";

const CommonDiagnostics = () => {
  const { commonDiagnostics } = useEngineSnapshot();

  if (commonDiagnostics.length === 0)
    return <p className="p-1.5 text-fg-subtle">No diagnostics found for common.</p>;

  return (
    <ul className="*:flex *:justify-between *:gap-x-3 *:border-b *:border-b-border *:p-1.5">
      {commonDiagnostics.map((diag) => (
        <li
          key={`${diag.line}:${diag.column}:${diag.message}:${diag.severity}`}
          className="@container hover:bg-base"
        >
          <div className="flex gap-x-1.5">
            {diag.severity === MewoDiagnosticSeverity.Error ? (
              <OctagonXIcon size={kIconSize} className="h-lh shrink-0" />
            ) : diag.severity === MewoDiagnosticSeverity.Warning ? (
              <TriangleAlertIcon size={kIconSize} className="h-lh shrink-0" />
            ) : (
              <InfoIcon size={kIconSize} className="h-lh shrink-0" />
            )}

            <div className="flex flex-col gap-x-3 @min-[29rem]:flex-row">
              <p className="whitespace-pre-wrap first-letter:uppercase">{diag.message}</p>
              <p className="shrink-0 text-fg-subtle">
                In {diag.passIds.length} pass{diag.passIds.length > 1 ? "es" : ""}
              </p>
            </div>
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

export { CommonDiagnostics };
