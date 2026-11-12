import type { Platform } from "../shared/platform";
import type { Versions } from "../shared/versions";

declare global {
  var versions: Versions;
  var platform: Platform | undefined;
}
