#include "enigme.h"
#include "game.h"
#include "joueur.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ================= IMPLEMENTATION ENIGMES =================

void initQuestions(GameManager* game) {
    if (!game) return;
    FILE *f = fopen("assets/enigmes.txt", "r");
    if (!f) f = fopen("enigmes.txt", "r");
    
    game->enigmeState.nbQuestions = 0;
    if (f) {
        while (game->enigmeState.nbQuestions < MAX_QUESTIONS && fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%d\n", 
               game->enigmeState.questions[game->enigmeState.nbQuestions].question, 
               game->enigmeState.questions[game->enigmeState.nbQuestions].repA, 
               game->enigmeState.questions[game->enigmeState.nbQuestions].repB, 
               game->enigmeState.questions[game->enigmeState.nbQuestions].repC, 
               &game->enigmeState.questions[game->enigmeState.nbQuestions].bonneRep) != EOF) {
            game->enigmeState.questions[game->enigmeState.nbQuestions].deja_vu = 0;
            game->enigmeState.nbQuestions++;
        }
        fclose(f);
    } else {
        strcpy(game->enigmeState.questions[0].question, "Quel est l'element chimique dont le symbole est O ?");
        strcpy(game->enigmeState.questions[0].repA, "Or");
        strcpy(game->enigmeState.questions[0].repB, "Oxygene");
        strcpy(game->enigmeState.questions[0].repC, "Osmium");
        game->enigmeState.questions[0].bonneRep = 1;
        game->enigmeState.nbQuestions = 1;
    }
}

int pickNextEnigme(GameManager* game) {
    if (!game || game->enigmeState.nbQuestions == 0) return 0;
    int count = 0;
    for (int i = 0; i < game->enigmeState.nbQuestions; i++) if (!game->enigmeState.questions[i].deja_vu) count++;
    
    if (count == 0) {
        for (int i = 0; i < game->enigmeState.nbQuestions; i++) game->enigmeState.questions[i].deja_vu = 0;
        count = game->enigmeState.nbQuestions;
    }
    
    int r = rand() % count;
    int current = 0;
    for (int i = 0; i < game->enigmeState.nbQuestions; i++) {
        if (!game->enigmeState.questions[i].deja_vu) {
            if (current == r) {
                game->enigmeState.questions[i].deja_vu = 1;
                game->currentEnigme = game->enigmeState.questions[i];
                return i;
            }
            current++;
        }
    }
    return 0;
}

void resetPuzzle(GameManager* game) {
    if (!game) return;
    
    game->enigmeState.rotoAngle = 0.0f;
    game->enigmeState.rotoScale = 0.1f;
    game->enigmeState.pieceCorrect = 0;
    game->enigmeState.pieceWrong = 0;
    game->enigmeState.currentPuzIdx = rand() % 3;
    PuzzleGame* p = &game->enigmeState.puzzles[game->enigmeState.currentPuzIdx];
    
    if (!p->surf) return;

    for (int i = 0; i < 3; i++) {
        if (p->pieces[i]) SDL_DestroyTexture(p->pieces[i]);
        p->pieces[i] = NULL;
    }

    int targetX = 50 + rand() % (500 - 150);
    int targetY = 100 + rand() % (400 - 100);
    p->targetRect = (SDL_Rect){targetX, targetY, 150, 100};

    float ratioX = (float)p->surf->w / 500.0f;
    float ratioY = (float)p->surf->h / 400.0f;

    SDL_Rect srcCorrect;
    srcCorrect.x = (int)((p->targetRect.x - 50) * ratioX);
    srcCorrect.y = (int)((p->targetRect.y - 100) * ratioY);
    srcCorrect.w = (int)(p->targetRect.w * ratioX);
    srcCorrect.h = (int)(p->targetRect.h * ratioY);

    SDL_Surface* dest = SDL_CreateRGBSurface(0, srcCorrect.w, srcCorrect.h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(p->surf, &srcCorrect, dest, NULL);
    p->pieces[0] = SDL_CreateTextureFromSurface(game->renderer, dest);
    SDL_FreeSurface(dest);

    for (int i = 1; i < 3; i++) {
        SDL_Rect srcFalse;
        srcFalse.x = rand() % (p->surf->w - srcCorrect.w);
        srcFalse.y = rand() % (p->surf->h - srcCorrect.h);
        srcFalse.w = srcCorrect.w;
        srcFalse.h = srcCorrect.h;
        
        dest = SDL_CreateRGBSurface(0, srcFalse.w, srcFalse.h, 32, 0, 0, 0, 0);
        SDL_BlitSurface(p->surf, &srcFalse, dest, NULL);
        p->pieces[i] = SDL_CreateTextureFromSurface(game->renderer, dest);
        SDL_FreeSurface(dest);
    }

    game->enigmeState.selectedPiece = -1;
    game->enigmeState.lastDroppedPiece = -1;
    game->enigmeState.pieceCorrect = 0;
    game->enigmeState.puzzleStartTime = SDL_GetTicks();

    for (int i = 0; i < 3; i++) game->enigmeState.pieceMapping[i] = i;
    for (int i = 0; i < 3; i++) {
        int r = rand() % 3;
        int tmp = game->enigmeState.pieceMapping[i];
        game->enigmeState.pieceMapping[i] = game->enigmeState.pieceMapping[r];
        game->enigmeState.pieceMapping[r] = tmp;
    }

    for (int i = 0; i < 3; i++) {
        game->enigmeState.startPieceRects[i] = (SDL_Rect){620, 80 + i * 160, 140, 90};
        game->enigmeState.pieceRects[i] = game->enigmeState.startPieceRects[i];
    }
}

void startQuiz(GameManager* game) { 
    if (!game) return;
    game->player.keyLeft = 0;
    game->player.keyRight = 0;
    game->player.vx = 0;
    game->enigmeState.rotoAngle = 0.0f;
    game->enigmeState.rotoScale = 0.1f;
    if (game->player.lives <= 0) game->player.lives = 2;
    game->enigmeState.quizAnswerState = 0;
    game->enigmeState.currentQuestionIdx = pickNextEnigme(game);
    game->enigmeState.quizStartTime = SDL_GetTicks();
    game->gameState = MENU_QUIZ; 
    if (game->enigmeState.enigmeMenu.music) Mix_PlayMusic(game->enigmeState.enigmeMenu.music, -1); 
}

void startPuzzle(GameManager* game) { 
    if (!game) return;
    game->player.keyLeft = 0;
    game->player.keyRight = 0;
    game->player.vx = 0;
    if (game->player.lives <= 0) game->player.lives = 2;
    resetPuzzle(game);
    game->gameState = MENU_PUZZLE; 
    if (game->enigmeState.enigmeMenu.music) Mix_PlayMusic(game->enigmeState.enigmeMenu.music, -1); 
}

void quizRetourCb(GameManager* game) { 
    if (!game) return;
    game->gameState = game->previousState; 
    changeMusic(game, game->previousState); 
}
void puzzleRetourCb(GameManager* game) { 
    if (!game) return;
    game->gameState = game->previousState; 
    changeMusic(game, game->previousState); 
}

void initEnigmeMenu(GameManager* game) {
    if (!game) return;
    game->enigmeState.quizTempsMax = 20; 
    game->enigmeState.puzzleTempsMax = 30;
    strcpy(game->enigmeState.enigmeMenu.title, "ENIGMES"); game->enigmeState.enigmeMenu.buttonCount = 3;
    game->enigmeState.enigmeMenu.buttons[0] = (Button){{150, 300, 200, 80}, "Quiz",   0, startQuiz,   NULL, NULL, 0, 1};
    game->enigmeState.enigmeMenu.buttons[1] = (Button){{450, 300, 200, 80}, "Puzzle", 0, startPuzzle, NULL, NULL, 0, 1};
    game->enigmeState.enigmeMenu.buttons[2] = (Button){{300, 500, 200, 60}, "Retour", 0, quizRetourCb, NULL, NULL, 0, 1};
    
    game->enigmeState.quizRetourBtn = (Button){{10, 10, 120, 40}, "Retour", 0, quizRetourCb, NULL, NULL, 0, 1};
    game->enigmeState.puzzleRetourBtn = (Button){{10, 550, 120, 40}, "Retour", 0, puzzleRetourCb, NULL, NULL, 0, 1};
    
    initQuestions(game);
}

void loadEnigmeMedia(GameManager* game) {
    if (!game) return;
    char path[256];
    
    sprintf(path, "%s%s", ASSETS_PATH, "bg.jpeg");
    game->enigmeState.enigmeMenu.background = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.enigmeMenu.background) {
        sprintf(path, "assets/bg.jpeg");
        game->enigmeState.enigmeMenu.background = IMG_LoadTexture(game->renderer, path);
    }
    
    sprintf(path, "%s%s", ASSETS_PATH, "tmbr.mp3"); game->enigmeState.enigmeMenu.music = Mix_LoadMUS(path);
    if (!game->enigmeState.enigmeMenu.music) {
        sprintf(path, "assets/tmbr.mp3");
        game->enigmeState.enigmeMenu.music = Mix_LoadMUS(path);
    }

    sprintf(path, "%s%s", ASSETS_PATH, "effect.wav"); game->enigmeState.correctSound = Mix_LoadWAV(path); game->enigmeState.wrongSound = Mix_LoadWAV(path);
    if (!game->enigmeState.correctSound) {
        sprintf(path, "assets/effect.wav");
        game->enigmeState.correctSound = Mix_LoadWAV(path);
        game->enigmeState.wrongSound = Mix_LoadWAV(path);
    }
    
    loadButtonTexture(game, &game->enigmeState.enigmeMenu.buttons[0], "quin.jpg", "qui.jpg");
    loadButtonTexture(game, &game->enigmeState.enigmeMenu.buttons[1], "pn.jpg", "puz.jpg");
    loadButtonTexture(game, &game->enigmeState.enigmeMenu.buttons[2], "ret1.jpg", "ret11.jpg");
    loadButtonTexture(game, &game->enigmeState.quizRetourBtn, "ret1.jpg", "ret11.jpg");
    loadButtonTexture(game, &game->enigmeState.puzzleRetourBtn, "ret1.jpg", "ret11.jpg");
    
    sprintf(path, "assets/fff.jpeg");
    game->enigmeState.fffBtnTex = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.fffBtnTex) {
        sprintf(path, "%s%s", ASSETS_PATH, "fff.jpeg");
        game->enigmeState.fffBtnTex = IMG_LoadTexture(game->renderer, path);
    }
    
    sprintf(path, "%s%s", ASSETS_PATH, "qes.jpg"); game->enigmeState.questionBg = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.questionBg) {
        sprintf(path, "assets/qes.jpg");
        game->enigmeState.questionBg = IMG_LoadTexture(game->renderer, path);
    }

    sprintf(path, "%s%s", ASSETS_PATH, "bg.jpeg");
    game->enigmeState.quizBg = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.quizBg) {
        sprintf(path, "assets/bg.jpeg");
        game->enigmeState.quizBg = IMG_LoadTexture(game->renderer, path);
    }

    sprintf(path, "%s%s", ASSETS_PATH, "rrr.png");
    game->enigmeState.ansNormalTex = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.ansNormalTex) {
        sprintf(path, "assets/rrr.png");
        game->enigmeState.ansNormalTex = IMG_LoadTexture(game->renderer, path);
    }

    sprintf(path, "%s%s", ASSETS_PATH, "rr2.png");
    game->enigmeState.ansHoverTex = IMG_LoadTexture(game->renderer, path);
    if (!game->enigmeState.ansHoverTex) {
        sprintf(path, "assets/rr2.png");
        game->enigmeState.ansHoverTex = IMG_LoadTexture(game->renderer, path);
    }

    for (int i = 0; i < 3; i++) {
        sprintf(path, "%simage%d.jpg", ASSETS_PATH, i + 1);
        game->enigmeState.puzzles[i].surf = IMG_Load(path);
        if (!game->enigmeState.puzzles[i].surf) {
            sprintf(path, "assets/image%d.jpg", i + 1);
            game->enigmeState.puzzles[i].surf = IMG_Load(path);
        }
        
        if (game->enigmeState.puzzles[i].surf) game->enigmeState.puzzles[i].mainImg = SDL_CreateTextureFromSurface(game->renderer, game->enigmeState.puzzles[i].surf);
        game->enigmeState.puzzles[i].pieces[0] = game->enigmeState.puzzles[i].pieces[1] = game->enigmeState.puzzles[i].pieces[2] = NULL;
    }
}

void handleEnigmeKeyPress(GameManager* game, SDL_Keycode key) {
    if (!game) return;

    if (game->quizActive) {
        int choice = -1;
        if (key == SDLK_a || key == SDLK_1) choice = 1;
        else if (key == SDLK_b || key == SDLK_2) choice = 2;
        else if (key == SDLK_c || key == SDLK_3) choice = 3;

        if (choice != -1) {
            if (choice == game->currentEnigme.bonneRep) {
                game->quizResult = 1;
                if (game->player.health < 100) {
                    game->player.health += 20;
                    if (game->player.health > 100) game->player.health = 100;
                } else {
                    game->player.score += 200; 
                }
                game->player.score += 100;
            } else {
                game->quizResult = -1;
                game->player.score -= 50; 
                if (game->player.score < 0) game->player.score = 0;
                joueurTakeDamage(&game->player, 10);
            }
            game->quizActive = 0;
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
        }
    } else if (game->gameState == MENU_ENIGME) {
        if (key == SDLK_ESCAPE || key == SDLK_BACKSPACE) {
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
        }
    } else if (game->gameState == MENU_QUIZ) {
        if (key == SDLK_ESCAPE || game->enigmeState.quizAnswerState != 0) {
            game->enigmeState.quizAnswerState = 0;
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
            return;
        }
        if (game->enigmeState.quizAnswerState == 0) {
            int choice = -1;
            if (key == SDLK_a || key == SDLK_1) choice = 1;
            else if (key == SDLK_b || key == SDLK_2) choice = 2;
            else if (key == SDLK_c || key == SDLK_3) choice = 3;

            if (choice != -1) {
                if (choice == game->enigmeState.questions[game->enigmeState.currentQuestionIdx].bonneRep) {
                    if (game->player.health < 100) {
                        game->player.health += 20;
                        if (game->player.health > 100) game->player.health = 100;
                    } else {
                        game->player.score += 200; 
                    }
                    game->player.score += 10;
                    game->enigmeState.quizAnswerState = 1;
                    if (game->enigmeState.correctSound) Mix_PlayChannel(-1, game->enigmeState.correctSound, 0);
                } else {
                    game->player.score -= 50; 
                    if (game->player.score < 0) game->player.score = 0;
                    game->player.lives--;
                    game->enigmeState.quizAnswerState = 2;
                    if (game->enigmeState.wrongSound) Mix_PlayChannel(-1, game->enigmeState.wrongSound, 0);
                }
                game->enigmeState.quizStateStartTime = SDL_GetTicks();
            }
        }
    } else if (game->gameState == MENU_PUZZLE) {
        if (key == SDLK_ESCAPE || game->enigmeState.pieceCorrect) {
            game->enigmeState.pieceCorrect = 0;
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
        }
    }
}

void renderEnigme(GameManager* game) {
    if (!game || !game->quizActive) return;

    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {100, 100, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200};
    SDL_RenderFillRect(game->renderer, &overlay);
    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_NONE);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    renderText(game, "--- ENIGME ---", SCREEN_WIDTH/2 - 80, 150, yellow, game->titleFont);
    renderText(game, game->currentEnigme.question, 150, 250, white, game->font);

    renderText(game, game->currentEnigme.repA, 200, 350, white, game->font);
    renderText(game, game->currentEnigme.repB, 200, 400, white, game->font);
    renderText(game, game->currentEnigme.repC, 200, 450, white, game->font);

    renderText(game, "Appuyez sur 1, 2 ou 3 pour repondre", SCREEN_WIDTH/2 - 150, 550, yellow, game->font);
}

void handleEnigmeMouseMotion(GameManager* game, int x, int y) {
    if (!game) return;
    handleMenuMouseMotion(game, &game->enigmeState.enigmeMenu, x, y);
}

void handleEnigmeMouseClick(GameManager* game, int x, int y) {
    if (!game) return;
    handleMenuMouseClick(game, &game->enigmeState.enigmeMenu, x, y);
}

void handleQuizMouseMotion(GameManager* game, int x, int y) {
    if (!game) return;
    game->enigmeState.quizRetourBtn.selected = INSIDE(game->enigmeState.quizRetourBtn.rect, x, y);
}

void handleQuizMouseClick(GameManager* game, int x, int y) {
    if (!game) return;
    if (game->enigmeState.quizRetourBtn.selected) { quizRetourCb(game); return; }

    if (game->enigmeState.quizAnswerState != 0) {
        game->enigmeState.quizAnswerState = 0;
        game->gameState = game->previousState;
        changeMusic(game, game->previousState);
        return;
    }

    if (game->enigmeState.quizAnswerState == 0) {
        SDL_Rect btnAns[3] = { {40, 430, 220, 80}, {290, 430, 220, 80}, {540, 430, 220, 80} };
        for (int i = 0; i < 3; i++) {
            if (INSIDE(btnAns[i], x, y)) {
                if (i + 1 == game->enigmeState.questions[game->enigmeState.currentQuestionIdx].bonneRep) {
                    if (game->player.health < 100) {
                        game->player.health += 20;
                        if (game->player.health > 100) game->player.health = 100;
                    } else {
                        game->player.score += 200; 
                    }
                    game->player.score += 10;
                    game->enigmeState.quizAnswerState = 1;
                    if (game->enigmeState.correctSound) Mix_PlayChannel(-1, game->enigmeState.correctSound, 0);
                } else {
                    game->player.score -= 50; 
                    if (game->player.score < 0) game->player.score = 0;
                    game->player.lives--;
                    game->enigmeState.quizAnswerState = 2;
                    if (game->enigmeState.wrongSound) Mix_PlayChannel(-1, game->enigmeState.wrongSound, 0);
                }
                game->enigmeState.quizStateStartTime = SDL_GetTicks();
                break;
            }
        }
    }
}

void handlePuzzleMouseMotion(GameManager* game, int x, int y) {
    if (!game || game->enigmeState.pieceCorrect || game->enigmeState.pieceWrong) return;
    game->enigmeState.puzzleRetourBtn.selected = INSIDE(game->enigmeState.puzzleRetourBtn.rect, x, y);
    if (game->enigmeState.selectedPiece != -1) {
        game->enigmeState.pieceRects[game->enigmeState.selectedPiece].x = x - game->enigmeState.pieceRects[game->enigmeState.selectedPiece].w / 2;
        game->enigmeState.pieceRects[game->enigmeState.selectedPiece].y = y - game->enigmeState.pieceRects[game->enigmeState.selectedPiece].h / 2;
    }
}

void handlePuzzleMouseClick(GameManager* game, int x, int y) {
    if (!game) return;
    if (game->enigmeState.puzzleRetourBtn.selected) { puzzleRetourCb(game); return; }
    
    if (game->enigmeState.pieceCorrect || game->enigmeState.pieceWrong) {
        game->enigmeState.pieceCorrect = 0;
        game->enigmeState.pieceWrong = 0;
        game->gameState = game->previousState;
        changeMusic(game, game->previousState);
        return;
    }
    
    if (!game->enigmeState.pieceCorrect && !game->enigmeState.pieceWrong) {
        for (int i = 0; i < 3; i++) {
            if (INSIDE(game->enigmeState.pieceRects[i], x, y)) {
                game->enigmeState.selectedPiece = i;
                break;
            }
        }
    }
}

void handlePuzzleMouseUp(GameManager* game, int x, int y) {
    (void)x; (void)y;
    if (!game) return;
    if (game->enigmeState.selectedPiece != -1) {
        SDL_Rect target = game->enigmeState.puzzles[game->enigmeState.currentPuzIdx].targetRect;
        SDL_Rect piece = game->enigmeState.pieceRects[game->enigmeState.selectedPiece];
        int centerX = piece.x + piece.w / 2;
        int centerY = piece.y + piece.h / 2;

        if (centerX > target.x && centerX < target.x + target.w &&
            centerY > target.y && centerY < target.y + target.h) {
            game->enigmeState.lastDroppedPiece = game->enigmeState.selectedPiece;
            if (game->enigmeState.pieceMapping[game->enigmeState.selectedPiece] == 0) { 
                game->enigmeState.pieceCorrect = 1;
                if (game->player.health < 100) {
                    game->player.health += 20;
                    if (game->player.health > 100) game->player.health = 100;
                } else {
                    game->player.score += 200; 
                }
                game->player.score += 20;
                if (game->enigmeState.correctSound) Mix_PlayChannel(-1, game->enigmeState.correctSound, 0);
                game->enigmeState.pieceRects[game->enigmeState.selectedPiece] = target;
                game->enigmeState.quizStateStartTime = SDL_GetTicks();
            } else {
                game->player.score -= 50; 
                if (game->player.score < 0) game->player.score = 0;
                game->player.lives--;
                if (game->enigmeState.wrongSound) Mix_PlayChannel(-1, game->enigmeState.wrongSound, 0);
                game->enigmeState.pieceWrong = 1;
                game->enigmeState.pieceRects[game->enigmeState.selectedPiece] = target; 
                game->enigmeState.quizStateStartTime = SDL_GetTicks(); 
            }
        } else game->enigmeState.pieceRects[game->enigmeState.selectedPiece] = game->enigmeState.startPieceRects[game->enigmeState.selectedPiece];
        game->enigmeState.selectedPiece = -1;
    }
}

void renderQuiz(GameManager* game) {
    if (!game || !game->renderer) return;
    
    if (game->enigmeState.quizBg) SDL_RenderCopy(game->renderer, game->enigmeState.quizBg, NULL, NULL);

    int mx, my; SDL_GetMouseState(&mx, &my);
    SDL_Rect qRect = {40, 130, 720, 100};
    if (game->enigmeState.ansNormalTex) SDL_RenderCopy(game->renderer, game->enigmeState.ansNormalTex, NULL, &qRect);
    renderText(game, game->enigmeState.questions[game->enigmeState.currentQuestionIdx].question, qRect.x + 20, qRect.y + 35, (SDL_Color){255, 255, 255, 255}, game->font);

    SDL_Rect btnAns[3] = { {40, 430, 220, 80}, {290, 430, 220, 80}, {540, 430, 220, 80} };
    for (int i = 0; i < 3; i++) {
        SDL_Texture* tex = INSIDE(btnAns[i], mx, my) ? game->enigmeState.ansHoverTex : game->enigmeState.ansNormalTex;
        if (tex) SDL_RenderCopy(game->renderer, tex, NULL, &btnAns[i]);
    }

    renderText(game, game->enigmeState.questions[game->enigmeState.currentQuestionIdx].repA, btnAns[0].x + 20, btnAns[0].y + 25, (SDL_Color){255, 255, 255, 255}, game->font);
    renderText(game, game->enigmeState.questions[game->enigmeState.currentQuestionIdx].repB, btnAns[1].x + 20, btnAns[1].y + 25, (SDL_Color){255, 255, 255, 255}, game->font);
    renderText(game, game->enigmeState.questions[game->enigmeState.currentQuestionIdx].repC, btnAns[2].x + 20, btnAns[2].y + 25, (SDL_Color){255, 255, 255, 255}, game->font);

    float progress = 1.0f - (float)(SDL_GetTicks() - game->enigmeState.quizStartTime) / (game->enigmeState.quizTempsMax * 1000);
    if (progress < 0) progress = 0;
    
    SDL_Rect timerBg = {200, 50, 400, 30};
    SDL_Rect timerBar = {200, 50, (int)(400 * progress), 30};
    SDL_SetRenderDrawColor(game->renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(game->renderer, &timerBg);
    SDL_SetRenderDrawColor(game->renderer, (int)(255 * (1 - progress)), (int)(255 * progress), 0, 255);
    SDL_RenderFillRect(game->renderer, &timerBar);

    if (game->enigmeState.quizAnswerState == 1 || game->enigmeState.quizAnswerState == 2) {
        game->enigmeState.rotoAngle += 2.0f;
        if (game->enigmeState.rotoScale < 2.0f) game->enigmeState.rotoScale += 0.05f;
        const char* msg = (game->enigmeState.quizAnswerState == 1) ? "Bravo !" : "Echec...";
        SDL_Color col = (game->enigmeState.quizAnswerState == 1) ? (SDL_Color){0, 255, 0, 255} : (SDL_Color){255, 0, 0, 255};
        SDL_Surface* s = TTF_RenderText_Blended(game->titleFont, msg, col);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(game->renderer, s);
            if (t) {
                int w = (int)(s->w * game->enigmeState.rotoScale);
                int h = (int)(s->h * game->enigmeState.rotoScale);
                SDL_Rect r = { SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT / 2 - h / 2, w, h };
                SDL_RenderCopyEx(game->renderer, t, NULL, &r, game->enigmeState.rotoAngle, NULL, SDL_FLIP_NONE);
                SDL_DestroyTexture(t);
            }
            SDL_FreeSurface(s);
        }
    }

    renderButton(game, &game->enigmeState.quizRetourBtn);

    if (game->enigmeState.quizAnswerState == 0) {
        if (progress <= 0) {
            game->player.lives--;
            game->enigmeState.quizAnswerState = 2;
            game->enigmeState.quizStateStartTime = SDL_GetTicks();
        }
    } else {
        if (SDL_GetTicks() - game->enigmeState.quizStateStartTime > 1500) {
            game->enigmeState.quizAnswerState = 0;
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
        }
    }
}

void renderPuzzle(GameManager* game) {
    if (!game) return;
    if (game->enigmeState.enigmeMenu.background) SDL_RenderCopy(game->renderer, game->enigmeState.enigmeMenu.background, NULL, NULL);

    PuzzleGame p = game->enigmeState.puzzles[game->enigmeState.currentPuzIdx];
    SDL_Rect mainRect = {50, 100, 500, 400};
    if (p.mainImg) SDL_RenderCopy(game->renderer, p.mainImg, NULL, &mainRect);

    SDL_SetRenderDrawColor(game->renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(game->renderer, &p.targetRect);
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(game->renderer, 580, 50, 580, 550);

    for (int i = 0; i < 3; i++) {
        if (game->enigmeState.pieceCorrect && i == game->enigmeState.lastDroppedPiece) continue;
        SDL_Rect border = {game->enigmeState.pieceRects[i].x - 5, game->enigmeState.pieceRects[i].y - 5, game->enigmeState.pieceRects[i].w + 10, game->enigmeState.pieceRects[i].h + 10};
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(game->renderer, &border);
        int texIdx = game->enigmeState.pieceMapping[i];
        if (p.pieces[texIdx]) SDL_RenderCopy(game->renderer, p.pieces[texIdx], NULL, &game->enigmeState.pieceRects[i]);
    }

    if (game->enigmeState.pieceCorrect) {
        int texIdx = game->enigmeState.pieceMapping[game->enigmeState.lastDroppedPiece];
        if (p.pieces[texIdx]) SDL_RenderCopy(game->renderer, p.pieces[texIdx], NULL, &p.targetRect);
    }

    float progress = 1.0f - (float)(SDL_GetTicks() - game->enigmeState.puzzleStartTime) / (game->enigmeState.puzzleTempsMax * 1000);
    if (progress < 0) progress = 0;
    
    SDL_Rect timerBg = {50, 40, 500, 15};
    SDL_Rect timerBar = {50, 40, (int)(500 * progress), 15};
    SDL_SetRenderDrawColor(game->renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(game->renderer, &timerBg);
    if (progress > 0.5f) SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
    else if (progress > 0.2f) SDL_SetRenderDrawColor(game->renderer, 255, 165, 0, 255);
    else SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(game->renderer, &timerBar);

    if (game->enigmeState.pieceCorrect || game->enigmeState.pieceWrong || progress <= 0) {
        game->enigmeState.rotoAngle += 2.0f;
        if (game->enigmeState.rotoScale < 2.0f) game->enigmeState.rotoScale += 0.05f;
        const char* msg = game->enigmeState.pieceCorrect ? "Bravo !" : "Echec...";
        SDL_Color col = game->enigmeState.pieceCorrect ? (SDL_Color){0, 255, 0, 255} : (SDL_Color){255, 0, 0, 255};
        SDL_Surface* s = TTF_RenderText_Blended(game->titleFont, msg, col);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(game->renderer, s);
            if (t) {
                int w = (int)(s->w * game->enigmeState.rotoScale);
                int h = (int)(s->h * game->enigmeState.rotoScale);
                SDL_Rect r = { SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT / 2 - h / 2, w, h };
                SDL_RenderCopyEx(game->renderer, t, NULL, &r, game->enigmeState.rotoAngle, NULL, SDL_FLIP_NONE);
                SDL_DestroyTexture(t);
            }
            SDL_FreeSurface(s);
        }
        if (game->enigmeState.pieceCorrect || game->enigmeState.pieceWrong) {
            if (SDL_GetTicks() - game->enigmeState.quizStateStartTime > 2000) {
                game->gameState = game->previousState;
                changeMusic(game, game->previousState);
            }
        } else if (SDL_GetTicks() - game->enigmeState.puzzleStartTime > (Uint32)(game->enigmeState.puzzleTempsMax + 2) * 1000) {
            game->gameState = game->previousState;
            changeMusic(game, game->previousState);
        }
    }
    renderButton(game, &game->enigmeState.puzzleRetourBtn);
}

void renderEnigmeMenu(GameManager* game) {
    if (!game) return;
    if (game->enigmeState.enigmeMenu.background) SDL_RenderCopy(game->renderer, game->enigmeState.enigmeMenu.background, NULL, NULL);
    renderText(game, game->enigmeState.enigmeMenu.title, 330, 100, (SDL_Color){255, 255, 255, 255}, game->titleFont);
    for (int i = 0; i < game->enigmeState.enigmeMenu.buttonCount; i++) renderButton(game, &game->enigmeState.enigmeMenu.buttons[i]);
}

void cleanupEnigmeMenu(GameManager* game) {
    if (!game) return;
    cleanupMenu(&game->enigmeState.enigmeMenu);
    if (game->enigmeState.correctSound) Mix_FreeChunk(game->enigmeState.correctSound);
    if (game->enigmeState.wrongSound) Mix_FreeChunk(game->enigmeState.wrongSound);
    if (game->enigmeState.questionBg) SDL_DestroyTexture(game->enigmeState.questionBg);
    if (game->enigmeState.quizBg) SDL_DestroyTexture(game->enigmeState.quizBg);
    if (game->enigmeState.ansNormalTex) SDL_DestroyTexture(game->enigmeState.ansNormalTex);
    if (game->enigmeState.ansHoverTex) SDL_DestroyTexture(game->enigmeState.ansHoverTex);
    for (int i = 0; i < 3; i++) {
        if (game->enigmeState.puzzles[i].surf) SDL_FreeSurface(game->enigmeState.puzzles[i].surf);
        if (game->enigmeState.puzzles[i].mainImg) SDL_DestroyTexture(game->enigmeState.puzzles[i].mainImg);
        for (int j = 0; j < 3; j++) if (game->enigmeState.puzzles[i].pieces[j]) SDL_DestroyTexture(game->enigmeState.puzzles[i].pieces[j]);
    }
}

