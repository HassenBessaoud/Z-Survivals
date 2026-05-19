#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Forward declaration
struct GameManager;

// ================= CONSTANTES =================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define LEVEL_WIDTH 6000.0f
#define LEVEL_HEIGHT 1500.0f
#define GROUND_Y 520
#define MOVE_SPEED 6.0f
#define GRAVITY 0.6f
#define JUMP_FORCE -15.0f

#define ASSETS_PATH "assets/group4/"

// ================= ANIMATIONS =================
#define MAX_ANIMATIONS 10

typedef struct {
    SDL_Texture* sprite;
    SDL_Rect posSprite;
    SDL_Rect posScreen;
    int nbFrames;
    int currentFrame;
    int direction;
    int active;
    int animSpeed;
    int animTimer;
    int spriteSheetW;
    int spriteSheetH;
} AnimBackground;

// ================= FONCTIONS =================

/* --- Gestion des Niveaux --- */
void initGameplay(struct GameManager* game);
void loadLevel(struct GameManager* game, int levelNum);
void updateGameplay(struct GameManager* game);
void renderGameplay(struct GameManager* game);
void spawnBonus(struct GameManager* game, int x, int y, int type);

/* --- Animations --- */
void initAnimations(struct GameManager* game);
void loadAnimationMedia(struct GameManager* game);
void lancerAnimation(struct GameManager* game, int x, int y);
void updateAnimations(struct GameManager* game);
void renderAnimations(struct GameManager* game);
void cleanupAnimations(struct GameManager* game);

/* --- Histoire et Regles --- */
void initHistoireMenu(struct GameManager* game);
void loadHistoireMedia(struct GameManager* game);
void renderHistoire(struct GameManager* game);
void handleHistoireKeyPress(struct GameManager* game, SDL_Keycode key);
void handleHistoireMouseMotion(struct GameManager* game, int x, int y);
void handleHistoireMouseClick(struct GameManager* game, int x, int y);
void cleanupHistoireMenu(struct GameManager* game);

#endif
