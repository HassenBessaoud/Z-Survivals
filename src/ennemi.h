#ifndef ENNEMI_H
#define ENNEMI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Forward declaration
struct GameManager;

// --- Structure Ennemi ---
typedef struct {
    SDL_Rect posEnnemi;
    SDL_Texture* texture;
    int vitesse;
    int direction; // 0: gauche, 1: droite
    int active;
    int vie;
    int destructible;
    int hit;
    // Animation
    SDL_Rect frame;
    int currentFrame;
    int animTimer;
    int animSpeed;
} Ennemi;

void initEnnemie(Ennemi* e, struct GameManager* game, int x, int y);
void updateEnnemie(Ennemi* e, struct GameManager* game, Uint32 dt);
void renderEnnemie(Ennemi* e, struct GameManager* game, SDL_Rect camera);
void handleEnnemieAI(Ennemi* e, SDL_Rect playerPos);

#endif
