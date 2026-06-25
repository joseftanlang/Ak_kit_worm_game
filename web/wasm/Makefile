# Worm Game WebAssembly build

SRC = fb.c gfx.c worm_game.c apple.c score.c lives.c settings.c render.c app.c
OUT = ../js/worm_game.js

EMCC_FLAGS = -O2 \
	-s WASM=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME='createWormGame' \
	-s EXPORTED_FUNCTIONS='["_game_init","_game_update","_game_input","_game_framebuffer","_game_get_event","_game_get_settings","_game_get_top_scores","_malloc","_free"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8"]' \
	-s ALLOW_MEMORY_GROWTH=0 \
	-s ENVIRONMENT=web \
	--no-entry

all: $(OUT)

$(OUT): $(SRC)
	emcc $(SRC) $(EMCC_FLAGS) -o $(OUT)

clean:
	rm -f $(OUT) ../js/worm_game.wasm

.PHONY: all clean
