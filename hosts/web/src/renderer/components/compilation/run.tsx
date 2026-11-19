import { Button, Checkbox } from "@base-ui/react";
import { CheckIcon, HammerIcon } from "lucide-react";

import { kIconSize } from "../../scripts/constants";

const Run = () => {
  return (
    <div className="flex items-center gap-x-3">
      <Button onClick={() => alert("run!")} className="btn btn-icon btn-accent">
        <HammerIcon size={kIconSize} />
        Run
      </Button>

      <label className="flex items-center gap-x-1.5 leading-none">
        <Checkbox.Root
          defaultChecked
          className="flex size-4 items-center justify-center rounded-sm border border-border bg-overlay data-checked:border-none data-checked:bg-accent-base"
        >
          <Checkbox.Indicator className="data-unchecked:hidden">
            <CheckIcon size={kIconSize} className="text-surface" />
          </Checkbox.Indicator>
        </Checkbox.Root>
        Auto
      </label>
    </div>
  );
};

export { Run };
