#include "ennemi.h"
#include "game.h"
#include "background.h"
#include <stdio.h>

void initEnnemie(Ennemi* e, GameManager* game, int x, int y) {
    if (!e || !game) return;
    e->posEnnemi.x = x;
    e->posEnnemi.y = y;
    e->posEnnemi.w = 36;
    e->posEnnemi.h = 44;
    e->vitesse = 0;
    e->direction = 0;
    e->destructible = 1;
    e->active = 1;
    e->hit = 0;
    e->animSpeed = 100;
    e->animTimer = 0;
    e->currentFrame = 0;
    
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "baril.png");
    e->texture = IMG_LoadTexture(game->renderer, path);
}

void updateEnnemie(Ennemi* e, GameManager* game, Uint32 dt) {
    if (!e || !game || !e->active) return;

    if (e->direction == 0) {
        e->posEnnemi.x -= e->vitesse;
        if (e->posEnnemi.x < 100) e->direction = 1;
    } else {
        e->posEnnemi.x += e->vitesse;
        if (e->posEnnemi.x > LEVEL_WIDTH - 100) e->direction = 0;
    }

    e->animTimer += dt;
    if (e->animTimer >= (Uint32)e->animSpeed) {
        e->currentFrame = (e->currentFrame + 1) % 4;
        e->frame.x = e->currentFrame * e->frame.w;
        e->animTimer = 0;
    }

    handleEnnemieAI(e, game->player.pos);
}

void renderEnnemie(Ennemi* e, GameManager* game, SDL_Rect camera) {
    if (!e || !game || !e->active) return;
    SDL_Rect dest = {e->posEnnemi.x - camera.x, e->posEnnemi.y - camera.y, e->posEnnemi.w, e->posEnnemi.h};
    SDL_RenderCopy(game->renderer, e->texture, &e->frame, &dest);
}

void handleEnnemieAI(Ennemi* e, SDL_Rect playerPos) {
    if (!e->active) return;
    
    if (playerPos.x < e->posEnnemi.x) e->direction = 0;
    else e->direction = 1;
}
