import fs from "node:fs";
import path from "node:path";

import { app, BrowserWindow } from "electron";

const distDir = path.join(import.meta.dirname, "..", "..", "dist");

const reloadOnChange = (win: BrowserWindow) => {
  fs.mkdirSync(distDir, { recursive: true });
  let timer: NodeJS.Timeout | undefined;

  const watcher = fs.watch(distDir, { recursive: true }, () => {
    clearTimeout(timer);
    timer = setTimeout(() => win.webContents.reloadIgnoringCache(), 50);
  });

  win.on("closed", () => watcher.close());
};

const createWindow = () => {
  const win = new BrowserWindow({
    show: false,
    width: 1280,
    height: 720,
    titleBarStyle: process.platform === "darwin" ? "hiddenInset" : "hidden",
    titleBarOverlay: true,
    webPreferences: {
      preload: path.join(distDir, "preload.cjs"),
    },
  });

  win.once("ready-to-show", () => win.show());

  win.loadFile(path.join(distDir, "index.html"));
  if (!app.isPackaged) reloadOnChange(win);
};

app.whenReady().then(() => {
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});
