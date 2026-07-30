#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define FIELD_W 50
#define FIELD_H 20

#define ENEMY_ROWS 3
#define ENEMY_COLS 6
#define BLOCK_W 3
#define BLOCK_H 2
#define GAP_X 1
#define GAP_Y 1
#define FORM_W (ENEMY_COLS*BLOCK_W + (ENEMY_COLS-1)*GAP_X)
#define FORM_H (ENEMY_ROWS*BLOCK_H + (ENEMY_ROWS-1)*GAP_Y)

#define MAX_PBULLETS 10
#define MAX_EBULLETS 20

typedef struct { int x, y, active; } Bullet;

typedef struct {
    int alive;
    int blocks[BLOCK_H][BLOCK_W];
} Enemy;

static Enemy enemies[ENEMY_ROWS][ENEMY_COLS];
static Bullet pbullets[MAX_PBULLETS];
static Bullet ebullets[MAX_EBULLETS];

static int form_x, form_y, form_dir = 1;
static int player_x;
static const int player_y = FIELD_H - 1;
static int lives = 3;
static int score = 0;
static int enemies_left;

static char grid[FIELD_H][FIELD_W];
static char screenbuf[32768];

static void enable_ansi(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static void init_game(void) {
    int r, c, br, bc;
    form_x = (FIELD_W - FORM_W) / 2;
    form_y = 1;
    form_dir = 1;
    player_x = FIELD_W / 2;
    lives = 3;
    score = 0;
    enemies_left = ENEMY_ROWS * ENEMY_COLS;

    for (r = 0; r < ENEMY_ROWS; r++)
        for (c = 0; c < ENEMY_COLS; c++) {
            enemies[r][c].alive = 1;
            for (br = 0; br < BLOCK_H; br++)
                for (bc = 0; bc < BLOCK_W; bc++)
                    enemies[r][c].blocks[br][bc] = 1;
        }

    for (r = 0; r < MAX_PBULLETS; r++) pbullets[r].active = 0;
    for (r = 0; r < MAX_EBULLETS; r++) ebullets[r].active = 0;
}

static void spawn_pbullet(int x, int y) {
    int i;
    for (i = 0; i < MAX_PBULLETS; i++)
        if (!pbullets[i].active) {
            pbullets[i].active = 1;
            pbullets[i].x = x;
            pbullets[i].y = y;
            return;
        }
}

static void spawn_ebullet(int x, int y) {
    int i;
    for (i = 0; i < MAX_EBULLETS; i++)
        if (!ebullets[i].active) {
            ebullets[i].active = 1;
            ebullets[i].x = x;
            ebullets[i].y = y;
            return;
        }
}

static void handle_input(int *shoot_req) {
    while (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 224) {
            int c2 = _getch();
            if (c2 == 75) player_x--;      /* left arrow */
            else if (c2 == 77) player_x++; /* right arrow */
        } else if (c == 'a' || c == 'A') player_x--;
        else if (c == 'd' || c == 'D') player_x++;
        else if (c == ' ') *shoot_req = 1;
        else if (c == 'q' || c == 'Q') { *shoot_req = -1; }
    }
    if (player_x < 0) player_x = 0;
    if (player_x > FIELD_W - 1) player_x = FIELD_W - 1;
}

/* returns 1 if enemy formation reached the player's row */
static int update_formation(void) {
    int would_x = form_x + form_dir;
    if (would_x < 0 || would_x + FORM_W > FIELD_W) {
        form_dir = -form_dir;
        form_y++;
    } else {
        form_x = would_x;
    }
    return (form_y + FORM_H >= player_y);
}

static void try_hit_enemy(Bullet *b) {
    int r, c, br, bc;
    for (r = 0; r < ENEMY_ROWS; r++)
        for (c = 0; c < ENEMY_COLS; c++) {
            if (!enemies[r][c].alive) continue;
            int ex = form_x + c * (BLOCK_W + GAP_X);
            int ey = form_y + r * (BLOCK_H + GAP_Y);
            if (b->x < ex || b->x >= ex + BLOCK_W) continue;
            if (b->y < ey || b->y >= ey + BLOCK_H) continue;
            bc = b->x - ex;
            br = b->y - ey;
            if (enemies[r][c].blocks[br][bc]) {
                enemies[r][c].blocks[br][bc] = 0;
                b->active = 0;
                score += 10;
                int any = 0, i, j;
                for (i = 0; i < BLOCK_H && !any; i++)
                    for (j = 0; j < BLOCK_W; j++)
                        if (enemies[r][c].blocks[i][j]) { any = 1; break; }
                if (!any) {
                    enemies[r][c].alive = 0;
                    enemies_left--;
                    score += 50;
                }
            }
            return;
        }
}

static void try_enemy_shoot(void) {
    int alive_idx[ENEMY_ROWS * ENEMY_COLS];
    int n = 0, r, c, i;
    for (r = 0; r < ENEMY_ROWS; r++)
        for (c = 0; c < ENEMY_COLS; c++)
            if (enemies[r][c].alive) alive_idx[n++] = r * ENEMY_COLS + c;
    if (n == 0) return;

    for (i = 0; i < 4; i++) {
        if (rand() % 100 >= 2) continue;
        int idx = alive_idx[rand() % n];
        int r2 = idx / ENEMY_COLS, c2 = idx % ENEMY_COLS;
        int ex = form_x + c2 * (BLOCK_W + GAP_X);
        int ey = form_y + r2 * (BLOCK_H + GAP_Y);
        spawn_ebullet(ex + BLOCK_W / 2, ey + BLOCK_H);
    }
}

static void render(int tick) {
    int y, x;
    int len = 0;

    for (y = 0; y < FIELD_H; y++)
        for (x = 0; x < FIELD_W; x++)
            grid[y][x] = ' ';

    {
        int r, c, br, bc;
        for (r = 0; r < ENEMY_ROWS; r++)
            for (c = 0; c < ENEMY_COLS; c++) {
                if (!enemies[r][c].alive) continue;
                int ex = form_x + c * (BLOCK_W + GAP_X);
                int ey = form_y + r * (BLOCK_H + GAP_Y);
                for (br = 0; br < BLOCK_H; br++)
                    for (bc = 0; bc < BLOCK_W; bc++)
                        if (enemies[r][c].blocks[br][bc])
                            grid[ey + br][ex + bc] = '#';
            }
    }

    for (x = 0; x < MAX_PBULLETS; x++)
        if (pbullets[x].active && pbullets[x].y >= 0 && pbullets[x].y < FIELD_H)
            grid[pbullets[x].y][pbullets[x].x] = '|';

    for (x = 0; x < MAX_EBULLETS; x++)
        if (ebullets[x].active && ebullets[x].y >= 0 && ebullets[x].y < FIELD_H)
            grid[ebullets[x].y][ebullets[x].x] = ':';

    grid[player_y][player_x] = '^';

    len += sprintf(screenbuf + len, "\x1b[H");
    len += sprintf(screenbuf + len, "\x1b[1;1H\x1b[K Score: %d   Lives: %d", score, lives);

    len += sprintf(screenbuf + len, "\x1b[2;1H+");
    for (x = 0; x < FIELD_W; x++) screenbuf[len++] = '-';
    screenbuf[len++] = '+';
    screenbuf[len] = '\0';

    for (y = 0; y < FIELD_H; y++) {
        len += sprintf(screenbuf + len, "\x1b[%d;1H|", y + 3);
        for (x = 0; x < FIELD_W; x++) screenbuf[len++] = grid[y][x];
        screenbuf[len++] = '|';
        screenbuf[len] = '\0';
    }

    len += sprintf(screenbuf + len, "\x1b[%d;1H+", FIELD_H + 3);
    for (x = 0; x < FIELD_W; x++) screenbuf[len++] = '-';
    screenbuf[len++] = '+';
    screenbuf[len] = '\0';

    fwrite(screenbuf, 1, len, stdout);
    fflush(stdout);
}

static void show_message(const char *msg) {
    printf("\x1b[%d;1H\x1b[K%*s", FIELD_H + 5, (int)(strlen(msg) + 10), msg);
    fflush(stdout);
}

int main(void) {
    enable_ansi();
    system("mode con: cols=56 lines=27 > nul 2>&1");
    srand((unsigned)time(NULL));

    printf("\x1b[?25l\x1b[2J");
    init_game();

    int tick = 0;
    int shoot_cooldown = 0;
    int running = 1;
    int win = 0;

    while (running) {
        int shoot_req = 0;
        handle_input(&shoot_req);
        if (shoot_req == -1) { running = 0; break; }
        if (shoot_req == 1 && shoot_cooldown == 0) {
            spawn_pbullet(player_x, player_y - 1);
            shoot_cooldown = 4;
        }
        if (shoot_cooldown > 0) shoot_cooldown--;

        int i;
        for (i = 0; i < MAX_PBULLETS; i++) {
            if (!pbullets[i].active) continue;
            pbullets[i].y--;
            if (pbullets[i].y < 0) { pbullets[i].active = 0; continue; }
            try_hit_enemy(&pbullets[i]);
        }

        if (tick % 2 == 0) {
            for (i = 0; i < MAX_EBULLETS; i++) {
                if (!ebullets[i].active) continue;
                ebullets[i].y++;
                if (ebullets[i].y >= FIELD_H) { ebullets[i].active = 0; continue; }
                if (ebullets[i].y == player_y && ebullets[i].x == player_x) {
                    ebullets[i].active = 0;
                    lives--;
                    if (lives <= 0) { running = 0; }
                }
            }
        }

        try_enemy_shoot();

        if (tick % 8 == 0) {
            if (update_formation()) { running = 0; }
        }

        if (enemies_left <= 0) { running = 0; win = 1; }

        render(tick);
        tick++;
        Sleep(50);
    }

    if (win) show_message("*** YOU WIN! ***");
    else if (lives <= 0) show_message("*** GAME OVER ***");
    else show_message("*** QUIT ***");

    printf("\x1b[%d;1H\x1b[?25h\n", FIELD_H + 7);
    return 0;
}


