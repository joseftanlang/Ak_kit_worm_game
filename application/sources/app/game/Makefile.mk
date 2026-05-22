CFLAGS		+= -I./sources/app/game
CPPFLAGS	+= -I./sources/app/game

VPATH += sources/app/game

# CPP source files
SOURCES_CPP += sources/app/game/worm_game_border.cpp
SOURCES_CPP += sources/app/game/worm_game_apple.cpp
SOURCES_CPP += sources/app/game/worm_game_eating.cpp
SOURCES_CPP += sources/app/game/worm_game_worm.cpp
SOURCES_CPP += sources/app/game/worm_game_score.cpp
SOURCES_CPP += sources/app/game/worm_game_lives.cpp
SOURCES_CPP += sources/app/game/worm_game_eating.cpp