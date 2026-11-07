import { MewoEvent, MewoResult, type Ptr } from "./types";
import createWasmModule from "./wasm/mewo";

const Module = await createWasmModule();

const Mewo = {
  lastResult: Module.cwrap("mewo_web_last_result", "number", []) as () => MewoResult,
  getVersion: Module.cwrap("mewo_web_get_version", "string", []) as () => string,

  runtimeCreate: Module.cwrap("mewo_web_runtime_create", "number", []) as () => Ptr,
  runtimeDestroy: Module.cwrap("mewo_web_runtime_destroy", null, ["number"]) as (
    runtime: Ptr,
  ) => void,
  runtimeTick: Module.cwrap("mewo_web_runtime_tick", null, ["number"]) as (runtime: Ptr) => void,
  runtimeRequestDevice: Module.cwrap("mewo_web_runtime_request_device", null, [
    "number",
    "number",
  ]) as (runtime: Ptr, surface: Ptr) => void,
  runtimePollEvent: Module.cwrap("mewo_web_runtime_poll_event", "number", ["number"]) as (
    runtime: Ptr,
  ) => MewoEvent,

  eventMessage: Module.cwrap("mewo_web_event_message", "string", []) as () => string,

  surfaceCreateCanvas: Module.cwrap("mewo_web_surface_create_canvas", "number", [
    "number",
    "string",
    "number",
    "number",
  ]) as (runtime: Ptr, selector: string, width: number, height: number) => Ptr,
  surfaceDestroy: Module.cwrap("mewo_web_surface_destroy", null, ["number"]) as (
    surface: Ptr,
  ) => void,
  surfaceResize: Module.cwrap("mewo_web_surface_resize", null, ["number", "number", "number"]) as (
    surface: Ptr,
    width: number,
    height: number,
  ) => void,

  projectCreate: Module.cwrap("mewo_web_project_create", "number", ["string", "string"]) as (
    firstPassName: string,
    firstPassSource: string,
  ) => Ptr,
  projectDestroy: Module.cwrap("mewo_web_project_destroy", null, ["number"]) as (
    project: Ptr,
  ) => void,

  sessionCreate: Module.cwrap("mewo_web_session_create", "number", [
    "number",
    "number",
    "number",
    "number",
  ]) as (runtime: Ptr, project: Ptr, width: number, height: number) => Ptr,
  sessionDestroy: Module.cwrap("mewo_web_session_destroy", null, ["number"]) as (
    session: Ptr,
  ) => void,
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

export { Mewo };
