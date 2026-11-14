import { useEngineSnapshot } from "../engine/context";
import { PassRow } from "./pass-row";

import styles from "./pass-list.module.css";

const PassList = () => {
  const { passes, selection } = useEngineSnapshot();

  return (
    <ul className={styles.passList}>
      {passes.map((pass, index) => (
        <PassRow
          key={pass.id}
          index={index + 1}
          pass={pass}
          active={selection.kind === "pass" && pass.id === selection.id}
          canDelete={passes.length > 1}
        />
      ))}
    </ul>
  );
};

export { PassList };
