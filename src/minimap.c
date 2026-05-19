#include "minimap.h"
#include "game.h"
#include "background.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ================= IMPLEMENTATION MINIMAP =================

void initMinimap(GameManager* game) {
    if (!game) return;
    game->minimap.minimapPosition = (SDL_Rect){MINIMAP_X, MINIMAP_Y, MINIMAP_W, MINIMAP_HEIGHT};
    game->minimap.playerPosition = (SDL_Rect){MINIMAP_X, MINIMAP_Y, MINIMAP_PLAYER_SIZE, MINIMAP_PLAYER_SIZE};
}

void loadMinimapMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    sprintf(path, "%s%s", ASSETS_PATH, "minimap.png");
    game->minimap.backgroundTexture = IMG_LoadTexture(game->renderer, path);
    if (!game->minimap.backgroundTexture) {
        sprintf(path, "%s%s", ASSETS_PATH, "minimap_level1.png");
        game->minimap.backgroundTexture = IMG_LoadTexture(game->renderer, path);
    }
    
    SDL_Surface* s = SDL_CreateRGBSurface(0, MINIMAP_PLAYER_SIZE, MINIMAP_PLAYER_SIZE, 32, 0, 0, 0, 0);
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 255, 0, 0));
    game->minimap.playerTexture = SDL_CreateTextureFromSurface(game->renderer, s);
    SDL_FreeSurface(s);
}

void MAJMinimap(GameManager* game) {
    if (!game) return;
    float ratioX = (float)MINIMAP_W / LEVEL_WIDTH;
    game->minimap.playerPosition.x = (int)(MINIMAP_X + game->player.pos.x * ratioX);
    game->minimap.playerPosition.y = (int)(MINIMAP_Y + (game->player.pos.y * MINIMAP_HEIGHT) / SCREEN_HEIGHT);
}

void afficherMinimap(GameManager* game) {
    if (!game) return;
    SDL_RenderCopy(game->renderer, game->minimap.backgroundTexture, NULL, &game->minimap.minimapPosition);
    SDL_RenderCopy(game->renderer, game->minimap.playerTexture, NULL, &game->minimap.playerPosition);
}

void libererMinimap(GameManager* game) {
    if (!game) return;
    if (game->minimap.backgroundTexture) SDL_DestroyTexture(game->minimap.backgroundTexture);
    if (game->minimap.playerTexture) SDL_DestroyTexture(game->minimap.playerTexture);
}

// ================= COLLISION PARFAITE =================

SDL_Color GetPixel(SDL_Surface *surface, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};
    if (!surface) return color;
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return color;
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    Uint8 *pPosition = (Uint8 *)surface->pixels + (y * surface->pitch) + (x * surface->format->BytesPerPixel);
    Uint32 pixel_value = 0;
    memcpy(&pixel_value, pPosition, surface->format->BytesPerPixel);
    SDL_GetRGB(pixel_value, surface->format, &color.r, &color.g, &color.b);
    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    return color;
}

int collisionParfaite(SDL_Surface* mask, SDL_Rect posPerso) {
    if (!mask) return 0;
    int Pos[8][2];
    Pos[0][0] = posPerso.x;                 Pos[0][1] = posPerso.y;
    Pos[1][0] = posPerso.x + posPerso.w/2; Pos[1][1] = posPerso.y;
    Pos[2][0] = posPerso.x + posPerso.w;     Pos[2][1] = posPerso.y;
    Pos[3][0] = posPerso.x;                 Pos[3][1] = posPerso.y + posPerso.h/2;
    Pos[4][0] = posPerso.x;                 Pos[4][1] = posPerso.y + posPerso.h;
    Pos[5][0] = posPerso.x + posPerso.w/2; Pos[5][1] = posPerso.y + posPerso.h;
    Pos[6][0] = posPerso.x + posPerso.w;     Pos[6][1] = posPerso.y + posPerso.h;
    Pos[7][0] = posPerso.x + posPerso.w;     Pos[7][1] = posPerso.y + posPerso.h/2;

    for (int i = 0; i < 8; i++) {
        int px = Pos[i][0];
        int py = Pos[i][1];
        if (px >= 0 && px < mask->w && py >= 0 && py < mask->h) {
            SDL_Color c = GetPixel(mask, px, py);
            if (c.r == 0 && c.g == 0 && c.b == 0) return 1;
        }
    }
    return 0;
}

int collisionParfaiteDirection(SDL_Surface* mask, SDL_Rect posPerso, int direction) {
    if (!mask) return 0;
    int x1 = posPerso.x, x2 = posPerso.x + posPerso.w;
    int y1 = posPerso.y, y2 = posPerso.y + posPerso.h;
    switch (direction) {
        case 0: for (int y = y1 + 5; y < y2 - 5; y++) if (GetPixel(mask, x2, y).r == 0) return 1; break;
        case 1: for (int y = y1 + 5; y < y2 - 5; y++) if (GetPixel(mask, x1, y).r == 0) return 1; break;
        case 2: for (int x = x1 + 5; x < x2 - 5; x++) if (GetPixel(mask, x, y1).r == 0) return 1; break;
        case 3: for (int x = x1 + 5; x < x2 - 5; x++) if (GetPixel(mask, x, y2).r == 0) return 1; break;
    }
    return 0;
}

// ================= SAUVEGARDE / CHARGEMENT =================

void getSaveFilename(int slot, char* filename, int size) { snprintf(filename, size, "save%d.dat", slot); }

int sauvegarderPartie(int slot, Sauvegarde* data) {
    char filename[64]; getSaveFilename(slot, filename, sizeof(filename));
    FILE* f = fopen(filename, "wb");
    if (!f) return 0;
    data->magic = SAVE_MAGIC; data->valide = 1;
    size_t written = fwrite(data, sizeof(Sauvegarde), 1, f);
    fclose(f);
    return (written == 1);
}

int chargerPartie(int slot, Sauvegarde* data) {
    char filename[64]; getSaveFilename(slot, filename, sizeof(filename));
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;
    size_t rd = fread(data, sizeof(Sauvegarde), 1, f);
    fclose(f);
    return (rd == 1 && data->magic == SAVE_MAGIC);
}

int slotExiste(int slot) {
    char filename[64]; getSaveFilename(slot, filename, sizeof(filename));
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;
    Sauvegarde temp; size_t rd = fread(&temp, sizeof(Sauvegarde), 1, f);
    fclose(f);
    return (rd == 1 && temp.magic == SAVE_MAGIC);
}

void showSlotButtons(GameManager* game) {
    game->saveState.saveMenu.buttons[5].visible = 1;
    game->saveState.saveMenu.buttons[6].visible = 1;
}

void hideSlotButtons(GameManager* game) {
    game->saveState.saveMenu.buttons[5].visible = 0;
    game->saveState.saveMenu.buttons[6].visible = 0;
}

void saveOui(GameManager* game) {
    game->saveState.showSaveOptions = 1;
    game->saveState.saveMenu.buttons[2].visible = 1;
    game->saveState.saveMenu.buttons[3].visible = 1;
}

void saveNon(GameManager* game) {
    game->gameState = MENU_MAIN; 
    game->saveState.showSaveOptions = 0; 
    game->saveState.showSlotSelection = 0;
    changeMusic(game, MENU_MAIN);
}

void saveCharger(GameManager* game) {
    game->saveState.saveMode = 1; game->saveState.showSlotSelection = 1; showSlotButtons(game);
}

void saveNouvelle(GameManager* game) {
    game->saveState.saveMode = 0; game->saveState.showSlotSelection = 1; showSlotButtons(game);
}

void saveRetour(GameManager* game) {
    game->gameState = MENU_MAIN; game->saveState.showSaveOptions = 0; game->saveState.showSlotSelection = 0;
    hideSlotButtons(game); game->saveState.saveMenu.buttons[2].visible = 0; game->saveState.saveMenu.buttons[3].visible = 0;
    changeMusic(game, MENU_MAIN);
}

void selectSlot(GameManager* game, int slot) {
    game->saveState.selectedSlot = slot;
    if (game->saveState.saveMode == 0) {
        startLoadingAnimation(game); initGameplay(game); 
        game->player.niveau = 1; loadLevel(game, 1);
        game->gameState = GAME_PLAY; changeMusic(game, GAME_PLAY);
    } else {
        Sauvegarde data;
        if (chargerPartie(slot, &data)) {
            startLoadingAnimation(game); initGameplay(game);
            game->player.niveau = data.niveau; game->player.score = data.score; game->player.health = data.vies;
            loadLevel(game, data.niveau); game->gameState = GAME_PLAY; changeMusic(game, GAME_PLAY);
        }
    }
}

void slot1(GameManager* game) { selectSlot(game, 1); }
void slot2(GameManager* game) { selectSlot(game, 2); }

void initSaveMenu(GameManager* game) {
    if (!game) return;
    strcpy(game->saveState.saveMenu.title, "SAUVEGARDE / CHARGEMENT");
    game->saveState.saveMenu.buttonCount = 7;
    game->saveState.saveMenu.buttons[0] = (Button){{250, 250, 120, 50}, "OUI",     0, saveOui,      NULL, NULL, 0, 1};
    game->saveState.saveMenu.buttons[1] = (Button){{430, 250, 120, 50}, "NON",     0, saveNon,      NULL, NULL, 0, 1};
    game->saveState.saveMenu.buttons[2] = (Button){{200, 350, 180, 55}, "CHARGER", 0, saveCharger,  NULL, NULL, 0, 1};
    game->saveState.saveMenu.buttons[3] = (Button){{420, 350, 180, 55}, "NOUVELLE",0, saveNouvelle, NULL, NULL, 0, 1};
    game->saveState.saveMenu.buttons[4] = (Button){{300, 580, 200, 55}, "RETOUR",  0, saveRetour,    NULL, NULL, 0, 1};
    game->saveState.saveMenu.buttons[5] = (Button){{225, 480, 150, 50}, "SLOT 1",  0, slot1,        NULL, NULL, 0, 0};
    game->saveState.saveMenu.buttons[6] = (Button){{425, 480, 150, 50}, "SLOT 2",  0, slot2,        NULL, NULL, 0, 0};
}

void loadSaveMedia(GameManager* game) {
    if (!game) return;
    char path[256]; sprintf(path, "%s%s", ASSETS_PATH, "a1.jpg");
    game->saveState.saveMenu.background = IMG_LoadTexture(game->renderer, path);
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[0], "j1.jpg", "j11.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[1], "j2.jpg", "j22.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[2], "j3.jpg", "j33.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[3], "j4.jpg", "j44.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[4], "j5.jpg", "j55.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[5], "av1.jpg", "av11.jpg");
    loadButtonTexture(game, &game->saveState.saveMenu.buttons[6], "av2.jpg", "av22.jpg");
}

void handleSaveKeyPress(GameManager* game, SDL_Keycode key) { if (key == SDLK_ESCAPE) saveRetour(game); }
void handleSaveMouseMotion(GameManager* game, int x, int y) { if (game) handleMenuMouseMotion(game, &game->saveState.saveMenu, x, y); }
void handleSaveMouseClick(GameManager* game, int x, int y) { if (game) handleMenuMouseClick(game, &game->saveState.saveMenu, x, y); }

void renderSaveMenu(GameManager* game) {
    if (!game) return;
    if (game->saveState.saveMenu.background) SDL_RenderCopy(game->renderer, game->saveState.saveMenu.background, NULL, NULL);
    renderText(game, game->saveState.saveMenu.title, 200, 80, (SDL_Color){255, 255, 255, 255}, game->titleFont);
    renderText(game, "Continuer la partie ?", 280, 180, (SDL_Color){255, 255, 255, 255}, game->font);
    for (int i = 0; i < game->saveState.saveMenu.buttonCount; i++) renderButton(game, &game->saveState.saveMenu.buttons[i]);
}

void cleanupSaveMenu(GameManager* game) { if (game) cleanupMenu(&game->saveState.saveMenu); }
