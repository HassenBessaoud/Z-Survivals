#ifndef JOUEUR_H
#define JOUEUR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Forward declaration
struct GameManager;

// ================= STRUCTURE JOUEUR =================
typedef struct {
    // Position et Physique
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Rect pos;      
    SDL_Rect hitbox;   

    // Mouvement
    float speed;
    float gravity;
    int onGround;
    int isJumping;
    int facingRight;
    int isMoving;
    int direction; 
    int keyLeft, keyRight; 

    // Animation
    SDL_Texture* texture;
    SDL_Texture* spriteSheet; 
    SDL_Rect spriteRect;
    int currentAnim;
    int currentFrame;
    int totalFrames;
    int animTimer;
    int animSpeed;
    Uint32 lastAnimTime;

    // Gameplay Stats
    int health;
    int maxHealth;
    int score;
    int lives;
    int vies; 
    int ammo;
    int niveau;

    // État
    int isInvincible;
    int invincible; 
    Uint32 invincibleTimer;
    Uint32 invincibleStart;

} Joueur;

// ================= FONCTIONS JOUEUR =================
void initJoueur(Joueur* j, struct GameManager* game);
void updateJoueur(Joueur* j, struct GameManager* game, Uint32 dt);
void renderJoueur(Joueur* j, struct GameManager* game, SDL_Rect camera);
void handleJoueurInput(Joueur* j, struct GameManager* game, SDL_Event* event);

void joueurJump(Joueur* j);
void joueurShoot(Joueur* j, struct GameManager* game);
void joueurTakeDamage(Joueur* j, int damage);
void cleanupJoueur(Joueur* j);

// ================= MENU SELECTION =================
void initPlayerMenu(struct GameManager* game);
void loadPlayerMedia(struct GameManager* game);
void renderPlayerMenu(struct GameManager* game);
void handlePlayerKeyPress(struct GameManager* game, SDL_Keycode key);
void handlePlayerMouseClick(struct GameManager* game, int x, int y);
void handlePlayerMouseMotion(struct GameManager* game, int x, int y);
void cleanupPlayerMenu(struct GameManager* game);

#endif
