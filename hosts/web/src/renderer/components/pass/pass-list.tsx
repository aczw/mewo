import { useEngineSnapshot } from "../../engine/context";
import { PassRow } from "./pass-row";

const PassList = () => {
  const { passes, selection } = useEngineSnapshot();

  return (
    <ul className="min-h-0 grow overflow-y-auto">
      {passes.map((pass, index) => (
        <PassRow
          key={pass.id}
          index={index}
          pass={pass}
          active={selection.kind === "pass" && pass.id === selection.id}
          canDelete={passes.length > 1}
        />
      ))}
    </ul>
  );
};

export { PassList };
