#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <queue>

// audio integration
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 
#endif

void startBackgroundMusic() {
#ifdef _WIN32
    PlaySound(TEXT("audio/game_loop02.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);
#endif
}

void playEatSound() {
#ifdef _WIN32
    mciSendString(TEXT("close ch1"), NULL, 0, NULL); // Reset if it was already open
    mciSendString(TEXT("open \"audio/coin_earn.wav\" type waveaudio alias ch1"), NULL, 0, NULL);
    mciSendString(TEXT("play ch1 from 0"), NULL, 0, NULL);
#endif
}

void playGameOverSound() {
#ifdef _WIN32
    PlaySound(NULL, 0, 0);
    mciSendString(TEXT("close ch2"), NULL, 0, NULL);
    mciSendString(TEXT("open \"audio/game_over.wav\" type waveaudio alias ch2"), NULL, 0, NULL);
    mciSendString(TEXT("play ch2 from 0"), NULL, 0, NULL);
#endif
}

void playResurrectionSound() {
#ifdef _WIN32
    // Opens and plays a brief life-lost/respawn alert effect on channel 3
    mciSendString(TEXT("close ch3"), NULL, 0, NULL);
    mciSendString(TEXT("open \"audio/resurrection_of_player.wav\" type waveaudio alias ch3"), NULL, 0, NULL);
    mciSendString(TEXT("play ch3 from 0"), NULL, 0, NULL);
#endif
}

enum Difficulty { EASY, MEDIUM, HARD };
Difficulty currentDifficulty = EASY;

// Game States (Added DIFFICULTY_SELECT stage)
enum State { MENU, DIFFICULTY_SELECT, GAME, GAME_OVER, WIN };
State gameState = MENU;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int mouseX = 0, mouseY = 0;

// Button Coordinates & Offsets
const int BTN_WIDTH = 200;
const int BTN_HEIGHT = 50;
const int START_BTN_Y = 250;
const int EXIT_BTN_Y = 170;
const int RESTART_BTN_Y = 200;

// Difficulty Specific Button Positions
const int EASY_BTN_Y = 320;
const int MEDIUM_BTN_Y = 240;
const int HARD_BTN_Y = 160;

// Map Setup (0: Empty, 1: Wall, 2: Dot, 3: 'A', 4: 'T', 5: 'G', 6: 'C')
const int MAP_ROWS = 15;
const int MAP_COLS = 19;
int initialMaze[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
    {1,1,1,1,2,1,1,1,0,1,0,1,1,1,2,1,1,1,1},
    {0,0,0,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,0},
    {1,1,1,1,2,1,0,1,1,0,1,1,0,1,2,1,1,1,1},
    {1,2,2,2,2,2,2,1,0,0,0,1,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,1,2,1,2,2,2,2,2,2,2,1,2,1,2,2,1},
    {1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
int mediumMaze[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,0,1},
    // ... 
};

int hardMaze[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0,1},
    // ... 
};

int maze[MAP_ROWS][MAP_COLS];

const int TILE_SIZE = 30;
const int MAP_OFFSET_X = (WINDOW_WIDTH - (MAP_COLS * TILE_SIZE)) / 2;
const int MAP_OFFSET_Y = (WINDOW_HEIGHT - (MAP_ROWS * TILE_SIZE)) / 2 - 40;

// Game Elements
float dnamanX, dnamanY;
float dnamanSpeed = 0.09f;
int dirX = 0, dirY = 0;
int nextDirX = 0, nextDirY = 0;
int score = 0;
int lives = 5;

std::string targetSequence = "";
std::string collectedSequence = "";
char nucChars[4] = { 'A', 'T', 'G', 'C' };

struct Ghost {
    float x, y;
    float r, g, b;
    float speed;
};
std::vector<Ghost> enzymes;

struct Point { int x, y; };

void resetGame();
bool isWall(float x, float y);

void drawText(int x, int y, std::string text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2i(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

bool isMouseOverButton(int bx, int by, int bw, int bh) {
    return (mouseX >= bx && mouseX <= bx + bw && mouseY >= by && mouseY <= by + bh);
}

bool isTileReachable(int targetX, int targetY) {
    if (initialMaze[targetY][targetX] == 1) return false;
    int startX = 9, startY = 7;
    if (startX == targetX && startY == targetY) return true;

    bool visited[MAP_ROWS][MAP_COLS] = { false };
    std::queue<Point> q;

    q.push({ startX, startY });
    visited[startY][startX] = true;

    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (curr.x == targetX && curr.y == targetY) {
            return true;
        }

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < MAP_COLS && ny >= 0 && ny < MAP_ROWS) {
                if (!visited[ny][nx] && initialMaze[ny][nx] != 1) {
                    visited[ny][nx] = true;
                    q.push({ nx, ny });
                }
            }
        }
    }
    return false;
}

void spawnNucleotideOnMap(char nuc) {
    int type = 3;
    if (nuc == 'T') type = 4;
    if (nuc == 'G') type = 5;
    if (nuc == 'C') type = 6;

    while (true) {
        int r = rand() % MAP_ROWS;
        int c = rand() % MAP_COLS;
        if ((maze[r][c] == 0 || maze[r][c] == 2) && isTileReachable(c, r)) {
            maze[r][c] = type;
            break;
        }
    }
}

void drawHeart(int cx, int cy, int size) {
    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(cx, cy - size / 4);
    for (int angle = 0; angle <= 360; angle += 10) {
        float rad = angle * 3.14159f / 180.0f;
        float x = 16 * pow(sin(rad), 3);
        float y = 13 * cos(rad) - 5 * cos(2 * rad) - 2 * cos(3 * rad) - cos(4 * rad);
        glVertex2f(cx + (x / 16.0f) * size, cy + (y / 16.0f) * size);
    }
    glEnd();
}

Point getBFSDirection(int startX, int startY, int targetX, int targetY) {
    if (startX == targetX && startY == targetY) return { 0, 0 };

    bool visited[MAP_ROWS][MAP_COLS] = { false };
    Point parent[MAP_ROWS][MAP_COLS];

    std::queue<Point> q;
    q.push({ startX, startY });
    visited[startY][startX] = true;

    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    bool found = false;

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (curr.x == targetX && curr.y == targetY) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 0 && nx < MAP_COLS && ny >= 0 && ny < MAP_ROWS) {
                if (!visited[ny][nx] && initialMaze[ny][nx] != 1) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = curr;
                    q.push({ nx, ny });
                }
            }
        }
    }

    if (!found) return { 0, 0 };

    Point curr = { targetX, targetY };
    while (parent[curr.y][curr.x].x != startX || parent[curr.y][curr.x].y != startY) {
        curr = parent[curr.y][curr.x];
    }
    return { curr.x - startX, curr.y - startY };
}

void resetGame() {
    score = 0;
    lives = (currentDifficulty == EASY) ? 5 : (currentDifficulty == MEDIUM ? 3 : 2);
    collectedSequence = "";
    dnamanX = 9.0f;
    dnamanY = 7.0f;
    dirX = 0; dirY = 0; nextDirX = 0; nextDirY = 0;

    // 1. Point to the correct source matrix based on selected level
    int (*selectedSourceMaze)[MAP_COLS] = mediumMaze; // Default fallback

    if (currentDifficulty == EASY) {
        selectedSourceMaze = initialMaze;
    }
    else if (currentDifficulty == MEDIUM) {
        selectedSourceMaze = mediumMaze;
    }
    else if (currentDifficulty == HARD) {
        selectedSourceMaze = hardMaze;
    }

    // 2. Copy the chosen layout into the active simulation maze array
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            maze[r][c] = selectedSourceMaze[r][c];
        }
    }

 //----------   // --- NEW: Scale DNA Target Sequence Length based on difficulty --- TASKS
 //----------   // --- NEW: Scale Enzyme (Ghost) Movement Speed --- TASKS

    int seqLength = 5 + (rand() % 2);
    targetSequence = "";
    for (int i = 0; i < seqLength; i++) {
        char nuc = nucChars[rand() % 4];
        targetSequence += nuc;
        spawnNucleotideOnMap(nuc);
    }

    float baselineSpeed = 0.025f;
    enzymes.clear();
    enzymes.push_back({ 1.0f,  1.0f,   1.0f, 0.2f, 0.2f, baselineSpeed });
    enzymes.push_back({ 1.0f,  13.0f,  0.2f, 0.9f, 0.2f, baselineSpeed });
    enzymes.push_back({ 17.0f, 13.0f,  1.0f, 0.5f, 0.0f, baselineSpeed });
}

void renderMenu() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(WINDOW_WIDTH / 2 - 120, 450, "ENZYME PAC-MAN", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.4f, 0.8f, 1.0f);
    drawText(WINDOW_WIDTH / 2 - 145, 410, "Sequence Protein Synthesis Mode", GLUT_BITMAP_HELVETICA_12);

    int startBtnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;
    glColor3f(isMouseOverButton(startBtnX, START_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.2f : 0.1f, 0.2f, 0.5f);
    glRecti(startBtnX, START_BTN_Y, startBtnX + BTN_WIDTH, START_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(startBtnX + 45, START_BTN_Y + 18, "START GAME");

    int exitBtnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;
    glColor3f(isMouseOverButton(exitBtnX, EXIT_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.5f : 0.3f, 0.1f, 0.1f);
    glRecti(exitBtnX, EXIT_BTN_Y, exitBtnX + BTN_WIDTH, EXIT_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(exitBtnX + 75, EXIT_BTN_Y + 18, "EXIT");

    glutSwapBuffers();
}

// Intermediate Screen View Implementation
void renderDifficultySelect() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WINDOW_WIDTH / 2 - 140, 450, "SELECT GAME DIFFICULTY", GLUT_BITMAP_TIMES_ROMAN_24);

    int btnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;

    // Easy Button
    glColor3f(isMouseOverButton(btnX, EASY_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.2f : 0.1f, 0.6f, 0.2f);
    glRecti(btnX, EASY_BTN_Y, btnX + BTN_WIDTH, EASY_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, EASY_BTN_Y + 18, "EASY");

    // Medium Button
    glColor3f(isMouseOverButton(btnX, MEDIUM_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.6f : 0.5f, 0.4f, 0.1f);
    glRecti(btnX, MEDIUM_BTN_Y, btnX + BTN_WIDTH, MEDIUM_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 60, MEDIUM_BTN_Y + 18, "MEDIUM");

    // Hard Button
    glColor3f(isMouseOverButton(btnX, HARD_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.7f : 0.5f, 0.1f, 0.1f);
    glRecti(btnX, HARD_BTN_Y, btnX + BTN_WIDTH, HARD_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, HARD_BTN_Y + 18, "HARD");

    glutSwapBuffers();
}

void renderGame() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(30, WINDOW_HEIGHT - 30, "TARGET SEQUENCE: " + targetSequence, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.0f, 1.0f, 0.0f);
    drawText(30, WINDOW_HEIGHT - 55, "COLLECTED: " + collectedSequence, GLUT_BITMAP_HELVETICA_12);

    glColor3f(1.0f, 0.84f, 0.0f);
    drawText(WINDOW_WIDTH - 150, WINDOW_HEIGHT - 30, "SCORE: " + std::to_string(score));

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WINDOW_WIDTH - 200, WINDOW_HEIGHT - 55, "LIVES:", GLUT_BITMAP_HELVETICA_12);
    for (int i = 0; i < lives; i++) {
        drawHeart(WINDOW_WIDTH - 140 + (i * 22), WINDOW_HEIGHT - 50, 14);
    }

    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            int x = MAP_OFFSET_X + c * TILE_SIZE;
            int y = MAP_OFFSET_Y + (MAP_ROWS - 1 - r) * TILE_SIZE;

            if (maze[r][c] == 1) {
                glColor3f(0.1f, 0.3f, 0.8f);
                glRecti(x + 1, y + 1, x + TILE_SIZE - 1, y + TILE_SIZE - 1);
            }
            else if (maze[r][c] == 2) {
                glColor3f(0.7f, 0.7f, 0.7f);
                glBegin(GL_QUADS);
                glVertex2i(x + 13, y + 13); glVertex2i(x + 17, y + 13);
                glVertex2i(x + 17, y + 17); glVertex2i(x + 13, y + 17);
                glEnd();
            }
            else if (maze[r][c] >= 3 && maze[r][c] <= 6) {
                char letter = nucChars[maze[r][c] - 3];
                if (letter == 'A') glColor3f(1.0f, 0.3f, 0.3f);
                else if (letter == 'T') glColor3f(0.3f, 1.0f, 0.3f);
                else if (letter == 'G') glColor3f(0.3f, 0.6f, 1.0f);
                else if (letter == 'C') glColor3f(1.0f, 0.4f, 1.0f);

                glBegin(GL_LINE_LOOP);
                glVertex2i(x + 4, y + 4);
                glVertex2i(x + TILE_SIZE - 4, y + 4);
                glVertex2i(x + TILE_SIZE - 4, y + TILE_SIZE - 4);
                glVertex2i(x + 4, y + TILE_SIZE - 4);
                glEnd();

                std::string s(1, letter);
                drawText(x + 10, y + 9, s, GLUT_BITMAP_HELVETICA_12);
            }
        }
    }

    int pacX = MAP_OFFSET_X + (int)(dnamanX * TILE_SIZE) + TILE_SIZE / 2;
    int pacY = MAP_OFFSET_Y + (int)((MAP_ROWS - 1 - dnamanY) * TILE_SIZE) + TILE_SIZE / 2;
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(pacX, pacY);
    for (int i = 0; i <= 360; i += 15) {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(pacX + cos(rad) * 12, pacY + sin(rad) * 12);
    }
    glEnd();

    for (const auto& g : enzymes) {
        int gx = MAP_OFFSET_X + (int)(g.x * TILE_SIZE) + TILE_SIZE / 2;
        int gy = MAP_OFFSET_Y + (int)((MAP_ROWS - 1 - g.y) * TILE_SIZE) + TILE_SIZE / 2;
        glColor3f(g.r, g.g, g.b);

        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(gx, gy);
        for (int i = 0; i <= 180; i += 15) {
            float rad = i * 3.14159f / 180.0f;
            glVertex2f(gx + cos(rad) * 12, gy + sin(rad) * 12);
        }
        glEnd();
        glRecti(gx - 12, gy - 12, gx + 12, gy);
    }

    glutSwapBuffers();
}

void renderGameOver(bool won) {
    glClear(GL_COLOR_BUFFER_BIT);
    if (won) {
        glColor3f(0.0f, 1.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 130, 400, "SYNTHESIS SUCCESSFUL!", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(WINDOW_WIDTH / 2 - 80, 400, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.8f, 0.8f, 0.8f);
        drawText(WINDOW_WIDTH / 2 - 110, 360, "All vital sequence lives depleted!", GLUT_BITMAP_HELVETICA_12);
    }

    int restartBtnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;
    glColor3f(isMouseOverButton(restartBtnX, RESTART_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.3f : 0.2f, 0.6f, 0.2f);
    glRecti(restartBtnX, RESTART_BTN_Y, restartBtnX + BTN_WIDTH, RESTART_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(restartBtnX + 55, RESTART_BTN_Y + 18, "RESTART");

    glutSwapBuffers();
}

void display() {
    if (gameState == MENU) renderMenu();
    else if (gameState == DIFFICULTY_SELECT) renderDifficultySelect();
    else if (gameState == GAME) renderGame();
    else if (gameState == GAME_OVER) renderGameOver(false);
    else if (gameState == WIN) renderGameOver(true);
}

bool isWall(float x, float y) {
    int cellX = (int)floor(x + 0.5f);
    int cellY = (int)floor(y + 0.5f);
    if (cellX < 0 || cellX >= MAP_COLS || cellY < 0 || cellY >= MAP_ROWS) return true;
    return (initialMaze[cellY][cellX] == 1);
}

void update(int value) {
    if (gameState == GAME) {
        if ((nextDirX != 0 || nextDirY != 0) && !isWall(dnamanX + nextDirX * 0.5f, dnamanY + nextDirY * 0.5f)) {
            dirX = nextDirX; dirY = nextDirY;
        }
        float nextX = dnamanX + dirX * dnamanSpeed;
        float nextY = dnamanY + dirY * dnamanSpeed;
        if (!isWall(nextX + dirX * 0.3f, nextY + dirY * 0.3f)) {
            dnamanX = nextX; dnamanY = nextY;
        }

        int pCellX = (int)(dnamanX + 0.5f);
        int pCellY = (int)(dnamanY + 0.5f);

        if (maze[pCellY][pCellX] == 2) {
            maze[pCellY][pCellX] = 0;
            score += 10;
            playEatSound();
        }
        else if (maze[pCellY][pCellX] >= 3 && maze[pCellY][pCellX] <= 6) {
            char eaten = nucChars[maze[pCellY][pCellX] - 3];
            maze[pCellY][pCellX] = 0;

            char expected = targetSequence[collectedSequence.length()];
            if (eaten == expected) {
                collectedSequence += eaten;
                score += 100;
                playEatSound();
                if (collectedSequence == targetSequence) {
                    gameState = WIN;
                }
            }
            else {
                playGameOverSound();
                gameState = GAME_OVER;
            }
        }

        for (auto& g : enzymes) {
            int gCellX = (int)floor(g.x + 0.5f);
            int gCellY = (int)floor(g.y + 0.5f);

            Point nextStep = getBFSDirection(gCellX, gCellY, pCellX, pCellY);

            if (nextStep.x != 0 || nextStep.y != 0) {
                g.x += nextStep.x * g.speed;
                g.y += nextStep.y * g.speed;
            }
            else {
                float diffX = dnamanX - g.x;
                float diffY = dnamanY - g.y;
                if (fabs(diffX) > 0.1f) g.x += (diffX > 0 ? 1 : -1) * g.speed;
                if (fabs(diffY) > 0.1f) g.y += (diffY > 0 ? 1 : -1) * g.speed;
            }

            if (fabs(dnamanX - g.x) < 0.6f && fabs(dnamanY - g.y) < 0.6f) {
                lives--;
                if (lives <= 0) {
                    playGameOverSound();
                    gameState = GAME_OVER;
                    break;
                }

                playResurrectionSound();

                if (!collectedSequence.empty()) {
                    char removed = collectedSequence.back();
                    collectedSequence.pop_back();
                    spawnNucleotideOnMap(removed);
                }

                dnamanX = 9.0f; dnamanY = 7.0f;
                dirX = 0; dirY = 0; nextDirX = 0; nextDirY = 0;

                enzymes[0].x = 1.0f;  enzymes[0].y = 1.0f;
                enzymes[1].x = 1.0f;  enzymes[1].y = 13.0f;
                enzymes[2].x = 17.0f; enzymes[2].y = 13.0f;
                break;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void specialKeys(int key, int x, int y) {
    if (gameState == GAME) {
        switch (key) {
        case GLUT_KEY_UP:    nextDirX = 0;  nextDirY = -1; break;
        case GLUT_KEY_DOWN:  nextDirX = 0;  nextDirY = 1;  break;
        case GLUT_KEY_LEFT:  nextDirX = -1; nextDirY = 0;  break;
        case GLUT_KEY_RIGHT: nextDirX = 1;  nextDirY = 0;  break;
        }
    }
}

void passiveMouse(int x, int y) {
    int currentWidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentHeight = glutGet(GLUT_WINDOW_HEIGHT);
    mouseX = (int)(((float)x / currentWidth) * WINDOW_WIDTH);
    mouseY = (int)(((float)(currentHeight - y) / currentHeight) * WINDOW_HEIGHT);

    if (gameState == MENU || gameState == DIFFICULTY_SELECT || gameState == GAME_OVER || gameState == WIN) {
        glutPostRedisplay();
    }
}

void mouseClicks(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int currentWidth = glutGet(GLUT_WINDOW_WIDTH);
        int currentHeight = glutGet(GLUT_WINDOW_HEIGHT);
        mouseX = (int)(((float)x / currentWidth) * WINDOW_WIDTH);
        mouseY = (int)(((float)(currentHeight - y) / currentHeight) * WINDOW_HEIGHT);

        int btnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;

        if (gameState == MENU) {
            if (isMouseOverButton(btnX, START_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                // Instantly transitions to the new difficulty setup window
                gameState = DIFFICULTY_SELECT;
            }
            else if (isMouseOverButton(btnX, EXIT_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                glutLeaveMainLoop();
            }
        }
        else if (gameState == DIFFICULTY_SELECT) {
            if (isMouseOverButton(btnX, EASY_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                currentDifficulty = EASY;
                resetGame();
                gameState = GAME;
            }
            else if (isMouseOverButton(btnX, MEDIUM_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                currentDifficulty = MEDIUM;
                resetGame();
                gameState = GAME;
            }
            else if (isMouseOverButton(btnX, HARD_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                currentDifficulty = HARD;
                resetGame();
                gameState = GAME;
            }
        }
        else if (gameState == GAME_OVER || gameState == WIN) {
            if (isMouseOverButton(btnX, RESTART_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                // Sending players straight back to selection options on loop restart
                gameState = DIFFICULTY_SELECT;
                startBackgroundMusic();
            }
        }
    }
}

void init() {
    srand(time(NULL));
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bio-Sequence Pac-Man");

    init();
    startBackgroundMusic();

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClicks);
    glutPassiveMotionFunc(passiveMouse);
    glutMotionFunc(passiveMouse);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}