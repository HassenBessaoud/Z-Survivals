#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "background.h"
#include "joueur.h"
#include "ennemi.h"
#include "minimap.h"
#include "enigme.h"

// ================= PLATEFORMES & COLLISIONS =================
#define MAX_PLATEFORMES 20

typedef int PlatType;
#define PLAT_FIXE 0
#define PLAT_MOBILE 1
#define PLAT_DESTRUCTIBLE 2

typedef int PlatDirection;
#define DIR_HORIZONTAL 0
#define DIR_VERTICAL 1

typedef struct {
    SDL_Rect rect;
    PlatType type;
    int active;
    SDL_Color color;
    SDL_Texture* texture;
    PlatDirection direction;
    float vitesse;
    int minPos;
    int maxPos;
    float posActuelle;
    int sensDirection;
    int vieRestante;
    int animDestruction;
} Plateforme;

// ================= CONSTANTES GLOBALES =================
#define MAX_ENEMIES 5
#define MAX_BULLETS 20
#define MAX_BONUS 10
#define MAX_OBSTACLES 12
#define MAX_ATTACKS 5
#define MAX_ZOMBIES 5
#define MAX_BOSS_PROJ 10
#define MAX_LEVELS 3
#define INVINCIBLE_DURATION 1000

#define BOSS_W 220
#define BOSS_H 220
#define ZOMBIE_W 130
#define ZOMBIE_H 130
#define LEVEL_LENGTH 5000.0f

// ================= GESTION DES ETATS =================
typedef int GameState;
#define MENU_MAIN 0
#define MENU_OPTIONS 1
#define MENU_SCORES 2
#define MENU_ENIGME 3
#define MENU_QUIZ 4
#define MENU_PUZZLE 5
#define MENU_RUNNER 6
#define MENU_HISTOIRE 7
#define MENU_EXIT 8
#define MENU_LEVEL_COMPLETE 9
#define MENU_VICTORY 10
#define MENU_LOADING 11
#define MENU_SAVE 12
#define GAME_PLAY 13

// ================= STRUCTURES DE BASE =================
#define MAX_BUTTONS 30
#define MAX_INPUT_LENGTH 50

#define INSIDE(r, X, Y) ((X) >= (r).x && (X) <= (r).x + (r).w && (Y) >= (r).y && (Y) <= (r).y + (r).h)

typedef struct {
    SDL_Rect rect;
    char text[50];
    int selected;
    void (*onClick)(struct GameManager*);
    SDL_Texture* normalTexture;
    SDL_Texture* selectedTexture;
    int useTexture;
    int visible;
} Button;

typedef struct {
    SDL_Texture* background;
    SDL_Texture* logo;
    Mix_Music* music;
    Button buttons[MAX_BUTTONS];
    int buttonCount;
    char title[50];
} Menu;

// ================= MODULE STATES =================

typedef struct {
    Menu mainMenu;
    Menu exitMenu;
    Mix_Music* currentLoadedMusic;
    GameState lastState;
} MenuState;

typedef struct {
    int gameplayActive;
    Uint32 lastTransitionTime;
    SDL_Texture* qomblaTex;
} GameplayState;

typedef struct {
    Menu enigmeMenu;
    Mix_Chunk* correctSound;
    Mix_Chunk* wrongSound;
    Enigme questions[MAX_QUESTIONS];
    int nbQuestions;
    int currentQuestionIdx;
    int quizAnswerState;
    float rotoAngle;
    float rotoScale;
    Uint32 quizStartTime;
    Uint32 quizStateStartTime;
    int quizTempsMax;
    PuzzleGame puzzles[3];
    int currentPuzIdx;
    SDL_Rect pieceRects[3];
    SDL_Rect startPieceRects[3];
    int pieceMapping[3];
    int selectedPiece;
    int lastDroppedPiece;
    int pieceCorrect;
    int pieceWrong;
    Uint32 puzzleStartTime;
    int puzzleTempsMax;
    SDL_Texture* questionBg;
    SDL_Texture* quizBg;
    SDL_Texture* ansNormalTex;
    SDL_Texture* ansHoverTex;
    SDL_Texture* fffBtnTex;
    Button quizRetourBtn;
    Button puzzleRetourBtn;
} EnigmeState;

typedef struct {
    float x, y, vx, vy;
    int w, h;
    int active;
    int on_ground;
    int health, max_health;
    int score;
    int invincible;
    Uint32 invincible_start;
    int currentFrame;
    int direction; 
    Uint32 lastAnimTime;
    int tex_idx; 
    int hit;     
} RunnerEntity;

typedef struct {
    RunnerEntity rPlayer;
    int r_game_over;
    int r_victory;
    Uint32 r_total_game_time;
    Uint32 r_level_start_time;
    int r_last_score_reward_threshold;
    float player_world_x;
    RunnerEntity attacks[3];
    RunnerEntity bonuses[3];
    RunnerEntity projectiles[10];
    RunnerEntity obstacles[12];
    int obstacle_passed[12];
    int current_level;
    int attacks_active;
    int bonuses_spawned;
    int direction;
    float world_x;
    float level_start_x;
    Uint32 last_attack_spawn;
    Uint32 last_bonus_spawn;
    Uint32 last_obstacle_spawn;
    int attack_spawn_delay;
    int obstacle_spawn_interval;
    Uint32 level_timer_start;
    float current_scroll_speed;
    float level_elapsed_seconds;
    int player_speed;
    SDL_Texture *player_tex;
    SDL_Texture *level_bg[3];
    int runner_initialized;
    Uint32 runner_last_shot;
} RunnerState;

typedef struct {
    SDL_Texture* explosionSpriteSheet;
    int explosionSheetW;
    int explosionSheetH;
    int explosionNbFrames;
} AnimationState;

typedef struct {
    Menu scoresMenu;
    int showScoresDisplay;
    int inputActive;
    char currentInput[MAX_INPUT_LENGTH];
    char playerName[MAX_INPUT_LENGTH];
} ScoresState;

typedef struct {
    Menu playerMenu;
    int showAvatarChoices;
    int selectedPlayerMode;
    int selectedAvatar1;
    int selectedAvatar2;
} PlayerMenuState;

typedef struct {
    Menu saveMenu;
    int showSaveOptions;
    int showSlotSelection;
    int selectedSlot;
    int saveMode;
} SaveState;

typedef struct {
    Menu optionsMenu;
} OptionsState;

typedef struct {
    Menu histoireMenu;
} HistoireState;

// ================= GAME MANAGER =================
typedef struct {
    SDL_Rect rect;
    float vx, vy;
    int active;
} Bullet;

typedef struct {
    SDL_Rect rect;
    float x, y;
    float vx, vy;
    int active;
} BossProj;

typedef struct {
    SDL_Rect rect;
    int type;
    int active;
    SDL_Texture* texture;
} Bonus;

typedef struct {
    SDL_Rect rect;
    int active;
    int triggered;
} QuizZone;

typedef struct {
    SDL_Rect rect;
    float vx, vy;
    int active;
    int tex_idx;
    int hit;
    int passed;
} Obstacle;

typedef struct {
    SDL_Rect rect;
    float vx, vy;
    int active;
} Attack;

typedef struct GameManager {
    GameState gameState;
    GameState previousState;
    
    int globalScore;
    int totalTime; 
    Uint32 startTime;
    int ammo;
    
    Joueur player;
    Ennemi enemies[MAX_ENEMIES];
    Ennemi zombies[MAX_ZOMBIES];
    Ennemi boss;
    Bullet bullets[MAX_BULLETS];
    Bonus bonuses[MAX_BONUS];
    Obstacle obstacles[MAX_OBSTACLES];
    Attack attacks[MAX_ATTACKS];
    QuizZone quizZones[2];
    BossProj bossProj[MAX_BOSS_PROJ];

    int currentLevel;
    int victory;
    int gameOver;
    Uint32 levelStartTime;
    Uint32 lastAttackSpawn;
    Uint32 lastBonusSpawn;
    Uint32 lastObstacleSpawn;
    float lastObstacleSpawnX;
    int attacksActive;
    int bonusesSpawned;
    int lastScoreRewardThreshold;

    int bossActive;
    int bossPhase;
    Uint32 bossLastShot;
    int bossSpawnedMinion;

    Enigme currentEnigme;
    int quizActive;
    int quizResult; 

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* titleFont;
    Mix_Chunk* hoverSound;
    Mix_Chunk* clickSound;
    int volume;
    int fullscreen;
    SDL_Texture* background;
    SDL_Rect camera;

    Minimap minimap;

    Plateforme plateformes[MAX_PLATEFORMES];
    int nbPlateformes;
    SDL_Surface* maskSurface;
    SDL_Texture* platFixeTex;
    SDL_Texture* platMobileTex;
    SDL_Texture* platDestructTex;

    AnimBackground animations[MAX_ANIMATIONS];
    int nbAnimations;

    SDL_Texture* zombieTex;
    SDL_Texture* obstacleTex[6];
    SDL_Texture* bulletTex;
    SDL_Texture* qomblaTex;

    MenuState menuState;
    GameplayState gameplayState;
    EnigmeState enigmeState;
    RunnerState runnerState;
    AnimationState animationState;
    ScoresState scoresState;
    PlayerMenuState playerMenuState;
    SaveState saveState;
    OptionsState optionsState;
    HistoireState histoireState;

} GameManager;

// ================= FONCTIONS GLOBALES =================

void initGame(GameManager* game, SDL_Renderer* renderer);
void updateGame(GameManager* game);
void renderGame(GameManager* game);
void cleanupGame(GameManager* game);

/* --- Collisions --- */
int collisionAABB(SDL_Rect a, SDL_Rect b);
int collisionBB(SDL_Rect* a, SDL_Rect* b);
void initPlateformes(GameManager* game);
void loadCollisionMedia(GameManager* game);
void updatePlateformes(GameManager* game);
void renderPlateformes(GameManager* game);
void handleCollisions(GameManager* game);
void cleanupCollision(GameManager* game);
void ajouterPlatFixe(GameManager* game, int x, int y, int w, int h, SDL_Color color);
void ajouterPlatMobile(GameManager* game, int x, int y, int w, int h, SDL_Color color, PlatDirection dir, float vitesse, int minP, int maxP);
void ajouterPlatDestructible(GameManager* game, int x, int y, int w, int h, SDL_Color color, int vies);

/* --- Utilitaires --- */
void renderText(GameManager* game, const char* text, int x, int y, SDL_Color color, TTF_Font* f);
void renderButton(GameManager* game, Button* btn);
void updateVolume(GameManager* game, int delta);
void toggleFullscreen(GameManager* game);
void changeMusic(GameManager* game, GameState newState);
void handleMenuMouseMotion(GameManager* game, Menu* menu, int x, int y);
void handleMenuMouseClick(GameManager* game, Menu* menu, int x, int y);
void cleanupMenu(Menu* menu);
void loadButtonTexture(GameManager* game, Button* btn, const char* normalFile, const char* selectedFile);

/* --- Menus --- */
void initMainMenu(GameManager* game);
void loadMainMenuMedia(GameManager* game);
void renderMainMenu(GameManager* game);
void handleMainMenuKeyPress(GameManager* game, SDL_Keycode key);
void cleanupMainMenu(GameManager* game);

void initOptionsMenu(GameManager* game);
void loadOptionsMedia(GameManager* game);
void renderOptionsMenu(GameManager* game);
void handleOptionsKeyPress(GameManager* game, SDL_Keycode key);
void cleanupOptionsMenu(GameManager* game);

void initScoresMenu(GameManager* game);
void loadScoresMedia(GameManager* game);
void renderScoresMenu(GameManager* game);
void handleScoresKeyPress(GameManager* game, SDL_Keycode key);
void cleanupScoresMenu(GameManager* game);

void initExitMenu(GameManager* game);
void loadExitMedia(GameManager* game);
void renderExitMenu(GameManager* game);
void handleExitKeyPress(GameManager* game, SDL_Keycode key);
void cleanupExitMenu(GameManager* game);

/* --- Runner --- */
void initRunner(GameManager* game);
void updateRunner(GameManager* game);
void renderRunner(GameManager* game);
void cleanupRunner(GameManager* game);

/* --- Loading --- */
void startLoadingAnimation(GameManager* game);

#endif

