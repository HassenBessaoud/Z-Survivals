#include "game.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ================= UTILITAIRES =================

void renderText(GameManager* game, const char* text, int x, int y, SDL_Color color, TTF_Font* f) {
    if (!game || !f || !text || strlen(text) == 0) return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(f, text, color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(game->renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderButton(GameManager* game, Button* btn) {
    if (!game || !btn->visible) return;
    if (btn->useTexture && btn->normalTexture && btn->selectedTexture) {
        SDL_Texture* tex = btn->selected ? btn->selectedTexture : btn->normalTexture;
        SDL_RenderCopy(game->renderer, tex, NULL, &btn->rect);
    } else {
        SDL_Color c = btn->selected ? (SDL_Color){230, 126, 34, 255} : (SDL_Color){52, 152, 219, 255};
        SDL_SetRenderDrawColor(game->renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(game->renderer, &btn->rect);
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(game->renderer, &btn->rect);
        renderText(game, btn->text, btn->rect.x + 10, btn->rect.y + 15, (SDL_Color){255,255,255,255}, game->font);
    }
}

void loadButtonTexture(GameManager* game, Button* btn, const char* normalFile, const char* selectedFile) {
    if (!game) return;
    char pathN[512], pathS[512];
    snprintf(pathN, sizeof(pathN), "assets/%s", normalFile);
    snprintf(pathS, sizeof(pathS), "assets/%s", selectedFile);
    btn->normalTexture = IMG_LoadTexture(game->renderer, pathN);
    btn->selectedTexture = IMG_LoadTexture(game->renderer, pathS);
    if (!btn->normalTexture) {
        snprintf(pathN, sizeof(pathN), "%s%s", ASSETS_PATH, normalFile);
        btn->normalTexture = IMG_LoadTexture(game->renderer, pathN);
    }
    if (!btn->selectedTexture) {
        snprintf(pathS, sizeof(pathS), "%s%s", ASSETS_PATH, selectedFile);
        btn->selectedTexture = IMG_LoadTexture(game->renderer, pathS);
    }
    if (btn->normalTexture && btn->selectedTexture) btn->useTexture = 1;
}

void updateVolume(GameManager* game, int delta) {
    if (!game) return;
    game->volume += delta;
    if (game->volume < 0) game->volume = 0;
    if (game->volume > 100) game->volume = 100;
    Mix_VolumeMusic((int)(game->volume * 1.28));
    Mix_Volume(-1, (int)(game->volume * 1.28));
}

void toggleFullscreen(GameManager* game) {
    if (!game || !game->window) return;
    game->fullscreen = !game->fullscreen;
    SDL_SetWindowFullscreen(game->window, game->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void handleMenuMouseMotion(GameManager* game, Menu* menu, int x, int y) {
    if (!menu) return;
    for (int i = 0; i < menu->buttonCount; i++) {
        if (!menu->buttons[i].visible) continue;
        int was = menu->buttons[i].selected;
        Button* b = &menu->buttons[i];
        if (INSIDE(b->rect, x, y)) {
            b->selected = 1;
            if (!was && game->hoverSound) Mix_PlayChannel(-1, game->hoverSound, 0);
        } else b->selected = 0;
    }
}

void handleMenuMouseClick(GameManager* game, Menu* menu, int x, int y) {
    if (!menu || !game) return;
    for (int i = 0; i < menu->buttonCount; i++) {
        if (!menu->buttons[i].visible) continue;
        Button* b = &menu->buttons[i];
        if (INSIDE(b->rect, x, y)) {
            if (b->onClick) {
                if (game->clickSound) Mix_PlayChannel(-1, game->clickSound, 0);
                b->onClick(game);
            }
            break;
        }
    }
}

void cleanupMenu(Menu* menu) {
    if (!menu) return;
    for (int i = 0; i < menu->buttonCount; i++) {
        if (menu->buttons[i].normalTexture) SDL_DestroyTexture(menu->buttons[i].normalTexture);
        if (menu->buttons[i].selectedTexture) SDL_DestroyTexture(menu->buttons[i].selectedTexture);
        menu->buttons[i].normalTexture = menu->buttons[i].selectedTexture = NULL;
    }
    if (menu->background) { SDL_DestroyTexture(menu->background); menu->background = NULL; }
    if (menu->logo) { SDL_DestroyTexture(menu->logo); menu->logo = NULL; }
    if (menu->music) { Mix_FreeMusic(menu->music); menu->music = NULL; }
}

void changeMusic(GameManager* game, GameState state) {
    if (!game || state == game->menuState.lastState) return;
    game->menuState.lastState = state;
    Mix_HaltMusic();
    if (game->menuState.currentLoadedMusic) { Mix_FreeMusic(game->menuState.currentLoadedMusic); game->menuState.currentLoadedMusic = NULL; }
    
    if (state == GAME_PLAY) {
        game->player.keyLeft = 0;
        game->player.keyRight = 0;
        game->player.vx = 0;
    }

    Mix_Music* next = NULL;
    switch(state) {
        case MENU_MAIN: 
            next = Mix_LoadMUS("assets/group4/sound1.mp3");
            if (!next) next = Mix_LoadMUS("assets/sound1.mp3"); 
            break;
        case MENU_OPTIONS: 
            next = Mix_LoadMUS("assets/group4/music_option.mp3");
            if (!next) next = Mix_LoadMUS("assets/music_option.mp3"); 
            break;
        case MENU_SCORES: 
            next = Mix_LoadMUS("assets/group4/victory.wav");
            if (!next) next = Mix_LoadMUS("assets/victory.wav"); 
            break;
        case MENU_HISTOIRE: 
            next = Mix_LoadMUS("assets/group4/sound1.mp3");
            if (!next) next = Mix_LoadMUS("assets/sound1.mp3"); 
            break;
        case MENU_SAVE: 
            next = Mix_LoadMUS("assets/group4/music_option.mp3");
            if (!next) next = Mix_LoadMUS("assets/music_option.mp3"); 
            break;
        case MENU_ENIGME: case MENU_QUIZ: case MENU_PUZZLE: 
            next = Mix_LoadMUS("assets/group4/tmbr.mp3");
            if (!next) next = Mix_LoadMUS("assets/tmbr.mp3"); 
            break;
        case MENU_RUNNER: 
            next = Mix_LoadMUS("assets/group4/sound1.mp3");
            if (!next) next = Mix_LoadMUS("assets/sound1.mp3"); 
            break;
        case GAME_PLAY: 
            next = Mix_LoadMUS("assets/group4/sound1.mp3");
            if (!next) next = Mix_LoadMUS("assets/sound1.mp3"); 
            break;
    }
    if (next) { Mix_PlayMusic(next, -1); game->menuState.currentLoadedMusic = next; }
}

// ================= INITIALISATION DU JEU =================

void initGame(GameManager* game, SDL_Renderer* renderer) {
    if (!game) return;
    memset(game, 0, sizeof(GameManager));
    game->renderer = renderer;
    game->gameState = MENU_MAIN;
    game->volume = 80;
    game->fullscreen = 0;
    game->totalTime = 300;
    
    game->font = TTF_OpenFont("assets/font.ttf", 20);
    if (!game->font) game->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    game->titleFont = TTF_OpenFont("assets/font.ttf", 36);
    if (!game->titleFont) game->titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 36);

    initMainMenu(game); loadMainMenuMedia(game);
    initOptionsMenu(game); loadOptionsMedia(game);
    initScoresMenu(game); loadScoresMedia(game);
    initHistoireMenu(game); loadHistoireMedia(game);
    initExitMenu(game); loadExitMedia(game);
    initPlayerMenu(game); loadPlayerMedia(game);
    initSaveMenu(game); loadSaveMedia(game);
    initEnigmeMenu(game); loadEnigmeMedia(game);
    initAnimations(game); loadAnimationMedia(game);
    initMinimap(game); loadMinimapMedia(game);
    
    srand((unsigned int)time(NULL));
    changeMusic(game, MENU_MAIN);
}

// ================= UPDATE & RENDER =================

void updateGame(GameManager* game) {
    if (game->gameState == GAME_PLAY) updateGameplay(game);
    else if (game->gameState == MENU_RUNNER) updateRunner(game);
}

void renderGame(GameManager* game) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    switch(game->gameState) {
        case MENU_MAIN: renderMainMenu(game); break;
        case MENU_OPTIONS: renderOptionsMenu(game); break;
        case MENU_SCORES: renderScoresMenu(game); break;
        case MENU_HISTOIRE: renderHistoire(game); break;
        case MENU_EXIT: renderExitMenu(game); break;
        case MENU_SAVE: renderSaveMenu(game); break;
        case MENU_ENIGME: renderEnigmeMenu(game); break;
        case MENU_QUIZ: renderQuiz(game); break;
        case MENU_PUZZLE: renderPuzzle(game); break;
        case GAME_PLAY: renderGameplay(game); break;
        case MENU_RUNNER: renderRunner(game); break;
    }
    SDL_RenderPresent(game->renderer);
}

// ================= MENUS IMPLEMENTATION =================

void mainJouer(GameManager* game) { game->gameState = MENU_SAVE; changeMusic(game, MENU_SAVE); }
void mainOptions(GameManager* game) { game->gameState = MENU_OPTIONS; changeMusic(game, MENU_OPTIONS); }
void mainScores(GameManager* game) { game->gameState = MENU_SCORES; changeMusic(game, MENU_SCORES); }
void mainHistoire(GameManager* game) { game->gameState = MENU_HISTOIRE; changeMusic(game, MENU_HISTOIRE); }
void mainQuitter(GameManager* game) { game->gameState = MENU_EXIT; }

void initMainMenu(GameManager* game) {
    strcpy(game->menuState.mainMenu.title, "Z SURVIVALS"); game->menuState.mainMenu.buttonCount = 5;
    game->menuState.mainMenu.buttons[0] = (Button){{275, 200, 250, 60}, "Jouer", 0, mainJouer, NULL, NULL, 0, 1};
    game->menuState.mainMenu.buttons[1] = (Button){{275, 280, 250, 60}, "Options", 0, mainOptions, NULL, NULL, 0, 1};
    game->menuState.mainMenu.buttons[2] = (Button){{275, 360, 250, 60}, "Scores", 0, mainScores, NULL, NULL, 0, 1};
    game->menuState.mainMenu.buttons[3] = (Button){{275, 440, 250, 60}, "Histoire", 0, mainHistoire, NULL, NULL, 0, 1};
    game->menuState.mainMenu.buttons[4] = (Button){{275, 520, 250, 60}, "Quitter", 0, mainQuitter, NULL, NULL, 0, 1};
}
void loadMainMenuMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    
    // Original path logic from menu.c
    sprintf(path, "assets/group4/a1.jpg");
    game->menuState.mainMenu.background = IMG_LoadTexture(game->renderer, path);
    if (!game->menuState.mainMenu.background) {
        game->menuState.mainMenu.background = IMG_LoadTexture(game->renderer, "assets/a1.jpg");
    }
    
    sprintf(path, "assets/logo.jpg");
    game->menuState.mainMenu.logo = IMG_LoadTexture(game->renderer, path);
    if (!game->menuState.mainMenu.logo) {
        sprintf(path, "%slogo.jpg", ASSETS_PATH);
        game->menuState.mainMenu.logo = IMG_LoadTexture(game->renderer, path);
    }
    
    sprintf(path, "%ssound1.mp3", ASSETS_PATH);
    game->menuState.mainMenu.music = Mix_LoadMUS(path);
    
    sprintf(path, "%sbip.wav", ASSETS_PATH);
    game->hoverSound = Mix_LoadWAV(path);
    sprintf(path, "%seffect.wav", ASSETS_PATH);
    game->clickSound = Mix_LoadWAV(path);
    
    loadButtonTexture(game, &game->menuState.mainMenu.buttons[0], "b1.jpg", "b11.jpg");
    loadButtonTexture(game, &game->menuState.mainMenu.buttons[1], "b2.jpg", "b22.jpg");
    loadButtonTexture(game, &game->menuState.mainMenu.buttons[2], "b3.jpg", "b33.jpg");
    loadButtonTexture(game, &game->menuState.mainMenu.buttons[3], "b4.jpg", "b44.jpg");
    loadButtonTexture(game, &game->menuState.mainMenu.buttons[4], "b5.jpg", "b55.jpg");
}
void renderMainMenu(GameManager* game) {
    if (game->menuState.mainMenu.background) SDL_RenderCopy(game->renderer, game->menuState.mainMenu.background, NULL, NULL);
    renderText(game, game->menuState.mainMenu.title, 280, 80, (SDL_Color){255,255,255,255}, game->titleFont);
    for (int i = 0; i < game->menuState.mainMenu.buttonCount; i++) renderButton(game, &game->menuState.mainMenu.buttons[i]);
}
void handleMainMenuKeyPress(GameManager* game, SDL_Keycode key) {
    if (key == SDLK_j) mainJouer(game); else if (key == SDLK_ESCAPE) mainQuitter(game);
}

void optVolPlus(GameManager* game) { updateVolume(game, 10); }
void optVolMoins(GameManager* game) { updateVolume(game, -10); }
void optFull(GameManager* game) { toggleFullscreen(game); }
void optBack(GameManager* game) { game->gameState = MENU_MAIN; changeMusic(game, MENU_MAIN); }

void initOptionsMenu(GameManager* game) {
    strcpy(game->optionsState.optionsMenu.title, "OPTIONS"); game->optionsState.optionsMenu.buttonCount = 4;
    game->optionsState.optionsMenu.buttons[0] = (Button){{300, 200, 200, 60}, "Volume +", 0, optVolPlus, NULL, NULL, 0, 1};
    game->optionsState.optionsMenu.buttons[1] = (Button){{300, 280, 200, 60}, "Volume -", 0, optVolMoins, NULL, NULL, 0, 1};
    game->optionsState.optionsMenu.buttons[2] = (Button){{300, 360, 200, 60}, "Fullscreen", 0, optFull, NULL, NULL, 0, 1};
    game->optionsState.optionsMenu.buttons[3] = (Button){{300, 480, 200, 60}, "Retour", 0, optBack, NULL, NULL, 0, 1};
}
void loadOptionsMedia(GameManager* game) {
    game->optionsState.optionsMenu.background = IMG_LoadTexture(game->renderer, "assets/a1.jpg");
    for (int i = 0; i < 4; i++) loadButtonTexture(game, &game->optionsState.optionsMenu.buttons[i], "b1.jpg", "b11.jpg");
}
void renderOptionsMenu(GameManager* game) {
    if (game->optionsState.optionsMenu.background) SDL_RenderCopy(game->renderer, game->optionsState.optionsMenu.background, NULL, NULL);
    renderText(game, "OPTIONS", 330, 80, (SDL_Color){255,255,255,255}, game->titleFont);
    for (int i = 0; i < 4; i++) renderButton(game, &game->optionsState.optionsMenu.buttons[i]);
}
void handleOptionsKeyPress(GameManager* game, SDL_Keycode key) { if (key == SDLK_ESCAPE) optBack(game); }

void scoreBack(GameManager* game) { game->gameState = MENU_MAIN; changeMusic(game, MENU_MAIN); }
void initScoresMenu(GameManager* game) {
    strcpy(game->scoresState.scoresMenu.title, "SCORES"); game->scoresState.scoresMenu.buttonCount = 1;
    game->scoresState.scoresMenu.buttons[0] = (Button){{300, 500, 200, 60}, "Retour", 0, scoreBack, NULL, NULL, 0, 1};
}
void loadScoresMedia(GameManager* game) {
    game->scoresState.scoresMenu.background = IMG_LoadTexture(game->renderer, "assets/a1.jpg");
    loadButtonTexture(game, &game->scoresState.scoresMenu.buttons[0], "b1.jpg", "b11.jpg");
}
void renderScoresMenu(GameManager* game) {
    if (game->scoresState.scoresMenu.background) SDL_RenderCopy(game->renderer, game->scoresState.scoresMenu.background, NULL, NULL);
    renderText(game, "MEILLEURS SCORES", 280, 80, (SDL_Color){255,255,255,255}, game->titleFont);
    renderButton(game, &game->scoresState.scoresMenu.buttons[0]);
}
void handleScoresKeyPress(GameManager* game, SDL_Keycode key) { if (key == SDLK_ESCAPE) scoreBack(game); }

void exitOui(GameManager* game) { (void)game; exit(0); }
void exitNon(GameManager* game) { game->gameState = MENU_MAIN; changeMusic(game, MENU_MAIN); }
void initExitMenu(GameManager* game) {
    strcpy(game->menuState.exitMenu.title, "QUITTER ?"); game->menuState.exitMenu.buttonCount = 2;
    game->menuState.exitMenu.buttons[0] = (Button){{250, 300, 120, 50}, "Oui", 0, exitOui, NULL, NULL, 0, 1};
    game->menuState.exitMenu.buttons[1] = (Button){{430, 300, 120, 50}, "Non", 0, exitNon, NULL, NULL, 0, 1};
}
void loadExitMedia(GameManager* game) {
    game->menuState.exitMenu.background = IMG_LoadTexture(game->renderer, "assets/a1.jpg");
    loadButtonTexture(game, &game->menuState.exitMenu.buttons[0], "j1.jpg", "j11.jpg");
    loadButtonTexture(game, &game->menuState.exitMenu.buttons[1], "j2.jpg", "j22.jpg");
}
void renderExitMenu(GameManager* game) {
    if (game->menuState.exitMenu.background) SDL_RenderCopy(game->renderer, game->menuState.exitMenu.background, NULL, NULL);
    renderText(game, "Voulez-vous quitter ?", 280, 200, (SDL_Color){255,255,255,255}, game->font);
    for (int i = 0; i < 2; i++) renderButton(game, &game->menuState.exitMenu.buttons[i]);
}
void handleExitKeyPress(GameManager* game, SDL_Keycode key) { if (key == SDLK_y || key == SDLK_o) exitOui(game); else if (key == SDLK_n || key == SDLK_ESCAPE) exitNon(game); }
void cleanupMainMenu(GameManager* game) { if (game) cleanupMenu(&game->menuState.mainMenu); }
void cleanupOptionsMenu(GameManager* game) { if (game) cleanupMenu(&game->optionsState.optionsMenu); }
void cleanupScoresMenu(GameManager* game) { if (game) cleanupMenu(&game->scoresState.scoresMenu); }
void cleanupExitMenu(GameManager* game) { if (game) cleanupMenu(&game->menuState.exitMenu); }

// ================= RUNNER IMPLEMENTATION =================

void initRunner(GameManager* game) {
    if (!game->runnerState.runner_initialized) {
        game->runnerState.player_tex = IMG_LoadTexture(game->renderer, "assets/hero_spritesheet.png");
        game->runnerState.level_bg[0] = IMG_LoadTexture(game->renderer, "assets/niv1.jpg");
        game->runnerState.runner_initialized = 1;
    }
    game->runnerState.current_level = 1;
    game->runnerState.player_world_x = 0;
    game->runnerState.world_x = 0;
    game->runnerState.rPlayer = (RunnerEntity){0, 490, 0, 0, 32, 48, 1, 1, 100, 100, 0, 0, 0, 0, 0, 0, 0, 0};
    game->runnerState.level_timer_start = SDL_GetTicks();
    game->runnerState.current_scroll_speed = 3.0f;
}
void updateRunner(GameManager* game) {
    if (game->runnerState.r_game_over) { game->gameState = MENU_MAIN; return; }
    game->runnerState.player_world_x += game->runnerState.current_scroll_speed;
    game->runnerState.world_x = game->runnerState.player_world_x - 100;
}
void renderRunner(GameManager* game) {
    if (game->runnerState.level_bg[0]) SDL_RenderCopy(game->renderer, game->runnerState.level_bg[0], NULL, NULL);
    SDL_Rect r = {100, (int)game->runnerState.rPlayer.y, 32, 48};
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(game->renderer, &r);
}
void cleanupRunner(GameManager* game) {
    if (game->runnerState.player_tex) SDL_DestroyTexture(game->runnerState.player_tex);
    for (int i = 0; i < 3; i++) if (game->runnerState.level_bg[i]) SDL_DestroyTexture(game->runnerState.level_bg[i]);
}

// ================= LOADING IMPLEMENTATION =================

void startLoadingAnimation(GameManager* game) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    renderText(game, "Chargement...", 350, 280, (SDL_Color){255,255,255,255}, game->font);
    SDL_RenderPresent(game->renderer);
    SDL_Delay(500);
}

// ================= CLEANUP =================

void cleanupGame(GameManager* game) {
    if (!game) return;
    cleanupMainMenu(game); cleanupOptionsMenu(game); cleanupScoresMenu(game); cleanupExitMenu(game);
    cleanupPlayerMenu(game); cleanupSaveMenu(game); cleanupEnigmeMenu(game); cleanupAnimations(game);
    cleanupJoueur(&game->player); cleanupCollision(game); cleanupRunner(game); libererMinimap(game);
    if (game->font) TTF_CloseFont(game->font);
    if (game->titleFont) TTF_CloseFont(game->titleFont);
    if (game->background) SDL_DestroyTexture(game->background);
    if (game->zombieTex) SDL_DestroyTexture(game->zombieTex);
    if (game->bulletTex) SDL_DestroyTexture(game->bulletTex);
    if (game->qomblaTex) SDL_DestroyTexture(game->qomblaTex);
    for (int i = 0; i < 6; i++) if (game->obstacleTex[i]) SDL_DestroyTexture(game->obstacleTex[i]);
}

// ================= COLLISIONS & PLATEFORMES =================

int collisionAABB(SDL_Rect a, SDL_Rect b) {
    if ((a.x + a.w < b.x) || (a.x > b.x + b.w) || (a.y + a.h < b.y) || (a.y > b.y + b.h)) return 0;
    return 1;
}

int collisionBB(SDL_Rect* a, SDL_Rect* b) { return SDL_HasIntersection(a, b); }

void ajouterPlatFixe(GameManager* game, int x, int y, int w, int h, SDL_Color color) {
    if (game->nbPlateformes >= MAX_PLATEFORMES) return;
    Plateforme* p = &game->plateformes[game->nbPlateformes];
    p->rect = (SDL_Rect){x, y, w, h}; p->type = PLAT_FIXE; p->active = 1;
    p->color = color; p->texture = NULL; p->vitesse = 0; p->vieRestante = 0; p->animDestruction = 0;
    game->nbPlateformes++;
}

void ajouterPlatMobile(GameManager* game, int x, int y, int w, int h, SDL_Color color, PlatDirection dir, float vitesse, int minP, int maxP) {
    if (game->nbPlateformes >= MAX_PLATEFORMES) return;
    Plateforme* p = &game->plateformes[game->nbPlateformes];
    p->rect = (SDL_Rect){x, y, w, h}; p->type = PLAT_MOBILE; p->active = 1;
    p->color = color; p->texture = NULL; p->direction = dir; p->vitesse = vitesse;
    p->minPos = minP; p->maxPos = maxP; p->posActuelle = (dir == DIR_HORIZONTAL) ? (float)x : (float)y;
    p->sensDirection = 1; p->vieRestante = 0; p->animDestruction = 0;
    game->nbPlateformes++;
}

void ajouterPlatDestructible(GameManager* game, int x, int y, int w, int h, SDL_Color color, int vies) {
    if (game->nbPlateformes >= MAX_PLATEFORMES) return;
    Plateforme* p = &game->plateformes[game->nbPlateformes];
    p->rect = (SDL_Rect){x, y, w, h}; p->type = PLAT_DESTRUCTIBLE; p->active = 1;
    p->color = color; p->texture = NULL; p->vitesse = 0; p->vieRestante = vies; p->animDestruction = 0;
    game->nbPlateformes++;
}

void initPlateformes(GameManager* game) {
    if (!game) return;
    game->nbPlateformes = 0;
    SDL_Color gris  = {100, 100, 100, 255}, bleu  = {50, 100, 200, 255}, rouge = {200, 50, 50, 255};
    ajouterPlatFixe(game, 300, 400, 300, 30, gris);
    ajouterPlatFixe(game, 750, 420, 280, 30, gris);
    ajouterPlatFixe(game, 1400, 390, 300, 30, gris);
    ajouterPlatFixe(game, 2050, 410, 280, 30, gris);
    ajouterPlatFixe(game, 2750, 380, 280, 30, gris);
    ajouterPlatFixe(game, 3100, 410, 900, 30, gris);
    ajouterPlatMobile(game, 1050, 430, 250, 30, bleu, DIR_HORIZONTAL, 1.5f, 950, 1250);
    ajouterPlatMobile(game, 1900, 400, 250, 30, bleu, DIR_VERTICAL, 1.0f, 350, 450);
    ajouterPlatMobile(game, 2500, 420, 220, 30, bleu, DIR_HORIZONTAL, 2.0f, 2400, 2700);
    ajouterPlatDestructible(game, 550, 430, 200, 30, rouge, 3);
    ajouterPlatDestructible(game, 1750, 420, 200, 30, rouge, 2);
    ajouterPlatDestructible(game, 3200, 410, 200, 30, rouge, 1);
}

void loadCollisionMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "plat_fixe.png"); game->platFixeTex = IMG_LoadTexture(game->renderer, path);
    sprintf(path, "%s%s", ASSETS_PATH, "plat_mobile.png"); game->platMobileTex = IMG_LoadTexture(game->renderer, path);
    sprintf(path, "%s%s", ASSETS_PATH, "plat_destructible.png"); game->platDestructTex = IMG_LoadTexture(game->renderer, path);
}

void updatePlateformes(GameManager* game) {
    if (!game) return;
    for (int i = 0; i < game->nbPlateformes; i++) {
        Plateforme* p = &game->plateformes[i];
        if (!p->active) continue;
        if (p->type == PLAT_MOBILE) {
            p->posActuelle += p->vitesse * p->sensDirection;
            if (p->posActuelle >= p->maxPos) { p->posActuelle = (float)p->maxPos; p->sensDirection = -1; }
            if (p->posActuelle <= p->minPos) { p->posActuelle = (float)p->minPos; p->sensDirection = 1; }
            if (p->direction == DIR_HORIZONTAL) p->rect.x = (int)p->posActuelle; else p->rect.y = (int)p->posActuelle;
        }
        if (p->type == PLAT_DESTRUCTIBLE && p->animDestruction > 0) { p->animDestruction++; if (p->animDestruction > 30) p->active = 0; }
    }
}

void handleCollisions(GameManager* game) {
    if (!game) return;
    game->player.onGround = 0;
    if (game->player.pos.y + game->player.pos.h >= GROUND_Y) {
        game->player.y = (float)(GROUND_Y - game->player.pos.h); game->player.pos.y = (int)game->player.y;
        game->player.vy = 0; game->player.onGround = 1; game->player.isJumping = 0;
    }
    if (game->player.niveau != 1) return;
    for (int i = 0; i < game->nbPlateformes; i++) {
        Plateforme* p = &game->plateformes[i];
        if (!p->active || (p->type == PLAT_DESTRUCTIBLE && p->animDestruction > 0) || game->player.vy < 0) continue;
        SDL_Rect pr = game->player.pos;
        if (pr.x + pr.w > p->rect.x && pr.x < p->rect.x + p->rect.w && pr.y + pr.h >= p->rect.y && pr.y + pr.h <= p->rect.y + p->rect.h) {
            game->player.y = (float)(p->rect.y - game->player.pos.h); game->player.pos.y = (int)game->player.y;
            game->player.vy = 0; game->player.onGround = 1; game->player.isJumping = 0;
            if (p->type == PLAT_MOBILE && p->direction == DIR_HORIZONTAL) { game->player.x += (p->vitesse * p->sensDirection); game->player.pos.x = (int)game->player.x; }
            if (p->type == PLAT_DESTRUCTIBLE) {
                p->vieRestante--; joueurTakeDamage(&game->player, 5);
                if (p->vieRestante <= 0) { p->animDestruction = 1; lancerAnimation(game, p->rect.x, p->rect.y); }
            }
        }
    }
}

void renderPlateformes(GameManager* game) {
    if (!game) return;
    for (int i = 0; i < game->nbPlateformes; i++) {
        Plateforme* p = &game->plateformes[i];
        if (!p->active) continue;
        SDL_Rect sr = { p->rect.x - game->camera.x, p->rect.y - game->camera.y, p->rect.w, p->rect.h };
        if (sr.x + sr.w < 0 || sr.x > SCREEN_WIDTH) continue;
        SDL_Rect rr = sr; rr.y -= 25; rr.h = 50;
        if (p->type == PLAT_DESTRUCTIBLE && p->animDestruction > 0 && (p->animDestruction / 3) % 2 == 0) continue;
        SDL_Texture* tex = (p->type == PLAT_FIXE) ? game->platMobileTex : (p->type == PLAT_MOBILE ? game->platFixeTex : game->platDestructTex);
        if (tex) SDL_RenderCopy(game->renderer, tex, NULL, &rr);
        else { SDL_SetRenderDrawColor(game->renderer, p->color.r, p->color.g, p->color.b, p->color.a); SDL_RenderFillRect(game->renderer, &sr); }
    }
}

void cleanupCollision(GameManager* game) {
    if (!game) return;
    game->nbPlateformes = 0;
    if (game->platFixeTex) SDL_DestroyTexture(game->platFixeTex);
    if (game->platMobileTex) SDL_DestroyTexture(game->platMobileTex);
    if (game->platDestructTex) SDL_DestroyTexture(game->platDestructTex);
}

