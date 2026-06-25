# Worm Game — Web (WebAssembly)

Browser port of the AK Embedded Base Kit worm game. Game logic lives in `wasm/` as portable C (ported from the STM32 firmware). The browser loads it through WebAssembly; until you build WASM, `js/worm_game.js` provides the same API in JavaScript.

## Run locally

```bash
cd web
python3 -m http.server 8080
```

Open [http://localhost:8080](http://localhost:8080).

## Build WebAssembly (optional)

Requires Emscripten or Docker:

```bash
cd web
./build.sh
```

This replaces `js/worm_game.js` with the Emscripten loader and generates `js/worm_game.wasm`.

Manual build with local `emcc`:

```bash
make -C web/wasm
```

## Controls

| Key | Action |
|-----|--------|
| W / ↑ | Up (turn worm left / menu up) |
| S / ↓ | Down (turn worm right / menu down) |
| A / D / Enter | Mode (enter / back) |
| H | Long mode (settings → menu) |

Settings and high scores persist in `localStorage`.

## Layout

- `wasm/` — portable C game core (framebuffer, gfx, worm, apples, screens)
- `js/main.js` — canvas renderer, input, audio, persistence
- `js/worm_game.js` — WASM loader (or JS fallback before build)
- `index.html` — page shell
