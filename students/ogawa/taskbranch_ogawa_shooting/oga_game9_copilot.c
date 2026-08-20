/*
 ShootingGameSample by T.Ogawa(2025/03/17)
   第2版: 2025/03/19
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define LINE 15
#define COL 26
#define ENEMY_COUNT 4
#define EXPLOSION_MAX 12

typedef struct {
  int line;
  int col;
  int move_up;
  int move_down;
  int move_right;
  int move_left;
  char symbol;
  int life;
} Player;

typedef struct {
  int line;
  int col;
  int direction;
  int hit;
  char symbol;
} Enemy;

typedef struct {
  int shoot;
  int line;
  int col;
  char symbol;
} Missile;

typedef struct {
  int shoot;
  int line;
  int col;
  char symbol;
} EnemyBullet;

typedef struct {
  int active;
  int line;
  int col;
  int timer;
  char symbol;
} Explosion;

char gamen[LINE + 1][COL + 3];
char gamen_template[LINE + 1][COL + 3];

Player player = { COL / 2 + 1, LINE - 1, 0, 0, 0, 0, 'M', 3 };
Enemy enemy[ENEMY_COUNT];
Missile missile = { 0, 0, 0, '|' };
EnemyBullet enemy_bullet[ENEMY_COUNT];
Explosion explosion[EXPLOSION_MAX];

int flg_end = 0;
int flg_shoot = 0;
int hit_count = 0;
int difficulty = 1;
int enemy_move_interval = 2;

int main(void);
void set_difficulty(void);
void init_gamen(void);
void clear_gamen(void);
void print_gamen(void);
void* game_main(void* p);
void init_enemy(void);
void move_enemy(void);
void move_enemy2(int i);
void move_player(void);
void move_missile(void);
void fire_enemy_bullet(void);
void move_enemy_bullet(void);
void spawn_explosion(int line, int col);
void update_explosion(void);
void render_scene(void);
void judge_hit(void);
void* key_hit(void* p);
void print_special(char* line, char special);

int main(void) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  set_difficulty();
  init_enemy();
  init_gamen();

  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, game_main, NULL);
  pthread_create(&thread2, NULL, key_hit, NULL);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("\e[?25h");
  return 0;
}

void set_difficulty(void) {
  char select;

  printf("Select Difficulty\n");
  printf("e: Easy\n");
  printf("n: Normal\n");
  printf("h: Hard\n");
  printf("Choose > ");
  scanf(" %c", &select);

  switch (select) {
    case 'e':
    case 'E':
      difficulty = 0;
      enemy_move_interval = 4;
      break;
    case 'h':
    case 'H':
      difficulty = 2;
      enemy_move_interval = 1;
      break;
    case 'n':
    case 'N':
    default:
      difficulty = 1;
      enemy_move_interval = 2;
      break;
  }

  printf("Difficulty: %s\n\n",
         (difficulty == 0) ? "Easy" : (difficulty == 2) ? "Hard" : "Normal");
  usleep(800000);
}

void init_gamen(void) {
  system("cls");
  printf("\e[?25l");
  memset(gamen, ' ', sizeof(gamen));
  for (int i = 0; i < LINE + 1; i++) {
    gamen[i][0] = '|';
    gamen[i][COL + 1] = '|';
    gamen[i][COL + 2] = '\0';
  }
  for (int i = 1; i < COL + 1; i++) {
    gamen[LINE][i] = '-';
  }
  gamen[LINE][0] = '+';
  gamen[LINE][COL + 1] = '+';
  memcpy(gamen_template, gamen, sizeof(gamen));
}

void clear_gamen(void) {
  memcpy(gamen, gamen_template, sizeof(gamen));
}

void print_gamen(void) {
  printf("\e[1;1H");
  for (int i = 0; i < LINE + 1; i++) {
    print_special(gamen[i], '$');
  }
  printf("\n上:w 下:s 左:a 右:d 発射:space\n\n");
  printf("Life: %d  Hit(Total): %d(%d)\n", player.life, hit_count, ENEMY_COUNT);
}

void* game_main(void* p) {
  static int enemy_tick = 0;

  while (flg_end == 0) {
    usleep(100000);
    clear_gamen();

    if (++enemy_tick >= enemy_move_interval) {
      move_enemy();
      enemy_tick = 0;
    }

    fire_enemy_bullet();
    move_enemy_bullet();
    move_player();
    move_missile();
    judge_hit();
    update_explosion();
    render_scene();
    print_gamen();
  }

  system("cls");
  if (flg_end == 1) {
    printf("\n*** YOU WIN! ***\n");
  } else if (flg_end == 2) {
    printf("\n*** YOU LOSE! ***\n");
  }
  return NULL;
}

void init_enemy(void) {
  srand((unsigned)time(NULL));
  for (int i = 0; i < EXPLOSION_MAX; i++) {
    explosion[i].active = 0;
  }
  for (int i = 0; i < ENEMY_COUNT; i++) {
    enemy[i].col = rand() % COL + 1;
    enemy[i].line = rand() % ENEMY_COUNT;
    enemy[i].direction = rand() % 4 + 1;
    enemy[i].hit = 0;
    enemy_bullet[i].shoot = 0;
    if ((rand() % 10) == 0) {
      enemy[i].symbol = '$';
    } else {
      enemy[i].symbol = '@';
    }
  }
}

void move_enemy(void) {
  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy[i].hit == 0) {
      move_enemy2(i);
    }
  }
}

void move_enemy2(int i) {
  if (enemy[i].direction == 1) {
    if (enemy[i].line <= 1) {
      enemy[i].direction = 2;
    } else {
      enemy[i].line--;
    }
  } else if (enemy[i].direction == 2) {
    if (enemy[i].line >= LINE - 2) {
      enemy[i].direction = 1;
    } else {
      enemy[i].line++;
    }
  } else if (enemy[i].direction == 3) {
    if (enemy[i].col <= 1) {
      enemy[i].direction = 4;
    } else {
      enemy[i].col--;
    }
  } else if (enemy[i].direction == 4) {
    if (enemy[i].col >= COL) {
      enemy[i].direction = 3;
    } else {
      enemy[i].col++;
    }
  }

  if (enemy[i].line >= LINE - 1) {
    flg_end = 2;
  }
}

void move_player(void) {
  if (player.move_up == 1) {
    if (player.line > 1) player.line--;
    player.move_up = 0;
  }
  if (player.move_down == 1) {
    if (player.line < LINE - 1) player.line++;
    player.move_down = 0;
  }
  if (player.move_left == 1) {
    if (player.col > 1) player.col--;
    player.move_left = 0;
  }
  if (player.move_right == 1) {
    if (player.col < COL) player.col++;
    player.move_right = 0;
  }
}

void move_missile(void) {
  if (flg_shoot == 1 && missile.shoot == 0) {
    missile.shoot = 1;
    missile.col = player.col;
    missile.line = player.line - 1;
    flg_shoot = 0;
  } else if (missile.shoot == 1) {
    if (--missile.line <= 0) {
      missile.shoot = 0;
      missile.line = 0;
      missile.col = 0;
    }
  }
}

void fire_enemy_bullet(void) {
  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy[i].hit == 0 && enemy_bullet[i].shoot == 0 && (rand() % 18 == 0)) {
      enemy_bullet[i].shoot = 1;
      enemy_bullet[i].line = enemy[i].line + 1;
      enemy_bullet[i].col = enemy[i].col;
      enemy_bullet[i].symbol = 'v';
    }
  }
}

void move_enemy_bullet(void) {
  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy_bullet[i].shoot == 1) {
      if (enemy_bullet[i].line >= LINE - 1) {
        enemy_bullet[i].shoot = 0;
      } else {
        enemy_bullet[i].line++;
      }
    }
  }
}

void spawn_explosion(int line, int col) {
  for (int i = 0; i < EXPLOSION_MAX; i++) {
    if (explosion[i].active == 0) {
      explosion[i].active = 1;
      explosion[i].line = line;
      explosion[i].col = col;
      explosion[i].timer = 2;
      explosion[i].symbol = '*';
      return;
    }
  }
}

void update_explosion(void) {
  for (int i = 0; i < EXPLOSION_MAX; i++) {
    if (explosion[i].active == 1) {
      explosion[i].timer--;
      if (explosion[i].timer <= 0) {
        explosion[i].active = 0;
      }
    }
  }
}

void render_scene(void) {
  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy[i].hit == 0) {
      gamen[enemy[i].line][enemy[i].col] = enemy[i].symbol;
    }
  }

  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy_bullet[i].shoot == 1) {
      gamen[enemy_bullet[i].line][enemy_bullet[i].col] = enemy_bullet[i].symbol;
    }
  }

  if (missile.shoot == 1) {
    gamen[missile.line][missile.col] = missile.symbol;
  }

  gamen[player.line][player.col] = player.symbol;

  for (int i = 0; i < EXPLOSION_MAX; i++) {
    if (explosion[i].active == 1) {
      gamen[explosion[i].line][explosion[i].col] = explosion[i].symbol;
    }
  }
}

void judge_hit(void) {
  for (int i = 0; i < ENEMY_COUNT; i++) {
    if (enemy[i].hit == 0 && missile.shoot == 1 && missile.line >= 1 &&
        enemy[i].col == missile.col && enemy[i].line == missile.line) {
      missile.shoot = 0;
      missile.line = 0;
      missile.col = 0;
      enemy[i].hit = 1;
      spawn_explosion(enemy[i].line, enemy[i].col);
      hit_count++;
      if (hit_count >= ENEMY_COUNT || enemy[i].symbol == '$') {
        flg_end = 1;
      }
    }

    if (enemy[i].hit == 0 && enemy[i].col == player.col && enemy[i].line == player.line) {
      enemy[i].hit = 1;
      spawn_explosion(player.line, player.col);
      player.life--;
      if (player.life <= 0) {
        flg_end = 2;
      }
    }

    if (enemy_bullet[i].shoot == 1 && enemy_bullet[i].col == player.col &&
        enemy_bullet[i].line == player.line) {
      enemy_bullet[i].shoot = 0;
      spawn_explosion(player.line, player.col);
      player.life--;
      if (player.life <= 0) {
        flg_end = 2;
      }
    }
  }
}

void* key_hit(void* p) {
  while (flg_end == 0) {
    if (kbhit()) {
      while (kbhit()) {
        char key = getch();
        switch (key) {
          case 'w':
          case 'W':
            player.move_up = 1;
            break;
          case 's':
          case 'S':
            player.move_down = 1;
            break;
          case 'a':
          case 'A':
            player.move_left = 1;
            break;
          case 'd':
          case 'D':
            player.move_right = 1;
            break;
          case ' ':
            flg_shoot = 1;
            break;
        }
      }
    }
    usleep(100000);
  }
  return NULL;
}

void print_special(char* line, char special) {
  char* idx = strchr(line, special);
  while (idx != NULL) {
    int idx2 = idx - line;
    char buf[strlen(line)];
    strcpy(buf, line);
    buf[idx2] = '\0';
    printf("%s", buf);
    printf("\e[31m");
    printf("\e[31m%c\e[39m", line[idx2]);
    line += idx2 + 1;
    idx = strchr(line, special);
  }
  printf("%s\n", line);
}
