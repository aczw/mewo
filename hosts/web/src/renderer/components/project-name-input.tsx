import { Input, Popover } from "@base-ui/react";
import { BadgeExclamationPoint } from "lucide-react";
import { useRef, useState } from "react";

import { MewoNameCheck } from "../../mewo/types";
import { useEngine, useEngineSnapshot } from "../engine/context";
import { kNameErrorMessages } from "../scripts/constants";

type Props = { onDone: () => void };

const ProjectNameInput = ({ onDone }: Props) => {
  const engine = useEngine();
  const { project } = useEngineSnapshot();

  const inputRef = useRef<HTMLInputElement>(null);
  const [draftName, setDraftName] = useState(project.name);

  const trimmed = draftName.trim();
  const check = engine.checkProjectName(trimmed);
  const message = check === MewoNameCheck.Ok ? null : kNameErrorMessages[check];

  const commitName = () => {
    if (check === MewoNameCheck.Ok && trimmed !== project.name) engine.setProjectName(trimmed);
    onDone();
  };

  return (
    <>
      <Input
        autoFocus
        value={draftName}
        onFocus={(ev) => ev.currentTarget.select()}
        onChange={(ev) => setDraftName(ev.currentTarget.value)}
        onBlur={commitName}
        onKeyDown={(ev) => {
          if (ev.key === "Enter" && check === MewoNameCheck.Ok) {
            ev.currentTarget.blur();
          } else if (ev.key === "Escape") {
            onDone();
          }
        }}
        placeholder={project.name}
        aria-invalid={message !== null}
        className="max-w-50 min-w-0 rounded-[calc(var(--tb-px)*5.25)] bg-surface px-2 py-1 leading-none text-fg-normal [app-region:no-drag] placeholder:text-fg-subtle focus:outline-[calc(var(--tb-px)*2)] focus:outline-accent-light"
        ref={inputRef}
      />

      <Popover.Root open={message !== null}>
        <Popover.Portal>
          <Popover.Positioner anchor={inputRef} side="bottom" sideOffset={4} align="start">
            <Popover.Popup
              initialFocus={false}
              finalFocus={false}
              onMouseDown={(ev) => ev.preventDefault()}
              className="rounded-lg border border-border bg-surface p-1.5 shadow-md *:leading-none"
            >
              <Popover.Description className="flex gap-x-1.5">
                <BadgeExclamationPoint className="size-(--text-base) h-lh shrink-0" />
                {message}
              </Popover.Description>
            </Popover.Popup>
          </Popover.Positioner>
        </Popover.Portal>
      </Popover.Root>
    </>
  );
};

export { ProjectNameInput };
