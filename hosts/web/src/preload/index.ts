import { contextBridge } from "electron";

import type { Versions } from "../shared/versions";

const versions: Versions = {
  node: () => process.versions.node,
  chrome: () => process.versions.chrome,
  electron: () => process.versions.electron,
};

contextBridge.exposeInMainWorld("versions", versions);
