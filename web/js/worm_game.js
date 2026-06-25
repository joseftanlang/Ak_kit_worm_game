/**
 * Worm Game runtime for the browser.
 *
 * When built with Emscripten (web/build.sh), this file is replaced by the
 * generated WASM loader. Until then, this JavaScript implementation mirrors
 * the C core in web/wasm/ and exposes the same createWormGame() API.
 */
(function (global) {
  'use strict';

  const SCR_W = 128;
  const SCR_H = 64;
  const FB = 1024;
  const WORM_STEP = 5;
  const WORM_MAX = 384;
  const MAX_APPLES = 8;
  const TICK_MS = [180, 150, 110, 90, 50];

  const KEY_UP = 1;
  const KEY_DOWN = 2;
  const KEY_MODE = 3;
  const KEY_LONG = 4;

  const EVENT_NONE = 0;
  const EVENT_CLICK = 1;
  const EVENT_EAT = 2;
  const EVENT_GAME_OVER = 3;
  const EVENT_SETTINGS = 4;
  const EVENT_MUSIC_START = 6;
  const EVENT_MUSIC_LOOP = 7;

  const FONT = [
    0x00,0x00,0x00,0x00,0x00,0x3E,0x5B,0x4F,0x5B,0x3E,0x3E,0x6B,0x4F,0x6B,0x3E,0x1C,
    0x3E,0x7C,0x3E,0x1C,0x18,0x3C,0x7E,0x3C,0x18,0x1C,0x57,0x7D,0x57,0x1C,0x1C,0x5E,
    0x7F,0x5E,0x1C,0x00,0x18,0x3C,0x18,0x00,0xFF,0xE7,0xC3,0xE7,0xFF,0x00,0x18,0x24,
    0x18,0x00,0xFF,0xE7,0xDB,0xE7,0xFF,0x30,0x48,0x3A,0x06,0x0E,0x26,0x29,0x79,0x29,
    0x26,0x40,0x7F,0x05,0x05,0x07,0x40,0x7F,0x05,0x25,0x3F,0x5A,0x3C,0xE7,0x3C,0x5A,
    0x7F,0x3E,0x1C,0x1C,0x08,0x08,0x1C,0x1C,0x3E,0x7F,0x14,0x22,0x7F,0x22,0x14,0x5F,
    0x5F,0x00,0x5F,0x5F,0x06,0x09,0x7F,0x01,0x7F,0x00,0x66,0x89,0x95,0x6A,0x60,0x60,
    0x60,0x60,0x60,0x94,0xA2,0xFF,0xA2,0x94,0x08,0x04,0x7E,0x04,0x08,0x10,0x20,0x7E,
    0x20,0x10,0x08,0x08,0x2A,0x1C,0x08,0x08,0x1C,0x2A,0x08,0x08,0x1E,0x10,0x10,0x10,
    0x10,0x0C,0x1E,0x0C,0x1E,0x0C,0x30,0x38,0x3E,0x38,0x30,0x06,0x0E,0x3E,0x0E,0x06,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5F,0x00,0x00,0x00,0x07,0x00,0x07,0x00,0x14,
    0x7F,0x14,0x7F,0x14,0x24,0x2A,0x7F,0x2A,0x12,0x23,0x13,0x08,0x64,0x62,0x36,0x49,
    0x56,0x20,0x50,0x00,0x08,0x07,0x03,0x00,0x00,0x1C,0x22,0x41,0x00,0x00,0x41,0x22,
    0x1C,0x00,0x2A,0x1C,0x7F,0x1C,0x2A,0x08,0x08,0x3E,0x08,0x08,0x00,0x80,0x70,0x30,
    0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x60,0x60,0x00,0x20,0x10,0x08,0x04,0x02,
    0x3E,0x51,0x49,0x45,0x3E,0x00,0x42,0x7F,0x40,0x00,0x72,0x49,0x49,0x49,0x46,0x21,
    0x41,0x49,0x4D,0x33,0x18,0x14,0x12,0x7F,0x10,0x27,0x45,0x45,0x45,0x39,0x3C,0x4A,
    0x49,0x49,0x31,0x41,0x21,0x11,0x09,0x07,0x36,0x49,0x49,0x49,0x36,0x46,0x49,0x49,
    0x29,0x1E,0x00,0x00,0x14,0x00,0x00,0x00,0x40,0x34,0x00,0x00,0x00,0x08,0x14,0x22,
    0x41,0x14,0x14,0x14,0x14,0x14,0x00,0x41,0x22,0x14,0x08,0x02,0x01,0x59,0x09,0x06,
    0x3E,0x41,0x5D,0x59,0x4E,0x7C,0x12,0x11,0x12,0x7C,0x7F,0x49,0x49,0x49,0x36,0x3E,
    0x41,0x41,0x41,0x22,0x7F,0x41,0x41,0x41,0x3E,0x7F,0x49,0x49,0x49,0x41,0x7F,0x09,
    0x09,0x09,0x01,0x3E,0x41,0x41,0x51,0x73,0x7F,0x08,0x08,0x08,0x7F,0x00,0x41,0x7F,
    0x41,0x00,0x20,0x40,0x41,0x3F,0x01,0x7F,0x08,0x14,0x22,0x41,0x7F,0x40,0x40,0x40,
    0x40,0x7F,0x02,0x1C,0x02,0x7F,0x7F,0x04,0x08,0x10,0x7F,0x3E,0x41,0x41,0x41,0x3E,
    0x7F,0x09,0x09,0x09,0x06,0x3E,0x41,0x51,0x21,0x5E,0x7F,0x09,0x19,0x29,0x46,0x26,
    0x49,0x49,0x49,0x32,0x03,0x01,0x7F,0x01,0x03,0x3F,0x40,0x40,0x40,0x3F,0x1F,0x20,
    0x40,0x20,0x1F,0x3F,0x40,0x38,0x40,0x3F,0x63,0x14,0x08,0x14,0x63,0x03,0x04,0x78,
    0x04,0x03,0x61,0x59,0x49,0x4D,0x43,0x00,0x7F,0x41,0x41,0x41,0x02,0x04,0x08,0x10,
    0x20,0x00,0x41,0x41,0x41,0x7F,0x04,0x02,0x01,0x02,0x04,0x40,0x40,0x40,0x40,0x40,
    0x00,0x03,0x07,0x08,0x00,0x20,0x54,0x54,0x78,0x40,0x7F,0x28,0x44,0x44,0x38,0x38,
    0x44,0x44,0x44,0x28,0x38,0x44,0x44,0x28,0x7F,0x38,0x54,0x54,0x54,0x18,0x00,0x08,
    0x7E,0x09,0x02,0x18,0xA4,0xA4,0x9C,0x78,0x7F,0x08,0x04,0x04,0x78,0x00,0x44,0x7D,
    0x40,0x00,0x20,0x40,0x40,0x3D,0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x41,0x7F,0x40,
    0x00,0x7C,0x04,0x78,0x04,0x78,0x7C,0x08,0x04,0x04,0x78,0x38,0x44,0x44,0x44,0x38,
    0xFC,0x18,0x24,0x24,0x18,0x18,0x24,0x24,0x18,0xFC,0x7C,0x08,0x04,0x04,0x08,0x48,
    0x54,0x54,0x54,0x24,0x04,0x04,0x3F,0x44,0x24,0x3C,0x40,0x40,0x20,0x7C,0x1C,0x20,
    0x40,0x20,0x1C,0x3C,0x40,0x30,0x40,0x3C,0x44,0x28,0x10,0x28,0x44,0x4C,0x90,0x90,
    0x90,0x7C,0x44,0x64,0x54,0x4C,0x44,0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x77,0x00,
    0x00,0x00,0x41,0x36,0x08,0x00,0x02,0x01,0x02,0x04,0x02,0x3C,0x26,0x23,0x26,0x3C
  ];

  const APPLE_BMP = [0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C];

  function Gfx(heap, fbOffset) {
    this.heap = heap;
    this.fbOffset = fbOffset;
    this.cx = 0;
    this.cy = 0;
    this.textSize = 1;
    this.textColor = 1;
  }

  Gfx.prototype.pixel = function (x, y, c) {
    if (x < 0 || x >= SCR_W || y < 0 || y >= SCR_H) return;
    const row = Math.floor(y / 8);
    const bit = y % 8;
    const idx = this.fbOffset + row * SCR_W + x;
    const mask = 1 << bit;
    if (c) this.heap[idx] |= mask;
    else this.heap[idx] &= ~mask;
  };

  Gfx.prototype.clear = function () {
    for (let i = 0; i < FB; i++) this.heap[this.fbOffset + i] = 0;
  };

  Gfx.prototype.rect = function (x, y, w, h, c) {
    for (let i = 0; i < w; i++) this.pixel(x + i, y, c);
    for (let i = 0; i < w; i++) this.pixel(x + i, y + h - 1, c);
    for (let j = 0; j < h; j++) {
      this.pixel(x, y + j, c);
      this.pixel(x + w - 1, y + j, c);
    }
  };

  Gfx.prototype.hline = function (x, y, w, c) {
    for (let i = 0; i < w; i++) this.pixel(x + i, y, c);
  };

  Gfx.prototype.fillCircle = function (x0, y0, r, c) {
    let f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    const plot = (x, y) => this.pixel(x, y, c);
    plot(x0, y0 + r); plot(x0, y0 - r); plot(x0 + r, y0); plot(x0 - r, y0);
    while (x < y) {
      if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
      x++; ddF_x += 2; f += ddF_x;
      plot(x0 + x, y0 + y); plot(x0 - x, y0 + y);
      plot(x0 + x, y0 - y); plot(x0 - x, y0 - y);
      plot(x0 + y, y0 + x); plot(x0 - y, y0 + x);
      plot(x0 + y, y0 - x); plot(x0 - y, y0 - x);
    }
  };

  Gfx.prototype.fillRoundRect = function (x, y, w, h, r, c) {
    r = Math.min(r, Math.floor(w / 2), Math.floor(h / 2));
    for (let i = x + r; i < x + w - r; i++)
      for (let j = y; j < y + h; j++) this.pixel(i, j, c);
    for (let j = y + r; j < y + h - r; j++)
      for (let i = x; i < x + w; i++) this.pixel(i, j, c);
    this.fillCircle(x + r, y + r, r, c);
    this.fillCircle(x + w - r - 1, y + r, r, c);
    this.fillCircle(x + r, y + h - r - 1, r, c);
    this.fillCircle(x + w - r - 1, y + h - r - 1, r, c);
  };

  Gfx.prototype.bitmap = function (x, y, bmp, w, h, c) {
    for (let j = 0; j < h; j++) {
      let line = bmp[j];
      for (let i = 0; i < w; i++) {
        if (line & (1 << (w - 1 - i))) this.pixel(x + i, y + j, c);
      }
    }
  };

  Gfx.prototype.setCursor = function (x, y) { this.cx = x; this.cy = y; };
  Gfx.prototype.setTextSize = function (s) { this.textSize = s < 1 ? 1 : s; };
  Gfx.prototype.setTextColor = function (c) { this.textColor = c; };

  Gfx.prototype.char = function (x, y, ch, c, size) {
    if (ch < 32 || ch > 127) return;
    const glyph = FONT.slice((ch - 32) * 5, (ch - 32) * 5 + 5);
    for (let i = 0; i < 5; i++) {
      let line = glyph[i];
      for (let j = 0; j < 8; j++) {
        if (line & 1) {
          if (size === 1) this.pixel(x + i, y + j, c);
          else for (let sy = 0; sy < size; sy++)
            for (let sx = 0; sx < size; sx++)
              this.pixel(x + i * size + sx, y + j * size + sy, c);
        }
        line >>= 1;
      }
    }
  };

  Gfx.prototype.print = function (str) {
    for (let i = 0; i < str.length; i++) {
      const ch = str.charCodeAt(i);
      if (ch === 10) { this.cy += this.textSize * 8; this.cx = 0; continue; }
      this.char(this.cx, this.cy, ch, this.textColor, this.textSize);
      this.cx += this.textSize * 6;
    }
  };

  Gfx.prototype.printInt = function (n) {
    this.print(String(n));
  };

  function Game(heap, fbOffset) {
    this.gfx = new Gfx(heap, fbOffset);
    this.pending = EVENT_NONE;
    this.screen = 0;
    this.menuIndex = 0;
    this.settingsSel = 0;
    this.wormFinished = false;
    this.wormWon = false;
    this.wormTick = 0;
    this.appleTick = 0;
    this.chartTick = 0;
    this.musicTick = 0;
    this.chartWormX = SCR_W;
    this.chartFrame = 0;
    this.chartRank = 0;
    this.chartStars = [
      { x: 8, y: 8, s: 1 }, { x: 22, y: 15, s: 2 }, { x: 33, y: 4, s: 1 },
      { x: 48, y: 12, s: 2 }, { x: 61, y: 18, s: 1 }, { x: 77, y: 10, s: 2 },
      { x: 89, y: 6, s: 1 }, { x: 101, y: 14, s: 2 }, { x: 114, y: 3, s: 1 },
      { x: 126, y: 16, s: 2 }
    ];
    this.settings = { speed: 3, apples: 1, song: 0, buzzer: 0 };
    this.top = [0, 0, 0];
    this.worm = {
      trail: Array(WORM_MAX).fill(null).map(() => ({ x: 0, y: 0 })),
      length: 4, grow: 0, dir: 0, x: 51, y: 36, w: 5, h: 5
    };
    this.border = { x: 1, y: 1, w: 125, h: 60 };
    this.apples = Array(MAX_APPLES).fill(null).map(() => ({
      x: 0, y: 0, w: 7, h: 7, active: 0, life: 0, respawn: 0
    }));
    this.lives = 3;
    this.score = 0;
  }

  Game.prototype.push = function (e) {
    if (this.pending === EVENT_NONE) this.pending = e;
  };

  Game.prototype.init = function (t1, t2, t3, speed, apples, song, buzzer) {
    this.top = [t1, t2, t3];
    this.settings = {
      speed: Math.max(1, Math.min(5, speed)),
      apples: Math.max(1, Math.min(8, apples)),
      song: song % 5,
      buzzer: buzzer ? 1 : 0
    };
    this.enterScreen(0);
  };

  Game.prototype.tickMs = function () {
    return TICK_MS[this.settings.speed - 1];
  };

  Game.prototype.turnLeft = function (d) {
    const m = { 0: 3, 3: 2, 2: 1, 1: 0 };
    return m[d] ?? 0;
  };

  Game.prototype.turnRight = function (d) {
    const m = { 0: 1, 1: 2, 2: 3, 3: 0 };
    return m[d] ?? 0;
  };

  Game.prototype.wormInit = function () {
    const w = this.worm;
    w.x = 51; w.y = 36; w.w = 5; w.h = 5; w.dir = 0; w.length = 4; w.grow = 0;
    for (let i = 0; i < WORM_MAX; i++) {
      w.trail[i].x = 0;
      w.trail[i].y = 0;
    }
    for (let i = 0; i < w.length; i++) {
      w.trail[i].x = Math.max(0, 51 - i * WORM_STEP);
      w.trail[i].y = 36;
    }
  };

  Game.prototype.rand = function (min, max) {
    return min + Math.floor(Math.random() * (max - min + 1));
  };

  Game.prototype.appleOverlap = function (x0, y0, w0, h0, x1, y1, w1, h1) {
    return !(x0 >= x1 + w1 || x0 + w0 <= x1 || y0 >= y1 + h1 || y0 + h0 <= y1);
  };

  Game.prototype.appleValid = function (x, y, w, h, skip) {
    for (let i = 0; i < MAX_APPLES; i++) {
      if (i === skip) continue;
      const a = this.apples[i];
      if (a.active && this.appleOverlap(x, y, w, h, a.x, a.y, a.w, a.h)) return false;
    }
    const wv = this.worm;
    return !this.appleOverlap(x, y, w, h, wv.x, wv.y, wv.w, wv.h);
  };

  Game.prototype.appleSpawn = function (idx) {
    const b = this.border;
    const cols = Math.floor(b.w / WORM_STEP);
    const rows = Math.floor(b.h / WORM_STEP);
    if (cols < 3 || rows < 3) return false;
    for (let t = 0; t < 32; t++) {
      const x = b.x + this.rand(1, cols - 2) * WORM_STEP;
      const y = b.y + this.rand(1, rows - 2) * WORM_STEP;
      if (!this.appleValid(x, y, 7, 7, idx)) continue;
      const a = this.apples[idx];
      a.x = x; a.y = y; a.w = 7; a.h = 7; a.active = 1;
      a.life = 3; a.respawn = 0;
      return true;
    }
    this.apples[idx].respawn = 1;
    return false;
  };

  Game.prototype.appleInit = function () {
    const limit = this.settings.apples;
    for (let i = 0; i < MAX_APPLES; i++) {
      const a = this.apples[i];
      a.active = 0; a.life = 0; a.respawn = 0;
      if (i < limit) {
        a.respawn = i * 2;
        this.appleSpawn(i);
      }
    }
  };

  Game.prototype.appleCollision = function () {
    const limit = this.settings.apples;
    for (let i = 0; i < limit; i++) {
      const a = this.apples[i];
      if (!a.active) continue;
      const w = this.worm;
      if (this.appleOverlap(w.x, w.y, w.w, w.h, a.x, a.y, a.w, a.h)) {
        this.score++;
        this.insertTop(this.score);
        w.grow++;
        a.active = 0;
        a.respawn = 2;
        this.push(EVENT_EAT);
        return true;
      }
    }
    return false;
  };

  Game.prototype.appleTimer = function () {
    const limit = this.settings.apples;
    for (let i = limit; i < MAX_APPLES; i++) {
      this.apples[i].active = 0;
    }
    for (let i = 0; i < limit; i++) {
      const a = this.apples[i];
      if (a.active) {
        if (a.life > 0) a.life--;
        if (a.life === 0) { a.active = 0; a.respawn = 2; }
      } else {
        if (a.respawn > 0) a.respawn--;
        if (a.respawn === 0) this.appleSpawn(i);
      }
    }
  };

  Game.prototype.insertTop = function (value) {
    const merged = [...this.top, value].filter(v => v > 0);
    const uniq = [];
    merged.sort((a, b) => b - a);
    for (const v of merged) {
      if (!uniq.includes(v)) uniq.push(v);
      if (uniq.length >= 3) break;
    }
    while (uniq.length < 3) uniq.push(0);
    this.top = uniq.slice(0, 3);
  };

  Game.prototype.wormAdvance = function () {
    const w = this.worm;
    const head = { x: w.trail[0].x, y: w.trail[0].y };
    let occupied = Math.min(w.length, WORM_MAX);
    if (w.grow === 0 && occupied > 1) occupied--;

    switch (w.dir) {
      case 0: head.x += WORM_STEP; break;
      case 2: head.x = head.x > WORM_STEP ? head.x - WORM_STEP : 0; break;
      case 1: head.y += WORM_STEP; break;
      case 3: head.y = head.y > WORM_STEP ? head.y - WORM_STEP : 0; break;
    }

    const b = this.border;
    if (head.x + WORM_STEP > b.x + b.w) head.x = b.x;
    else if (head.x < b.x) head.x = b.x + b.w - WORM_STEP;
    if (head.y + WORM_STEP > b.y + b.h) head.y = b.y;
    else if (head.y < b.y) head.y = b.y + b.h - WORM_STEP;

    for (let i = 0; i < occupied; i++) {
      if (w.trail[i].x === head.x && w.trail[i].y === head.y) return true;
    }

    let nextLen = Math.min(w.length, WORM_MAX);
    if (w.grow > 0 && nextLen < WORM_MAX) { nextLen++; w.grow--; }

    for (let i = nextLen; i > 1; i--) {
      w.trail[i - 1].x = w.trail[i - 2].x;
      w.trail[i - 1].y = w.trail[i - 2].y;
    }
    w.trail[0] = head;
    w.x = head.x; w.y = head.y; w.length = nextLen;
    return false;
  };

  Game.prototype.drawWorm = function () {
    const g = this.gfx;
    const w = this.worm;
    const len = Math.min(w.length, WORM_MAX);
    for (let i = len; i > 1; i--) {
      const s = w.trail[i - 1];
      g.fillRoundRect(s.x, s.y, WORM_STEP, WORM_STEP, 1, 1);
    }
    const cx = w.trail[0].x + 2;
    const cy = w.trail[0].y + 2;
    g.fillCircle(cx, cy, 3, 1);
    g.fillCircle(cx, cy, 2, 1);
    let e1x = 0, e1y = 0, e2x = 0, e2y = 0;
    switch (w.dir) {
      case 0: e1x = 1; e1y = -1; e2x = 1; e2y = 1; break;
      case 2: e1x = -1; e1y = -1; e2x = -1; e2y = 1; break;
      case 1: e1x = -1; e1y = 1; e2x = 1; e2y = 1; break;
      default: e1x = -1; e1y = -1; e2x = 1; e2y = -1; break;
    }
    g.pixel(cx + e1x, cy + e1y, 0);
    g.pixel(cx + e2x, cy + e2y, 0);
  };

  Game.prototype.renderWorm = function () {
    const g = this.gfx;
    g.clear();
    if (this.wormFinished) {
      g.rect(0, 0, SCR_W, SCR_H, 1);
      g.setTextColor(1);
      g.setTextSize(2);
      g.setCursor(8, 16);
      g.print(this.wormWon ? 'YOU WIN' : 'GAME OVER');
      g.setTextSize(1);
      g.setCursor(33, 34);
      g.print('SCORE: ' + this.score);
      g.setCursor(10, 46);
      g.print('MODE TO VIEW TOP 3');
      return;
    }
    g.rect(1, 1, 126, 62, 1);
    for (let i = 0; i < MAX_APPLES; i++) {
      if (this.apples[i].active) {
        g.bitmap(this.apples[i].x, this.apples[i].y, APPLE_BMP, 7, 7, 1);
      }
    }
    this.drawWorm();
    g.setTextSize(1);
    g.setTextColor(1);
    g.setCursor(4, 2);
    g.print('Score:' + this.score);
  };

  Game.prototype.renderMenu = function () {
    const items = ['Worm', 'Charts', 'Settings'];
    const g = this.gfx;
    g.clear();
    g.setTextSize(1);
    g.setTextColor(1);
    g.setCursor(35, 3);
    g.print('WORM GAME');
    g.hline(0, 14, SCR_W, 1);
    for (let row = 0; row < 3; row++) {
      const y = 20 + row * 10;
      g.setCursor(8, y + 5);
      g.print((row === this.menuIndex ? '> ' : '  ') + items[row]);
    }
  };

  Game.prototype.renderSettings = function () {
    const labels = ['SPEED', 'APPLE', 'SONG', 'BUZZER'];
    const vals = [
      String(this.settings.speed),
      String(this.settings.apples),
      ['ROCK', 'MARIO', 'PIRATES', 'BUS', 'XMAS'][this.settings.song],
      this.settings.buzzer ? 'ON' : 'OFF'
    ];
    const g = this.gfx;
    g.clear();
    g.setTextColor(1);
    g.setTextSize(1);
    g.setCursor(30, 3);
    g.print('SETTINGS');
    for (let i = 0; i < 4; i++) {
      const y = 18 + i * 10;
      g.setCursor(8, y + 2);
      g.print((i === this.settingsSel ? '> ' : '  ') + labels[i]);
      g.setCursor(82, y + 2);
      g.print(vals[i]);
    }
  };

  Game.prototype.renderCharts = function () {
    const g = this.gfx;
    const rankShort = ['1ST', '2ND', '3RD'];
    const rankName = ['FIRST', 'SECOND', 'THIRD'];
    const slots = this.top.map((s, i) => `${i + 1}:${s || '-'}`);
    const cur = this.top[this.chartRank];
    const bubble = cur ? `${rankShort[this.chartRank]} ${cur}` : `${rankShort[this.chartRank]} -`;
    const bubbleW = bubble.length * 6 + 4;
    let bubbleX = this.chartWormX - bubbleW / 2;
    bubbleX = Math.max(2, Math.min(bubbleX, SCR_W - bubbleW - 2));
    const highlightX = [4, 45, 86][this.chartRank];

    g.clear();
    g.rect(0, 0, SCR_W, SCR_H, 1);
    g.hline(1, 34, SCR_W - 2, 1);
    g.setTextColor(1);
    g.setTextSize(1);
    g.setCursor(18, 2);
    g.print('WORM LEADER RUN');

    for (const st of this.chartStars) {
      if (((this.chartFrame + this.chartStars.indexOf(st)) & 1) === 0 || st.s > 1)
        g.pixel(st.x, st.y, 1);
    }

    const yBase = 24 + ((this.chartFrame >> 1) & 1);
    for (let i = 1; i < 7; i++) {
      const sx = this.chartWormX + i * 5;
      const sy = yBase + (((i + this.chartFrame) & 1) ? 1 : -1);
      g.fillCircle(sx, sy, 2, 1);
    }
    g.fillCircle(this.chartWormX, yBase, 3, 1);
    g.pixel(this.chartWormX - 1, yBase + 1, 0);
    g.pixel(this.chartWormX - 1, yBase - 1, 0);

    g.rect(bubbleX, 10, bubbleW, 10, 1);
    g.setCursor(bubbleX + 2, 12);
    g.print(bubble);
    g.rect(highlightX, 41, 38, 11, 1);
    g.setCursor(10, 44); g.print(slots[0]);
    g.setCursor(51, 44); g.print(slots[1]);
    g.setCursor(92, 44); g.print(slots[2]);
    g.setCursor(8, 56); g.print(rankName[this.chartRank]);
    g.setCursor(70, 56); g.print('MODE:MENU');
  };

  Game.prototype.chartAnim = function () {
    const tail = (7 - 1) * 5 + 6;
    this.chartWormX -= 3;
    this.chartFrame++;
    if (this.chartWormX < -tail) {
      this.chartWormX = SCR_W;
      this.chartRank = (this.chartRank + 1) % 3;
    }
    for (const st of this.chartStars) {
      if (st.x <= st.s) {
        st.x = SCR_W - 1;
        st.y = (3 + this.chartStars.indexOf(st) * 5 + this.chartFrame) % 20;
      } else st.x -= st.s;
    }
  };

  Game.prototype.startWorm = function () {
    this.wormFinished = false;
    this.wormWon = false;
    this.wormTick = 0;
    this.appleTick = 0;
    this.musicTick = 0;
    this.score = 0;
    this.lives = 3;
    this.wormInit();
    this.appleInit();
    if (this.settings.buzzer) this.push(EVENT_MUSIC_START);
    this.renderWorm();
  };

  Game.prototype.finishWorm = function (won) {
    if (this.wormFinished) return;
    this.wormFinished = true;
    this.wormWon = won;
    this.insertTop(this.score);
    this.push(EVENT_GAME_OVER);
    this.renderWorm();
  };

  Game.prototype.wormStep = function () {
    if (this.wormFinished) {
      this.renderWorm();
      return;
    }
    if (this.wormAdvance()) {
      this.lives--;
      if (this.lives <= 0) this.finishWorm(false);
    }
    this.appleCollision();
    this.renderWorm();
  };

  Game.prototype.enterScreen = function (s) {
    this.screen = s;
    if (s === 0) { this.menuIndex = 0; this.renderMenu(); }
    else if (s === 1) this.startWorm();
    else if (s === 2) {
      this.chartWormX = SCR_W;
      this.chartFrame = 0;
      this.chartRank = 0;
      this.chartTick = 0;
      this.renderCharts();
    } else {
      this.settingsSel = 0;
      this.renderSettings();
    }
  };

  Game.prototype.update = function (dt) {
    if (this.screen === 1 && !this.wormFinished) {
      this.wormTick += dt;
      this.appleTick += dt;
      this.musicTick += dt;
      const interval = this.tickMs();
      while (this.wormTick >= interval) {
        this.wormTick -= interval;
        this.wormStep();
        if (this.wormFinished) break;
      }
      if (!this.wormFinished) {
        while (this.appleTick >= 1000) {
          this.appleTick -= 1000;
          this.appleTimer();
        }
        if (this.settings.buzzer) {
          while (this.musicTick >= 1000) {
            this.musicTick -= 1000;
            this.push(EVENT_MUSIC_LOOP);
          }
        }
      }
    } else if (this.screen === 2) {
      this.chartTick += dt;
      while (this.chartTick >= 110) {
        this.chartTick -= 110;
        this.chartAnim();
        this.renderCharts();
      }
    }
  };

  Game.prototype.input = function (key) {
    if (this.screen === 0) {
      if (key === KEY_UP) {
        this.menuIndex = this.menuIndex > 0 ? this.menuIndex - 1 : 2;
        this.push(EVENT_CLICK);
        this.renderMenu();
      } else if (key === KEY_DOWN) {
        this.menuIndex = (this.menuIndex + 1) % 3;
        this.push(EVENT_CLICK);
        this.renderMenu();
      } else if (key === KEY_MODE) {
        this.push(EVENT_CLICK);
        this.enterScreen(this.menuIndex);
      }
    } else if (this.screen === 1) {
      if (this.wormFinished) {
        if (key === KEY_UP || key === KEY_DOWN) {
          this.push(EVENT_CLICK);
          this.enterScreen(0);
        } else if (key === KEY_MODE) {
          this.push(EVENT_CLICK);
          this.enterScreen(2);
        }
      } else {
        if (key === KEY_UP) {
          this.worm.dir = this.turnLeft(this.worm.dir);
          this.push(EVENT_CLICK);
        } else if (key === KEY_DOWN) {
          this.worm.dir = this.turnRight(this.worm.dir);
          this.push(EVENT_CLICK);
        } else if (key === KEY_MODE) {
          this.push(EVENT_CLICK);
          this.enterScreen(0);
        }
      }
    } else if (this.screen === 2) {
      this.push(EVENT_CLICK);
      this.enterScreen(0);
    } else if (this.screen === 3) {
      if (key === KEY_UP) {
        this.settingsSel = this.settingsSel === 0 ? 3 : this.settingsSel - 1;
        this.push(EVENT_CLICK);
        this.renderSettings();
      } else if (key === KEY_DOWN) {
        this.settingsSel = (this.settingsSel + 1) % 4;
        this.push(EVENT_CLICK);
        this.renderSettings();
      } else if (key === KEY_MODE) {
        this.push(EVENT_CLICK);
        if (this.settingsSel === 0)
          this.settings.speed = this.settings.speed >= 5 ? 1 : this.settings.speed + 1;
        else if (this.settingsSel === 1)
          this.settings.apples = this.settings.apples >= 8 ? 1 : this.settings.apples + 1;
        else if (this.settingsSel === 2)
          this.settings.song = (this.settings.song + 1) % 5;
        else this.settings.buzzer = this.settings.buzzer ? 0 : 1;
        this.push(EVENT_SETTINGS);
        this.renderSettings();
      } else if (key === KEY_LONG) {
        this.push(EVENT_CLICK);
        this.enterScreen(0);
      }
    }
  };

  Game.prototype.popEvent = function () {
    const e = this.pending;
    this.pending = EVENT_NONE;
    return e;
  };

  Game.prototype.exportSettings = function () {
    return { ...this.settings };
  };

  Game.prototype.exportTop = function () {
    return [...this.top];
  };

  function createWormGame(moduleArg = {}) {
    const memory = new WebAssembly.Memory({ initial: 4 });
    const heap = new Uint8Array(memory.buffer);
    const FB_OFFSET = 0;
    const game = new Game(heap, FB_OFFSET);
    let allocPtr = FB + 64;

    return Promise.resolve({
      HEAPU8: heap,
      HEAPU32: new Uint32Array(memory.buffer),
      _malloc: (n) => {
        const p = allocPtr;
        allocPtr += (n + 7) & ~7;
        return p;
      },
      _free: () => {},
      _game_init: (t1, t2, t3, sp, ap, sg, bz) => game.init(t1, t2, t3, sp, ap, sg, bz),
      _game_update: (dt) => game.update(dt),
      _game_input: (k) => game.input(k),
      _game_framebuffer: () => FB_OFFSET,
      _game_get_event: () => game.popEvent(),
      _game_get_settings: (sp, ap, sgp, bp) => {
        const s = game.exportSettings();
        heap[sp] = s.speed;
        heap[ap] = s.apples;
        heap[sgp] = s.song;
        heap[bp] = s.buzzer;
      },
      _game_get_top_scores: (t1, t2, t3) => {
        const t = game.exportTop();
        const u32 = new Uint32Array(memory.buffer);
        u32[t1 >> 2] = t[0];
        u32[t2 >> 2] = t[1];
        u32[t3 >> 2] = t[2];
      }
    });
  }

  global.createWormGame = createWormGame;
})(typeof globalThis !== 'undefined' ? globalThis : window);
