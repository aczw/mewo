import createMewo from "../wasm/mewo";

const mewo = await createMewo();
const getVersion = mewo.cwrap("mewo_web_get_version", "string", []) as () => string;

console.log(`[info] libmewo ${getVersion()}`);

const information = document.getElementById("info")!;
information.innerText = `Chrome (v${versions.chrome()}) / Node.js (v${versions.node()}) / Electron (v${versions.electron()})`;
