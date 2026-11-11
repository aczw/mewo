import { SquarePlusIcon } from "lucide-react";

import { useEngine } from "../engine/context";
import { PassList } from "./pass-list";

import styles from "./controller.module.css";

const Controller = () => {
  const engine = useEngine();

  return (
    <div className={styles.controller}>
      <p className={styles.heading}>Passes</p>

      <div className={styles.common}>
        <button onClick={() => alert("add common!")} className={styles.add}>
          <SquarePlusIcon size="var(--icon)" /> Add common
        </button>
      </div>

      <div className={styles.passes}>
        <PassList />

        <button onClick={() => engine.addPass()} className={styles.add}>
          <SquarePlusIcon size="var(--icon)" /> Add pass
        </button>
      </div>
    </div>
  );
};

export { Controller };
