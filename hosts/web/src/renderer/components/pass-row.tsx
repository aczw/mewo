import { Button, Input } from "@base-ui/react";
import { ArrowDownIcon, ArrowUpIcon, PencilIcon, XIcon } from "lucide-react";
import { useState } from "react";

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
  const { passes, dirty } = useEngineSnapshot();

  const [isEditingName, setIsEditingName] = useState(false);

  const isDirty = dirty.passes.has(pass.id);

  return (
    <li
      className="relative flex items-center justify-between gap-x-1.5 border-b border-b-border p-1.5 hover:bg-base data-[active='true']:bg-base"
      data-active={active}
    >
      <div className="flex min-w-0 items-center">
        <p className="shrink-0 px-1.5 text-fg-subtle">{index + 1}</p>

        {active ? (
          isEditingName ? (
            <Input
              autoFocus
              onFocus={(ev) => ev.currentTarget.select()}
              onBlur={(ev) => {
                const trimmed = ev.currentTarget.value.trim();
                if (trimmed.length > 0) engine.renamePass(pass.id, trimmed);
                setIsEditingName(false);
              }}
              onKeyDown={(ev) => {
                if (ev.key === "Enter") {
                  ev.currentTarget.blur();
                } else if (ev.key === "Escape") {
                  ev.currentTarget.value = pass.name;
                  ev.currentTarget.blur();
                }
              }}
              defaultValue={pass.name}
              placeholder={pass.name}
              className="min-w-0 rounded-md bg-surface px-2 py-0.75 leading-none text-fg-normal placeholder:text-fg-subtle focus:outline-2 focus:outline-accent-light"
            />
          ) : (
            <Button
              className="btn btn-icon group min-w-0 hover:bg-overlay"
              onClick={() => setIsEditingName(true)}
            >
              <span className="truncate">{pass.name}</span>
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
