#include "game_logic.h"

#include <stddef.h>
#include <string.h>

static bool in_bounds(int x, int y) {
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

static bool is_spawn_safe_zone(int x, int y) {
    Position safe_cells[] = {
        {1, 1}, {2, 1}, {1, 2},
        {MAP_WIDTH - 2, 1}, {MAP_WIDTH - 3, 1}, {MAP_WIDTH - 2, 2},
        {1, MAP_HEIGHT - 2}, {2, MAP_HEIGHT - 2}, {1, MAP_HEIGHT - 3},
        {MAP_WIDTH - 2, MAP_HEIGHT - 2}, {MAP_WIDTH - 3, MAP_HEIGHT - 2}, {MAP_WIDTH - 2, MAP_HEIGHT - 3}
    };
    size_t count = sizeof(safe_cells) / sizeof(safe_cells[0]);
    for (size_t i = 0; i < count; ++i) {
        if (safe_cells[i].x == x && safe_cells[i].y == y) {
            return true;
        }
    }
    return false;
}

static void rebuild_display_map(GameState *game) {
    memcpy(game->display_map, game->base_map, sizeof(game->base_map));
    if (game->explosion.active) {
        for (int i = 0; i < game->explosion.cell_count; ++i) {
            int x = game->explosion.cells[i].x;
            int y = game->explosion.cells[i].y;
            if (in_bounds(x, y)) {
                game->display_map[y][x] = TILE_FIRE;
            }
        }
    }
}

static void clear_explosion(GameState *game) {
    game->explosion.active = false;
    game->explosion.cell_count = 0;
    game->explosion.timer = 0;
    rebuild_display_map(game);
}

static void add_explosion_cell(GameState *game, int x, int y) {
    if (!in_bounds(x, y) || game->explosion.cell_count >= MAX_EXPLOSION_CELLS) {
        return;
    }
    for (int i = 0; i < game->explosion.cell_count; ++i) {
        if (game->explosion.cells[i].x == x && game->explosion.cells[i].y == y) {
            return;
        }
    }
    game->explosion.cells[game->explosion.cell_count].x = x;
    game->explosion.cells[game->explosion.cell_count].y = y;
    game->explosion.cell_count++;
}

static void apply_bonus(Player *player, BonusType bonus) {
    switch (bonus) {
        case BONUS_EXTRA_BOMB:
            player->max_active_bombs++;
            break;
        case BONUS_EXTRA_RANGE:
            player->bomb_range++;
            break;
        default:
            break;
    }
}

BonusType game_collect_bonus(GameState *game, Player *player) {
    TileType tile = game->base_map[player->y][player->x];
    BonusType bonus = BONUS_NONE;
    if (tile == TILE_BONUS_BOMB) {
        bonus = BONUS_EXTRA_BOMB;
    } else if (tile == TILE_BONUS_RANGE) {
        bonus = BONUS_EXTRA_RANGE;
    }
    if (bonus != BONUS_NONE) {
        apply_bonus(player, bonus);
        game->base_map[player->y][player->x] = TILE_EMPTY;
        rebuild_display_map(game);
    }
    return bonus;
}

static void init_map(GameState *game) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                game->base_map[y][x] = TILE_WALL;
            } else if (x % 2 == 0 && y % 2 == 0) {
                game->base_map[y][x] = TILE_WALL;
            } else if (!is_spawn_safe_zone(x, y) && ((x + y) % 3 == 0)) {
                game->base_map[y][x] = TILE_BREAKABLE;
            } else {
                game->base_map[y][x] = TILE_EMPTY;
            }
        }
    }
}

static void init_players(GameState *game) {
    Position spawns[MAX_PLAYERS] = {
        {1, 1},
        {MAP_WIDTH - 2, 1},
        {1, MAP_HEIGHT - 2},
        {MAP_WIDTH - 2, MAP_HEIGHT - 2}
    };
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        Player *player = &game->players[i];
        player->id = i;
        player->spawn_x = spawns[i].x;
        player->spawn_y = spawns[i].y;
        player->x = spawns[i].x;
        player->y = spawns[i].y;
        player->score = 0;
        player->bomb_range = DEFAULT_BOMB_RANGE;
        player->max_active_bombs = DEFAULT_MAX_ACTIVE_BOMBS;
        player->active_bombs = 0;
        player->alive = i < game->player_count;
        player->facing = DIR_DOWN;
        player->state = PLAYER_IDLE;
    }
}

void game_init(GameState *game, int player_count) {
    memset(game, 0, sizeof(*game));
    if (player_count < 2) {
        player_count = 2;
    }
    if (player_count > MAX_PLAYERS) {
        player_count = MAX_PLAYERS;
    }
    game->player_count = player_count;
    game->winner_id = -1;
    init_map(game);
    init_players(game);
    rebuild_display_map(game);
}

void game_reset_round(GameState *game) {
    int players = game->player_count;
    game_init(game, players);
}

bool game_is_cell_walkable(const GameState *game, int x, int y, int player_id) {
    if (!in_bounds(x, y)) {
        return false;
    }
    TileType tile = game->base_map[y][x];
    if (tile == TILE_WALL || tile == TILE_BREAKABLE) {
        return false;
    }
    for (int i = 0; i < MAX_BOMBS; ++i) {
        const Bomb *bomb = &game->bombs[i];
        if (bomb->active && bomb->x == x && bomb->y == y && bomb->blocking) {
            return false;
        }
    }
    for (int i = 0; i < game->player_count; ++i) {
        if (i != player_id && game->players[i].alive && game->players[i].x == x && game->players[i].y == y) {
            return false;
        }
    }
    return true;
}

bool game_move_player(GameState *game, int player_id, Direction dir) {
    if (player_id < 0 || player_id >= game->player_count) {
        return false;
    }
    Player *player = &game->players[player_id];
    if (!player->alive) {
        return false;
    }

    int nx = player->x;
    int ny = player->y;
    switch (dir) {
        case DIR_UP: ny--; break;
        case DIR_DOWN: ny++; break;
        case DIR_LEFT: nx--; break;
        case DIR_RIGHT: nx++; break;
        default: return false;
    }

    player->facing = dir;
    if (!game_is_cell_walkable(game, nx, ny, player_id)) {
        player->state = PLAYER_IDLE;
        return false;
    }

    player->x = nx;
    player->y = ny;
    player->state = PLAYER_MOVING;
    game_collect_bonus(game, player);
    return true;
}

bool game_place_bomb(GameState *game, int player_id) {
    if (player_id < 0 || player_id >= game->player_count) {
        return false;
    }
    Player *player = &game->players[player_id];
    if (!player->alive || player->active_bombs >= player->max_active_bombs) {
        return false;
    }

    for (int i = 0; i < MAX_BOMBS; ++i) {
        if (game->bombs[i].active && game->bombs[i].x == player->x && game->bombs[i].y == player->y) {
            return false;
        }
    }

    for (int i = 0; i < MAX_BOMBS; ++i) {
        Bomb *bomb = &game->bombs[i];
        if (!bomb->active) {
            bomb->active = true;
            bomb->id = i;
            bomb->x = player->x;
            bomb->y = player->y;
            bomb->owner_id = player_id;
            bomb->range = player->bomb_range;
            bomb->timer = BOMB_TIMER_TICKS;
            bomb->blocking = false;
            player->active_bombs++;
            return true;
        }
    }
    return false;
}

static void release_bomb_blocking(GameState *game) {
    for (int i = 0; i < MAX_BOMBS; ++i) {
        Bomb *bomb = &game->bombs[i];
        if (!bomb->active || bomb->blocking) {
            continue;
        }
        bool owner_still_on_tile = false;
        if (bomb->owner_id >= 0 && bomb->owner_id < game->player_count) {
            Player *owner = &game->players[bomb->owner_id];
            owner_still_on_tile = owner->alive && owner->x == bomb->x && owner->y == bomb->y;
        }
        if (!owner_still_on_tile) {
            bomb->blocking = true;
        }
    }
}

static void kill_players_in_fire(GameState *game) {
    for (int i = 0; i < game->player_count; ++i) {
        Player *player = &game->players[i];
        if (!player->alive) {
            continue;
        }
        for (int j = 0; j < game->explosion.cell_count; ++j) {
            if (game->explosion.cells[j].x == player->x && game->explosion.cells[j].y == player->y) {
                player->alive = false;
                player->state = PLAYER_DEAD;
                break;
            }
        }
    }
}

static void update_round_state(GameState *game) {
    int alive_count = 0;
    int last_alive = -1;
    for (int i = 0; i < game->player_count; ++i) {
        if (game->players[i].alive) {
            alive_count++;
            last_alive = i;
        }
    }
    game->round_over = alive_count <= 1;
    game->winner_id = (alive_count == 1) ? last_alive : -1;
    if (game->round_over && game->winner_id >= 0) {
        game->players[game->winner_id].score++;
    }
}

void game_trigger_bomb(GameState *game, int bomb_index) {
    if (bomb_index < 0 || bomb_index >= MAX_BOMBS || !game->bombs[bomb_index].active) {
        return;
    }

    Bomb bomb = game->bombs[bomb_index];
    game->bombs[bomb_index].active = false;
    game->bombs[bomb_index].blocking = false;
    if (bomb.owner_id >= 0 && bomb.owner_id < game->player_count && game->players[bomb.owner_id].active_bombs > 0) {
        game->players[bomb.owner_id].active_bombs--;
    }

    game->explosion.active = true;
    game->explosion.cell_count = 0;
    game->explosion.timer = EXPLOSION_TIMER_TICKS;
    add_explosion_cell(game, bomb.x, bomb.y);

    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};
    for (int dir = 0; dir < 4; ++dir) {
        for (int step = 1; step <= bomb.range; ++step) {
            int x = bomb.x + dx[dir] * step;
            int y = bomb.y + dy[dir] * step;
            if (!in_bounds(x, y)) {
                break;
            }
            TileType tile = game->base_map[y][x];
            if (tile == TILE_WALL) {
                break;
            }
            add_explosion_cell(game, x, y);
            for (int b = 0; b < MAX_BOMBS; ++b) {
                if (game->bombs[b].active && game->bombs[b].x == x && game->bombs[b].y == y) {
                    game_trigger_bomb(game, b);
                }
            }
            if (tile == TILE_BREAKABLE) {
                game->base_map[y][x] = ((x + y) % 2 == 0) ? TILE_BONUS_RANGE : TILE_BONUS_BOMB;
                break;
            }
            if (tile == TILE_BONUS_BOMB || tile == TILE_BONUS_RANGE) {
                game->base_map[y][x] = TILE_EMPTY;
            }
        }
    }

    rebuild_display_map(game);
    kill_players_in_fire(game);
    update_round_state(game);
}

static Direction command_to_direction(const InputCommand *cmd) {
    if (cmd->move_up) return DIR_UP;
    if (cmd->move_down) return DIR_DOWN;
    if (cmd->move_left) return DIR_LEFT;
    if (cmd->move_right) return DIR_RIGHT;
    return DIR_NONE;
}

void game_tick(GameState *game, const InputCommand commands[MAX_PLAYERS]) {
    if (game->round_over) {
        return;
    }

    game->tick++;
    release_bomb_blocking(game);

    for (int i = 0; i < game->player_count; ++i) {
        Player *player = &game->players[i];
        if (!player->alive) {
            continue;
        }
        player->state = PLAYER_IDLE;
        Direction dir = command_to_direction(&commands[i]);
        if (dir != DIR_NONE) {
            game_move_player(game, i, dir);
        }
        if (commands[i].place_bomb) {
            game_place_bomb(game, i);
        }
    }

    for (int i = 0; i < MAX_BOMBS; ++i) {
        Bomb *bomb = &game->bombs[i];
        if (!bomb->active) {
            continue;
        }
        bomb->timer--;
        if (bomb->timer <= 0) {
            game_trigger_bomb(game, i);
        }
    }

    if (game->explosion.active) {
        game->explosion.timer--;
        kill_players_in_fire(game);
        if (game->explosion.timer <= 0) {
            clear_explosion(game);
        } else {
            rebuild_display_map(game);
        }
    }

    update_round_state(game);
}

bool game_is_round_over(const GameState *game) {
    return game->round_over;
}

int game_get_winner(const GameState *game) {
    return game->winner_id;
}