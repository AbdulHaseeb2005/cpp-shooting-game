#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <ctime>
#include <conio.h>
#include <windows.h>

#define SCREEN_WIDTH 90
#define SCREEN_HEIGHT 26
#define WIN_WIDTH 70
#define ENEMY_SPEED_INITIAL 3
#define BONUS_ENEMY_SPEED 1
#define MAX_BULLETS 20

using namespace std;

// Structure to store player scores
struct PlayerScore {
    string name;
    int score;
    int maxStreak;
    string date;
};

// Global variables for high scores
vector<PlayerScore> highScores;
const string SCORE_FILE = "highscores.txt";
const int MAX_HIGH_SCORES = 5;

// Game variables
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

int playerPos = WIN_WIDTH / 2;
int enemyX[3], enemyY[3], enemyMoveCounter[3] = {0};
bool enemyAlive[3] = {false};
int enemyType[3] = {0}; // 0: Normal, 1: Fast, 2: Zigzag

int bulletX[MAX_BULLETS], bulletY[MAX_BULLETS];
bool bulletActive[MAX_BULLETS];
int score = 0;

int bonusEnemyX, bonusEnemyY;
bool bonusEnemyAlive = false;
int bonusEnemyMoveCounter = 0;
int streakCount = 0;

bool shieldActive = false;
int shieldDuration = 0;

bool rapidFireActive = false;
int rapidFireDuration = 0;

int level = 1;
int enemySpeed = ENEMY_SPEED_INITIAL - level; // Increase speed as level increases

void gotoxy(int x, int y);
void setcursor(bool visible);
void setColor(int color);
void drawBorder();
void updateBorderColors();
void drawPlayer();
void erasePlayer();
void generateEnemy(int index);
void generateBonusEnemy();
void drawEnemy(int index);
void eraseEnemy(int index);
void moveEnemy(int index);
void shootBullet();
void moveBullets();
void updateScoreDisplay();
bool bulletHit();
bool collision();
void gameover();
void play();
void loadHighScores();
void saveHighScores();
void displayHighScores();
string getCurrentDate();
void handleNewScore(int score, int maxStreak);
void activateShield();
void activateRapidFire();
void checkPowerUpStatus();
void levelUp();
void displayAchievements();

void gotoxy(int x, int y) {
    CursorPosition.X = x;
    CursorPosition.Y = y;
    SetConsoleCursorPosition(console, CursorPosition);
}

void setcursor(bool visible) {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.bVisible = visible;
    cursorInfo.dwSize = 20;
    SetConsoleCursorInfo(console, &cursorInfo);
}

void setColor(int color) {
    SetConsoleTextAttribute(console, color);
}

void drawBorder() {
    // Top border - Rainbow effect
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        gotoxy(i, 0);
        setColor(9 + (i % 6)); // Cycles through colors 9-14
        cout << "▄";
    }

    // Bottom border - Rainbow effect
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        gotoxy(i, SCREEN_HEIGHT);
        setColor(9 + (i % 6));
        cout << "▀";
    }

    // Left border - Gradient effect
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(0, i);
        setColor(10 + (i % 5));
        cout << "█";
    }

    // Right border - Gradient effect
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(SCREEN_WIDTH, i);
        setColor(10 + (i % 5));
        cout << "█";
    }

    // Game area divider - Pulsing effect
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(WIN_WIDTH, i);
        setColor(11 + (i % 4));
        cout << "║";
    }

    // Corner decorations
    setColor(14);
    gotoxy(0, 0); cout << "╔";
    gotoxy(SCREEN_WIDTH, 0); cout << "╗";
    gotoxy(0, SCREEN_HEIGHT); cout << "╚";
    gotoxy(SCREEN_WIDTH, SCREEN_HEIGHT); cout << "╝";

    setColor(7);
}

void updateBorderColors() {
    static int colorOffset = 0;
    colorOffset = (colorOffset + 1) % 6;

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        gotoxy(i, 0);
        setColor(9 + ((i + colorOffset) % 6));
        cout << "▄";

        gotoxy(i, SCREEN_HEIGHT);
        setColor(9 + ((i + colorOffset) % 6));
        cout << "▀";
    }
}

void drawPlayer() {
    setColor(shieldActive ? 11 : 10); // Blue if shield is active
    gotoxy(playerPos, SCREEN_HEIGHT - 2); cout << "<^>";
    setColor(7);
}

void erasePlayer() {
    gotoxy(playerPos, SCREEN_HEIGHT - 2); cout << "   ";
}

void generateEnemy(int index) {
    enemyX[index] = 3 + rand() % (WIN_WIDTH - 6);
    enemyY[index] = 1;
    enemyAlive[index] = true;
    enemyMoveCounter[index] = 0;
    enemyType[index] = rand() % 3; // Random enemy type
}

void generateBonusEnemy() {
    bonusEnemyX = 3 + rand() % (WIN_WIDTH - 6);
    bonusEnemyY = 1;
    bonusEnemyAlive = true;
    bonusEnemyMoveCounter = 0;
}

void drawEnemy(int index) {
    if (enemyAlive[index]) {
        switch (enemyType[index]) {
            case 0: // Normal Enemy
                setColor(12);
                gotoxy(enemyX[index], enemyY[index]); cout << "***";
                break;
            case 1: // Fast Enemy
                setColor(13);
                gotoxy(enemyX[index], enemyY[index]); cout << ">>>";
                break;
            case 2: // Zigzag Enemy
                setColor(14);
                gotoxy(enemyX[index], enemyY[index]); cout << "/|\\";
                break;
        }
        setColor(7);
    }

    if (bonusEnemyAlive) {
        setColor(14);
        gotoxy(bonusEnemyX, bonusEnemyY); cout << "@#@";
        setColor(7);
    }
}

void eraseEnemy(int index) {
    if (enemyAlive[index]) {
        gotoxy(enemyX[index], enemyY[index]); cout << "   ";
    }
    if (bonusEnemyAlive) {
        gotoxy(bonusEnemyX, bonusEnemyY); cout << "   ";
    }
}

void moveEnemy(int index) {
    if (enemyAlive[index]) {
        int speed = enemySpeed;
        if (enemyType[index] == 1) speed = max(1, enemySpeed - 1); // Fast enemy moves faster

        if (++enemyMoveCounter[index] >= speed) {
            eraseEnemy(index);
            enemyY[index]++;
            enemyMoveCounter[index] = 0;

            // Zigzag movement
            if (enemyType[index] == 2) {
                if (rand() % 2)
                    enemyX[index] += (enemyX[index] < WIN_WIDTH - 4) ? 1 : -1;
                else
                    enemyX[index] -= (enemyX[index] > 2) ? 1 : -1;
            }

            if (enemyY[index] > SCREEN_HEIGHT - 2) {
                generateEnemy(index);
                score--;
                streakCount = 0;
                updateScoreDisplay();
            }
            drawEnemy(index);
        }
    }

    if (bonusEnemyAlive) {
        if (++bonusEnemyMoveCounter >= BONUS_ENEMY_SPEED) {
            eraseEnemy(-1);
            bonusEnemyY++;
            bonusEnemyMoveCounter = 0;
            if (bonusEnemyY > SCREEN_HEIGHT - 2) {
                bonusEnemyAlive = false;
            }
            drawEnemy(-1);
        }
    }
}

void shootBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bulletActive[i]) {
            bulletX[i] = playerPos + 1;
            bulletY[i] = SCREEN_HEIGHT - 3;
            bulletActive[i] = true;
            if (!rapidFireActive) break; // If rapid fire is inactive, shoot one bullet
        }
    }
}

void moveBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bulletActive[i]) {
            gotoxy(bulletX[i], bulletY[i]); cout << " ";
            bulletY[i]--;
            if (bulletY[i] <= 0) bulletActive[i] = false;
            else {
                setColor(14);
                gotoxy(bulletX[i], bulletY[i]); cout << "^";
                setColor(7);
            }
        }
    }
}

bool bulletHit() {
    bool hit = false;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bulletActive[i]) {
            for (int j = 0; j < 3; j++) {
                if (enemyAlive[j] && bulletY[i] == enemyY[j] && bulletX[i] >= enemyX[j] && bulletX[i] <= enemyX[j] + 2) {
                    bulletActive[i] = false;
                    eraseEnemy(j);
                    enemyAlive[j] = false;
                    streakCount++;
                    score += 10;
                    if (rand() % 10 < 2) activateShield();
                    if (rand() % 10 < 2) activateRapidFire();
                    if (streakCount > 0 && streakCount % 5 == 0) {
                        generateBonusEnemy();
                    }
                    updateScoreDisplay();
                    hit = true;
                }
            }
            if (bonusEnemyAlive && bulletY[i] == bonusEnemyY && bulletX[i] >= bonusEnemyX && bulletX[i] <= bonusEnemyX + 2) {
                bulletActive[i] = false;
                eraseEnemy(-1);
                bonusEnemyAlive = false;
                score += 30;
                updateScoreDisplay();
                hit = true;
            }
        }
    }
    return hit;
}

bool collision() {
    for (int i = 0; i < 3; i++) {
        if (enemyAlive[i] && enemyY[i] == SCREEN_HEIGHT - 2 && enemyX[i] <= playerPos + 2 && enemyX[i] + 2 >= playerPos) {
            if (shieldActive) {
                enemyAlive[i] = false;
                eraseEnemy(i);
                shieldActive = false;
                drawPlayer();
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}

void updateScoreDisplay() {
    gotoxy(WIN_WIDTH + 5, 5); cout << "Score: " << score << "    ";
    gotoxy(WIN_WIDTH + 5, 6); cout << "Streak: " << streakCount << "    ";
    gotoxy(WIN_WIDTH + 5, 7); cout << "Level: " << level << "    ";
    if (shieldActive) {
        gotoxy(WIN_WIDTH + 5, 8); cout << "Shield: ON ";
    } else {
        gotoxy(WIN_WIDTH + 5, 8); cout << "Shield: OFF";
    }
    if (rapidFireActive) {
        gotoxy(WIN_WIDTH + 5, 9); cout << "Rapid Fire: ON ";
    } else {
        gotoxy(WIN_WIDTH + 5, 9); cout << "Rapid Fire: OFF";
    }
}

string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char date[11];
    sprintf_s(date, "%02d/%02d/%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    return string(date);
}

void loadHighScores() {
    highScores.clear();
    ifstream file(SCORE_FILE);
    if (file.is_open()) {
        PlayerScore ps;
        while (file >> ps.name >> ps.score >> ps.maxStreak >> ps.date) {
            highScores.push_back(ps);
        }
        file.close();
    }
}

void saveHighScores() {
    ofstream file(SCORE_FILE);
    if (file.is_open()) {
        for (const PlayerScore& ps : highScores) {
            file << ps.name << " " << ps.score << " "
                 << ps.maxStreak << " " << ps.date << endl;
        }
        file.close();
    }
}

void displayHighScores() {
    system("cls");
    gotoxy(10, 3); cout << "=== HIGH SCORES ===";

    if (highScores.empty()) {
        gotoxy(10, 5); cout << "No high scores yet!";
    } else {
        gotoxy(8, 4); cout << "Rank  Name          Score    Streak    Date";
        for (size_t i = 0; i < highScores.size(); i++) {
            gotoxy(8, 5 + i);
            cout << i + 1 << ".    ";
            cout << left << setw(14) << highScores[i].name;
            cout << right << setw(6) << highScores[i].score;
            cout << right << setw(10) << highScores[i].maxStreak;
            cout << "    " << highScores[i].date;
        }
    }

    gotoxy(10, 12); cout << "Press any key to continue...";
    _getch();
}

void handleNewScore(int score, int maxStreak) {
    if (score > 0) {  // Only handle positive scores
        char name_buffer[20];
        system("cls");
        gotoxy(10, 5); cout << "You got a score of " << score << "!";
        gotoxy(10, 6); cout << "Enter your name (max 15 chars): ";
        cin.ignore();
        cin.getline(name_buffer, 15);

        PlayerScore newScore;
        newScore.name = string(name_buffer);
        newScore.score = score;
        newScore.maxStreak = maxStreak;
        newScore.date = getCurrentDate();

        highScores.push_back(newScore);

        // Sort by score in descending order
        sort(highScores.begin(), highScores.end(),
            [](const PlayerScore& a, const PlayerScore& b) {
                return a.score > b.score;
            });

        // Keep only top MAX_HIGH_SCORES
        if (highScores.size() > MAX_HIGH_SCORES) {
            highScores.resize(MAX_HIGH_SCORES);
        }

        saveHighScores();
        displayHighScores();
    }
}

void activateShield() {
    shieldActive = true;
    shieldDuration = 50; // Lasts for 50 iterations
    drawPlayer();
}

void activateRapidFire() {
    rapidFireActive = true;
    rapidFireDuration = 50; // Lasts for 50 iterations
}

void checkPowerUpStatus() {
    if (shieldActive) {
        shieldDuration--;
        if (shieldDuration <= 0) {
            shieldActive = false;
            drawPlayer();
        }
    }
    if (rapidFireActive) {
        rapidFireDuration--;
        if (rapidFireDuration <= 0) {
            rapidFireActive = false;
        }
    }
}

void levelUp() {
    if (score >= level * 100) {
        level++;
        enemySpeed = max(1, ENEMY_SPEED_INITIAL - level);
        gotoxy(WIN_WIDTH + 5, 10); cout << "Level Up!          ";
    }
}

void gameover() {
    system("cls");
    cout << "\n\t\t----- Game Over -----";
    cout << "\n\t\tYour Score: " << score;
    cout << "\n\t\tMax Streak: " << streakCount;
    cout << "\n\t\tPress any key to continue...";
    _getch();
    handleNewScore(score, streakCount);
}

void play() {
    score = 0;
    streakCount = 0;
    level = 1;
    enemySpeed = ENEMY_SPEED_INITIAL - level;
    shieldActive = false;
    rapidFireActive = false;
    playerPos = WIN_WIDTH / 2;

    for (int i = 0; i < 3; i++) {
        generateEnemy(i);
    }

    bonusEnemyAlive = false;
    fill(begin(bulletActive), end(bulletActive), false);
    system("cls");
    drawBorder();
    updateScoreDisplay();
    drawPlayer();

    int frameCount = 0;
    while (1) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'a' || ch == 'A') {
                if (playerPos > 2) {
                    erasePlayer();
                    playerPos -= 2;
                    drawPlayer();
                }
            } else if (ch == 'd' || ch == 'D') {
                if (playerPos < WIN_WIDTH - 4) {
                    erasePlayer();
                    playerPos += 2;
                    drawPlayer();
                }
            } else if (ch == 32) { // Spacebar
                shootBullet();
            } else if (ch == 27) { // ESC key
                break;
            }
        }

        if (++frameCount >= 10) {
            updateBorderColors();
            frameCount = 0;
            checkPowerUpStatus();
        }

        for (int i = 0; i < 3; i++) {
            moveEnemy(i);
        }
        moveBullets();
        if (bulletHit()) {
            for (int i = 0; i < 3; i++) {
                if (!enemyAlive[i]) {
                    generateEnemy(i);
                }
            }
        }
        if (collision()) {
            gameover();
            return;
        }

        levelUp(); // Check if we should level up
        Sleep(100);
    }
}

int main() {
    setcursor(0);
    srand((unsigned)time(0));

    loadHighScores();  // Load high scores at startup

    do {
        system("cls");
        gotoxy(10, 5); cout << "********************************";
        gotoxy(10, 6); cout << "*        Space Shooter         *";
        gotoxy(10, 7); cout << "********************************";
        gotoxy(10, 9); cout << "1. Start Game";
        gotoxy(10, 10); cout << "2. High Scores";
        gotoxy(10, 11); cout << "3. Quit";
        gotoxy(10, 13); cout << "Select option: ";
        char op = _getch();
        if (op == '1') play();
        else if (op == '2') displayHighScores();
        else if (op == '3') exit(0);
    } while (1);

    return 0;
}
