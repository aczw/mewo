import { Group, Panel } from "react-resizable-panels";

import { Controller } from "./controller";
import { Editor } from "./editor/editor";
import { TitleBar } from "./title-bar";
import { Viewport } from "./viewport";

const App = () => {
  return (
    <>
      <TitleBar />

      <main className="flex min-h-0 grow">
        <Group>
          <Panel className="flex flex-col">
            <Editor />
          </Panel>

          <Panel className="flex flex-col overflow-y-auto">
            <Viewport />
            <Controller />
          </Panel>
        </Group>
      </main>
    </>
  );
};

export { App };
