import { useEngine } from "../engine/context";
import type { Pass } from "../engine/engine";

type Props = {
  pass: Pass;
  active: boolean;
  canDelete: boolean;
};

const PassRow = ({ pass, active, canDelete }: Props) => {
  const engine = useEngine();

  return (
    <li className={active ? "pass-row is-active" : "pass-row"}>
      <button className="pass-row-name" onClick={() => engine.setActivePass(pass.id)}>
        {pass.name}
      </button>
      <button
        className="pass-row-rename"
        onClick={() => engine.renamePass(pass.id, "incredible new name")}
      >
        Rename
      </button>
      <button
        className="pass-row-delete"
        disabled={!canDelete}
        onClick={() => engine.removePass(pass.id)}
      >
        Delete
      </button>
    </li>
  );
};

export { PassRow };
