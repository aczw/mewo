import { Menu } from "@base-ui/react";
import { ChevronDown } from "lucide-react";
import { useState } from "react";

import { useEngineSnapshot } from "../engine/context";
import { ProjectNameInput } from "./project-name-input";

const ProjectButton = () => {
  const { project } = useEngineSnapshot();

  const [renaming, setRenaming] = useState(false);

  if (renaming) {
    return <ProjectNameInput onDone={() => setRenaming(false)} />;
  } else {
    return (
      <Menu.Root>
        <Menu.Trigger className="btn btn-icon btn-accent max-w-50 shrink-0 [app-region:no-drag]">
          <span className="-my-0.5 min-w-0 truncate py-0.5">{project.name}</span>
          <ChevronDown className="size-(--text-base) shrink-0" />
        </Menu.Trigger>

        <Menu.Portal>
          <Menu.Positioner sideOffset={4} align="start">
            <Menu.Popup className="rounded-lg border border-border bg-surface p-1.5 shadow-lg *:leading-none">
              <Menu.Item
                className="rounded-md px-2 py-1.5 data-highlighted:bg-overlay"
                onClick={() => setRenaming(true)}
              >
                Rename project…
              </Menu.Item>
              <Menu.Separator className="my-1.5 h-px bg-border" />
              <Menu.Item className="rounded-md px-2 py-1.5 data-highlighted:bg-overlay">
                Save
              </Menu.Item>
              <Menu.Item className="rounded-md px-2 py-1.5 data-highlighted:bg-overlay">
                Save as…
              </Menu.Item>
            </Menu.Popup>
          </Menu.Positioner>
        </Menu.Portal>
      </Menu.Root>
    );
  }
};

export { ProjectButton };
