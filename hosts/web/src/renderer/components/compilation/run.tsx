import { Button, Checkbox } from "@base-ui/react";
import { CheckIcon, HammerIcon } from "lucide-react";

import { useEngine, useEngineSnapshot } from "../../engine/context";
import { kIconSize } from "../../scripts/constants";

const Run = () => {
  const engine = useEngine();
  const { autoCompile, dirty } = useEngineSnapshot();

  const isDirty = dirty.passes.size !== 0 || dirty.common;

  return (
    <div className="flex items-center gap-x-3">
      <Button
        disabled={!isDirty || autoCompile}
        onClick={() => engine.compile()}
        className="btn btn-icon btn-accent data-disabled:opacity-50"
      >
        <HammerIcon size={kIconSize} />
        Run
      </Button>

      <label className="flex items-center gap-x-1.5 leading-none">
        <Checkbox.Root
          checked={autoCompile}
          onCheckedChange={(checked) => engine.setAutoCompile(checked)}
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
