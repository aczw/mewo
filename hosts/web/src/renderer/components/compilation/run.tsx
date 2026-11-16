import { Button, Checkbox } from "@base-ui/react";
import { CheckIcon, HammerIcon } from "lucide-react";

import { kIconSize } from "../../scripts/constants";

const Run = () => {
  return (
    <div className="flex items-center gap-x-3">
      <Button className="btn btn-icon bg-gray-light hover:bg-white">
        <HammerIcon size={kIconSize} />
        Run
      </Button>

      <label className="flex items-center gap-x-1 leading-none font-medium">
        <Checkbox.Root
          defaultChecked
          className="flex size-4 items-center justify-center rounded-sm border-[1.5px] border-fg-dark data-checked:bg-fg-dark"
        >
          <Checkbox.Indicator className="data-unchecked:hidden">
            <CheckIcon size={kIconSize} strokeWidth={3} className="text-gray-med" />
          </Checkbox.Indicator>
        </Checkbox.Root>
        Auto
      </label>
    </div>
  );
};

export { Run };
