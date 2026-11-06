// @ts-ignore
import createMewo from "./wasm/mewo.js";

const information = document.getElementById("info")!;
information.innerText = `Chrome (v${versions.chrome()}) / Node.js (v${versions.node()}) / Electron (v${versions.electron()})`;

await createMewo();
