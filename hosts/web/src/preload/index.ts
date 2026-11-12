import { contextBridge } from "electron";

import type { Platform } from "../shared/platform";
import type { Versions } from "../shared/versions";

const versions: Versions = {
  node: () => process.versions.node,
  chrome: () => process.versions.chrome,
  electron: () => process.versions.electron,
};

const isPlatform = (platform: NodeJS.Platform): platform is Platform =>
  platform === "darwin" || platform === "win32" || platform === "linux";

contextBridge.exposeInMainWorld("versions", versions);
contextBridge.exposeInMainWorld(
  "platform",
  isPlatform(process.platform) ? process.platform : undefined,
);
