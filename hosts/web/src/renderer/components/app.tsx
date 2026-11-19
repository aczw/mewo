import { Group, Panel } from "react-resizable-panels";

import { Compilation } from "./compilation.tsx";
import { Controller } from "./controller";
import { Editor } from "./editor";
import { TitleBar } from "./title-bar";
import { Viewport } from "./viewport";

const App = () => {
  return (
    <>
      <TitleBar />

      <main className="flex min-h-0 grow">
        <Group>
          <Panel className="flex flex-col border-r border-r-border">
            <Editor />
            <Compilation />
          </Panel>

          <Panel className="flex flex-col">
            <Viewport />
            <Controller />
          </Panel>
        </Group>
      </main>
    </>
  );
};

export { App };
