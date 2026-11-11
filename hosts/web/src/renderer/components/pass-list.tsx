import { useEngineSnapshot } from "../engine/context";
import { PassRow } from "./pass-row";

import styles from "./pass-list.module.css";

const PassList = () => {
  const { passes, activePass } = useEngineSnapshot();

  return (
    <ul className={styles.passList}>
      {passes.map((pass) => (
        <PassRow
          key={pass.id}
          pass={pass}
          active={pass.id === activePass}
          canDelete={passes.length > 1}
        />
      ))}
    </ul>
  );
};

export { PassList };
