#include "background.h"
#include "game.h"
#include "joueur.h"
#include "ennemi.h"
#include "minimap.h"
#include "enigme.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// ================= GESTION DES NIVEAUX =================

void initGameplay(GameManager* game) {
    if (!game) return;
    game->gameplayState.gameplayActive = 1;
    
    initJoueur(&game->player, game);
    game->player.pos.x = 100;
    game->player.pos.y = GROUND_Y - 48;
    game->player.w = 32;
    game->player.h = 48;
    game->player.lives = 2;
    game->player.vies = 2;
    game->player.health = 100;
    game->player.score = 0;
    game->player.ammo = 30;
    game->player.niveau = 1;

    game->camera.x = 0;
    game->camera.y = 0;
    game->camera.w = SCREEN_WIDTH;
    game->camera.h = SCREEN_HEIGHT;
}

void loadLevel(GameManager* game, int levelNum) {
    if (!game) return;
    if (game->background) {
        SDL_DestroyTexture(game->background);
        game->background = NULL;
    }
    
    char path[256];
    if (levelNum == 1) {
        sprintf(path, "%s%s", ASSETS_PATH, "background.png");
    } else if (levelNum == 2) {
        sprintf(path, "assets/group4/niv2.jpg");
    } else if (levelNum == 3) {
        sprintf(path, "assets/group4/level2_bg.png");
    } else {
        sprintf(path, "assets/group4/level%d_bg.png", levelNum);
    }
    game->background = IMG_LoadTexture(game->renderer, path);
    
    if (!game->background) {
        sprintf(path, "assets/level%d_bg.png", levelNum);
        game->background = IMG_LoadTexture(game->renderer, path);
    }
    
    if (!game->background) { 
        sprintf(path, "%s%s", ASSETS_PATH, "a1.jpg"); 
        game->background = IMG_LoadTexture(game->renderer, path);
    }

    switch(levelNum) {
        case 1: game->player.speed = MOVE_SPEED; break;
        case 2: game->player.speed = 8.0f; break;
        case 3: game->player.speed = 10.0f; break;
        default: game->player.speed = 5.0f; break;
    }

    game->player.x = 200.0f;
    game->player.y = GROUND_Y - 48;
    game->player.pos.x = 200;
    game->player.pos.y = GROUND_Y - 48;
    game->player.niveau = levelNum;
    game->currentLevel = levelNum;

    game->victory = 0;
    game->gameOver = 0;
    game->levelStartTime = SDL_GetTicks();
    game->lastAttackSpawn = game->levelStartTime;
    game->lastBonusSpawn = game->levelStartTime;
    game->lastObstacleSpawn = game->levelStartTime;
    game->lastObstacleSpawnX = -1000;
    game->attacksActive = 0;
    game->bonusesSpawned = 0;
    game->lastScoreRewardThreshold = 0;

    if (!game->zombieTex) game->zombieTex = IMG_LoadTexture(game->renderer, "assets/group4/zombiesprite.png");
    if (!game->bulletTex) game->bulletTex = IMG_LoadTexture(game->renderer, "assets/group4/cartoucha.png");
    if (!game->qomblaTex) game->qomblaTex = IMG_LoadTexture(game->renderer, "assets/group4/qombla.png");
    for (int i = 0; i < 6; i++) {
        if (!game->obstacleTex[i]) {
            char oPath[256];
            sprintf(oPath, "assets/group4/v%d.jpg", i + 1);
            game->obstacleTex[i] = IMG_LoadTexture(game->renderer, oPath);
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) game->enemies[i].active = 0;
    for (int i = 0; i < MAX_ZOMBIES; i++) game->zombies[i].active = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) game->obstacles[i].active = 0;
    for (int i = 0; i < MAX_ATTACKS; i++) game->attacks[i].active = 0;
    for (int i = 0; i < MAX_BOSS_PROJ; i++) game->bossProj[i].active = 0;
    game->boss.active = 0;

    if (levelNum == 1) {
        int barilPositions[] = {600, 1200, 1850, 2550, 3200};
        for (int i = 0; i < 5; i++) {
            initEnnemie(&game->enemies[i], game, barilPositions[i], GROUND_Y - 55);
        }
        game->bossActive = 0;
    } else if (levelNum == 3) {
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            game->zombies[i].active = 1;
            game->zombies[i].posEnnemi.w = 60; 
            game->zombies[i].posEnnemi.h = 100;
            game->zombies[i].posEnnemi.x = 1000 + i * 500;
            game->zombies[i].posEnnemi.y = GROUND_Y - 100;
            game->zombies[i].vitesse = 2;
            game->zombies[i].direction = (i % 2 == 0) ? 1 : 0;
            game->zombies[i].vie = 3;
            game->zombies[i].texture = game->zombieTex;
            game->zombies[i].currentFrame = 0;
            game->zombies[i].animTimer = SDL_GetTicks();
        }

        game->boss.active = 1;
        game->boss.posEnnemi.w = 100;
        game->boss.posEnnemi.h = 160;
        game->boss.posEnnemi.x = 3400; 
        game->boss.posEnnemi.y = GROUND_Y - 160;
        game->boss.vitesse = 2;
        game->boss.direction = 0;
        game->boss.vie = 50; 
        game->boss.texture = game->zombieTex;
        game->boss.currentFrame = 0;
        game->boss.animTimer = SDL_GetTicks();
        
        game->bossActive = 1;
        game->bossPhase = 1;
        game->bossLastShot = SDL_GetTicks();
        game->bossSpawnedMinion = 0;
    }

    for (int i = 0; i < MAX_BONUS; i++) {
        game->bonuses[i].active = 0;
        if (levelNum == 1) {
            int platX[] = {300, 750, 1400, 2050, 2750};
            if (i < 5) spawnBonus(game, platX[i] + 50, 300, i % 4);
        } else {
            spawnBonus(game, 1500 + i * 1000, GROUND_Y - 50, i % 4);
        }
    }

    for (int i = 0; i < 2; i++) {
        if (levelNum == 1) {
            game->quizZones[i].active = 1;
            game->quizZones[i].triggered = 0;
            game->quizZones[i].rect = (SDL_Rect){1200 + i * 1500, GROUND_Y - 100, 100, 100};
        } else {
            game->quizZones[i].active = 0;
        }
    }

    if (levelNum == 1) {
        initPlateformes(game);
        loadCollisionMedia(game);
    }
}

void spawnBonus(GameManager* game, int x, int y, int type) {
    if (!game) return;
    for (int i = 0; i < MAX_BONUS; i++) {
        if (!game->bonuses[i].active) {
            game->bonuses[i].active = 1;
            game->bonuses[i].rect = (SDL_Rect){x, y, 40, 40};
            game->bonuses[i].type = type;
            
            char path[256];
            const char* bonusFiles[] = {"health.png", "ammo.png", "time.png", "score.png"};
            sprintf(path, "assets/%s", bonusFiles[type % 4]);
            game->bonuses[i].texture = IMG_LoadTexture(game->renderer, path);
            if (!game->bonuses[i].texture) {
                sprintf(path, "%s%s", ASSETS_PATH, bonusFiles[type % 4]);
                game->bonuses[i].texture = IMG_LoadTexture(game->renderer, path);
            }
            break;
        }
    }
}

void checkCollisions(GameManager* game) {
    if (!game) return;
    handleCollisions(game);

    if (game->player.niveau == 1) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (game->enemies[i].active) {
                if (SDL_HasIntersection(&game->player.hitbox, &game->enemies[i].posEnnemi)) {
                    joueurTakeDamage(&game->player, 10);
                    game->enemies[i].hit = 1;
                }
            }
        }
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            if (SDL_HasIntersection(&game->player.hitbox, &game->obstacles[i].rect)) {
                joueurTakeDamage(&game->player, 10);
                game->player.score -= 10;
                if (game->player.score < 0) game->player.score = 0;
                game->obstacles[i].hit = 1;
            }
        }
    }

    for (int i = 0; i < MAX_ATTACKS; i++) {
        if (game->attacks[i].active) {
            if (SDL_HasIntersection(&game->player.hitbox, &game->attacks[i].rect)) {
                joueurTakeDamage(&game->player, 10);
            }
        }
    }

    if (game->player.niveau == 3) {
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->zombies[i].active) {
                if (SDL_HasIntersection(&game->player.hitbox, &game->zombies[i].posEnnemi)) {
                    joueurTakeDamage(&game->player, 15);
                }
            }
        }
        if (game->boss.active) {
            if (SDL_HasIntersection(&game->player.hitbox, &game->boss.posEnnemi)) {
                joueurTakeDamage(&game->player, 20);
            }
        }
        for (int i = 0; i < MAX_BOSS_PROJ; i++) {
            if (game->bossProj[i].active) {
                if (SDL_HasIntersection(&game->player.hitbox, &game->bossProj[i].rect)) {
                    joueurTakeDamage(&game->player, 10);
                    game->bossProj[i].active = 0;
                }
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->bullets[i].active) {
            if (game->player.niveau == 1) {
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (game->enemies[j].active) {
                        if (SDL_HasIntersection(&game->bullets[i].rect, &game->enemies[j].posEnnemi)) {
                            game->bullets[i].active = 0;
                            game->player.score += 50;
                            goto next_bullet;
                        }
                    }
                }
            }

            for (int j = 0; j < MAX_ATTACKS; j++) {
                if (game->attacks[j].active) {
                    if (SDL_HasIntersection(&game->bullets[i].rect, &game->attacks[j].rect)) {
                        game->bullets[i].active = 0;
                        game->player.score += 10;
                        goto next_bullet;
                    }
                }
            }
            
            if (game->player.niveau == 3) {
                for (int j = 0; j < MAX_ZOMBIES; j++) {
                    if (game->zombies[j].active) {
                        if (SDL_HasIntersection(&game->bullets[i].rect, &game->zombies[j].posEnnemi)) {
                            game->zombies[j].vie--;
                            if (game->zombies[j].vie <= 0) {
                                game->zombies[j].active = 0;
                                game->player.score += 100;
                            }
                            game->bullets[i].active = 0;
                            goto next_bullet;
                        }
                    }
                }
                if (game->bullets[i].active && game->boss.active) {
                    if (SDL_HasIntersection(&game->bullets[i].rect, &game->boss.posEnnemi)) {
                        game->boss.vie--;
                        if (game->boss.vie <= 0) {
                            game->boss.active = 0;
                            game->player.score += 500;
                            game->victory = 1;
                            game->gameOver = 1;
                        }
                        game->bullets[i].active = 0;
                        goto next_bullet;
                    }
                }

                for (int j = 0; j < MAX_BOSS_PROJ; j++) {
                    if (game->bossProj[j].active) {
                        if (SDL_HasIntersection(&game->bullets[i].rect, &game->bossProj[j].rect)) {
                            game->bossProj[j].active = 0;
                            game->bullets[i].active = 0;
                            game->player.score += 5;
                            goto next_bullet;
                        }
                    }
                }
            }
        }
        next_bullet:;
    }

    for (int i = 0; i < MAX_BONUS; i++) {
        if (game->bonuses[i].active) {
            if (SDL_HasIntersection(&game->player.hitbox, &game->bonuses[i].rect)) {
                game->bonuses[i].active = 0;
                switch (game->bonuses[i].type) {
                    case 0: game->player.health += 20; if(game->player.health > 100) game->player.health = 100; break;
                    case 1: game->player.ammo += 10; break;
                    case 2: game->totalTime += 30; break;
                    case 3: game->player.score += 100; break;
                }
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        if (game->quizZones[i].active && !game->quizZones[i].triggered) {
            if (SDL_HasIntersection(&game->player.hitbox, &game->quizZones[i].rect)) {
                game->quizZones[i].triggered = 1;
                game->previousState = game->gameState;
                game->gameState = MENU_ENIGME;
                changeMusic(game, MENU_ENIGME);
                game->player.keyLeft = 0;
                game->player.keyRight = 0;
                game->player.vx = 0;
            }
        }
    }
}

void updateGameplay(GameManager* game) {
    if (!game) return;

    Uint32 currentTime = SDL_GetTicks();
    if (game->startTime == 0) game->startTime = currentTime;

    Uint32 dt = 16; 

    updateJoueur(&game->player, game, dt);
    if (game->player.niveau == 1) {
        updatePlateformes(game);
    }
    
    Uint32 now = SDL_GetTicks();

    Uint32 attackDelay = (game->player.niveau == 2) ? 800 : 999999;
    if (game->player.niveau >= 2 && game->attacksActive < MAX_ATTACKS && now - game->lastAttackSpawn > attackDelay) {
        for (int i = 0; i < MAX_ATTACKS; i++) {
            if (!game->attacks[i].active) {
                game->attacks[i].active = 1;
                game->attacks[i].rect = (SDL_Rect){(int)game->player.pos.x + (rand() % 400 - 200), -64, 64, 64};
                float dx = (float)game->player.pos.x - game->attacks[i].rect.x;
                float dy = (float)game->player.pos.y - game->attacks[i].rect.y;
                float len = sqrtf(dx*dx + dy*dy);
                float spd = 5.0f;
                if (len > 0) {
                    game->attacks[i].vx = (dx / len) * spd;
                    game->attacks[i].vy = (dy / len) * spd;
                } else {
                    game->attacks[i].vx = 0;
                    game->attacks[i].vy = spd;
                }
                game->attacksActive++;
                break;
            }
        }
        game->lastAttackSpawn = now;
    }

    if (game->player.niveau >= 2 && now - game->lastObstacleSpawn > 800 && game->player.pos.x < LEVEL_WIDTH - 800) {
        float spawnX = (float)(game->player.pos.x + SCREEN_WIDTH + (rand() % 200));
        if (spawnX > game->lastObstacleSpawnX + 450) {
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (!game->obstacles[i].active) {
                    game->obstacles[i].active = 1;
                    game->obstacles[i].hit = 0;
                    game->obstacles[i].passed = 0;
                    
                    int allowed_indices[] = {0, 1, 5};
                    game->obstacles[i].tex_idx = allowed_indices[rand() % 3];

                    int w = 100;
                    int h = 140; 
                    
                    if (game->player.niveau == 3) {
                        w = 75;
                        h = 105;
                    }

                    if (game->obstacles[i].tex_idx == 5) {
                        if (game->player.niveau == 3) {
                            w = 150; 
                            h = 185; 
                        } else {
                            w = 140;
                            h = 180;
                        }
                    }
                    game->obstacles[i].rect = (SDL_Rect){(int)spawnX, GROUND_Y - h + 80, w, h};
                    game->lastObstacleSpawnX = spawnX;
                    break;
                }
            }
            game->lastObstacleSpawn = now;
        }
    }

    for (int i = 0; i < MAX_ATTACKS; i++) {
        if (game->attacks[i].active) {
            game->attacks[i].rect.x += (int)game->attacks[i].vx;
            game->attacks[i].rect.y += (int)game->attacks[i].vy;
            if (game->attacks[i].rect.y > SCREEN_HEIGHT + 100 || game->attacks[i].rect.x < game->camera.x - 200 || game->attacks[i].rect.x > game->camera.x + SCREEN_WIDTH + 200) {
                game->attacks[i].active = 0;
                game->attacksActive--;
            }
        }
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            if (!game->obstacles[i].passed && !game->obstacles[i].hit && 
                game->player.pos.x > game->obstacles[i].rect.x + game->obstacles[i].rect.w) {
                game->player.score += 20;
                game->obstacles[i].passed = 1;
            }
        }
    }

    int currentThreshold = game->player.score / 100;
    if (currentThreshold > game->lastScoreRewardThreshold) {
        game->player.ammo += 10 * (currentThreshold - game->lastScoreRewardThreshold);
        game->lastScoreRewardThreshold = currentThreshold;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        updateEnnemie(&game->enemies[i], game, dt);
    }

    if (game->player.niveau == 3) {
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (!game->zombies[i].active) continue;
            
            if (game->zombies[i].direction == 1) { 
                game->zombies[i].posEnnemi.x += game->zombies[i].vitesse;
                if (game->zombies[i].posEnnemi.x > (1000 + i * 500) + 200) game->zombies[i].direction = 0;
            } else { 
                game->zombies[i].posEnnemi.x -= game->zombies[i].vitesse;
                if (game->zombies[i].posEnnemi.x < (1000 + i * 500) - 200) game->zombies[i].direction = 1;
            }
            
            if (now - game->zombies[i].animTimer > 100) {
                game->zombies[i].currentFrame = (game->zombies[i].currentFrame + 1) % 8;
                game->zombies[i].animTimer = now;
            }
        }

        if (game->boss.active) {
            if (game->boss.vie >= 35) game->bossPhase = 1;
            else if (game->boss.vie >= 15) game->bossPhase = 2;
            else game->bossPhase = 3;

            float bossStart = LEVEL_WIDTH - 220 - 500;

            if (game->bossPhase == 1) {
                if (game->boss.direction == 1) {
                    game->boss.posEnnemi.x += game->boss.vitesse;
                    if (game->boss.posEnnemi.x > bossStart + 300) game->boss.direction = 0;
                } else {
                    game->boss.posEnnemi.x -= game->boss.vitesse;
                    if (game->boss.posEnnemi.x < bossStart - 300) game->boss.direction = 1;
                }

                if (now - game->bossLastShot > 2000) {
                    for (int i = 0; i < MAX_BOSS_PROJ; i++) {
                        if (!game->bossProj[i].active) {
                            float cx = game->boss.posEnnemi.x + game->boss.posEnnemi.w / 2.0f;
                            float cy = game->boss.posEnnemi.y + game->boss.posEnnemi.h / 2.0f;
                            float tx = (float)(game->player.pos.x + 16);
                            float ty = (float)(game->player.pos.y + 24);
                            float ddx = tx - cx, ddy = ty - cy;
                            float len = sqrtf(ddx*ddx + ddy*ddy);
                            if (len < 1) len = 1;
                            game->bossProj[i].x = cx;
                            game->bossProj[i].y = cy;
                            game->bossProj[i].vx = (ddx / len) * 4.0f;
                            game->bossProj[i].vy = (ddy / len) * 4.0f;
                            game->bossProj[i].active = 1;
                            game->bossProj[i].rect = (SDL_Rect){(int)cx, (int)cy, 40, 40};
                            game->bossLastShot = now;
                            break;
                        }
                    }
                }
            } else if (game->bossPhase == 2) {
                float dx = (float)(game->player.pos.x - game->boss.posEnnemi.x);
                game->boss.posEnnemi.x += (dx > 0) ? 3.0f : -3.0f;
                game->boss.direction = (dx > 0) ? 1 : 0;

                if (now - game->bossLastShot > 1500) {
                    for (int i = 0; i < MAX_BOSS_PROJ; i++) {
                        if (!game->bossProj[i].active) {
                            float cx = game->boss.posEnnemi.x + game->boss.posEnnemi.w / 2.0f;
                            float cy = game->boss.posEnnemi.y + game->boss.posEnnemi.h / 2.0f;
                            float tx = (float)(game->player.pos.x + 16); 
                            float ty = (float)(game->player.pos.y + 24); 
                            float ddx = tx - cx, ddy = ty - cy;
                            float len = sqrtf(ddx*ddx + ddy*ddy);
                            if (len < 1) len = 1;
                            game->bossProj[i].x = cx;
                            game->bossProj[i].y = cy;
                            game->bossProj[i].vx = (ddx / len) * 5.0f;
                            game->bossProj[i].vy = (ddy / len) * 5.0f;
                            game->bossProj[i].active = 1;
                            game->bossProj[i].rect = (SDL_Rect){(int)cx, (int)cy, 40, 40};
                            game->bossLastShot = now;
                            break;
                        }
                    }
                }
            } else {
                float dx = (float)(game->player.pos.x - game->boss.posEnnemi.x);
                game->boss.posEnnemi.x += (dx > 0) ? 5.5f : -5.5f;
                game->boss.direction = (dx > 0) ? 1 : 0;

                if (!game->bossSpawnedMinion) {
                    for (int i = 0; i < MAX_ZOMBIES; i++) {
                        if (!game->zombies[i].active) {
                            game->zombies[i].active = 1;
                            game->zombies[i].posEnnemi = (SDL_Rect){game->boss.posEnnemi.x - 300, GROUND_Y - 130 + 80, 130, 130};
                            game->zombies[i].vitesse = 4;
                            game->zombies[i].vie = 2;
                            game->zombies[i].active = 1;
                            break;
                        }
                    }
                    game->bossSpawnedMinion = 1;
                }

                if (now - game->bossLastShot > 700) {
                    float cx = game->boss.posEnnemi.x + game->boss.posEnnemi.w / 2.0f;
                    float cy = game->boss.posEnnemi.y + game->boss.posEnnemi.h / 2.0f;
                    float tx = (float)(game->player.pos.x + 16);
                    float ty = (float)(game->player.pos.y + 24);
                    float ddx = tx - cx, ddy = ty - cy;
                    float len = sqrtf(ddx*ddx + ddy*ddy);
                    if (len < 1) len = 1;
                    float angles[3] = {-0.3f, 0.0f, 0.3f};
                    int spawned = 0;
                    for (int a = 0; a < 3 && spawned < 3; a++) {
                        float cosA = cosf(angles[a]), sinA = sinf(angles[a]);
                        float bvx = (ddx/len)*6.0f, bvy = (ddy/len)*6.0f;
                        float rvx = bvx*cosA - bvy*sinA;
                        float rvy = bvx*sinA + bvy*cosA;
                        for (int i = 0; i < MAX_BOSS_PROJ; i++) {
                            if (!game->bossProj[i].active) {
                                game->bossProj[i].x = cx;
                                game->bossProj[i].y = cy;
                                game->bossProj[i].vx = rvx;
                                game->bossProj[i].vy = rvy;
                                game->bossProj[i].active = 1;
                                game->bossProj[i].rect = (SDL_Rect){(int)cx, (int)cy, 40, 40};
                                spawned++;
                                break;
                            }
                        }
                    }
                    game->bossLastShot = now;
                }
            }

            if (now - game->boss.animTimer > 150) {
                game->boss.currentFrame = (game->boss.currentFrame + 1) % 8;
                game->boss.animTimer = now;
            }
        }

        for (int i = 0; i < MAX_BOSS_PROJ; i++) {
            if (!game->bossProj[i].active) continue;

            float tx = (float)(game->player.pos.x + 16);
            float ty = (float)(game->player.pos.y + 24);
            float ddx = tx - game->bossProj[i].x;
            float ddy = ty - game->bossProj[i].y;
            float len = sqrtf(ddx * ddx + ddy * ddy);
            if (len > 1.0f) {
                float speed = (game->bossPhase == 3) ? 6.0f : 4.5f;
                float target_vx = (ddx / len) * speed;
                float target_vy = (ddy / len) * speed;
                game->bossProj[i].vx += (target_vx - game->bossProj[i].vx) * 0.12f;
                game->bossProj[i].vy += (target_vy - game->bossProj[i].vy) * 0.12f;
            }

            game->bossProj[i].x += game->bossProj[i].vx;
            game->bossProj[i].y += game->bossProj[i].vy;
            game->bossProj[i].rect.x = (int)game->bossProj[i].x - 20;
            game->bossProj[i].rect.y = (int)game->bossProj[i].y - 20;

            if (game->bossProj[i].x < game->camera.x - 500 || game->bossProj[i].x > game->camera.x + SCREEN_WIDTH + 500)
                game->bossProj[i].active = 0;
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->bullets[i].active) {
            game->bullets[i].rect.x += (int)game->bullets[i].vx;
            game->bullets[i].rect.y += (int)game->bullets[i].vy;
            if (game->bullets[i].rect.x < 0 || game->bullets[i].rect.x > LEVEL_WIDTH) {
                game->bullets[i].active = 0;
            }
        }
    }

    checkCollisions(game);

    int remainingTime = game->totalTime - (int)(SDL_GetTicks() - game->startTime) / 1000;
    if (remainingTime <= 0 || game->player.lives <= 0 || game->player.health <= 0) {
        if (!game->victory) { 
            game->gameOver = 1;
            game->victory = 0;
        }
    }

    MAJMinimap(game);
    updateAnimations(game);

    game->camera.x = (int)(game->player.pos.x - SCREEN_WIDTH / 2);
    if (game->camera.x < 0) game->camera.x = 0;
    if (game->camera.x + SCREEN_WIDTH > LEVEL_WIDTH) game->camera.x = (int)(LEVEL_WIDTH - SCREEN_WIDTH);
    game->camera.y = 0;

    if (game->player.pos.x > LEVEL_WIDTH - 200 && now - game->gameplayState.lastTransitionTime > 2000) {
        if (game->player.niveau < MAX_LEVELS) {
            loadLevel(game, game->player.niveau + 1);
            game->gameplayState.lastTransitionTime = now;
        } else if (game->player.niveau < 3) { 
            game->victory = 1;
            game->gameOver = 1;
        }
    }
}

void renderGameplay(GameManager* game) {
    if (!game) return;
    
    if (game->background) {
        int bgTexW, bgTexH;
        SDL_QueryTexture(game->background, NULL, NULL, &bgTexW, &bgTexH);
        
        SDL_Rect srcRect, dstRect;
        float level_w = (LEVEL_WIDTH > 0) ? (float)LEVEL_WIDTH : 3840.0f;
        if (bgTexW > SCREEN_WIDTH) {
            float ratio = (float)bgTexW / level_w;
            srcRect.x = (int)(game->camera.x * ratio);
            srcRect.y = 0;
            srcRect.w = (int)(SCREEN_WIDTH * ratio);
            srcRect.h = bgTexH;
        } else {
            srcRect = (SDL_Rect){0, 0, bgTexW, bgTexH};
        }
        dstRect = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(game->renderer, game->background, &srcRect, &dstRect);
    }

    if (game->player.niveau == 1) {
        renderPlateformes(game);
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            SDL_Rect dest = {game->obstacles[i].rect.x - game->camera.x, game->obstacles[i].rect.y, game->obstacles[i].rect.w, game->obstacles[i].rect.h};
            int tidx = game->obstacles[i].tex_idx;
            if (tidx >= 0 && tidx < 6 && game->obstacleTex[tidx]) {
                SDL_RenderCopy(game->renderer, game->obstacleTex[tidx], NULL, &dest);
            }
        }
    }

    for (int i = 0; i < MAX_ATTACKS; i++) {
        if (game->attacks[i].active) {
            SDL_Rect dest = {game->attacks[i].rect.x - game->camera.x, game->attacks[i].rect.y, game->attacks[i].rect.w, game->attacks[i].rect.h};
            if (game->qomblaTex) {
                SDL_RenderCopy(game->renderer, game->qomblaTex, NULL, &dest);
            }
        }
    }

    if (game->player.niveau == 1) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (game->enemies[i].active) {
                SDL_Rect br;
                br.x = game->enemies[i].posEnnemi.x - game->camera.x - 8;
                br.y = game->enemies[i].posEnnemi.y - 16;
                br.w = 52;
                br.h = 60;
                SDL_RenderCopy(game->renderer, game->enemies[i].texture, NULL, &br);
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game->bullets[i].active) {
            SDL_Rect dest = {game->bullets[i].rect.x - game->camera.x, game->bullets[i].rect.y - game->camera.y, 30, 10};
            if (game->bulletTex) {
                SDL_RenderCopy(game->renderer, game->bulletTex, NULL, &dest);
            }
        }
    }
    for (int i = 0; i < MAX_BONUS; i++) {
        if (game->bonuses[i].active) {
            SDL_Rect dest = {game->bonuses[i].rect.x - game->camera.x, game->bonuses[i].rect.y - game->camera.y, 40, 40};
            SDL_RenderCopy(game->renderer, game->bonuses[i].texture, NULL, &dest);
        }
    }

    if (game->player.niveau == 3) {
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (game->zombies[i].active) {
                SDL_Rect dest = {
                    game->zombies[i].posEnnemi.x - game->camera.x - (130 - game->zombies[i].posEnnemi.w)/2, 
                    game->zombies[i].posEnnemi.y - (130 - game->zombies[i].posEnnemi.h), 
                    130, 
                    130
                };
                if (game->zombies[i].texture) {
                    int tw, th;
                    SDL_QueryTexture(game->zombies[i].texture, NULL, NULL, &tw, &th);
                    int fw = tw / 8;
                    int fh = th / 6;
                    SDL_Rect src = {game->zombies[i].currentFrame * fw, 0, fw, fh};
                    SDL_RendererFlip flip = (game->zombies[i].direction == 1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
                    SDL_RenderCopyEx(game->renderer, game->zombies[i].texture, &src, &dest, 0, NULL, flip);
                }
            }
        }
        if (game->boss.active) {
            SDL_Rect dest = {
                game->boss.posEnnemi.x - game->camera.x - (220 - game->boss.posEnnemi.w)/2, 
                game->boss.posEnnemi.y - (220 - game->boss.posEnnemi.h), 
                220, 
                220
            };
            if (game->boss.texture) {
                int tw, th;
                SDL_QueryTexture(game->boss.texture, NULL, NULL, &tw, &th);
                int fw = tw / 8;
                int fh = th / 6;
                SDL_Rect src = {game->boss.currentFrame * fw, 1 * fh, fw, fh};
                SDL_RendererFlip flip = (game->boss.direction == 1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
                
                if (game->bossPhase == 3) SDL_SetTextureColorMod(game->boss.texture, 255, 80, 80);
                else if (game->bossPhase == 2) SDL_SetTextureColorMod(game->boss.texture, 255, 180, 80);
                
                SDL_RenderCopyEx(game->renderer, game->boss.texture, &src, &dest, 0, NULL, flip);
                SDL_SetTextureColorMod(game->boss.texture, 255, 255, 255);
            }
            
            SDL_Rect h_bg = {dest.x, dest.y - 24, dest.w, 12};
            SDL_Rect h_fg = {dest.x, dest.y - 24, (int)(dest.w * (game->boss.vie / 50.0f)), 12};
            SDL_SetRenderDrawColor(game->renderer, 30, 0, 0, 255);
            SDL_RenderFillRect(game->renderer, &h_bg);
            if (game->bossPhase == 3) SDL_SetRenderDrawColor(game->renderer, 255, 40, 40, 255);
            else if (game->bossPhase == 2) SDL_SetRenderDrawColor(game->renderer, 255, 160, 0, 255);
            else SDL_SetRenderDrawColor(game->renderer, 50, 220, 50, 255);
            SDL_RenderFillRect(game->renderer, &h_fg);
        }
        for (int i = 0; i < MAX_BOSS_PROJ; i++) {
            if (game->bossProj[i].active) {
                SDL_Rect dest = {game->bossProj[i].rect.x - game->camera.x, game->bossProj[i].rect.y, 40, 40};
                if (game->qomblaTex) {
                    SDL_RenderCopy(game->renderer, game->qomblaTex, NULL, &dest);
                }
            }
        }
    }

    if (game->player.niveau == 1) {
        for (int i = 0; i < 2; i++) {
            if (game->quizZones[i].active) {
                SDL_Rect dest = {game->quizZones[i].rect.x - game->camera.x, game->quizZones[i].rect.y - game->camera.y, game->quizZones[i].rect.w, game->quizZones[i].rect.h};
                if (game->enigmeState.fffBtnTex) {
                    SDL_RenderCopy(game->renderer, game->enigmeState.fffBtnTex, NULL, &dest);
                }
            }
        }
    }

    renderJoueur(&game->player, game, game->camera);
    renderAnimations(game);
    afficherMinimap(game);
    
    char hudText[256];
    SDL_Color textColor = {255, 255, 255, 255};
    
    SDL_Rect hudBg = {10, 10, 350, 100};
    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 160); 
    SDL_RenderFillRect(game->renderer, &hudBg);
    
    TTF_Font* mainFont = game->font ? game->font : game->titleFont;

    if (mainFont) {
        sprintf(hudText, "Score: %d", game->player.score);
        renderText(game, hudText, 20, 20, textColor, mainFont);
        
        sprintf(hudText, "Sante: %d", game->player.health);
        renderText(game, hudText, 20, 45, (game->player.health < 30 ? (SDL_Color){255, 0, 0, 255} : textColor), mainFont);
        
        sprintf(hudText, "Vies: %d", game->player.lives);
        renderText(game, hudText, 20, 70, textColor, mainFont);

        sprintf(hudText, "Munitions: %d", game->player.ammo);
        renderText(game, hudText, 200, 70, textColor, mainFont);

        sprintf(hudText, "Niveau: %d", game->player.niveau);
        renderText(game, hudText, 200, 45, textColor, mainFont);
        
        int remainingTime = game->totalTime - (int)(SDL_GetTicks() - game->startTime) / 1000;
        if (remainingTime < 0) remainingTime = 0;
        sprintf(hudText, "Temps: %02d:%02d", remainingTime / 60, remainingTime % 60);
        renderText(game, hudText, 200, 20, (remainingTime < 30 ? (SDL_Color){255, 0, 0, 255} : textColor), mainFont);
    }

    if (game->gameOver) {
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(game->renderer, &overlay);
        
        if (game->victory) {
            renderText(game, "CONGRATULATIONS YOU WON!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, (SDL_Color){255, 255, 0, 255}, game->titleFont);
        } else {
            renderText(game, "GAME OVER", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2, (SDL_Color){255, 0, 0, 255}, game->titleFont);
        }
        renderText(game, "Appuyez sur ECHAP pour quitter", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 60, textColor, game->font);
    }
}

// ================= ANIMATIONS =================

void initAnimations(GameManager* game) {
    if (!game) return;
    game->animationState.explosionNbFrames = 8;
    game->nbAnimations = 0;
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        game->animations[i].active = 0;
        game->animations[i].sprite = NULL;
        game->animations[i].currentFrame = 0;
        game->animations[i].animTimer = 0;
    }
}

void loadAnimationMedia(GameManager* game) {
    if (!game || !game->renderer) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "explosion.png");
    game->animationState.explosionSpriteSheet = IMG_LoadTexture(game->renderer, path);
    if (game->animationState.explosionSpriteSheet) {
        SDL_QueryTexture(game->animationState.explosionSpriteSheet, NULL, NULL, &game->animationState.explosionSheetW, &game->animationState.explosionSheetH);
    }
}

void lancerAnimation(GameManager* game, int x, int y) {
    if (!game || !game->animationState.explosionSpriteSheet) return;
    int slot = -1;
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!game->animations[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return;

    AnimBackground* a = &game->animations[slot];
    a->sprite = game->animationState.explosionSpriteSheet;
    a->active = 1;
    a->nbFrames = game->animationState.explosionNbFrames;
    a->currentFrame = 0;
    a->direction = 0;
    a->animSpeed = 4;
    a->animTimer = 0;
    a->spriteSheetW = game->animationState.explosionSheetW;
    a->spriteSheetH = game->animationState.explosionSheetH;

    a->posSprite.x = 0;
    a->posSprite.y = 0;
    a->posSprite.w = game->animationState.explosionSheetW / game->animationState.explosionNbFrames;
    a->posSprite.h = game->animationState.explosionSheetH;

    a->posScreen.x = x;
    a->posScreen.y = y;
    a->posScreen.w = a->posSprite.w * 2;
    a->posScreen.h = a->posSprite.h * 2;

    a->posScreen.x -= a->posScreen.w / 2;
    a->posScreen.y -= a->posScreen.h / 2;

    if (game->nbAnimations < slot + 1) game->nbAnimations = slot + 1;
}

void updateAnimations(GameManager* game) {
    if (!game) return;
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        AnimBackground* a = &game->animations[i];
        if (!a->active) continue;
        a->animTimer++;
        if (a->animTimer >= a->animSpeed) {
            a->animTimer = 0;
            a->currentFrame++;
            a->posSprite.x += a->posSprite.w;
            if (a->posSprite.x >= a->spriteSheetW || a->currentFrame >= a->nbFrames) {
                a->active = 0;
                a->posSprite.x = 0;
                a->currentFrame = 0;
            }
        }
    }
}

void renderAnimations(GameManager* game) {
    if (!game) return;
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        AnimBackground* a = &game->animations[i];
        if (!a->active) continue;
        SDL_Rect dest = {a->posScreen.x - game->camera.x, a->posScreen.y - game->camera.y, a->posScreen.w, a->posScreen.h};
        SDL_RenderCopy(game->renderer, a->sprite, &a->posSprite, &dest);
    }
}

void cleanupAnimations(GameManager* game) {
    if (game->animationState.explosionSpriteSheet) {
        SDL_DestroyTexture(game->animationState.explosionSpriteSheet);
        game->animationState.explosionSpriteSheet = NULL;
    }
}

// ================= HISTOIRE ET REGLES =================

void histoireCommencer(GameManager* game) { 
    if (!game) return;
    game->player.niveau = 1; 
    loadLevel(game, 1); 
    game->gameState = GAME_PLAY; 
    changeMusic(game, GAME_PLAY);
}

void initHistoireMenu(GameManager* game) {
    if (!game) return;
    strcpy(game->histoireState.histoireMenu.title, "REGLES ET CONTROLES"); 
    game->histoireState.histoireMenu.buttonCount = 1;
    game->histoireState.histoireMenu.background = NULL; 
    game->histoireState.histoireMenu.music = NULL;
    game->histoireState.histoireMenu.buttons[0] = (Button){{275, 620, 250, 60}, "Commencer", 0, (void(*)(struct GameManager*))histoireCommencer, NULL, NULL, 0, 1};
}

void loadHistoireMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "guide.png");
    game->histoireState.histoireMenu.background = IMG_LoadTexture(game->renderer, path);
    if (!game->histoireState.histoireMenu.background) {
        sprintf(path, "%s%s", ASSETS_PATH, "a1.jpg");
        game->histoireState.histoireMenu.background = IMG_LoadTexture(game->renderer, path);
    }
    loadButtonTexture(game, &game->histoireState.histoireMenu.buttons[0], "av1.jpg", "av11.jpg");
}

void handleHistoireKeyPress(GameManager* game, SDL_Keycode key) { 
    if (!game) return;
    if (key == SDLK_ESCAPE) game->gameState = MENU_MAIN; 
    else if (key == SDLK_RETURN || key == SDLK_SPACE) histoireCommencer(game); 
}

void handleHistoireMouseMotion(GameManager* game, int x, int y) { 
    if (game) handleMenuMouseMotion(game, &game->histoireState.histoireMenu, x, y); 
}

void handleHistoireMouseClick(GameManager* game, int x, int y) { 
    if (game) handleMenuMouseClick(game, &game->histoireState.histoireMenu, x, y); 
}

void renderHistoire(GameManager* game) {
    if (!game) return;
    if (game->histoireState.histoireMenu.background) SDL_RenderCopy(game->renderer, game->histoireState.histoireMenu.background, NULL, NULL);
    renderText(game, "REGLES ET CONTROLES", 280, 50, (SDL_Color){255,255,0,255}, game->titleFont);
    
    SDL_Color white = {255,255,255,255};
    renderText(game, "- Fleches / ZQSD : Deplacement et Saut", 200, 180, white, game->font);
    renderText(game, "- Touche F : Tirer des cartouches", 200, 240, white, game->font);
    renderText(game, "- Collectez les bonus (Vie, Temps, Munitions)", 200, 300, white, game->font);
    renderText(game, "- Evitez ou eliminez les ennemis", 200, 360, white, game->font);
    renderText(game, "- Resolvez les enigmes dans les zones speciales", 200, 420, white, game->font);
    renderText(game, "- Terminez avant la fin du compte a rebours !", 200, 480, white, game->font);
    
    renderButton(game, &game->histoireState.histoireMenu.buttons[0]);
}

void cleanupHistoireMenu(GameManager* game) { if (game) cleanupMenu(&game->histoireState.histoireMenu); }

