#ifndef WORM_TYPES_H
#define WORM_TYPES_H

#include "config.h"
#include <stdint.h>

typedef enum
{
	WORM_DIR_RIGHT = 0,
	WORM_DIR_DOWN,
	WORM_DIR_LEFT,
	WORM_DIR_UP
} worm_dir_t;

typedef struct
{
	uint8_t x;
	uint8_t y;
} worm_point_t;

typedef struct
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint8_t worm_image;
	uint8_t dir;
	uint16_t length;
	uint16_t grow_pending;
	worm_point_t trail[WORM_MAX_TRAIL];
} worm_state_t;

typedef struct
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
} worm_border_t;

typedef struct
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint8_t is_active;
	uint8_t life_seconds;
	uint8_t respawn_seconds;
	uint8_t apple_image;
} worm_apple_t;

typedef struct
{
	uint32_t x;
	uint32_t y;
	uint32_t score;
} worm_score_t;

typedef struct
{
	uint32_t amount_lives;
} worm_lives_t;

#endif
