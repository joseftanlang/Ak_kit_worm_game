/**
 * Worm Game Web host — loads WASM core and drives the 128×64 canvas.
 */

const createWormGame = globalThis.createWormGame;

const SCR_WIDTH = 128;
const SCR_HEIGHT = 64;
const SCALE = 4;
const STORAGE_KEY = 'worm_game_v1';

const canvas = document.getElementById('screen');
const ctx = canvas.getContext('2d');
const statusEl = document.getElementById('status');

const imageData = ctx.createImageData(SCR_WIDTH * SCALE, SCR_HEIGHT * SCALE);

let module = null;
let lastTime = 0;
let settingsDirty = false;
let scoresDirty = false;

function loadPersisted() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      return {
        top: [0, 0, 0],
        speed: 3,
        apples: 1,
        song: 0,
        buzzer: 0,
      };
    }
    const data = JSON.parse(raw);
    return {
      top: data.top || [0, 0, 0],
      speed: data.speed ?? 3,
      apples: data.apples ?? 1,
      song: data.song ?? 0,
      buzzer: data.buzzer ?? 0,
    };
  } catch {
    return { top: [0, 0, 0], speed: 3, apples: 1, song: 0, buzzer: 0 };
  }
}

function savePersisted() {
  if (!module) return;

  const speedPtr = module._malloc(1);
  const applesPtr = module._malloc(1);
  const songPtr = module._malloc(1);
  const buzzerPtr = module._malloc(1);

  module._game_get_settings(speedPtr, applesPtr, songPtr, buzzerPtr);

  const top1Ptr = module._malloc(4);
  const top2Ptr = module._malloc(4);
  const top3Ptr = module._malloc(4);
  module._game_get_top_scores(top1Ptr, top2Ptr, top3Ptr);

  const data = {
    top: [
      module.HEAPU32[top1Ptr >> 2],
      module.HEAPU32[top2Ptr >> 2],
      module.HEAPU32[top3Ptr >> 2],
    ],
    speed: module.HEAPU8[speedPtr],
    apples: module.HEAPU8[applesPtr],
    song: module.HEAPU8[songPtr],
    buzzer: module.HEAPU8[buzzerPtr],
  };

  localStorage.setItem(STORAGE_KEY, JSON.stringify(data));

  module._free(speedPtr);
  module._free(applesPtr);
  module._free(songPtr);
  module._free(buzzerPtr);
  module._free(top1Ptr);
  module._free(top2Ptr);
  module._free(top3Ptr);
}

function playClick() {
  playTone(880, 0.04, 0.08);
}

function playEat() {
  playTone(1200, 0.06, 0.12);
}

function playGameOver() {
  playTone(220, 0.25, 0.2);
}

function playTone(freq, duration, volume) {
  try {
    const ac = new (window.AudioContext || window.webkitAudioContext)();
    const osc = ac.createOscillator();
    const gain = ac.createGain();
    osc.type = 'square';
    osc.frequency.value = freq;
    gain.gain.value = volume;
    osc.connect(gain);
    gain.connect(ac.destination);
    osc.start();
    gain.gain.exponentialRampToValueAtTime(0.001, ac.currentTime + duration);
    osc.stop(ac.currentTime + duration);
  } catch {
    /* audio optional */
  }
}

function playMusicLoop(songIndex) {
  const patterns = [
    [523, 494, 440, 440],
    [659, 659, 659, 523],
    [392, 440, 494, 523],
    [440, 440, 494, 494],
    [659, 659, 659, 784],
  ];
  const notes = patterns[songIndex % patterns.length];
  let i = 0;
  const playNext = () => {
    if (i >= notes.length) return;
    playTone(notes[i], 0.12, 0.06);
    i += 1;
    if (i < notes.length) setTimeout(playNext, 140);
  };
  playNext();
}

function blitFramebuffer() {
  const fbPtr = module._game_framebuffer();
  const fb = module.HEAPU8.subarray(fbPtr, fbPtr + SCR_WIDTH * (SCR_HEIGHT / 8));
  const pixels = imageData.data;

  for (let y = 0; y < SCR_HEIGHT; y++) {
    const page = Math.floor(y / 8);
    const bit = y % 8;
    const mask = 1 << bit;

    for (let x = 0; x < SCR_WIDTH; x++) {
      const on = fb[page * SCR_WIDTH + x] & mask;
      const color = on ? 255 : 0;

      for (let sy = 0; sy < SCALE; sy++) {
        for (let sx = 0; sx < SCALE; sx++) {
          const px = x * SCALE + sx;
          const py = y * SCALE + sy;
          const idx = (py * SCR_WIDTH * SCALE + px) * 4;
          pixels[idx] = color;
          pixels[idx + 1] = color;
          pixels[idx + 2] = color;
          pixels[idx + 3] = 255;
        }
      }
    }
  }

  ctx.putImageData(imageData, 0, 0);
}

function drainEvents() {
  let event;
  while ((event = module._game_get_event()) !== 0) {
    switch (event) {
      case 1:
        playClick();
        break;
      case 2:
        playEat();
        scoresDirty = true;
        break;
      case 3:
        playGameOver();
        scoresDirty = true;
        break;
      case 4:
        settingsDirty = true;
        break;
      case 6:
      case 7:
        const sp = module._malloc(1);
        const ap = module._malloc(1);
        const songPtr = module._malloc(1);
        const bp = module._malloc(1);
        module._game_get_settings(sp, ap, songPtr, bp);
        playMusicLoop(module.HEAPU8[songPtr]);
        module._free(sp);
        module._free(ap);
        module._free(songPtr);
        module._free(bp);
        break;
    }
  }

  if (settingsDirty || scoresDirty) {
    savePersisted();
    settingsDirty = false;
    scoresDirty = false;
  }
}

function frame(now) {
  const dt = lastTime ? Math.min(now - lastTime, 50) : 16;
  lastTime = now;

  module._game_update(Math.round(dt));
  blitFramebuffer();
  drainEvents();

  requestAnimationFrame(frame);
}

function sendKey(keyCode) {
  if (!module) return;
  module._game_input(keyCode);
  blitFramebuffer();
  drainEvents();
}

function mapKeyboard(code, key) {
  const k = key.toLowerCase();
  if (k === 'w') return 1;
  if (k === 's') return 2;
  if (k === 'a' || k === 'd') return 3;
  if (k === 'h' || k === 'g' || k === 'j') return 4;
  if (code === 'ArrowUp') return 1;
  if (code === 'ArrowDown') return 2;
  if (code === 'Enter' || code === 'Escape') return 3;
  return 0;
}

document.addEventListener('keydown', (e) => {
  const mapped = mapKeyboard(e.code, e.key);
  if (!mapped) return;
  e.preventDefault();
  sendKey(mapped);
});

document.querySelectorAll('.btn').forEach((btn) => {
  btn.addEventListener('click', () => {
    const key = btn.dataset.key;
    if (key === 'up') sendKey(1);
    if (key === 'down') sendKey(2);
    if (key === 'mode') sendKey(3);
  });
});

async function boot() {
  const persisted = loadPersisted();

  if (typeof createWormGame !== 'function') {
    statusEl.textContent = 'WASM loader missing — run web/build.sh first.';
    return;
  }

  module = await createWormGame({
    locateFile: (path) => `js/${path}`,
  });

  module._game_init(
    persisted.top[0],
    persisted.top[1],
    persisted.top[2],
    persisted.speed,
    persisted.apples,
    persisted.song,
    persisted.buzzer,
  );

  blitFramebuffer();
  statusEl.textContent = 'Ready — use keyboard or on-screen buttons.';
  requestAnimationFrame(frame);
}

boot().catch((err) => {
  statusEl.textContent = `Failed to load: ${err.message}`;
  console.error(err);
});
