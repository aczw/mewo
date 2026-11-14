import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import { MewoDiagnosticSeverity, MewoDiagnosticSource } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";

import styles from "./diagnostics-count.module.css";

const DiagnosticsCount = () => {
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
    <ul className={styles.diagnostics}>
      <li className={styles.iconLabel}>
        <OctagonXIcon size="var(--icon)" />
        {count(MewoDiagnosticSeverity.Error)}
      </li>
      <li className={styles.iconLabel}>
        <TriangleAlertIcon size="var(--icon)" />
        {count(MewoDiagnosticSeverity.Warning)}
      </li>
      <li className={styles.iconLabel}>
        <InfoIcon size="var(--icon)" />
        {count(MewoDiagnosticSeverity.Info)}
      </li>
    </ul>
  );
};

export { DiagnosticsCount };
