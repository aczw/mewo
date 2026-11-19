import { Button } from "@base-ui/react";
import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import { MewoDiagnosticSeverity, MewoDiagnosticSource } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";
import { kIconSize } from "../../scripts/constants";

type Props = {
  showDiagnostics: boolean;
  setShowDiagnostics: (show: boolean) => void;
};

const DiagnosticsCount = ({ showDiagnostics, setShowDiagnostics }: Props) => {
  const { passes, selection, commonDiagnostics } = useEngineSnapshot();

  const diagnostics =
    selection.kind === "common"
      ? commonDiagnostics
      : passes
          .find((pass) => pass.id === selection.id)
          ?.diagnostics.filter((diag) => diag.source !== MewoDiagnosticSource.Common);

  const count = (severity: MewoDiagnosticSeverity) =>
    diagnostics?.filter((diag) => diag.severity === severity).length.toString() ?? "?";

  return (
    <Button
      className={() => (showDiagnostics ? "btn bg-overlay" : "btn hover:bg-overlay")}
      onClick={() => setShowDiagnostics(!showDiagnostics)}
    >
      <ul className="flex gap-x-2 *:flex *:gap-x-1">
        <li>
          <OctagonXIcon size={kIconSize} />
          {count(MewoDiagnosticSeverity.Error)}
        </li>
        <li>
          <TriangleAlertIcon size={kIconSize} />
          {count(MewoDiagnosticSeverity.Warning)}
        </li>
        <li>
          <InfoIcon size={kIconSize} />
          {count(MewoDiagnosticSeverity.Info)}
        </li>
      </ul>
    </Button>
  );
};

export { DiagnosticsCount };
