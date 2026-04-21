import { defineConfig, type Plugin } from "vite";
import path from "path";

const bindImguiPath = path.resolve(__dirname, "../build/bind-imgui.js");

const bindImguiPlugin: Plugin = {
  name: "resolve-bind-imgui",
  enforce: "pre",
  resolveId(source) {
    if (source === "bind-imgui" || source === "./bind-imgui") {
      return bindImguiPath;
    }
    if (source.endsWith("/bind-imgui") || source.endsWith("\\bind-imgui")) {
      return bindImguiPath;
    }
  },
};

export default defineConfig({
  plugins: [bindImguiPlugin],
  resolve: {
    alias: {
      "imgui-js": path.resolve(__dirname, "../build/imgui.js"),
    },
  },
  optimizeDeps: {
    exclude: ["bind-imgui"],
  },
  server: {
    open: true,
    fs: {
      allow: [
        path.resolve(__dirname, ".."),
      ],
    },
  },
});
