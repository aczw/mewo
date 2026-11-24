import { Button } from "@base-ui/react";
import { ArrowDownIcon, ArrowUpIcon, PencilIcon, XIcon } from "lucide-react";
import { useState } from "react";

import type { Pass } from "../../../mewo/types";
import { useEngine, useEngineSnapshot } from "../../engine/context";
import { kIconSize } from "../../scripts/constants";
import { PassNameInput } from "./pass-name-input";

type Props = {
  index: number;
  pass: Pass;
  active: boolean;
  canDelete: boolean;
};

const PassRow = ({ index, pass, active, canDelete }: Props) => {
  const engine = useEngine();
  const { passes, dirty } = useEngineSnapshot();

  const [isEditingName, setIsEditingName] = useState(false);

  const isDirty = dirty.passes.has(pass.id);

  return (
    <li
      className="relative flex items-start justify-between gap-x-1.5 border-b border-b-border px-1.5 py-2.5 hover:bg-base data-[active='true']:bg-base"
      data-active={active}
    >
      <div className="flex min-w-0 items-start">
        <p className="shrink-0 p-1.5 leading-none text-fg-subtle">{index + 1}</p>

        {active ? (
          isEditingName ? (
            <PassNameInput pass={pass} onDone={() => setIsEditingName(false)} />
          ) : (
            <Button
              className="btn btn-icon group min-w-0 hover:bg-overlay"
              onClick={() => setIsEditingName(true)}
            >
              <span className="-my-0.5 truncate py-0.5">{pass.name}</span>
              <PencilIcon size={kIconSize} className="hidden shrink-0 group-hover:block" />
            </Button>
          )
        ) : (
          <button
            className="btn after:absolute after:inset-0"
            onClick={() => engine.selectPass(pass.id)}
          >
            {pass.name}
          </button>
        )}
      </div>

      <div className="flex shrink-0 items-center">
        {isDirty && <p className="pr-2 text-fg-subtle">Edited</p>}

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
