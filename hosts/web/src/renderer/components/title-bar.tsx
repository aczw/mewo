import { Button } from "@base-ui/react";
import { ChevronDown } from "lucide-react";

import { useEngineSnapshot } from "../engine/context";
import { Wordmark } from "./logo/wordmark";

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
    <header className="flex h-(--env-tb-h) w-full items-center justify-between bg-gray-dark tb-insets text-base tb-units [app-region:drag]">
      <nav className="flex items-center gap-x-2">
        <Button
          onClick={() => alert("project!")}
          className="btn btn-icon bg-gray-light [app-region:no-drag] hover:bg-white"
        >
          <h1>Untitled</h1>
          <ChevronDown className="size-[calc(var(--tb-px)*15)]" />
        </Button>

        <p className="leading-none">{sourceName}</p>
      </nav>

      <div className="flex h-full items-center gap-x-4 pr-4">
        <p className="leading-none text-fg-light">Unsaved</p>
        <Wordmark className="h-1/3 w-auto" />
      </div>
    </header>
  );
};

export { TitleBar };
