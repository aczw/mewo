import createWasmModule from "./mewo";

const Module = await createWasmModule();

type Ptr = number;

type PtrFn = () => Ptr;
type StringFn = () => string;

const NULL = 0 as const;

const MewoEvent = { None: 0, DeviceReady: 1, DeviceFailed: 2, DeviceLost: 3, GpuError: 4 } as const;
type MewoEvent = (typeof MewoEvent)[keyof typeof MewoEvent];

const MewoResult = {
  Success: 0,
  NotReady: -1,
  Internal: -2,
  InvalidArgument: -3,
  InvalidOperation: -4,
} as const;
type MewoResult = (typeof MewoResult)[keyof typeof MewoResult];

const Mewo = {
  lastResult: Module.cwrap("mewo_web_last_result", "number", []) as () => MewoResult,
  getVersion: Module.cwrap("mewo_web_get_version", "string", []) as StringFn,

  runtimeCreate: Module.cwrap("mewo_web_runtime_create", "number", []) as PtrFn,
  runtimeTick: Module.cwrap("mewo_web_runtime_tick", null, ["number"]) as (runtime: Ptr) => void,
  runtimeRequestDevice: Module.cwrap("mewo_web_runtime_request_device", null, [
    "number",
    "number",
  ]) as (runtime: Ptr, surface: Ptr) => void,
  runtimePollEvent: Module.cwrap("mewo_web_runtime_poll_event", "number", ["number"]) as (
    runtime: Ptr,
  ) => MewoEvent,

  eventMessage: Module.cwrap("mewo_web_event_message", "string", []) as StringFn,

  surfaceCreateCanvas: Module.cwrap("mewo_web_surface_create_canvas", "number", [
    "number",
    "string",
    "number",
    "number",
  ]) as (runtime: Ptr, selector: string, width: number, height: number) => Ptr,
  surfaceResize: Module.cwrap("mewo_web_surface_resize", null, ["number", "number", "number"]) as (
    surface: Ptr,
    width: number,
    height: number,
  ) => void,

  projectCreate: Module.cwrap("mewo_web_project_create", "number", []) as PtrFn,

  sessionCreate: Module.cwrap("mewo_web_session_create", "number", [
    "number",
    "number",
    "number",
    "number",
  ]) as (runtime: Ptr, project: Ptr, width: number, height: number) => Ptr,
  sessionSetRenderSize: Module.cwrap("mewo_web_session_set_render_size", "boolean", [
    "number",
    "number",
    "number",
  ]) as (session: Ptr, width: number, height: number) => boolean,
  sessionRender: Module.cwrap("mewo_web_session_render", "boolean", [
    "number",
    "number",
    "number",
  ]) as (session: Ptr, surface: Ptr, timeMs: number) => boolean,
};

export { Mewo, MewoResult, MewoEvent, NULL, type Ptr };
