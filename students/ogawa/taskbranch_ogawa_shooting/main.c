#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define HEIGHT 24
#define WIDTH 80
#define MAX_BULLETS 32
#define MAX_ENEMIES 24

struct Bullet {
    int x;
    int y;
    int alive;
    int dir; /* 1: player -> up, -1: enemy -> down */
};

struct EnemyBlock {
    int x;
    int y;
    int alive;
};

void enableAnsi(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

void clearScreen(void) {
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
}

void hideCursor(void) {
    printf("\x1b[?25l");
    fflush(stdout);
}

void showCursor(void) {
    printf("\x1b[?25h");
    fflush(stdout);
}

void drawFrame(int playerX, int playerY, int lives, int enemyCount, int score,
               struct Bullet playerBullets[], struct Bullet enemyBullets[],
               struct EnemyBlock enemies[]) {
    clearScreen();

    printf("\x1b[33m");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int draw = 0;
            char ch = ' ';

            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                ch = '#';
                draw = 1;
            }

            if (!draw && x == playerX && y == playerY) {
                ch = '^';
                draw = 1;
            }

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (playerBullets[i].alive && playerBullets[i].x == x && playerBullets[i].y == y) {
                    ch = '|';
                    draw = 1;
                    break;
                }
                if (enemyBullets[i].alive && enemyBullets[i].x == x && enemyBullets[i].y == y) {
                    ch = '!';
                    draw = 1;
                    break;
                }
            }

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
                    ch = 'X';
                    draw = 1;
                    break;
                }
            }

            putchar(ch);
        }
        putchar('\n');
    }

    printf("\x1b[37m");
    printf("Lives: %d  Enemies: %d  Score: %d\n", lives, enemyCount, score);
    printf("Move: A/D  Shoot: Space  Quit: Q\n");
    fflush(stdout);
}

int spawnPlayerBullet(struct Bullet bullets[], int x, int y) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].alive = 1;
            bullets[i].dir = 1;
            return 1;
        }
    }
    return 0;
}

int spawnEnemyBullet(struct Bullet bullets[], int x, int y) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].alive = 1;
            bullets[i].dir = -1;
            return 1;
        }
    }
    return 0;
}

int updateBullets(struct Bullet bullets[], int dir, struct EnemyBlock enemies[], int *enemyCount,
                  int playerX, int playerY, int *lives) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) {
            continue;
        }

        bullets[i].y += dir;

        if (dir == 1) {
            if (bullets[i].y <= 1) {
                bullets[i].alive = 0;
                continue;
            }

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (enemies[e].alive && bullets[i].x == enemies[e].x && bullets[i].y == enemies[e].y) {
                    enemies[e].alive = 0;
                    (*enemyCount)--;
                    bullets[i].alive = 0;
                    break;
                }
            }
        } else {
            if (bullets[i].y >= HEIGHT - 2) {
                bullets[i].alive = 0;
                continue;
            }

            if (bullets[i].x == playerX && bullets[i].y == playerY) {
                (*lives)--;
                bullets[i].alive = 0;
            }
        }
    }
    return 0;
}

int main(void) {
    enableAnsi();
    hideCursor();
    srand((unsigned int)time(NULL));

    int playerX = WIDTH / 2;
    int playerY = HEIGHT - 2;
    int lives = 3;
    int enemyCount = 0;
    int score = 0;
    int attackTimer = 0;

    struct Bullet playerBullets[MAX_BULLETS];
    struct Bullet enemyBullets[MAX_BULLETS];
    struct EnemyBlock enemies[MAX_ENEMIES];

    for (int i = 0; i < MAX_BULLETS; i++) {
        playerBullets[i].alive = 0;
        enemyBullets[i].alive = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x = 10 + (i % 6) * 2;
        enemies[i].y = 3 + (i / 6);
        enemies[i].alive = 1;
    }
    enemyCount = MAX_ENEMIES;

    int running = 1;
    while (running) {
        drawFrame(playerX, playerY, lives, enemyCount, score, playerBullets, enemyBullets, enemies);

        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'a' || ch == 'A') {
                if (playerX > 1) playerX--;
            } else if (ch == 'd' || ch == 'D') {
                if (playerX < WIDTH - 2) playerX++;
            } else if (ch == ' ' || ch == 13) {
                spawnPlayerBullet(playerBullets, playerX, playerY - 1);
            } else if (ch == 'q' || ch == 'Q') {
                running = 0;
            }
        }

        updateBullets(playerBullets, -1, enemies, &enemyCount, playerX, playerY, &lives);
        updateBullets(enemyBullets, 1, enemies, &enemyCount, playerX, playerY, &lives);

        if (attackTimer <= 0) {
            int selected = -1;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].alive) {
                    selected = i;
                    break;
                }
            }
            if (selected >= 0) {
                int index = rand() % MAX_ENEMIES;
                while (!enemies[index].alive) {
                    index = rand() % MAX_ENEMIES;
                }
                spawnEnemyBullet(enemyBullets, enemies[index].x, enemies[index].y);
                attackTimer = 10 + rand() % 8;
            }
        } else {
            attackTimer--;
        }

        if (enemyCount <= 0) {
            clearScreen();
            printf("\x1b[32m");
            printf("Victory! All enemies destroyed.\n");
            break;
        }

        if (lives <= 0) {
            clearScreen();
            printf("\x1b[31m");
            printf("Game Over! The enemy hit you.\n");
            break;
        }

        Sleep(80);
    }

    showCursor();
    return 0;
}
