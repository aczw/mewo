import { Button, Tabs } from "@base-ui/react";
import { AsteriskIcon, ComponentIcon, SquarePlusIcon } from "lucide-react";

import { useEngine, useEngineSnapshot } from "../engine/context";
import { kIconSize } from "../scripts/constants";
import { PassList } from "./pass/pass-list";

const Controller = () => {
  const engine = useEngine();
  const { selection, dirty } = useEngineSnapshot();

  return (
    <Tabs.Root defaultValue="passes" className="flex min-h-0 grow flex-col bg-base">
      <Tabs.List className="relative z-1 flex h-10 shrink-0 items-center gap-x-1.5 border-b border-b-border px-1.5">
        <Tabs.Tab value="passes" className="btn not-data-active:hover:bg-overlay">
          Passes
        </Tabs.Tab>
        <Tabs.Tab value="assets" className="btn not-data-active:hover:bg-overlay">
          Assets
        </Tabs.Tab>

        <Tabs.Indicator className="absolute top-(--active-tab-top) left-0 -z-1 h-[calc(var(--active-tab-height)+var(--active-tab-top)+1px)] w-(--active-tab-width) translate-x-(--active-tab-left) rounded-t-lg border-x border-t border-border bg-surface" />
      </Tabs.List>

      <div className="flex min-h-0 grow flex-col bg-surface">
        <Tabs.Panel value="passes" className="@container flex h-full flex-col">
          <div className="flex shrink-0 items-center justify-between gap-x-1.5 border-b border-b-border p-1.5">
            <Button
              onClick={() => engine.addPass()}
              className="btn btn-icon w-1/2 hover:bg-overlay"
            >
              <SquarePlusIcon size={kIconSize} /> Add new pass
            </Button>

            <Button
              onClick={() => engine.selectCommon()}
              className={() =>
                selection.kind === "common"
                  ? "btn flex w-1/2 justify-between bg-overlay"
                  : "btn flex w-1/2 justify-between bg-base hover:bg-overlay"
              }
            >
              <span className="flex items-center gap-x-1.5">
                <ComponentIcon size={kIconSize} /> Common
              </span>
              {dirty.common && (
                <p className="text-fg-subtle">
                  <span className="hidden @min-[21rem]:inline">Edited</span>
                  <AsteriskIcon size={kIconSize} className="@min-[21rem]:hidden" />
                </p>
              )}
            </Button>
          </div>

          <PassList />
        </Tabs.Panel>
      </div>
    </Tabs.Root>
  );
};

export { Controller };
