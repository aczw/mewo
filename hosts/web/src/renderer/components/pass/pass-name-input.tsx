import { Input } from "@base-ui/react";
import { BadgeExclamationPointIcon } from "lucide-react";
import { useState } from "react";

import { MewoNameCheck, type Pass } from "../../../mewo/types";
import { useEngine } from "../../engine/context";
import { kIconSize, kNameErrorMessages } from "../../scripts/constants";

type Props = {
  pass: Pass;
  onDone: () => void;
};

const PassNameInput = ({ pass, onDone }: Props) => {
  const engine = useEngine();
  const [draftName, setDraftName] = useState(pass.name);

  const trimmed = draftName.trim();
  const check = engine.checkPassName(trimmed, pass.id);

  const commitName = () => {
    if (check === MewoNameCheck.Ok && trimmed !== pass.name) engine.renamePass(pass.id, trimmed);
    onDone();
  };

  return (
    <div className="space-y-1.5">
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
        placeholder={pass.name}
        aria-invalid={check !== MewoNameCheck.Ok}
        className="min-w-0 rounded-md bg-surface px-2 py-1 leading-none text-fg-normal placeholder:text-fg-subtle focus:outline-2 focus:outline-accent-light"
      />
      {check !== MewoNameCheck.Ok && (
        <div className="flex items-start gap-x-1.5 text-sm">
          <span className="flex h-lh items-center">
            <BadgeExclamationPointIcon size={0.875 * kIconSize} className="shrink-0" />
          </span>
          <p>{kNameErrorMessages[check]}</p>
        </div>
      )}
    </div>
  );
};

export { PassNameInput };
