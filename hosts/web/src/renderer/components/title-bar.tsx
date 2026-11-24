import { Button } from "@base-ui/react";
import { MenuIcon } from "lucide-react";

import { useEngineSnapshot } from "../engine/context";
import { Wordmark } from "./logo/wordmark";
import { ProjectButton } from "./project-button";

const TitleBar = () => {
  const { selection, passes } = useEngineSnapshot();

  let sourceName: string;
  if (selection.kind === "pass") {
    let activePassName = "<unknown pass>";
    for (const pass of passes) {
      if (pass.id === selection.id) {
        activePassName = pass.name;
        break;
      }
    }
    sourceName = activePassName;
  } else {
    sourceName = "Common";
  }

  return (
    <header className="flex h-(--env-tb-h) w-full shrink-0 items-center justify-between gap-x-3 border-b border-b-border bg-overlay tb-insets text-fg-normal tb-units [app-region:drag] **:text-(length:--text-base) **:tb-units">
      <nav className="flex min-w-0 items-center gap-x-3 win32:pl-2">
        {window.platform === "win32" && (
          <Button
            onClick={() => alert("menu!")}
            className="btn btn-icon shrink-0 [app-region:no-drag] hover:bg-surface"
          >
            <Wordmark className="h-(--text-base) w-auto text-accent-base" />
            <MenuIcon className="size-(--text-base)" />
          </Button>
        )}

        <ProjectButton />

        <p className="truncate leading-none">{sourceName}</p>
      </nav>

      <div className="flex h-full shrink-0 items-center pr-4 darwin:gap-x-4">
        <p className="leading-none text-fg-subtle">Unsaved</p>
        {window.platform === "darwin" && <Wordmark className="h-1/3 w-auto text-accent-base" />}
      </div>
    </header>
  );
};

export { TitleBar };
