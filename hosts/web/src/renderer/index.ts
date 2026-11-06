import { Mewo, MewoEvent, MewoResult, NULL, type Ptr } from "../wasm/bindings";

type Dimensions = { width: number; height: number };

const kCanvasId = "mewo" as const;

const information = document.getElementById("info")!;
information.innerText = `Chrome (v${versions.chrome()}) / Node.js (v${versions.node()}) / Electron (v${versions.electron()})`;

const Log = {
  info: (...message: string[]) => console.log("[libmewo]", ...message),
  err: (...message: string[]) => console.error("[libmewo]", ...message),
};

const canvasElt = document.getElementById(kCanvasId) as HTMLCanvasElement;

const resultToString = (result: MewoResult): string => {
  switch (result) {
    case MewoResult.Success:
      return "success";
    case MewoResult.NotReady:
      return "not ready";
    case MewoResult.Internal:
      return "internal";
    case MewoResult.InvalidArgument:
      return "invalid argument";
    case MewoResult.InvalidOperation:
      return "invalid operation";
    default:
      return `unknown (${result})`;
  }
};

const getCanvasDimensions = (): Dimensions => ({
  width: Math.round(canvasElt.clientWidth * window.devicePixelRatio),
  height: Math.round(canvasElt.clientHeight * window.devicePixelRatio),
});

const main = () => {
  const runtime = Mewo.runtimeCreate();

  if (runtime === NULL) {
    Log.err("runtime creation failed:", resultToString(Mewo.lastResult()));
    return;
  } else {
    Log.info("created runtime");
  }

  let lastDims = getCanvasDimensions();
  const surface = Mewo.surfaceCreateCanvas(
    runtime,
    `#${kCanvasId}`,
    lastDims.width,
    lastDims.height,
  );

  if (surface === NULL) {
    Log.err("surface init failed:", resultToString(Mewo.lastResult()));
    return;
  } else {
    Log.info("created surface");
  }

  const project = Mewo.projectCreate();

  if (project === NULL) {
    Log.err("project creation failed:", resultToString(Mewo.lastResult()));
    return;
  } else {
    Log.info("created project");
  }

  Mewo.runtimeRequestDevice(runtime, surface);

  let session: Ptr = NULL;

  const onFrame: FrameRequestCallback = (time) => {
    Mewo.runtimeTick(runtime);

    for (let event; (event = Mewo.runtimePollEvent(runtime)) !== MewoEvent.None;) {
      if (event === MewoEvent.DeviceReady) {
        Log.info("device is ready");

        session = Mewo.sessionCreate(runtime, project, lastDims.width, lastDims.height);

        if (session === NULL)
          Log.err("session creation failed:", resultToString(Mewo.lastResult()));
      } else {
        Log.err(Mewo.eventMessage());
      }
    }

    if (session === NULL) {
      window.requestAnimationFrame(onFrame);
      return;
    }

    const dims = getCanvasDimensions();
    if (dims.width !== lastDims.width || dims.height !== lastDims.height) {
      Mewo.surfaceResize(surface, dims.width, dims.height);

      if (dims.width !== 0 && dims.height !== 0) {
        if (!Mewo.sessionSetRenderSize(session, dims.width, dims.height))
          Log.err("failed to set session render size:", resultToString(Mewo.lastResult()));
      }

      lastDims = dims;
    }

    if (!Mewo.sessionRender(session, surface, time))
      Log.err("session failed to render:", resultToString(Mewo.lastResult()));

    window.requestAnimationFrame(onFrame);
  };

  window.requestAnimationFrame(onFrame);
};

main();
