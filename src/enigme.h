#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// Forward declaration
struct GameManager;

#define MAX_QUESTIONS 50

typedef struct {
    char question[256];
    char repA[100];
    char repB[100];
    char repC[100];
    int bonneRep;
    int deja_vu;
} Enigme;

typedef struct {
    SDL_Surface *surf;
    SDL_Texture *mainImg;
    SDL_Rect targetRect;
    SDL_Texture *pieces[3];
} PuzzleGame;

// ================= FONCTIONS =================

void initEnigmeMenu(struct GameManager* game);
void loadEnigmeMedia(struct GameManager* game);
void renderEnigmeMenu(struct GameManager* game);
void renderEnigme(struct GameManager* game);

/* --- Quiz & Puzzle logic --- */
int pickNextEnigme(struct GameManager* game);
void renderQuiz(struct GameManager* game);
void renderPuzzle(struct GameManager* game);
void handleEnigmeKeyPress(struct GameManager* game, SDL_Keycode key);
void handleEnigmeMouseMotion(struct GameManager* game, int x, int y);
void handleEnigmeMouseClick(struct GameManager* game, int x, int y);
void handleQuizMouseClick(struct GameManager* game, int x, int y);
void handleQuizMouseMotion(struct GameManager* game, int x, int y);
void handlePuzzleMouseClick(struct GameManager* game, int x, int y);
void handlePuzzleMouseMotion(struct GameManager* game, int x, int y);
void handlePuzzleMouseUp(struct GameManager* game, int x, int y);

void cleanupEnigmeMenu(struct GameManager* game);

void startQuiz(struct GameManager* game);
void startPuzzle(struct GameManager* game);
void quizRetourCb(struct GameManager* game);
void puzzleRetourCb(struct GameManager* game);
void resetPuzzle(struct GameManager* game);
void initQuestions(struct GameManager* game);

#endif

