#ifndef INPUT_H
#define INPUT_H

#include "common.h"

#define MAX_PSEUDO_LENGTH 32

/* ===== Module Input (Clavier/Souris) ===== */

typedef struct {
    Direction move_dir;
    int bomb_action;    /* 1 = poser une bombe, 0 sinon */
    int quit_requested;
} InputState;

/* Initialiser le système d'input */
int input_init(void);

/* Récupérer les entrées du joueur local */
InputState input_get_state(void);

/* Obtenir les pseudos des joueurs */
void input_get_player_names(char pseudos[MAX_PLAYERS][MAX_PSEUDO_LENGTH]);

/* Finaliser le système d'input */
void input_cleanup(void);

#endif
