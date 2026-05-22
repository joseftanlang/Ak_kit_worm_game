#include "worm_game_score.h"
#include "app_eeprom.h"
#include "eeprom.h"

static worm_game_score_display_t game_score = {0};
static uint32_t game_top_scores[3] = {0};
static bool game_top_scores_loaded = false;

static void score_top_load_if_needed(void) {
	if (game_top_scores_loaded) {
		return;
	}

	uint32_t magic = 0;
	if (eeprom_read(EEPROM_WORM_SCORE_MAGIC_ADDR, (uint8_t*)&magic, sizeof(magic)) != EEPROM_DRIVER_OK ||
		magic != EEPROM_WORM_SCORE_MAGIC) {
		magic = EEPROM_WORM_SCORE_MAGIC;
		game_top_scores[0] = 0;
		game_top_scores[1] = 0;
		game_top_scores[2] = 0;
		eeprom_write(EEPROM_WORM_SCORE_MAGIC_ADDR, (uint8_t*)&magic, sizeof(magic));
		eeprom_write(EEPROM_WORM_SCORE_TOP3_ADDR, (uint8_t*)game_top_scores, sizeof(game_top_scores));
		game_top_scores_loaded = true;
		return;
	}

	if (eeprom_read(EEPROM_WORM_SCORE_TOP3_ADDR, (uint8_t*)game_top_scores, sizeof(game_top_scores)) != EEPROM_DRIVER_OK) {
		game_top_scores[0] = 0;
		game_top_scores[1] = 0;
		game_top_scores[2] = 0;
	}

	game_top_scores_loaded = true;
}

static bool score_top_insert_unique(uint32_t value) {
	uint32_t merged[4] = {game_top_scores[0], game_top_scores[1], game_top_scores[2], value};
	uint32_t normalized[3] = {0};
	uint8_t count = 0;

	for (uint8_t i = 0; i < 4; i++) {
		uint32_t candidate = merged[i];
		if (candidate == 0) {
			continue;
		}

		bool duplicate = false;
		for (uint8_t j = 0; j < count; j++) {
			if (normalized[j] == candidate) {
				duplicate = true;
				break;
			}
		}

		if (duplicate) {
			continue;
		}

		uint8_t pos = count;
		for (uint8_t j = 0; j < count; j++) {
			if (candidate > normalized[j]) {
				pos = j;
				break;
			}
		}

		if (pos >= 3) {
			continue;
		}

		if (count < 3) {
			count++;
		}

		for (int8_t j = (int8_t)count - 1; j > (int8_t)pos; j--) {
			normalized[j] = normalized[j - 1];
		}
		normalized[pos] = candidate;
	}

	bool changed = false;
	for (uint8_t i = 0; i < 3; i++) {
		if (game_top_scores[i] != normalized[i]) {
			changed = true;
			break;
		}
	}

	if (changed) {
		game_top_scores[0] = normalized[0];
		game_top_scores[1] = normalized[1];
		game_top_scores[2] = normalized[2];
	}

	return changed;
}

void score_init(void) {
	score_top_load_if_needed();
	game_score.x = 2;
	game_score.y = 2;
	game_score.score = START_SCORE;
}

void score_reset(void) {
	game_score.score = START_SCORE;
}

void score_inc(void) {
	game_score.score += INCREMENT_SCORE;
}

uint32_t score_get(void) {
	return game_score.score;
}

void score_commit_current(void) {
	uint32_t magic = EEPROM_WORM_SCORE_MAGIC;

	score_top_load_if_needed();
	if (!score_top_insert_unique(game_score.score)) {
		return;
	}

	eeprom_write(EEPROM_WORM_SCORE_MAGIC_ADDR, (uint8_t*)&magic, sizeof(magic));
	eeprom_write(EEPROM_WORM_SCORE_TOP3_ADDR, (uint8_t*)game_top_scores, sizeof(game_top_scores));
}

uint32_t score_top_get(uint8_t index) {
	score_top_load_if_needed();
	if (index >= 3) {
		return 0;
	}

	return game_top_scores[index];
}

void game_score_handler(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SCORE_INIT:
		score_init();
		break;
	case AC_SCORE_INC:
		score_inc();
		break;
	case AC_SCORE_RESET:
		score_reset();
		break;
	default:
		break;
	}
}