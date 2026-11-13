import { InfoIcon, OctagonXIcon, TriangleAlertIcon } from "lucide-react";

import { MewoDiagnosticSeverity } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";

import styles from "./diagnostics-count.module.css";

const DiagnosticsCount = () => {
  const { passes, activePassId } = useEngineSnapshot();
  const activePass = passes.find((pass) => pass.id === activePassId);

  let errorCount = "0";
  let warningCount = "0";
  let infoCount = "0";

  if (!activePass) {
    errorCount = "?";
    warningCount = "?";
    infoCount = "?";
  } else {
    const { diagnostics } = activePass;
    errorCount = diagnostics
      .filter((diagnostic) => diagnostic.severity === MewoDiagnosticSeverity.Error)
      .length.toString();
    warningCount = diagnostics
      .filter((diagnostic) => diagnostic.severity === MewoDiagnosticSeverity.Warning)
      .length.toString();
    infoCount = diagnostics
      .filter((diagnostic) => diagnostic.severity === MewoDiagnosticSeverity.Info)
      .length.toString();
  }

  return (
    <ul className={styles.diagnostics}>
      <li className={styles.iconLabel}>
        <OctagonXIcon size="var(--icon)" />
        {errorCount}
      </li>
      <li className={styles.iconLabel}>
        <TriangleAlertIcon size="var(--icon)" />
        {warningCount}
      </li>
      <li className={styles.iconLabel}>
        <InfoIcon size="var(--icon)" />
        {infoCount}
      </li>
    </ul>
  );
};

export { DiagnosticsCount };
