import { Tabs } from "@base-ui/react";
import type { Ref } from "react";

type Tab = "uniforms" | "snippets" | "appearance" | null;

type Props = {
  tab: Tab;
  onTabChange: (tab: Tab) => void;
  contentRef: Ref<HTMLDivElement>;
};

const Toolbar = ({ tab, onTabChange, contentRef }: Props) => {
  return (
    <Tabs.Root
      value={tab}
      onValueChange={onTabChange}
      className="h-full border-b border-b-border bg-base"
    >
      <Tabs.List className="relative z-1 flex h-[36px] items-center justify-between border-b border-b-border px-1.5">
        <div className="flex items-center gap-x-1.5">
          <Tabs.Tab
            className="btn not-data-active:hover:bg-overlay"
            value="uniforms"
            onClick={() => {
              if (tab === "uniforms") onTabChange(null);
            }}
          >
            Uniforms
          </Tabs.Tab>
          <Tabs.Tab
            className="btn not-data-active:hover:bg-overlay"
            value="snippets"
            onClick={() => {
              if (tab === "snippets") onTabChange(null);
            }}
          >
            Snippets
          </Tabs.Tab>
        </div>

        <Tabs.Tab
          className="btn not-data-active:hover:bg-overlay"
          value="appearance"
          onClick={() => {
            if (tab === "appearance") onTabChange(null);
          }}
        >
          Appearance
        </Tabs.Tab>

        <Tabs.Indicator className="absolute top-(--active-tab-top) left-0 -z-1 h-[calc(var(--active-tab-height)+var(--active-tab-top)+1px)] w-(--active-tab-width) translate-x-(--active-tab-left) rounded-t-lg border-x border-t border-border bg-surface" />
      </Tabs.List>

      <div className="bg-surface" ref={contentRef}>
        <Tabs.Panel value="uniforms" className="data-ending-style:hidden">
          Uniforms here
        </Tabs.Panel>
        <Tabs.Panel value="snippets" className="data-ending-style:hidden">
          Snippets here
        </Tabs.Panel>
        <Tabs.Panel value="appearance" className="data-ending-style:hidden">
          Appearance here
        </Tabs.Panel>
      </div>
    </Tabs.Root>
  );
};

export { Toolbar, type Tab as ToolbarTab };
