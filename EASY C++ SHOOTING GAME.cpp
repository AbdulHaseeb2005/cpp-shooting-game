#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <chrono>

#define SCREEN_WIDTH 90
#define SCREEN_HEIGHT 26
#define WIN_WIDTH 70
#define GAP_SIZE 7

using namespace std;
using namespace std::chrono; 

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

int playerPos = WIN_WIDTH / 2;
int enemyX, enemyY;
bool enemyAlive = true;
int bonusEnemyX, bonusEnemyY;
bool bonusEnemyAlive = false;
int bulletX[20], bulletY[20];
bool bulletActive[20];
int score = 0;
int hitCount = 0; 

void gotoxy(int x, int y) {
    CursorPosition.X = x;
    CursorPosition.Y = y;
    SetConsoleCursorPosition(console, CursorPosition);
}

void setcursor(bool visible, DWORD size) {
    if (size == 0) size = 20;
    CONSOLE_CURSOR_INFO lpCursor;
    lpCursor.bVisible = visible;
    lpCursor.dwSize = size;
    SetConsoleCursorInfo(console, &lpCursor);
}

void drawBorder() {
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        gotoxy(i, 0); cout << "±";
        gotoxy(i, SCREEN_HEIGHT); cout << "±";
    }
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(0, i); cout << "±";
        gotoxy(SCREEN_WIDTH, i); cout << "±";
    }
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(WIN_WIDTH, i); cout << "±";
    }
}

void drawPlayer() {
    gotoxy(playerPos, SCREEN_HEIGHT - 2); cout << "<^>";
}

void erasePlayer() {
    gotoxy(playerPos, SCREEN_HEIGHT - 2); cout << "   ";
}

void generateEnemy() {
    enemyX = 3 + rand() % (WIN_WIDTH - 6);
    enemyY = 1;
    enemyAlive = true;
}

void generateBonusEnemy() {
    bonusEnemyX = 3 + rand() % (WIN_WIDTH - 6);
    bonusEnemyY = 1;
    bonusEnemyAlive = true;
}

void drawEnemy() {
    if (enemyAlive) {
        gotoxy(enemyX, enemyY); cout << "***";
    }
}

void drawBonusEnemy() {
    if (bonusEnemyAlive) {
        gotoxy(bonusEnemyX, bonusEnemyY); cout << "@@@";
    }
}

void eraseEnemy() {
    if (enemyAlive) {
        gotoxy(enemyX, enemyY); cout << "   ";
    }
    if (bonusEnemyAlive) {
        gotoxy(bonusEnemyX, bonusEnemyY); cout << "   ";
    }
}

void moveEnemy() {
    if (enemyAlive) {
        eraseEnemy();
        enemyY++;
        if (enemyY > SCREEN_HEIGHT - 2) {
            generateEnemy();
            score--;
        }
        drawEnemy();
    }
    if (bonusEnemyAlive) {
        eraseEnemy();
        bonusEnemyY++;
        if (bonusEnemyY > SCREEN_HEIGHT - 2) {
            bonusEnemyAlive = false;
        }
        drawBonusEnemy();
    }
}

void shootBullet() {
    for (int i = 0; i < 20; i++) {
        if (!bulletActive[i]) {
            bulletX[i] = playerPos + 1;
            bulletY[i] = SCREEN_HEIGHT - 3;
            bulletActive[i] = true;
            break;
        }
    }
}

void moveBullets() {
    for (int i = 0; i < 20; i++) {
        if (bulletActive[i]) {
            gotoxy(bulletX[i], bulletY[i]); cout << " ";
            bulletY[i]--;
            if (bulletY[i] <= 0) {
                bulletActive[i] = false;
            } else {
                gotoxy(bulletX[i], bulletY[i]); cout << "|";
            }
        }
    }
}

bool bulletHit() {
    for (int i = 0; i < 20; i++) {
        if (bulletActive[i]) {
            if (enemyAlive && bulletY[i] == enemyY && 
                (bulletX[i] >= enemyX && bulletX[i] <= enemyX + 2)) {
                bulletActive[i] = false;
                eraseEnemy();
                enemyAlive = false;
                hitCount++;
                if (hitCount % 5 == 0) {
                    generateBonusEnemy();
                }
                return true;
            }
            if (bonusEnemyAlive && bulletY[i] == bonusEnemyY && 
                (bulletX[i] >= bonusEnemyX && bulletX[i] <= bonusEnemyX + 2)) {
                bulletActive[i] = false;
                eraseEnemy();
                bonusEnemyAlive = false;
                score += 20; 
                return true;
            }
        }
    }
    return false;
}

bool collision() {
    if (enemyAlive && enemyY == SCREEN_HEIGHT - 2) {
        if (enemyX <= playerPos + 2 && enemyX + 2 >= playerPos) {
            return true;
        }
    }
    if (bonusEnemyAlive && bonusEnemyY == SCREEN_HEIGHT - 2) {
        if (bonusEnemyX <= playerPos + 2 && bonusEnemyX + 2 >= playerPos) {
            return true;
        }
    }
    return false;
}

void updateScore() {
    gotoxy(WIN_WIDTH + 5, 5); cout << "Score: " << score;
}

void saveScore(const string &playerName) {
    auto now = system_clock::to_time_t(system_clock::now());
    stringstream timeStream;
    timeStream << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S");

    ofstream file("highscores.txt", ios::app);
    if (file.is_open()) {
        file << left << setw(20) << playerName
             << setw(10) << score
             << setw(20) << timeStream.str()
             << endl;
        file.close();
    }
}

void showHighScores() {
    ifstream file("highscores.txt");
    system("cls");
    gotoxy(10, 4); cout << "---------------------------------------------------";
    gotoxy(10, 5); cout << left << setw(20) << "Name" << setw(10) << "Score" << setw(20) << "Date/Time";
    gotoxy(10, 6); cout << "---------------------------------------------------";
    
    if (file.is_open()) {
        string line;
        int yPos = 7;
        while (getline(file, line)) {
            gotoxy(10, yPos++);
            cout << line;
        }
        file.close();
    } else {
        gotoxy(10, 7); cout << "No high scores recorded yet!";
    }

    gotoxy(10, 24); cout << "Press any key to return to the menu...";
    _getch();
}

void gameover(const string &playerName) {
    saveScore(playerName);
    system("cls");
    cout << "\n\t\t---------------------------";
    cout << "\n\t\t-------- Game Over --------";
    cout << "\n\t\t---------------------------";
    cout << "\n\t\tYour Score: " << score;
    cout << "\n\t\tPress any key to return to the menu.";
    _getch();
}

void play() {
    string playerName;
    cout << "\n\t  Enter your name: ";
    getline(cin, playerName);

    score = 0;
    hitCount = 0;
    playerPos = WIN_WIDTH / 2;
    generateEnemy();
    bonusEnemyAlive = false;

    for (int i = 0; i < 20; i++) {
        bulletActive[i] = false;
    }

    system("cls");
    drawBorder();
    updateScore();

    gotoxy(WIN_WIDTH + 5, 2); cout << "Shooting Game";
    gotoxy(WIN_WIDTH + 5, 4); cout << "Controls:";
    gotoxy(WIN_WIDTH + 5, 6); cout << "A - Move Left";
    gotoxy(WIN_WIDTH + 5, 7); cout << "D - Move Right";
    gotoxy(WIN_WIDTH + 5, 8); cout << "Space - Shoot";

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
            } else if (ch == 32) {
                shootBullet();
            } else if (ch == 27) {
                break;
            }
        }

        moveEnemy();
        moveBullets();

        if (bulletHit()) {
            score += 10;
            updateScore();
            generateEnemy();
        }

        if (collision()) {
            gameover(playerName);
            return;
        }

        Sleep(100); 
    }
}

int main() {
    setcursor(0, 0);
    srand((unsigned)time(0));

    do {
        system("cls");
        gotoxy(10, 5); cout << " ------------------------------------------ ";
        gotoxy(10, 6); cout << " |      Shooting Game  By ABDUL HASEEB    | ";
        gotoxy(10, 7); cout << " ------------------------------------------ ";
        gotoxy(10, 9); cout << "1. Start Game";
        gotoxy(10, 10); cout << "2. View High Scores";
        gotoxy(10, 11); cout << "3. Quit";
        gotoxy(10, 13); cout << "Select option: ";
        char op = _getch();

        if (op == '1') play();
        else if (op == '2') showHighScores();
        else if (op == '3') exit(0);

    } while (1);

    return 0;
}
