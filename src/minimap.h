#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Forward declaration
struct GameManager;

// ================= MINIMAP =================
#define MINIMAP_W 200
#define MINIMAP_HEIGHT 120
#define MINIMAP_X (800 - MINIMAP_W - 15)
#define MINIMAP_Y 15
#define MINIMAP_PLAYER_SIZE 8
#define MINIMAP_REDIM 20  

typedef struct {
    SDL_Texture* backgroundTexture;
    SDL_Rect minimapPosition;
    SDL_Texture* playerTexture;
    SDL_Rect playerPosition;
} Minimap;

void initMinimap(struct GameManager* game);
void loadMinimapMedia(struct GameManager* game);
void MAJMinimap(struct GameManager* game);
void afficherMinimap(struct GameManager* game);
void libererMinimap(struct GameManager* game);

// ================= COLLISION PARFAITE =================
SDL_Color GetPixel(SDL_Surface *surface, int x, int y);
int collisionParfaite(SDL_Surface* mask, SDL_Rect posPerso);
int collisionParfaiteDirection(SDL_Surface* mask, SDL_Rect posPerso, int direction);

// ================= SAUVEGARDE / CHARGEMENT =================
#define NB_SLOTS 3
#define SAVE_MAGIC 0x53415645

typedef struct {
    unsigned int magic;
    char nomJoueur[50];
    int score;
    int vies;
    int niveau;
    float posX, posY;
    int volume;
    int fullscreen;
    int playerMode;
    int avatar;
    int valide; 
} Sauvegarde;

void initSaveMenu(struct GameManager* game);
void loadSaveMedia(struct GameManager* game);
void renderSaveMenu(struct GameManager* game);
void handleSaveKeyPress(struct GameManager* game, SDL_Keycode key);
void handleSaveMouseMotion(struct GameManager* game, int x, int y);
void handleSaveMouseClick(struct GameManager* game, int x, int y);
void cleanupSaveMenu(struct GameManager* game);

int sauvegarderPartie(int slot, Sauvegarde* data);
int chargerPartie(int slot, Sauvegarde* data);
int slotExiste(int slot);

#endif
