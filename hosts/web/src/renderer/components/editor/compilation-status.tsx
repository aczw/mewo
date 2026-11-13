import { CheckIcon, HammerIcon, XIcon } from "lucide-react";

import { MewoPassCompileStatus } from "../../../mewo/types";
import { useEngineSnapshot } from "../../engine/context";

import styles from "./compilation-status.module.css";

const CompilationStatus = () => {
  const { passes, activePassId } = useEngineSnapshot();

  let activePass = passes.find((pass) => pass.id === activePassId);

  if (!activePass || activePass.compilation === null) return <p>Waiting...</p>;

  const { compilation } = activePass;
  const { status } = compilation;
  if (compilation.status === MewoPassCompileStatus.Pending) {
    return (
      <p className={styles.compilationStatus}>
        <span className={styles.hiddenWhenSmall}>Compiling...</span>
        <HammerIcon size="var(--icon)" className={styles.hiddenWhenLarge} />
      </p>
    );
  }

  const compiledText = status === MewoPassCompileStatus.Success ? "Compiled" : "Failed";
  const compiledIcon =
    status === MewoPassCompileStatus.Success ? (
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

      <span className={styles.time}>{compilation.duration.toFixed(0)}ms</span>
    </p>
  );
};

export { CompilationStatus };
