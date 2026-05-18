#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_PLAYERS 4
#define MAX_BOMBS 64
#define MAP_WIDTH 15
#define MAP_HEIGHT 13
#define MAX_EXPLOSION_CELLS 128
#define BOMB_TIMER_TICKS 180
#define EXPLOSION_TIMER_TICKS 30
#define DEFAULT_BOMB_RANGE 2
#define DEFAULT_MAX_ACTIVE_BOMBS 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL,
    TILE_BREAKABLE,
    TILE_BONUS_BOMB,
    TILE_BONUS_RANGE,
    TILE_FIRE
} TileType;

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
} Direction;

typedef enum {
    PLAYER_IDLE = 0,
    PLAYER_MOVING,
    PLAYER_DEAD
} PlayerState;

typedef enum {
    BONUS_NONE = 0,
    BONUS_EXTRA_BOMB,
    BONUS_EXTRA_RANGE
} BonusType;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    bool active;
    int id;
    int x;
    int y;
    int owner_id;
    int range;
    int timer;
    bool blocking;
} Bomb;

typedef struct {
    bool alive;
    int id;
    int x;
    int y;
    int spawn_x;
    int spawn_y;
    int score;
    int bomb_range;
    int max_active_bombs;
    int active_bombs;
    Direction facing;
    PlayerState state;
} Player;

typedef struct {
    bool active;
    Position cells[MAX_EXPLOSION_CELLS];
    int cell_count;
    int timer;
} Explosion;

typedef struct {
    TileType base_map[MAP_HEIGHT][MAP_WIDTH];
    TileType display_map[MAP_HEIGHT][MAP_WIDTH];
    Player players[MAX_PLAYERS];
    Bomb bombs[MAX_BOMBS];
    Explosion explosion;
    int player_count;
    bool round_over;
    int winner_id;
    uint32_t tick;
} GameState;

typedef struct {
    bool move_up;
    bool move_down;
    bool move_left;
    bool move_right;
    bool place_bomb;
} InputCommand;

void game_init(GameState *game, int player_count);
void game_reset_round(GameState *game);
void game_tick(GameState *game, const InputCommand commands[MAX_PLAYERS]);
bool game_move_player(GameState *game, int player_id, Direction dir);
bool game_place_bomb(GameState *game, int player_id);
void game_trigger_bomb(GameState *game, int bomb_index);
bool game_is_round_over(const GameState *game);
int game_get_winner(const GameState *game);
bool game_is_cell_walkable(const GameState *game, int x, int y, int player_id);
BonusType game_collect_bonus(GameState *game, Player *player);

#ifdef __cplusplus
}
#endif

#endif