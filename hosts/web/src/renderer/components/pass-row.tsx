import { ArrowDownIcon, ArrowUpIcon, XIcon } from "lucide-react";

import { useEngine, useEngineSnapshot } from "../engine/context";
import type { Pass } from "../engine/engine";

import styles from "./pass-row.module.css";

type Props = {
  index: number;
  pass: Pass;
  active: boolean;
  canDelete: boolean;
};

const PassRow = ({ index, pass, active, canDelete }: Props) => {
  const engine = useEngine();
  const { passes } = useEngineSnapshot();

  return (
    <li className={styles.passRow}>
      {active ? (
        <div className={styles.active}>
          <span className={styles.index}>{index}</span>
          {pass.name}
        </div>
      ) : (
        <button onClick={() => engine.setActivePass(pass.id)} className={styles.inactive}>
          <span className={styles.index}>{index}</span>
          {pass.name}
        </button>
      )}
      <button className={styles.buttonIcon} data-disabled={index === 1}>
        <ArrowUpIcon size="var(--icon)" />
      </button>
      <button className={styles.buttonIcon} data-disabled={index === passes.length}>
        <ArrowDownIcon size="var(--icon)" />
      </button>
      <button
        disabled={!canDelete}
        onClick={() => engine.removePass(pass.id)}
        className={styles.buttonIcon}
        data-disabled={canDelete ? "false" : "true"}
      >
        <XIcon size="var(--icon)" />
      </button>
    </li>
  );
};

export { PassRow };
