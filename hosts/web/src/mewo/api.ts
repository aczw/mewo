import { MewoEvent, MewoResult, type PassId, type Ptr } from "./types";
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
  projectSetPassSource: Module.cwrap("mewo_web_project_set_pass_source", "boolean", [
    "number",
    "number",
    "string",
  ]) as (project: Ptr, passId: PassId, source: string) => boolean,
  projectSetPassName: Module.cwrap("mewo_web_project_set_pass_name", "boolean", [
    "number",
    "number",
    "string",
  ]) as (project: Ptr, passId: PassId, name: string) => boolean,
  projectSetCommonSource: Module.cwrap("mewo_web_project_set_common_source", "boolean", [
    "number",
    "string",
  ]) as (project: Ptr, source: string) => boolean,
  projectAddPass: Module.cwrap("mewo_web_project_add_pass", "number", [
    "number",
    "string",
    "string",
  ]) as (project: Ptr, name: string, source: string) => PassId,
  projectRemovePass: Module.cwrap("mewo_web_project_remove_pass", "boolean", [
    "number",
    "number",
  ]) as (project: Ptr, passId: PassId) => boolean,
  projectMovePass: Module.cwrap("mewo_web_project_move_pass", "boolean", [
    "number",
    "number",
    "number",
  ]) as (project: Ptr, passId: PassId, newIndex: number) => boolean,
  projectGetPassCount: Module.cwrap("mewo_web_project_get_pass_count", "number", ["number"]) as (
    project: Ptr,
  ) => number,
  projectGetPassId: Module.cwrap("mewo_web_project_get_pass_id", "number", [
    "number",
    "number",
  ]) as (project: Ptr, index: number) => PassId,
  projectGetPassName: Module.cwrap("mewo_web_project_get_pass_name", "string", [
    "number",
    "number",
  ]) as (project: Ptr, passId: PassId) => string,
  projectGetPassSource: Module.cwrap("mewo_web_project_get_pass_source", "string", [
    "number",
    "number",
  ]) as (project: Ptr, passId: PassId) => string,
  projectGetCommonSource: Module.cwrap("mewo_web_project_get_common_source", "string", [
    "number",
  ]) as (project: Ptr) => string,

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
  sessionGetTime: Module.cwrap("mewo_web_session_get_time", "number", ["number"]) as (
    session: Ptr,
  ) => number,
  // `cwrap` has no "boolean" argument type, so we use a number instead
  sessionSetPaused: Module.cwrap("mewo_web_session_set_paused", null, ["number", "number"]) as (
    session: Ptr,
    paused: boolean,
  ) => void,
  sessionIsPaused: Module.cwrap("mewo_web_session_is_paused", "boolean", ["number"]) as (
    session: Ptr,
  ) => boolean,
  // Milliseconds, or -1 (pending or unknown pass ID, call `lastResult()` to find out)
  sessionGetPassCompileDuration: Module.cwrap(
    "mewo_web_session_get_pass_compile_duration",
    "number",
    ["number", "number"],
  ) as (session: Ptr, passId: PassId) => number,
};

export { Mewo };
