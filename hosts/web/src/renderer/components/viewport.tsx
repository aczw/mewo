import { ExpandIcon, FocusIcon, PauseIcon, PlayIcon, RotateCcwIcon } from "lucide-react";
import { useEffect, useRef } from "react";

import { useEngine, useEngineSnapshot } from "../engine/context";
import { FpsCounter } from "../scripts/fps-counter";

import styles from "./viewport.module.css";

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
    <div className={styles.viewport}>
      <canvas id="mewo" ref={canvasRef}></canvas>

      <div className={styles.info}>
        <p>
          <span ref={timeRef}>0.0</span>
          <span className={styles.unit}>s</span>
        </p>

        <div className={styles.infoRight}>
          <p>
            <span ref={fpsRef}>0.0</span> <span className={styles.unit}>FPS</span>
          </p>
          <p>
            <span ref={widthRef}>0</span>
            <span className={styles.unit}>×</span>
            <span ref={heightRef}>0</span>
          </p>
        </div>
      </div>

      <div className={styles.controls}>
        <div className={styles.controlsGroup}>
          <button onClick={() => engine.setPaused(!paused)} className={styles.playPause}>
            {paused ? (
              <>
                <PlayIcon size="var(--icon)" /> Play
              </>
            ) : (
              <>
                <PauseIcon size="var(--icon)" /> Pause
              </>
            )}
          </button>

          <button className={styles.button}>
            <RotateCcwIcon size="var(--icon)" /> Reset
          </button>
        </div>

        <div className={styles.controlsGroup}>
          <button className={styles.button}>
            <FocusIcon size="var(--icon)" /> Capture
          </button>
          <button className={styles.button}>
            <ExpandIcon size="var(--icon)" /> Fullscreen
          </button>
        </div>
      </div>
    </div>
  );
};

export { Viewport };
