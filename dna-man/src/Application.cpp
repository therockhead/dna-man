#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <queue>
using namespace std;

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
    //mciSendString(TEXT("close ch1"), NULL, 0, NULL);
    mciSendString(TEXT("open \"audio/coin_earn.wav\" type waveaudio alias ch1"), NULL, 0, NULL);
    mciSendString(TEXT("play ch1 from 0"), NULL, 0, NULL);
#endif
}

void playGameOverSound() {
#ifdef _WIN32
    PlaySound(NULL, 0, 0);
    //mciSendString(TEXT("close ch2"), NULL, 0, NULL);
    mciSendString(TEXT("open \"audio/game_over.wav\" type waveaudio alias ch2"), NULL, 0, NULL);
    mciSendString(TEXT("play ch2 from 0"), NULL, 0, NULL);
#endif
}

void playResurrectionSound() {
#ifdef _WIN32
    //mciSendString(TEXT("close ch3"), NULL, 0, NULL);
    mciSendString(TEXT("open \"audio/resurrection_of_player.wav\" type waveaudio alias ch3"), NULL, 0, NULL);
    mciSendString(TEXT("play ch3 from 0"), NULL, 0, NULL);
#endif
}



enum Difficulty { EASY, MEDIUM, HARD, BRUTAL };
Difficulty currentDifficulty = EASY;

// Added PAUSE to the game engine state list
enum State { MENU, DIFFICULTY_SELECT, GAME, PAUSE, GAME_OVER, WIN };
State gameState = MENU;

int WINDOW_WIDTH = 900;
int WINDOW_HEIGHT = 700;

int mouseX = 0, mouseY = 0;

const int BTN_WIDTH = 200;
const int BTN_HEIGHT = 50;
const int START_BTN_Y = 250;
const int EXIT_BTN_Y = 170;
const int RESTART_BTN_Y = 200;

const int EASY_BTN_Y = 320;
const int MEDIUM_BTN_Y = 240;
const int HARD_BTN_Y = 160;
const int BRUTAL_BTN_Y = 80;

// Dynamic Pause Button Coordinates
const int PAUSE_RESTART_Y = 330;
const int PAUSE_MENU_Y = 250;
const int PAUSE_EXIT_Y = 170;

// "semiMediumaze" -- Briti
// "semiHardmaze" -- Shoumya
vector<vector<int>> initialMaze = {
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

vector<vector<int>> mediumMaze = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
    {1,1,1,1,2,1,1,1,0,0,0,1,1,1,2,1,1,1,1},
    {1,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
    {1,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1,2,2,1},
    {1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

vector<vector<int>> hardMazeTemplate = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,1,2,1,2,1,2,1,2,1,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,1,2,2,2,2,2,1,2,1,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,1},
    {1,2,1,2,1,1,1,2,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,2,1},
    {1,2,1,2,1,2,1,2,1,2,2,2,2,2,1,2,1,2,2,2,2,2,1,2,2,2,1,2,1,2,1},
    {1,1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,2,1,1,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,2,1,1,1,2,1,0,0,0,0,0,1,2,1,1,1,2,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,1,2,2,2,1,2,1,2,2,2,2,2,1,2,1,2,2,2,1,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,2,1,2,2,2,2,2,2,2,2,2,2,2,1,2,1,2,2,2,2,2,2,2,2,2,1,2,1,2,1},
    {1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1},
    {1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,2,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,1,2,2,2,2,2,2,2,2,2,1,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,1,1,1,1,1,1,2,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,1,2,1},
    {1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

std::vector<std::vector<int>> BrutalMazeTemplate = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1},
    {1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1},
    {1,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,1},
    {1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
    {1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,2,1,2,1},
    {1,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,1},
    {1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1},
    {1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,1,2,2,2,1,2,1,2,2,2,1,2,2,2,1,2,1,2,2,2,1,2,2,2,1,2,1,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,2,1,1,1,2,1,2,1,1,1,2,1,2,1,1,1,2,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,1,1,2,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
    {1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1},
    {1,2,1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,2,1,2,1,1,1,1,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,1},
    {1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
    {1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1},
    {1,2,1,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,2,1},
    {1,2,1,2,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int MAP_ROWS = 15;
int MAP_COLS = 19;
vector<vector<int>> maze;

//const int TILE_SIZE = 18;
int TILE_SIZE = 18;
int MAP_OFFSET_X;
int MAP_OFFSET_Y;

float dnamanX, dnamanY;
float dnamanSpeed = 0.12f;
int dirX = 0, dirY = 0;
int nextDirX = 0, nextDirY = 0;
int score = 0;
int lives = 5;

string targetSequence = "";
string collectedSequence = "";
char nucChars[4] = { 'A', 'T', 'G', 'C' };

struct Ghost {
    float x, y;
    float r, g, b;
    float speed;
};
vector<Ghost> enzymes;

struct Point { int x, y; };

void resetGame();
bool isWall(float x, float y);
//
//
//
void updateLayout() {
    const int MARGIN_X = 40;   // side padding
    const int MARGIN_Y = 140;  // room for HUD text top/bottom

    int maxTileW = (WINDOW_WIDTH - MARGIN_X) / MAP_COLS;
    int maxTileH = (WINDOW_HEIGHT - MARGIN_Y) / MAP_ROWS;

    TILE_SIZE = std::max(4, std::min(maxTileW, maxTileH)); // clamp so it never hits 0

    MAP_OFFSET_X = (WINDOW_WIDTH - (MAP_COLS * TILE_SIZE)) / 2;
    MAP_OFFSET_Y = (WINDOW_HEIGHT - (MAP_ROWS * TILE_SIZE)) / 2 - 40;
}

void reshape(int w, int h) {
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // re-center the maze for the new window size
    updateLayout();
}
//
//
//
void drawText(int x, int y, string text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2i(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

//*****************************************************
void drawStrokeText(float x, float y, string text, float scale = 0.25f, float lineWidth = 3.0f, void* font = GLUT_STROKE_MONO_ROMAN) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, scale);
    glLineWidth(lineWidth);
    for (char c : text) glutStrokeCharacter(font, c);
    glLineWidth(1.0f);
    glPopMatrix();
}
//*****************************************************

bool isMouseOverButton(int bx, int by, int bw, int bh) {
    return (mouseX >= bx && mouseX <= bx + bw && mouseY >= by && mouseY <= by + bh);
}

bool isTileReachable(int targetX, int targetY) {
    if (maze[targetY][targetX] == 1) return false;
    int startX = (int)dnamanX;
    int startY = (int)dnamanY;
    if (startX == targetX && startY == targetY) return true;

    vector<vector<bool>> visited(MAP_ROWS, vector<bool>(MAP_COLS, false));
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
                if (!visited[ny][nx] && maze[ny][nx] != 1) {
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

    vector<vector<bool>> visited(MAP_ROWS, vector<bool>(MAP_COLS, false));
    vector<vector<Point>> parent(MAP_ROWS, vector<Point>(MAP_COLS));

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
                if (!visited[ny][nx] && maze[ny][nx] != 1) {
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
    lives = (currentDifficulty == EASY) ? 5: (currentDifficulty == MEDIUM ? 3 : (currentDifficulty == HARD ? 2 : 1));
    collectedSequence = "";
    dirX = 0; dirY = 0; nextDirX = 0; nextDirY = 0;

    vector<vector<int>> selectedSourceMaze;
    if (currentDifficulty == EASY) {
        selectedSourceMaze = initialMaze;
        dnamanX = 9.0f; dnamanY = 7.0f;
    }
    else if (currentDifficulty == MEDIUM) {
        selectedSourceMaze = mediumMaze;
        dnamanX = 9.0f; dnamanY = 7.0f;
    }
    else if (currentDifficulty == HARD) {
        selectedSourceMaze = hardMazeTemplate;
        dnamanX = 15.0f; dnamanY = 12.0f;
    }
    else if (currentDifficulty == BRUTAL) {
        selectedSourceMaze = BrutalMazeTemplate;
        dnamanX = 17.0f; dnamanY = 15.0f;
    }

    //MAP_ROWS = selectedSourceMaze.size();
    //MAP_COLS = selectedSourceMaze[0].size();

    //MAP_OFFSET_X = (WINDOW_WIDTH - (MAP_COLS * TILE_SIZE)) / 2;
    //MAP_OFFSET_Y = (WINDOW_HEIGHT - (MAP_ROWS * TILE_SIZE)) / 2 - 40;

    MAP_ROWS = selectedSourceMaze.size();
    MAP_COLS = selectedSourceMaze[0].size();
    maze = selectedSourceMaze;
    updateLayout();

    maze = selectedSourceMaze;

    int seqLength = 4;
    if (currentDifficulty == MEDIUM) seqLength = 6;
    else if (currentDifficulty == HARD) seqLength = 8;
    else if (currentDifficulty == BRUTAL) seqLength = 12;

    targetSequence = "";
    for (int i = 0; i < seqLength; i++) {
        char nuc = nucChars[rand() % 4];
        targetSequence += nuc;
        spawnNucleotideOnMap(nuc);
    }

    float baselineSpeed = 0.02f;
    if (currentDifficulty == MEDIUM) baselineSpeed = 0.04f;
    else if (currentDifficulty == HARD) baselineSpeed = 0.06f;
    else if (currentDifficulty == BRUTAL) baselineSpeed = 0.10f;

    enzymes.clear();
    enzymes.push_back({ 1.0f,  1.0f,   1.0f, 0.2f, 0.2f, baselineSpeed });
    enzymes.push_back({ 1.0f,  (float)(MAP_ROWS - 2),  0.2f, 0.9f, 0.2f, baselineSpeed });
    enzymes.push_back({ (float)(MAP_COLS - 2), (float)(MAP_ROWS - 2),  1.0f, 0.5f, 0.0f, baselineSpeed });
    enzymes.push_back({ (float)(MAP_COLS - 2), (float)(MAP_ROWS - 2),  1.0f, 0.5f, 0.0f, baselineSpeed });
}

void renderMenu() {
    //glClear(GL_COLOR_BUFFER_BIT);
    //glColor3f(1.0f, 1.0f, 0.0f);
    //drawText(WINDOW_WIDTH / 2 - 65, WINDOW_HEIGHT / 2 + 100, "DNA-man", GLUT_BITMAP_TIMES_ROMAN_24);
    //glColor3f(0.4f, 0.8f, 1.0f);
    //drawText(WINDOW_WIDTH / 2 - 115, WINDOW_HEIGHT / 2 + 80, "Nucleotides Sequencer Maze Arcade", GLUT_BITMAP_HELVETICA_12);

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 0.0f);
    drawStrokeText(WINDOW_WIDTH / 2 - 128, 450, "DNA-man", 0.35f, 4.0f, GLUT_STROKE_MONO_ROMAN);
    glColor3f(0.4f, 0.8f, 1.0f);
    drawStrokeText(WINDOW_WIDTH / 2 - 176, 410, "Nucleotides Sequencer Maze Arcade", 0.13f, 1.5f, GLUT_STROKE_ROMAN);

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

void renderDifficultySelect() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WINDOW_WIDTH / 2 - 140, 450, "SELECT GAME DIFFICULTY", GLUT_BITMAP_TIMES_ROMAN_24);

    int btnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;

    glColor3f(isMouseOverButton(btnX, EASY_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.2f : 0.1f, 0.7f, 0.2f);
    glRecti(btnX, EASY_BTN_Y, btnX + BTN_WIDTH, EASY_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, EASY_BTN_Y + 18, "EASY");

    glColor3f(isMouseOverButton(btnX, MEDIUM_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.6f : 0.5f, 0.5f, 0.1f);
    glRecti(btnX, MEDIUM_BTN_Y, btnX + BTN_WIDTH, MEDIUM_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 60, MEDIUM_BTN_Y + 18, "MEDIUM");

    glColor3f(isMouseOverButton(btnX, HARD_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.7f : 0.5f, 0.3f, 0.1f);
    glRecti(btnX, HARD_BTN_Y, btnX + BTN_WIDTH, HARD_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, HARD_BTN_Y + 18, "HARD");

    glColor3f(isMouseOverButton(btnX, BRUTAL_BTN_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.8f : 0.5f, 0.1f, 0.1f);
    glRecti(btnX, BRUTAL_BTN_Y, btnX + BTN_WIDTH, BRUTAL_BTN_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, BRUTAL_BTN_Y + 18, "BRUTAL");

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
                glVertex2i(x + TILE_SIZE / 2 - 2, y + TILE_SIZE / 2 - 2);
                glVertex2i(x + TILE_SIZE / 2 + 2, y + TILE_SIZE / 2 - 2);
                glVertex2i(x + TILE_SIZE / 2 + 2, y + TILE_SIZE / 2 + 2);
                glVertex2i(x + TILE_SIZE / 2 - 2, y + TILE_SIZE / 2 + 2);
                glEnd();
            }
            else if (maze[r][c] >= 3 && maze[r][c] <= 6) {
                char letter = nucChars[maze[r][c] - 3];
                if (letter == 'A') glColor3f(1.0f, 0.3f, 0.3f);
                else if (letter == 'T') glColor3f(0.3f, 1.0f, 0.3f);
                else if (letter == 'G') glColor3f(0.3f, 0.6f, 1.0f);
                else if (letter == 'C') glColor3f(1.0f, 0.4f, 1.0f);

                glBegin(GL_LINE_LOOP);
                glVertex2i(x + 2, y + 2);
                glVertex2i(x + TILE_SIZE - 2, y + 2);
                glVertex2i(x + TILE_SIZE - 2, y + TILE_SIZE - 2);
                glVertex2i(x + 2, y + TILE_SIZE - 2);
                glEnd();

                std::string s(1, letter);
                drawText(x + TILE_SIZE / 2 - 4, y + TILE_SIZE / 2 - 4, s, GLUT_BITMAP_HELVETICA_10);
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
        glVertex2f(pacX + cos(rad) * (TILE_SIZE / 2 - 1), pacY + sin(rad) * (TILE_SIZE / 2 - 1));
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
            glVertex2f(gx + cos(rad) * (TILE_SIZE / 2 - 1), gy + sin(rad) * (TILE_SIZE / 2 - 1));
        }
        glEnd();
        glRecti(gx - (TILE_SIZE / 2 - 1), gy - (TILE_SIZE / 2 - 1), gx + (TILE_SIZE / 2 - 1), gy);
    }

    glutSwapBuffers();
}

// Added option interface layer for the execution break state
void renderPauseMenu() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.84f, 0.0f);
    drawText(WINDOW_WIDTH / 2 - 75, 450, "GAME PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.6f, 0.6f, 0.6f);
    drawText(WINDOW_WIDTH / 2 - 105, 415, "Press 'ESC' to resume simulation", GLUT_BITMAP_HELVETICA_12);

    int btnX = (WINDOW_WIDTH - BTN_WIDTH) / 2;

    // Restart Button Setup
    glColor3f(isMouseOverButton(btnX, PAUSE_RESTART_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.2f : 0.1f, 0.5f, 0.2f);
    glRecti(btnX, PAUSE_RESTART_Y, btnX + BTN_WIDTH, PAUSE_RESTART_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 55, PAUSE_RESTART_Y + 18, "RESTART");

    // Main Menu Return Button Setup
    glColor3f(isMouseOverButton(btnX, PAUSE_MENU_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.2f : 0.1f, 0.3f, 0.6f);
    glRecti(btnX, PAUSE_MENU_Y, btnX + BTN_WIDTH, PAUSE_MENU_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 70, PAUSE_MENU_Y + 18, "MENU");

    // Close Game/Exit Button Setup
    glColor3f(isMouseOverButton(btnX, PAUSE_EXIT_Y, BTN_WIDTH, BTN_HEIGHT) ? 0.6f : 0.4f, 0.1f, 0.1f);
    glRecti(btnX, PAUSE_EXIT_Y, btnX + BTN_WIDTH, PAUSE_EXIT_Y + BTN_HEIGHT);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(btnX + 75, PAUSE_EXIT_Y + 18, "EXIT");

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
    else if (gameState == PAUSE) renderPauseMenu(); // Route logic tree block here
    else if (gameState == GAME_OVER) renderGameOver(false);
    else if (gameState == WIN) renderGameOver(true);
}

bool isWall(float x, float y) {
    int cellX = (int)floor(x + 0.5f);
    int cellY = (int)floor(y + 0.5f);
    if (cellX < 0 || cellX >= MAP_COLS || cellY < 0 || cellY >= MAP_ROWS) return true;
    return (maze[cellY][cellX] == 1);
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

                if (currentDifficulty == HARD) {
                    dnamanX = 15.0f; dnamanY = 12.0f;
                }
                else {
                    dnamanX = 9.0f; dnamanY = 7.0f;
                }
                dirX = 0; dirY = 0; nextDirX = 0; nextDirY = 0;

                enzymes[0].x = 1.0f;  enzymes[0].y = 1.0f;
                enzymes[1].x = 1.0f;  enzymes[1].y = (float)(MAP_ROWS - 2);
                enzymes[2].x = (float)(MAP_COLS - 2); enzymes[2].y = (float)(MAP_ROWS - 2);
                break;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Added basic ASCII processor function for capturing Esc mechanics
void keyboardKeys(unsigned char key, int x, int y) {
    if (key == 27) { // 27 matches the default ASCII Esc key map assignment
        if (gameState == GAME) {
            gameState = PAUSE;
        }
        else if (gameState == PAUSE) {
            gameState = GAME;
        }
    }
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

    if (gameState == MENU || gameState == DIFFICULTY_SELECT || gameState == PAUSE || gameState == GAME_OVER || gameState == WIN) {
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
            else if (isMouseOverButton(btnX, BRUTAL_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
                currentDifficulty = BRUTAL;
                resetGame();
                gameState = GAME;
            }
        }
        // Added Pause tracking handlers
        else if (gameState == PAUSE) {
            if (isMouseOverButton(btnX, PAUSE_RESTART_Y, BTN_WIDTH, BTN_HEIGHT)) {
                resetGame();
                gameState = GAME;
            }
            else if (isMouseOverButton(btnX, PAUSE_MENU_Y, BTN_WIDTH, BTN_HEIGHT)) {
                gameState = MENU;
            }
            else if (isMouseOverButton(btnX, PAUSE_EXIT_Y, BTN_WIDTH, BTN_HEIGHT)) {
                glutLeaveMainLoop();
            }
        }
        else if (gameState == GAME_OVER || gameState == WIN) {
            if (isMouseOverButton(btnX, RESTART_BTN_Y, BTN_WIDTH, BTN_HEIGHT)) {
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
    glutCreateWindow("DNA-man");

    glutReshapeFunc(reshape);

    init();
    startBackgroundMusic();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardKeys); // Registered keyboard listener function block here
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClicks);
    glutPassiveMotionFunc(passiveMouse);
    glutMotionFunc(passiveMouse);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}