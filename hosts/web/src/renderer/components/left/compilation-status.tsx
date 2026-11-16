import { CheckIcon, HammerIcon, XIcon } from "lucide-react";

import {
  MewoDiagnosticSeverity,
  MewoDiagnosticSource,
  MewoPassCompileStatus,
  type Pass,
} from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";

import styles from "./compilation-status.module.css";

const Pending = () => (
  <p className={styles.compilationStatus}>
    <span className={styles.hiddenWhenSmall}>Compiling...</span>
    <HammerIcon size="var(--icon)" className={styles.hiddenWhenLarge} />
  </p>
);

const SuccessOrFailure = ({ isSuccess, duration }: { isSuccess: boolean; duration: number }) => {
  const compiledText = isSuccess ? "Compiled" : "Failed";
  const compiledIcon = isSuccess ? (
    <CheckIcon size="var(--icon)" className={styles.hiddenWhenLarge} />
  ) : (
    <XIcon size="var(--icon)" className={styles.hiddenWhenLarge} />
  );

  return (
    <p className={styles.compilationStatus}>
      <span className={styles.hiddenWhenSmall}>
        {compiledText} <span className={styles.inText}>in</span>
      </span>
      {compiledIcon}

      <span className={styles.time}>{duration.toFixed(0)}ms</span>
    </p>
  );
};

const CommonCompilationStatus = ({ passes }: { passes: Pass[] }) => {
  const compilations = passes.map((pass) => pass.compilation);
  if (!compilations.every((comp) => comp !== null)) return <p>Waiting...</p>;

  if (!compilations.every((comp) => comp.status !== MewoPassCompileStatus.Pending))
    return <Pending />;

  const commonFailed = passes.some((pass) =>
    pass.diagnostics.some(
      (diag) =>
        diag.source === MewoDiagnosticSource.Common &&
        diag.severity === MewoDiagnosticSeverity.Error,
    ),
  );
  const maxDuration = Math.max(...compilations.map((comp) => comp.duration));

  return <SuccessOrFailure isSuccess={!commonFailed} duration={maxDuration} />;
};

const CompilationStatus = () => {
  const { passes, selection } = useEngineSnapshot();

  if (selection.kind === "common") return <CommonCompilationStatus passes={passes} />;

  const activePass = passes.find((pass) => pass.id === selection.id);
  if (!activePass || activePass.compilation === null) return <p>Waiting...</p>;

  const { compilation } = activePass;
  if (compilation.status === MewoPassCompileStatus.Pending) return <Pending />;

  return (
    <SuccessOrFailure
      isSuccess={compilation.status === MewoPassCompileStatus.Success}
      duration={compilation.duration}
    />
  );
};

export { CompilationStatus };
