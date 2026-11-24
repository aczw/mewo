import { Input } from "@base-ui/react";
import { BadgeExclamationPointIcon } from "lucide-react";
import { useState } from "react";

import { MewoPassNameCheck, type Pass } from "../../../mewo/types";
import { useEngine } from "../../engine/context";

const kNameErrorMessages = {
  [MewoPassNameCheck.Empty]: "Name cannot be empty",
  [MewoPassNameCheck.TooLong]: "Name is too long",
  [MewoPassNameCheck.InvalidCharacter]: "Name has invalid characters",
  [MewoPassNameCheck.LeadingOrTrailing]:
    "Name cannot have leading or trailing whitespace or periods",
  [MewoPassNameCheck.Reserved]: "Name cannot use a reserved name",
  [MewoPassNameCheck.Taken]: "Name is already taken",
};

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
    console.log(trimmed);
    if (check === MewoPassNameCheck.Ok && trimmed !== pass.name)
      engine.renamePass(pass.id, trimmed);
    console.log("here");
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
          if (ev.key === "Enter" && check === MewoPassNameCheck.Ok) {
            ev.currentTarget.blur();
          } else if (ev.key === "Escape") {
            onDone();
          }
        }}
        defaultValue={pass.name}
        placeholder={pass.name}
        aria-invalid={check !== MewoPassNameCheck.Ok}
        className="min-w-0 rounded-md bg-surface px-2 py-1 leading-none text-fg-normal placeholder:text-fg-subtle focus:outline-2 focus:outline-accent-light"
      />
      {check !== MewoPassNameCheck.Ok && (
        <div className="flex items-start gap-x-1.5 text-sm">
          <span className="flex h-lh items-center">
            <BadgeExclamationPointIcon size={12} className="shrink-0" />
          </span>
          <p>{kNameErrorMessages[check]}</p>
        </div>
      )}
    </div>
  );
};

export { PassNameInput };
