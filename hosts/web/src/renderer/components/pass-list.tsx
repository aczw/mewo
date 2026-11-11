import { useEngineSnapshot } from "../engine/context";
import { PassRow } from "./pass-row";

import styles from "./pass-list.module.css";

const PassList = () => {
  const { passes, activePass } = useEngineSnapshot();

  return (
    <ul className={styles.passList}>
      {passes.map((pass, index) => (
        <PassRow
          key={pass.id}
          index={index + 1}
          pass={pass}
          active={pass.id === activePass}
          canDelete={passes.length > 1}
        />
      ))}
    </ul>
  );
};

export { PassList };
