#include "../../include/input.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>

/* ===== Module Input (Clavier/Souris) ===== */
/* Gestion du clavier pour capturer les mouvements et actions */

static int is_initialized = 0;
static uint8_t key_state[256];  /* État de chaque touche */

int input_init(void) {
    printf("[INPUT] Initialisation du système d'entrée...\n");
    memset(key_state, 0, sizeof(key_state));
    is_initialized = 1;
    return 0;
}

InputState input_get_state(void) {
    InputState state = {
        .move_dir = IDLE,
        .bomb_action = 0,
        .quit_requested = 0
    };
    
    if (!is_initialized) {
        return state;
    }
    
    /* Vérifier les touches disponibles */
    if (_kbhit()) {
        int key = _getch();
        
        switch (key) {
            /* Mouvements avec flèches */
            case 72:  /* Flèche haut */
                state.move_dir = UP;
                break;
            case 80:  /* Flèche bas */
                state.move_dir = DOWN;
                break;
            case 75:  /* Flèche gauche */
                state.move_dir = LEFT;
                break;
            case 77:  /* Flèche droite */
                state.move_dir = RIGHT;
                break;
            
            /* Mouvements avec ZQSD (AZERTY français) */
            case 'z':
            case 'Z':
                state.move_dir = UP;
                break;
            case 's':
            case 'S':
                state.move_dir = DOWN;
                break;
            case 'q':
            case 'Q':
                state.move_dir = LEFT;
                break;
            case 'd':
            case 'D':
                state.move_dir = RIGHT;
                break;
        
            
            /* Actions */
            case ' ':  /* Espace pour poser une bombe */
                state.bomb_action = 1;
                break;
            
            case 27:   /* Échap pour quitter */
            case 'q':
            case 'Q':
                state.quit_requested = 1;
                break;
            
            default:
                break;
        }
    }
    
    return state;
}

void input_get_player_names(char pseudos[MAX_PLAYERS][MAX_PSEUDO_LENGTH]) {
    printf("\n====================================\n");
    printf("     CONFIGURATION DES PSEUDOS\n");
    printf("====================================\n\n");
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("Joueur %d - Entrez votre pseudo (max %d caractères): ", i + 1, MAX_PSEUDO_LENGTH - 1);
        fflush(stdout);
        
        /* Lire une ligne de l'utilisateur */
        if (fgets(pseudos[i], MAX_PSEUDO_LENGTH, stdin) != NULL) {
            /* Supprimer le '\n' à la fin */
            size_t len = strlen(pseudos[i]);
            if (len > 0 && pseudos[i][len - 1] == '\n') {
                pseudos[i][len - 1] = '\0';
            }
        }
        
        /* Vérifier que le pseudo n'est pas vide */
        if (strlen(pseudos[i]) == 0) {
            sprintf(pseudos[i], "Joueur_%d", i + 1);
            printf("  -> Pseudo par défaut: %s\n", pseudos[i]);
        } else {
            printf("  -> Pseudo enregistré: %s\n", pseudos[i]);
        }
    }
    
    printf("\n====================================\n");
    printf("Pseudos confirmés! Appuyez sur une touche...\n");
    if (getchar()) {}  /* Attendre une touche */
    printf("====================================\n\n");
}

void input_cleanup(void) {
    printf("[INPUT] Nettoyage du système d'entrée...\n");
    is_initialized = 0;
}
