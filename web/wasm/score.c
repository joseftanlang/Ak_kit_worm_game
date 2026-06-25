#include "score.h"

worm_score_t game_score;
static uint32_t game_top_scores[3];
static uint8_t game_top_scores_loaded = 0;

static uint8_t score_top_insert_unique(uint32_t value)
{
	uint32_t merged[4] = {game_top_scores[0], game_top_scores[1], game_top_scores[2], value};
	uint32_t normalized[3] = {0};
	uint8_t count = 0;

	for (uint8_t i = 0; i < 4; i++)
	{
		uint32_t candidate = merged[i];
		if (candidate == 0)
		{
			continue;
		}

		uint8_t duplicate = 0;
		for (uint8_t j = 0; j < count; j++)
		{
			if (normalized[j] == candidate)
			{
				duplicate = 1;
				break;
			}
		}

		if (duplicate)
		{
			continue;
		}

		uint8_t pos = count;
		for (uint8_t j = 0; j < count; j++)
		{
			if (candidate > normalized[j])
			{
				pos = j;
				break;
			}
		}

		if (pos >= 3)
		{
			continue;
		}

		if (count < 3)
		{
			count++;
		}

		for (int8_t j = (int8_t)count - 1; j > (int8_t)pos; j--)
		{
			normalized[j] = normalized[j - 1];
		}
		normalized[pos] = candidate;
	}

	uint8_t changed = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		if (game_top_scores[i] != normalized[i])
		{
			changed = 1;
			break;
		}
	}

	if (changed)
	{
		game_top_scores[0] = normalized[0];
		game_top_scores[1] = normalized[1];
		game_top_scores[2] = normalized[2];
	}

	return changed;
}

void score_import_top(uint32_t top1, uint32_t top2, uint32_t top3)
{
	game_top_scores[0] = top1;
	game_top_scores[1] = top2;
	game_top_scores[2] = top3;
	game_top_scores_loaded = 1;
}

void score_init(void)
{
	game_score.x = 2;
	game_score.y = 2;
	game_score.score = START_SCORE;
}

void score_reset(void)
{
	game_score.score = START_SCORE;
}

void score_inc(void)
{
	game_score.score += INCREMENT_SCORE;
}

uint32_t score_get(void)
{
	return game_score.score;
}

void score_commit_current(void)
{
	if (!game_top_scores_loaded)
	{
		game_top_scores[0] = 0;
		game_top_scores[1] = 0;
		game_top_scores[2] = 0;
		game_top_scores_loaded = 1;
	}

	score_top_insert_unique(game_score.score);
}

uint32_t score_top_get(uint8_t index)
{
	if (!game_top_scores_loaded)
	{
		return 0;
	}
	if (index >= 3)
	{
		return 0;
	}
	return game_top_scores[index];
}
