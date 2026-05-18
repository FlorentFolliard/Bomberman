#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

/* ===== Constantes ===== */
#define GRID_WIDTH 13
#define GRID_HEIGHT 13
#define TILE_SIZE 32
#define MAX_PLAYERS 4
#define MAX_BOMBS 40

/* ===== Types énumérés ===== */
typedef enum {
    PLAYER_1,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4
} PlayerID;

typedef enum {
    EMPTY,
    WALL,
    DESTRUCTIBLE,
    BOMB,
    FLAME
} TileType;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    IDLE
} Direction;

/* ===== Structures ===== */
typedef struct {
    int x, y;           /* Position en tiles (0-12) */
    Direction dir;      /* Direction du mouvement */
    int alive;
    uint8_t bomb_count; /* Nombre de bombes disponibles */
} Player;

typedef struct {
    int x, y;           /* Position en tiles */
    int timer;          /* Temps avant explosion */
} Bomb;

typedef struct {
    Player players[MAX_PLAYERS];
    Bomb bombs[MAX_BOMBS];
    TileType grid[GRID_HEIGHT][GRID_WIDTH];
    int active_players;
    int current_frame;
} GameState;

#endif
