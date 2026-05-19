#include "joueur.h"
#include "game.h"
#include "background.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

// ================= IMPLEMENTATION JOUEUR =================

void initJoueur(Joueur* j, GameManager* game) {
    if (!j || !game) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "hero_spritesheet.png");
    
    j->spriteSheet = IMG_LoadTexture(game->renderer, path);
    j->texture = j->spriteSheet; 
    if (!j->spriteSheet) {
        printf("Erreur chargement texture joueur: %s\n", IMG_GetError());
    }

    j->x = 100.0f;
    j->y = GROUND_Y - 48;
    j->w = 32; 
    j->h = 48; 
    
    j->pos.x = (int)j->x;
    j->pos.y = (int)j->y;
    j->pos.w = j->w;
    j->pos.h = j->h;

    j->hitbox.w = j->w;
    j->hitbox.h = j->h;

    j->spriteRect.x = 0;
    j->spriteRect.y = 0;
    j->spriteRect.w = 128; 
    j->spriteRect.h = 127; 

    j->vx = 0;
    j->vy = 0;
    j->speed = MOVE_SPEED;
    j->gravity = GRAVITY;
    j->onGround = 0;
    j->isJumping = 0;
    j->facingRight = 1;
    j->direction = 0;
    j->keyLeft = 0;
    j->keyRight = 0;

    j->currentFrame = 0;
    j->totalFrames = 8;
    j->lastAnimTime = 0;
    j->animTimer = 0;
    j->animSpeed = 100;

    j->health = 100;
    j->maxHealth = 100;
    j->score = 0;
    j->lives = 2;
    j->vies = 2;
    j->ammo = 100;

    j->isInvincible = 0;
    j->invincible = 0;
    j->invincibleTimer = 0;
}

void updateJoueur(Joueur* j, GameManager* game, Uint32 dt) {
    if (!j || !game || game->gameOver) return;
    (void)dt;

    j->vx = (j->keyRight - j->keyLeft) * j->speed;

    j->vy += j->gravity;
    if (j->vy > 15) j->vy = 15;
    j->y += j->vy;
    j->x += j->vx;

    if (j->x < 0) j->x = 0;
    if (j->x > LEVEL_WIDTH - j->w) j->x = LEVEL_WIDTH - j->w;
    
    j->pos.x = (int)j->x;
    j->pos.y = (int)j->y;

    j->hitbox.x = j->pos.x;
    j->hitbox.y = j->pos.y;

    Uint32 currentTime = SDL_GetTicks();
    
    j->animTimer++;
    if (j->vx != 0) {
        j->direction = (j->vx > 0) ? 0 : 1;
        j->facingRight = (j->vx > 0);
        if (j->animTimer > 8) {
            j->currentFrame = (j->currentFrame + 1) % j->totalFrames;
            j->animTimer = 0;
        }
    } else if (!j->onGround) {
        if (j->animTimer > 8) {
            j->currentFrame = (j->currentFrame + 1) % j->totalFrames;
            j->animTimer = 0;
        }
    } else {
        if (j->animTimer > 12) {
            j->currentFrame = (j->currentFrame + 1) % j->totalFrames;
            j->animTimer = 0;
        }
    }

    j->spriteRect.x = j->currentFrame * j->spriteRect.w;
    j->spriteRect.y = j->direction * j->spriteRect.h;

    if (j->isInvincible) {
        if (currentTime > j->invincibleTimer + 2000) {
            j->isInvincible = 0;
            j->invincible = 0;
        }
    }
}

void renderJoueur(Joueur* j, GameManager* game, SDL_Rect camera) {
    if (!j || !game) return;
    
    SDL_Rect dest = {j->pos.x - camera.x - 40, j->pos.y - camera.y - 92, 112, 140};
    
    if (j->isInvincible && (SDL_GetTicks() / 100) % 2 == 0) {
        return;
    }
    
    SDL_RendererFlip flip = j->facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(game->renderer, j->spriteSheet, &j->spriteRect, &dest, 0.0, NULL, flip);
}

void handleJoueurInput(Joueur* j, GameManager* game, SDL_Event* event) {
    if (!j || !game || !event || game->gameOver) return;
    
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_q:
                j->keyLeft = 1;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                j->keyRight = 1;
                break;
            case SDLK_SPACE:
            case SDLK_z:
            case SDLK_UP:
                joueurJump(j);
                break;
            case SDLK_f:
                joueurShoot(j, game);
                break;
        }
    } else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_q:
                j->keyLeft = 0;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                j->keyRight = 0;
                break;
        }
    }
}

void joueurJump(Joueur* j) {
    if (j->onGround) {
        j->vy = JUMP_FORCE;
        j->onGround = 0;
        j->isJumping = 1;
        j->y -= 5;
    }
}

void joueurShoot(Joueur* j, GameManager* game) {
    if (!j || !game || j->ammo <= 0) return;
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!game->bullets[i].active) {
            game->bullets[i].active = 1;
            float start_x = (float)(j->pos.x + (j->facingRight ? j->w : -20));
            float start_y = (float)(j->pos.y + j->h / 2);
            game->bullets[i].rect.x = (int)start_x;
            game->bullets[i].rect.y = (int)start_y;
            game->bullets[i].rect.w = 20;
            game->bullets[i].rect.h = 10;
            
            int targetX = -1, targetY = -1;
            float min_dist = 1e9f;
            float max_range = 600.0f;
            float range_sq = max_range * max_range;

            if (game->bossActive && game->boss.active) {
                float dx = (game->boss.posEnnemi.x + game->boss.posEnnemi.w/2) - start_x;
                float dy = (game->boss.posEnnemi.y + game->boss.posEnnemi.h/2) - start_y;
                float dist = dx*dx + dy*dy;
                if (dist < range_sq) {
                    min_dist = dist;
                    targetX = game->boss.posEnnemi.x + game->boss.posEnnemi.w/2;
                    targetY = game->boss.posEnnemi.y + game->boss.posEnnemi.h/2;
                }
            }

            if (targetX == -1) {
                for (int k = 0; k < MAX_ZOMBIES; k++) {
                    if (game->zombies[k].active) {
                        float dx = (game->zombies[k].posEnnemi.x + game->zombies[k].posEnnemi.w/2) - start_x;
                        float dy = (game->zombies[k].posEnnemi.y + game->zombies[k].posEnnemi.h/2) - start_y;
                        float dist = dx*dx + dy*dy;
                        if (dist < min_dist && dist < range_sq) {
                            min_dist = dist;
                            targetX = game->zombies[k].posEnnemi.x + game->zombies[k].posEnnemi.w/2;
                            targetY = game->zombies[k].posEnnemi.y + game->zombies[k].posEnnemi.h/2;
                        }
                    }
                }
            }

            if (targetX == -1) {
                for (int k = 0; k < MAX_ATTACKS; k++) {
                    if (game->attacks[k].active) {
                        float dx = (game->attacks[k].rect.x + game->attacks[k].rect.w/2) - start_x;
                        float dy = (game->attacks[k].rect.y + game->attacks[k].rect.h/2) - start_y;
                        float dist = dx*dx + dy*dy;
                        if (dist < min_dist && dist < range_sq) {
                            min_dist = dist;
                            targetX = game->attacks[k].rect.x + game->attacks[k].rect.w/2;
                            targetY = game->attacks[k].rect.y + game->attacks[k].rect.h/2;
                        }
                    }
                }
            }

            if (targetX == -1 && j->niveau == 1) {
                for (int k = 0; k < MAX_ENEMIES; k++) {
                    if (game->enemies[k].active) {
                        float dx = (game->enemies[k].posEnnemi.x + game->enemies[k].posEnnemi.w/2) - start_x;
                        float dy = (game->enemies[k].posEnnemi.y + game->enemies[k].posEnnemi.h/2) - start_y;
                        float dist = dx*dx + dy*dy;
                        if (dist < min_dist && dist < range_sq) {
                            min_dist = dist;
                            targetX = game->enemies[k].posEnnemi.x + game->enemies[k].posEnnemi.w/2;
                            targetY = game->enemies[k].posEnnemi.y + game->enemies[k].posEnnemi.h/2;
                        }
                    }
                }
            }

            if (targetX != -1) {
                float dx = (float)targetX - start_x;
                float dy = (float)targetY - start_y;
                float len = sqrtf(dx*dx + dy*dy);
                game->bullets[i].vx = (dx / len) * 12.0f;
                game->bullets[i].vy = (dy / len) * 12.0f;
            } else {
                game->bullets[i].vx = j->facingRight ? 12.0f : -12.0f;
                game->bullets[i].vy = 0;
            }
            
            j->ammo--;
            break;
        }
    }
}

void joueurTakeDamage(Joueur* j, int damage) {
    if (!j || j->isInvincible) return;
    j->health -= damage;
    j->isInvincible = 1;
    j->invincibleTimer = SDL_GetTicks();
    if (j->health <= 0) {
        j->lives--;
        j->vies = j->lives; 
        if (j->lives > 0) {
            j->health = 100;
        } else {
            j->health = 0; 
        }
    }
}

void cleanupJoueur(Joueur* j) {
    if (j && j->spriteSheet) {
        SDL_DestroyTexture(j->spriteSheet);
        j->spriteSheet = NULL;
        j->texture = NULL;
    }
}

// ================= IMPLEMENTATION MENU SELECTION =================

void playerMono(GameManager* game) {
    if (!game) return;
    game->playerMenuState.selectedPlayerMode = 0; game->playerMenuState.showAvatarChoices = 1;
    for (int i = 2; i <= 6; i++) game->playerMenuState.playerMenu.buttons[i].visible = 1;
    game->playerMenuState.playerMenu.buttons[4].visible = 0;
    game->playerMenuState.playerMenu.buttons[5].visible = 0;
}

void playerMulti(GameManager* game) {
    if (!game) return;
    game->playerMenuState.selectedPlayerMode = 1; game->playerMenuState.showAvatarChoices = 1;
    for (int i = 2; i <= 6; i++) game->playerMenuState.playerMenu.buttons[i].visible = 1;
}

void playerAvatar1J1(GameManager* game) { if (game) game->playerMenuState.selectedAvatar1 = 1; }
void playerAvatar2J1(GameManager* game) { if (game) game->playerMenuState.selectedAvatar1 = 2; }
void playerAvatar1J2(GameManager* game) { if (game) game->playerMenuState.selectedAvatar2 = 1; }
void playerAvatar2J2(GameManager* game) { if (game) game->playerMenuState.selectedAvatar2 = 2; }
void playerValider(GameManager* game) {
    if (!game) return;
    game->gameState = MENU_SCORES;
    changeMusic(game, MENU_SCORES);
}

void playerRetour(GameManager* game) {
    if (!game) return;
    game->gameState = MENU_MAIN;
    game->playerMenuState.showAvatarChoices = 0;
    for (int i = 2; i <= 6; i++) game->playerMenuState.playerMenu.buttons[i].visible = 0;
    changeMusic(game, MENU_MAIN);
}

void initPlayerMenu(GameManager* game) {
    if (!game) return;
    strcpy(game->playerMenuState.playerMenu.title, "CHOIX DU JOUEUR");
    game->playerMenuState.playerMenu.buttonCount = 8;
    game->playerMenuState.playerMenu.buttons[0] = (Button){{175, 200, 200, 55}, "Mono joueur",   0, playerMono,      NULL, NULL, 0, 1};
    game->playerMenuState.playerMenu.buttons[1] = (Button){{425, 200, 200, 55}, "Multi joueurs", 0, playerMulti,     NULL, NULL, 0, 1};
    game->playerMenuState.playerMenu.buttons[2] = (Button){{195, 310, 180, 50}, "Avatar 1 (J1)", 0, playerAvatar1J1, NULL, NULL, 0, 0};
    game->playerMenuState.playerMenu.buttons[3] = (Button){{425, 310, 180, 50}, "Avatar 2 (J1)", 0, playerAvatar2J1, NULL, NULL, 0, 0};
    game->playerMenuState.playerMenu.buttons[4] = (Button){{195, 380, 180, 50}, "Avatar 1 (J2)", 0, playerAvatar1J2, NULL, NULL, 0, 0};
    game->playerMenuState.playerMenu.buttons[5] = (Button){{425, 380, 180, 50}, "Avatar 2 (J2)", 0, playerAvatar2J2, NULL, NULL, 0, 0};
    game->playerMenuState.playerMenu.buttons[6] = (Button){{275, 460, 250, 60}, "Valider",       0, playerValider,   NULL, NULL, 0, 0};
    game->playerMenuState.playerMenu.buttons[7] = (Button){{275, 540, 250, 60}, "Retour",        0, playerRetour,    NULL, NULL, 0, 1};
}

void loadPlayerMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "a2.jpg");
    game->playerMenuState.playerMenu.background = IMG_LoadTexture(game->renderer, path);
    sprintf(path, "%s%s", ASSETS_PATH, "music_option.mp3");
    game->playerMenuState.playerMenu.music = Mix_LoadMUS(path);
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[0], "mono1.jpg",  "mono11.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[1], "multi1.jpg", "multi11.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[2], "av1.jpg",    "av11.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[3], "av2.jpg",    "av22.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[4], "av1.jpg",    "av11.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[5], "av2.jpg",    "av22.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[6], "d1.jpg",     "d22.jpg");
    loadButtonTexture(game, &game->playerMenuState.playerMenu.buttons[7], "ret1.jpg",   "ret11.jpg");
}

void handlePlayerKeyPress(GameManager* game, SDL_Keycode key) {
    if (key == SDLK_RETURN) playerValider(game);
    else if (key == SDLK_ESCAPE) playerRetour(game);
}

void handlePlayerMouseMotion(GameManager* game, int x, int y) { if (game) handleMenuMouseMotion(game, &game->playerMenuState.playerMenu, x, y); }
void handlePlayerMouseClick(GameManager* game, int x, int y) { if (game) handleMenuMouseClick(game, &game->playerMenuState.playerMenu, x, y); }

void renderPlayerMenu(GameManager* game) {
    if (!game) return;
    if (game->playerMenuState.playerMenu.background) SDL_RenderCopy(game->renderer, game->playerMenuState.playerMenu.background, NULL, NULL);
    renderText(game, game->playerMenuState.playerMenu.title, 300, 80, (SDL_Color){255,255,255,255}, game->titleFont);
    if (game->playerMenuState.showAvatarChoices) renderText(game, "Choisissez vos avatars", 300, 270, (SDL_Color){255,255,255,255}, game->font);
    for (int i = 0; i < game->playerMenuState.playerMenu.buttonCount; i++) renderButton(game, &game->playerMenuState.playerMenu.buttons[i]);
}

void cleanupPlayerMenu(GameManager* game) { if (game) cleanupMenu(&game->playerMenuState.playerMenu); }
