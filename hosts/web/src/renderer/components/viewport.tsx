import { useEffect, useRef } from "react";

import { useEngine, useEngineSnapshot } from "../engine/context";
import { FpsCounter } from "../scripts/fps-counter";

import styles from "./viewport.module.css";

const Viewport = () => {
  const engine = useEngine();
  const { paused } = useEngineSnapshot();

  const canvasRef = useRef<HTMLCanvasElement>(null);
  const timeRef = useRef<HTMLParagraphElement>(null);
  const dimsRef = useRef<HTMLParagraphElement>(null);
  const fpsRef = useRef<HTMLParagraphElement>(null);

  useEffect(() => engine.createSurfaceAndAttachCanvas(canvasRef.current!), [engine]);

  useEffect(() => {
    const fpsCounter = new FpsCounter();
    return engine.subscribeFrame((stats) => {
      timeRef.current!.textContent = `${(stats.sessionTime * 0.001).toFixed(2)}s`;
      dimsRef.current!.textContent = `${stats.dimensions.width}×${stats.dimensions.height}`;
      fpsCounter.push(stats.deltaTime);
      fpsRef.current!.textContent = `${fpsCounter.average.toFixed(1)} FPS`;
    });
  }, [engine]);

  return (
    <div className={styles.viewport}>
      <canvas id="mewo" ref={canvasRef}></canvas>

      <div>
        <button onClick={() => engine.setPaused(!paused)}>{paused ? "Resume" : "Pause"}</button>
      </div>

      <div className={styles.info}>
        <p ref={timeRef}></p>

        <div className={styles.right}>
          <p ref={dimsRef}></p>
          <p ref={fpsRef}></p>
        </div>
      </div>
    </div>
  );
};

export { Viewport };
