import { Toolbar } from "@base-ui/react";
import { ExpandIcon, FocusIcon, PauseIcon, PlayIcon, RotateCcwIcon } from "lucide-react";
import { useEffect, useRef } from "react";

import { useEngine, useEngineSnapshot } from "../engine/context";
import { kIconSize } from "../scripts/constants";
import { FpsCounter } from "../scripts/fps-counter";

const Viewport = () => {
  const engine = useEngine();
  const { paused } = useEngineSnapshot();

  const canvasRef = useRef<HTMLCanvasElement>(null);

  const timeRef = useRef<HTMLSpanElement>(null);
  const fpsRef = useRef<HTMLSpanElement>(null);
  const widthRef = useRef<HTMLSpanElement>(null);
  const heightRef = useRef<HTMLSpanElement>(null);

  useEffect(() => engine.createSurfaceAndAttachCanvas(canvasRef.current!), [engine]);

  useEffect(() => {
    const fpsCounter = new FpsCounter();
    return engine.subscribeFrame((stats) => {
      fpsCounter.push(stats.deltaTime);

      timeRef.current!.textContent = (stats.sessionTime * 0.001).toFixed(2);
      fpsRef.current!.textContent = fpsCounter.average.toFixed(1);

      widthRef.current!.textContent = stats.dimensions.width.toString();
      heightRef.current!.textContent = stats.dimensions.height.toString();
    });
  }, [engine]);

  return (
    <div className="flex flex-col py-1.5 pr-1.5 pl-0.75">
      <Toolbar.Root className="flex justify-between rounded-t-lg bg-gray-light p-1.5">
        <Toolbar.Button className="btn btn-icon hover:bg-gray-med">
          <FocusIcon size={kIconSize} />
          Capture
        </Toolbar.Button>
        <Toolbar.Button className="btn hover:bg-gray-med">Viewport</Toolbar.Button>
      </Toolbar.Root>

      <canvas id="mewo" ref={canvasRef} className="aspect-video h-auto w-full"></canvas>

      <div className="flex justify-between bg-gray-dark p-1.5">
        <p className="leading-none">
          <span ref={timeRef}>0.0</span>
          <span className="text-fg-light">s</span>
        </p>

        <div className="flex gap-x-3 *:leading-none">
          <p>
            <span ref={fpsRef}>0.0</span> <span className="text-fg-light">FPS</span>
          </p>
          <p>
            <span ref={widthRef}>0</span>
            <span className="text-fg-light">×</span>
            <span ref={heightRef}>0</span>
          </p>
        </div>
      </div>

      <Toolbar.Root className="flex justify-between rounded-b-lg bg-gray-light p-1.5">
        <Toolbar.Group className="flex">
          <Toolbar.Button className="btn btn-icon hover:bg-gray-med">
            <RotateCcwIcon size={kIconSize} />
          </Toolbar.Button>

          <Toolbar.Button
            className="btn btn-icon w-21 hover:bg-gray-med"
            onClick={() => engine.setPaused(!paused)}
          >
            {paused ? (
              <>
                <PlayIcon size={kIconSize} />
                Play
              </>
            ) : (
              <>
                <PauseIcon size={kIconSize} />
                Pause
              </>
            )}
          </Toolbar.Button>
        </Toolbar.Group>

        <Toolbar.Button className="btn btn-icon hover:bg-gray-med">
          <ExpandIcon size={kIconSize} />
          Fullscreen
        </Toolbar.Button>
      </Toolbar.Root>
    </div>
  );
};

export { Viewport };
