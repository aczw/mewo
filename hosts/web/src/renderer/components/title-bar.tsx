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
          className="flex items-center gap-x-1 rounded-full bg-gray-light py-0.5 pr-1.5 pl-2.5 font-medium [app-region:no-drag] hover:bg-gray-med"
        >
          <h1>Untitled</h1>
          <ChevronDown className="size-4" />
        </Button>

        <p>{sourceName}</p>
      </nav>

      <div className="flex h-full items-center gap-x-4 pr-4">
        <p className="text-fg-light">Unsaved</p>
        <Wordmark className="h-1/3 w-auto" />
      </div>
    </header>
  );
};

export { TitleBar };
