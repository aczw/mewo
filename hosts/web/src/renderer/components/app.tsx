import { Group, Panel } from "react-resizable-panels";

import { Run } from "./compilation/run.tsx";
import { Status } from "./compilation/status.tsx";
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
          <Panel minSize="18rem" className="flex flex-col border-r border-r-border">
            <Editor />

            <div className="flex items-center justify-between p-1.5">
              <Run />
              <Status />
            </div>
          </Panel>

          <Panel minSize="19rem" className="flex flex-col">
            <Viewport />
            <Controller />
          </Panel>
        </Group>
      </main>
    </>
  );
};

export { App };
