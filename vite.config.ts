import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// League's Live Client Data API runs on your PC during a game. It uses a self-signed
// certificate and sends no CORS headers, so the browser reaches it through this proxy.
// Set LIVE_CLIENT_URL to point at a mock (see scripts/mock-live-client.ts).
const liveClientProxy = {
  "/liveclientdata": {
    target: process.env.LIVE_CLIENT_URL ?? "https://127.0.0.1:2999",
    secure: false,
    changeOrigin: true,
  },
};

export default defineConfig({
  plugins: [react()],
  server: { proxy: liveClientProxy },
  preview: { proxy: liveClientProxy },
});
