import { Button } from "@base-ui/react";
import { ArrowDownIcon, ArrowUpIcon, PencilIcon, XIcon } from "lucide-react";

import type { Pass } from "../../mewo/types";
import { useEngine, useEngineSnapshot } from "../engine/context";
import { kIconSize } from "../scripts/constants";

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
    <li
      className="relative flex items-center justify-between border-b border-b-border p-1.5 hover:bg-base data-[active='true']:bg-base"
      data-active={active}
    >
      <div className="flex items-center">
        <p className="px-1.5 text-fg-subtle">{index + 1}</p>

        {active ? (
          <Button className="btn btn-icon group hover:bg-overlay">
            {pass.name} <PencilIcon size={kIconSize} className="hidden group-hover:block" />
          </Button>
        ) : (
          <button
            className="btn after:absolute after:inset-0"
            onClick={() => engine.selectPass(pass.id)}
          >
            {pass.name}
          </button>
        )}
      </div>

      <div className="flex items-center">
        <button
          disabled={index === 0}
          onClick={() => engine.movePass(pass.id, index - 1)}
          className="btn z-1 not-disabled:hover:bg-overlay disabled:text-fg-subtle"
        >
          <ArrowUpIcon size={kIconSize} />
        </button>

        <button
          disabled={index === passes.length - 1}
          onClick={() => engine.movePass(pass.id, index + 1)}
          className="btn z-1 not-disabled:hover:bg-overlay disabled:text-fg-subtle"
        >
          <ArrowDownIcon size={kIconSize} />
        </button>

        <button
          disabled={!canDelete}
          onClick={() => engine.removePass(pass.id)}
          className="btn z-1 not-disabled:hover:bg-overlay disabled:text-fg-subtle"
        >
          <XIcon size={kIconSize} />
        </button>
      </div>
    </li>
  );
};

export { PassRow };
