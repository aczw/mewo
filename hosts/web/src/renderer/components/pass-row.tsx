import { useEngine } from "../engine/context";
import type { Pass } from "../engine/engine";

import styles from "./pass-row.module.css";

type Props = {
  pass: Pass;
  active: boolean;
  canDelete: boolean;
};

const PassRow = ({ pass, active, canDelete }: Props) => {
  const engine = useEngine();

  return (
    <li className={styles.passRow} data-active={active ? "true" : "false"}>
      <button className={styles.name} onClick={() => engine.setActivePass(pass.id)}>
        {pass.name}
      </button>
      <button
        className={styles.rename}
        onClick={() => engine.renamePass(pass.id, "incredible new name")}
      >
        Rename
      </button>
      <button
        className={styles.delete}
        disabled={!canDelete}
        onClick={() => engine.removePass(pass.id)}
      >
        Delete
      </button>
    </li>
  );
};

export { PassRow };
