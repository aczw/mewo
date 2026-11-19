import { Toolbar } from "@base-ui/react";
import {
  ExpandIcon,
  FocusIcon,
  PauseIcon,
  PlayIcon,
  ProportionsIcon,
  RotateCcwIcon,
} from "lucide-react";
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
    <div className="@container flex flex-col border-b border-b-border">
      <Toolbar.Root className="flex justify-between border-b border-b-border bg-base p-1.5">
        <Toolbar.Button className="btn btn-icon hover:bg-overlay">
          <FocusIcon size={kIconSize} />
          Capture
        </Toolbar.Button>

        <Toolbar.Group className="flex items-center gap-x-4 *:leading-none">
          <p className="@max-[24.5rem]:hidden">
            <span ref={fpsRef}>0.0</span> <span className="text-fg-subtle">FPS</span>
          </p>
          <p>
            <span ref={widthRef}>0</span>
            <span className="text-fg-subtle">×</span>
            <span ref={heightRef}>0</span>
          </p>

          <Toolbar.Button className="btn btn-icon hover:bg-overlay">
            <ProportionsIcon size={kIconSize} />
            Viewport
          </Toolbar.Button>
        </Toolbar.Group>
      </Toolbar.Root>

      <canvas
        id="mewo"
        ref={canvasRef}
        className="aspect-video h-auto w-full border-b border-b-border"
      ></canvas>

      <Toolbar.Root className="bg-gray-light flex justify-between rounded-b-lg p-1.5">
        <Toolbar.Group className="flex items-center gap-x-2">
          <div className="flex items-center gap-x-1">
            <Toolbar.Button
              className="btn btn-icon btn-accent w-22"
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

            <Toolbar.Button className="btn hover:bg-overlay" onClick={() => engine.reset()}>
              <RotateCcwIcon size={kIconSize} />
            </Toolbar.Button>
          </div>

          <p className="leading-none">
            <span ref={timeRef}>0.0</span>
            <span className="text-fg-subtle">s</span>
          </p>
        </Toolbar.Group>

        <Toolbar.Button className="btn btn-icon hover:bg-overlay">
          <ExpandIcon size={kIconSize} />
          Fullscreen
        </Toolbar.Button>
      </Toolbar.Root>
    </div>
  );
};

export { Viewport };
