/*
 * PixelWall Pixel-Grid Animations
 *
 * Inspired by https://github.com/MusIF-MIAI/PixelWall
 */

#include "pixelwall-demo.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * Color constants (Color, pw_pos_t, pw_grid_t, pw_design_t from header)
 * ---------------------------------------------------------------------- */

/* Convert RGBA Color to libIUI ARGB uint32_t */
#define COLOR_TO_IUI_ARGB(c)                               \
    (((uint32_t) (c).a << 24) | ((uint32_t) (c).r << 16) | \
     ((uint32_t) (c).g << 8) | (uint32_t) (c).b)

/* Well-known colors (no Raylib dependency) */
static const Color PW_BLACK = {0, 0, 0, 255};
static const Color PW_WHITE = {255, 255, 255, 255};
static const Color PW_GREEN = {76, 175, 80, 255};    /* Material green 500 */
static const Color PW_RED = {229, 57, 53, 255};      /* Material red 600  */
static const Color PW_CYAN = {0, 188, 212, 255};     /* Material cyan 500 */
static const Color PW_YELLOW = {255, 235, 59, 255};  /* Material yellow 500 */
static const Color PW_MAGENTA = {156, 39, 176, 255}; /* Material purple 500 */

/* -------------------------------------------------------------------------
 * Grid helpers
 * ---------------------------------------------------------------------- */

static void pw_grid_fill_color(pw_grid_t *g, Color c)
{
    for (int r = 0; r < g->rows; r++)
        for (int col = 0; col < g->cols; col++)
            g->color[r][col] = c;
}

static void pw_grid_fill_data(pw_grid_t *g, uintptr_t d)
{
    for (int r = 0; r < g->rows; r++)
        for (int col = 0; col < g->cols; col++)
            g->data[r][col] = d;
}


static void pw_grid_set_color(pw_grid_t *g, pw_pos_t pos, Color c)
{
    if (pos.x < 0 || pos.x >= g->cols || pos.y < 0 || pos.y >= g->rows)
        return;
    g->color[pos.y][pos.x] = c;
}

static uintptr_t pw_grid_get_data(const pw_grid_t *g, pw_pos_t pos)
{
    if (pos.x < 0 || pos.x >= g->cols || pos.y < 0 || pos.y >= g->rows)
        return 0;
    return g->data[pos.y][pos.x];
}

static void pw_grid_set_data(pw_grid_t *g, pw_pos_t pos, uintptr_t d)
{
    if (pos.x < 0 || pos.x >= g->cols || pos.y < 0 || pos.y >= g->rows)
        return;
    g->data[pos.y][pos.x] = d;
}

/* -------------------------------------------------------------------------
 * Helper: random position inside grid
 * ---------------------------------------------------------------------- */

static pw_pos_t pw_random_pos(int rows, int cols)
{
    return (pw_pos_t) {rand() % cols, rand() % rows};
}

static pw_pos_t pw_random_dir(void)
{
    static const pw_pos_t dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    return dirs[rand() % 4];
}

/* =========================================================================
 * Design 0: Snake
 * ======================================================================= */

#define SNAKE_MAX_LEN 100
#define SNAKE_INIT_LEN 5

typedef struct {
    pw_pos_t position;
    Color color;
} SnakeSegment;

typedef struct {
    SnakeSegment *body;
    int length;
    int max_length;
} SnakeWorm;

typedef struct {
    SnakeWorm worm;
    pw_pos_t fruit_pos;
    Color fruit_color;
    pw_pos_t dir;
    bool game_over;
} SnakeState;

#define CELL_WORM (1u << 0)
#define CELL_FRUIT (1u << 1)

static bool snake_is_worm(const pw_grid_t *g, pw_pos_t p)
{
    return !!(pw_grid_get_data(g, p) & CELL_WORM);
}

static bool snake_is_fruit(const pw_grid_t *g, pw_pos_t p)
{
    return !!(pw_grid_get_data(g, p) & CELL_FRUIT);
}

static void snake_set_worm(pw_grid_t *g, pw_pos_t p, bool v)
{
    uintptr_t d = pw_grid_get_data(g, p);
    pw_grid_set_data(g, p, (d & ~(uintptr_t) CELL_WORM) | (v ? CELL_WORM : 0));
}

static void snake_set_fruit(pw_grid_t *g, pw_pos_t p, bool v)
{
    uintptr_t d = pw_grid_get_data(g, p);
    pw_grid_set_data(g, p,
                     (d & ~(uintptr_t) CELL_FRUIT) | (v ? CELL_FRUIT : 0));
}

static void snake_place_fruit(pw_grid_t *g, SnakeState *st)
{
    pw_pos_t p;
    int tries = 0;
    do {
        p = pw_random_pos(g->rows, g->cols);
        tries++;
    } while (snake_is_worm(g, p) && tries < 200);
    st->fruit_pos = p;
    st->fruit_color = PW_RED;
    pw_grid_set_color(g, p, st->fruit_color);
    snake_set_fruit(g, p, true);
}

static void snake_init_worm(pw_grid_t *g, SnakeState *st)
{
    st->worm.max_length = SNAKE_MAX_LEN;
    st->worm.body = malloc(SNAKE_MAX_LEN * sizeof(SnakeSegment));
    if (!st->worm.body)
        return;
    st->worm.length = SNAKE_INIT_LEN;
    int sr = g->rows / 2;
    int sc = g->cols / 2;
    for (int i = 0; i < SNAKE_INIT_LEN; i++) {
        pw_pos_t p = {sc - i, sr};
        st->worm.body[i].position = p;
        st->worm.body[i].color = PW_GREEN;
        pw_grid_set_color(g, p, PW_GREEN);
        snake_set_worm(g, p, true);
    }
}

static bool snake_safe(const pw_grid_t *g, pw_pos_t p)
{
    return p.x >= 0 && p.x < g->cols && p.y >= 0 && p.y < g->rows &&
           !snake_is_worm(g, p);
}

static void snake_ai(pw_grid_t *g, SnakeState *st)
{
    pw_pos_t head = st->worm.body[0].position;
    pw_pos_t fp = st->fruit_pos;
    pw_pos_t desired = {0, 0};

    if (fp.x != head.x) {
        desired.x = (fp.x > head.x) ? 1 : -1;
        desired.y = 0;
    } else if (fp.y != head.y) {
        desired.x = 0;
        desired.y = (fp.y > head.y) ? 1 : -1;
    }

    pw_pos_t next = {head.x + desired.x, head.y + desired.y};
    if (snake_safe(g, next)) {
        st->dir = desired;
        return;
    }

    static const pw_pos_t dirs4[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int i = 0; i < 4; i++) {
        pw_pos_t np = {head.x + dirs4[i].x, head.y + dirs4[i].y};
        if (snake_safe(g, np)) {
            st->dir = dirs4[i];
            return;
        }
    }
}

static void *snake_create(pw_grid_t *g)
{
    SnakeState *st = malloc(sizeof(SnakeState));
    if (!st)
        return NULL;
    memset(st, 0, sizeof(*st));
    snake_init_worm(g, st);
    if (!st->worm.body) {
        free(st);
        return NULL;
    }
    snake_place_fruit(g, st);
    st->dir = pw_random_dir();
    st->game_over = false;
    g->moveInterval = 0.12f;
    return st;
}

static void snake_update(pw_grid_t *g, void *data)
{
    SnakeState *st = (SnakeState *) data;

    if (st->game_over) {
        /* reset: free old body first to avoid leak */
        free(st->worm.body);
        st->worm.body = NULL;
        pw_grid_fill_color(g, g->bg_color);
        pw_grid_fill_data(g, 0);
        snake_init_worm(g, st);
        if (!st->worm.body)
            return;
        snake_place_fruit(g, st);
        st->dir = pw_random_dir();
        st->game_over = false;
        return;
    }

    snake_ai(g, st);

    pw_pos_t head = st->worm.body[0].position;
    pw_pos_t new_head = {head.x + st->dir.x, head.y + st->dir.y};

    if (!snake_safe(g, new_head)) {
        st->game_over = true;
        return;
    }

    bool grow = snake_is_fruit(g, new_head);

    /* Always clear the tail cell: when growing at max length the tail shifts
     * but doesn't extend, so the old tail must still be erased. */
    pw_pos_t tail = st->worm.body[st->worm.length - 1].position;
    pw_grid_set_color(g, tail, g->bg_color);
    snake_set_worm(g, tail, false);

    if (grow) {
        /* Clear fruit marker; extend only if below max length */
        snake_set_fruit(g, new_head, false);
        if (st->worm.length < st->worm.max_length)
            st->worm.length++;
    }

    for (int i = st->worm.length - 1; i > 0; i--)
        st->worm.body[i] = st->worm.body[i - 1];
    st->worm.body[0].position = new_head;

    for (int i = 0; i < st->worm.length; i++) {
        pw_grid_set_color(g, st->worm.body[i].position, PW_GREEN);
        snake_set_worm(g, st->worm.body[i].position, true);
    }

    if (grow)
        snake_place_fruit(g, st);
}

static void snake_destroy(void *data)
{
    SnakeState *st = (SnakeState *) data;
    if (st) {
        free(st->worm.body);
        free(st);
    }
}

static pw_design_t pw_design_snake = {"Snake", snake_create, snake_update,
                                      snake_destroy};

/* =========================================================================
 * Design 1: Pong
 * ======================================================================= */

typedef struct {
    int rows, cols;
    pw_pos_t ball;
    pw_pos_t direction;
    int paddle1_y;
    int paddle2_y;
    int prev_paddle1_y;
    int prev_paddle2_y;
    int margin;
    int paddle_len;
} PongData;

static void *pong_create(pw_grid_t *g)
{
    PongData *pd = malloc(sizeof(PongData));
    if (!pd)
        return NULL;
    pd->margin = 1;
    pd->paddle_len = 1;
    pd->rows = g->rows - 2 * pd->margin;
    pd->cols = g->cols - 2 * pd->margin;
    pd->paddle1_y = pd->rows / 2;
    pd->paddle2_y = pd->rows / 2;
    pd->ball = (pw_pos_t) {pd->cols / 2, pd->rows / 2};
    pd->direction = (pw_pos_t) {1, 1};
    pd->prev_paddle1_y = pd->paddle1_y;
    pd->prev_paddle2_y = pd->paddle2_y;
    g->moveInterval = 0.05f;
    return pd;
}

static void pong_update(pw_grid_t *g, void *data)
{
    PongData *pd = (PongData *) data;

    /* Clear previous ball */
    pw_pos_t pb = {pd->margin + pd->ball.x, pd->margin + pd->ball.y};
    pw_grid_set_color(g, pb, g->bg_color);

    /* Clear previous paddles */
    for (int i = -pd->paddle_len; i <= pd->paddle_len; i++) {
        pw_pos_t p1 = {pd->margin, pd->margin + pd->prev_paddle1_y + i};
        pw_pos_t p2 = {pd->margin + pd->cols - 1,
                       pd->margin + pd->prev_paddle2_y + i};
        pw_grid_set_color(g, p1, g->bg_color);
        pw_grid_set_color(g, p2, g->bg_color);
    }

    /* Move ball */
    pd->ball.x += pd->direction.x;
    pd->ball.y += pd->direction.y;

    /* Wall bounce */
    if (pd->ball.y < 0 || pd->ball.y >= pd->rows) {
        pd->direction.y *= -1;
        pd->ball.y += pd->direction.y;
    }

    /* Paddle bounces */
    if (pd->ball.x == 1 && pd->ball.y >= pd->paddle1_y - pd->paddle_len &&
        pd->ball.y <= pd->paddle1_y + pd->paddle_len)
        pd->direction.x *= -1;
    if (pd->ball.x == pd->cols - 2 &&
        pd->ball.y >= pd->paddle2_y - pd->paddle_len &&
        pd->ball.y <= pd->paddle2_y + pd->paddle_len)
        pd->direction.x *= -1;

    /* Ball out of bounds: reset — capture exit side before recentering */
    if (pd->ball.x < 0 || pd->ball.x >= pd->cols) {
        int exited_left = (pd->ball.x < 0);
        pd->ball.x = pd->cols / 2;
        pd->ball.y = pd->rows / 2;
        pd->direction.x = exited_left ? 1 : -1;
    }

    /* Simple paddle AI */
    if (pd->ball.y > pd->paddle1_y)
        pd->paddle1_y++;
    if (pd->ball.y < pd->paddle1_y)
        pd->paddle1_y--;
    if (pd->ball.y > pd->paddle2_y)
        pd->paddle2_y++;
    if (pd->ball.y < pd->paddle2_y)
        pd->paddle2_y--;

    /* Clamp paddles */
    if (pd->paddle1_y < pd->paddle_len)
        pd->paddle1_y = pd->paddle_len;
    if (pd->paddle1_y >= pd->rows - pd->paddle_len)
        pd->paddle1_y = pd->rows - pd->paddle_len - 1;
    if (pd->paddle2_y < pd->paddle_len)
        pd->paddle2_y = pd->paddle_len;
    if (pd->paddle2_y >= pd->rows - pd->paddle_len)
        pd->paddle2_y = pd->rows - pd->paddle_len - 1;

    /* Draw paddles */
    for (int i = -pd->paddle_len; i <= pd->paddle_len; i++) {
        pw_pos_t p1 = {pd->margin, pd->margin + pd->paddle1_y + i};
        pw_pos_t p2 = {pd->margin + pd->cols - 1,
                       pd->margin + pd->paddle2_y + i};
        pw_grid_set_color(g, p1, PW_WHITE);
        pw_grid_set_color(g, p2, PW_WHITE);
    }

    /* Draw ball */
    pw_pos_t ball_pos = {pd->margin + pd->ball.x, pd->margin + pd->ball.y};
    pw_grid_set_color(g, ball_pos, PW_CYAN);

    pd->prev_paddle1_y = pd->paddle1_y;
    pd->prev_paddle2_y = pd->paddle2_y;
}

static void pong_destroy(void *data)
{
    free(data);
}

static pw_design_t pw_design_pong = {"Pong", pong_create, pong_update,
                                     pong_destroy};

/* =========================================================================
 * Design 3: Tetris
 * ======================================================================= */

#define FIELD_W 10
#define FIELD_H 16
#define FIELD_X 6
#define NUM_PIECES 7
#define DROP_TICKS 4

static const uint16_t tetris_pieces[NUM_PIECES][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, /* I */
    {0x6600, 0x6600, 0x6600, 0x6600}, /* O */
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, /* T */
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, /* S */
    {0x0C60, 0x4C80, 0xC600, 0x2640}, /* Z */
    {0x0E80, 0xC440, 0x2E00, 0x44C0}, /* L */
    {0x0E20, 0x44C0, 0x8E00, 0xC880}, /* J */
};

static const Color tetris_colors[NUM_PIECES] = {
    {85, 255, 255, 255}, /* I - Cyan */
    {255, 255, 85, 255}, /* O - Yellow */
    {255, 85, 255, 255}, /* T - Magenta */
    {85, 255, 85, 255},  /* S - Green */
    {255, 85, 85, 255},  /* Z - Red */
    {255, 170, 0, 255},  /* L - Orange */
    {85, 85, 255, 255},  /* J - Blue */
};

typedef struct {
    int field[FIELD_H][FIELD_W];
    int field_color[FIELD_H][FIELD_W];
    int piece, rotation;
    int piece_x, piece_y;
    int tick;
    int target_x, target_rot;
} TetrisData;

static bool tetris_piece_cell(int type, int rot, int r, int c)
{
    uint16_t bits = tetris_pieces[type][rot];
    int idx = r * 4 + c;
    return (bits >> (15 - idx)) & 1;
}

static bool tetris_collides(TetrisData *td, int type, int rot, int px, int py)
{
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!tetris_piece_cell(type, rot, r, c))
                continue;
            int fx = px + c;
            int fy = py + r;
            if (fx < 0 || fx >= FIELD_W || fy >= FIELD_H)
                return true;
            if (fy >= 0 && td->field[fy][fx])
                return true;
        }
    }
    return false;
}

static int tetris_field_height(TetrisData *td)
{
    int h = 0;
    for (int c = 0; c < FIELD_W; c++) {
        for (int r = 0; r < FIELD_H; r++) {
            if (td->field[r][c]) {
                int col_h = FIELD_H - r;
                if (col_h > h)
                    h = col_h;
                break;
            }
        }
    }
    return h;
}

static int tetris_count_holes(TetrisData *td)
{
    int holes = 0;
    for (int c = 0; c < FIELD_W; c++) {
        bool found = false;
        for (int r = 0; r < FIELD_H; r++) {
            if (td->field[r][c])
                found = true;
            else if (found)
                holes++;
        }
    }
    return holes;
}

static int tetris_count_complete(TetrisData *td)
{
    int n = 0;
    for (int r = 0; r < FIELD_H; r++) {
        bool full = true;
        for (int c = 0; c < FIELD_W; c++) {
            if (!td->field[r][c]) {
                full = false;
                break;
            }
        }
        if (full)
            n++;
    }
    return n;
}

static void tetris_ai_choose(TetrisData *td)
{
    int best_score = -100000;
    int best_x = td->piece_x;
    int best_rot = td->rotation;

    for (int rot = 0; rot < 4; rot++) {
        for (int x = -2; x < FIELD_W; x++) {
            if (tetris_collides(td, td->piece, rot, x, td->piece_y))
                continue;
            int y = td->piece_y;
            while (!tetris_collides(td, td->piece, rot, x, y + 1))
                y++;
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    if (tetris_piece_cell(td->piece, rot, r, c) && y + r >= 0 &&
                        y + r < FIELD_H && x + c >= 0 && x + c < FIELD_W)
                        td->field[y + r][x + c] = 1;
            int score = tetris_count_complete(td) * 100 -
                        tetris_field_height(td) * 10 -
                        tetris_count_holes(td) * 30;
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    if (tetris_piece_cell(td->piece, rot, r, c) && y + r >= 0 &&
                        y + r < FIELD_H && x + c >= 0 && x + c < FIELD_W)
                        td->field[y + r][x + c] = 0;
            if (score > best_score) {
                best_score = score;
                best_x = x;
                best_rot = rot;
            }
        }
    }
    td->target_x = best_x;
    td->target_rot = best_rot;
}

static void tetris_spawn(TetrisData *td)
{
    td->piece = rand() % NUM_PIECES;
    td->rotation = 0;
    td->piece_x = FIELD_W / 2 - 2;
    td->piece_y = -1;
    tetris_ai_choose(td);
}

static void tetris_clear_rows(TetrisData *td)
{
    for (int r = FIELD_H - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < FIELD_W; c++) {
            if (!td->field[r][c]) {
                full = false;
                break;
            }
        }
        if (full) {
            for (int rr = r; rr > 0; rr--) {
                for (int c = 0; c < FIELD_W; c++) {
                    td->field[rr][c] = td->field[rr - 1][c];
                    td->field_color[rr][c] = td->field_color[rr - 1][c];
                }
            }
            for (int c = 0; c < FIELD_W; c++) {
                td->field[0][c] = 0;
                td->field_color[0][c] = 0;
            }
            r++;
        }
    }
}

static void *tetris_create(pw_grid_t *g)
{
    (void) g;
    TetrisData *td = calloc(1, sizeof(TetrisData));
    if (!td)
        return NULL;
    tetris_spawn(td);
    g->moveInterval = 0.05f;
    return td;
}

static void tetris_update(pw_grid_t *g, void *data)
{
    TetrisData *td = (TetrisData *) data;

    td->tick++;
    if (td->tick < DROP_TICKS)
        return;
    td->tick = 0;

    /* Rotate toward target */
    if (td->rotation != td->target_rot) {
        int nr = (td->rotation + 1) % 4;
        if (!tetris_collides(td, td->piece, nr, td->piece_x, td->piece_y))
            td->rotation = nr;
    }
    /* Slide toward target x */
    if (td->piece_x < td->target_x) {
        if (!tetris_collides(td, td->piece, td->rotation, td->piece_x + 1,
                             td->piece_y))
            td->piece_x++;
    } else if (td->piece_x > td->target_x) {
        if (!tetris_collides(td, td->piece, td->rotation, td->piece_x - 1,
                             td->piece_y))
            td->piece_x--;
    }

    /* Drop */
    if (!tetris_collides(td, td->piece, td->rotation, td->piece_x,
                         td->piece_y + 1)) {
        td->piece_y++;
    } else {
        /* Lock */
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (!tetris_piece_cell(td->piece, td->rotation, r, c))
                    continue;
                int fy = td->piece_y + r;
                int fx = td->piece_x + c;
                if (fy >= 0 && fy < FIELD_H && fx >= 0 && fx < FIELD_W) {
                    td->field[fy][fx] = 1;
                    td->field_color[fy][fx] = td->piece;
                }
            }
        }
        tetris_clear_rows(td);
        tetris_spawn(td);
        if (tetris_collides(td, td->piece, td->rotation, td->piece_x,
                            td->piece_y)) {
            memset(td->field, 0, sizeof(td->field));
            memset(td->field_color, 0, sizeof(td->field_color));
            tetris_spawn(td);
        }
    }

    /* Draw */
    pw_grid_fill_color(g, g->bg_color);

    /* Locked cells */
    for (int r = 0; r < FIELD_H; r++) {
        for (int c = 0; c < FIELD_W; c++) {
            if (td->field[r][c]) {
                Color col = tetris_colors[td->field_color[r][c]];
                pw_pos_t p = {FIELD_X + c, r};
                pw_grid_set_color(g, p, col);
            }
        }
    }

    /* Current piece */
    Color pc = tetris_colors[td->piece];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!tetris_piece_cell(td->piece, td->rotation, r, c))
                continue;
            int gy = td->piece_y + r;
            int gx = td->piece_x + c;
            if (gy >= 0 && gy < FIELD_H && gx >= 0 && gx < FIELD_W) {
                pw_pos_t p = {FIELD_X + gx, gy};
                pw_grid_set_color(g, p, pc);
            }
        }
    }

    /* Borders */
    Color border = {170, 170, 170, 255};
    for (int r = 0; r < FIELD_H; r++) {
        pw_grid_set_color(g, (pw_pos_t) {FIELD_X - 1, r}, border);
        pw_grid_set_color(g, (pw_pos_t) {FIELD_X + FIELD_W, r}, border);
    }
}

static void tetris_destroy(void *data)
{
    free(data);
}

static pw_design_t pw_design_tetris = {"Tetris", tetris_create, tetris_update,
                                       tetris_destroy};

/* =========================================================================
 * Design 4: Space Invaders
 * ======================================================================= */

#define INV_ENEMY_ROWS 3
#define INV_ENEMY_COLS 5
#define INV_ENEMY_SPACING 3
#define INV_PLAYER_ROW 14
#define INV_PLAYER_HALF 1
#define INV_MOVE_TICKS 6

typedef struct {
    bool enemies[INV_ENEMY_ROWS][INV_ENEMY_COLS];
    int enemy_x, enemy_y;
    int enemy_dir;
    int player_x;
    int bullet_x, bullet_y;
    int ebullet_x, ebullet_y;
    int tick;
    int enemies_alive;
} InvadersData;

static void invaders_reset_wave(InvadersData *id)
{
    id->enemies_alive = INV_ENEMY_ROWS * INV_ENEMY_COLS;
    for (int r = 0; r < INV_ENEMY_ROWS; r++)
        for (int c = 0; c < INV_ENEMY_COLS; c++)
            id->enemies[r][c] = true;
    id->enemy_x = 2;
    id->enemy_y = 1;
    id->enemy_dir = 1;
    id->bullet_y = -1;
    id->ebullet_y = -1;
}

static int invaders_enemy_grid_col(int c)
{
    return c * INV_ENEMY_SPACING;
}

static int invaders_nearest_col(InvadersData *id)
{
    int best = -1, best_dist = 100;
    for (int c = 0; c < INV_ENEMY_COLS; c++) {
        for (int r = INV_ENEMY_ROWS - 1; r >= 0; r--) {
            if (!id->enemies[r][c])
                continue;
            int ex = id->enemy_x + invaders_enemy_grid_col(c);
            int dist = abs(ex - id->player_x);
            if (dist < best_dist) {
                best_dist = dist;
                best = ex;
            }
            break;
        }
    }
    return best;
}

static void *invaders_create(pw_grid_t *g)
{
    (void) g;
    InvadersData *id = calloc(1, sizeof(InvadersData));
    if (!id)
        return NULL;
    id->player_x = 11;
    invaders_reset_wave(id);
    g->moveInterval = 0.05f;
    return id;
}

static void invaders_update(pw_grid_t *g, void *data)
{
    InvadersData *id = (InvadersData *) data;

    id->tick++;
    if (id->tick < INV_MOVE_TICKS)
        return;
    id->tick = 0;

    /* Move enemies */
    int leftmost = 100, rightmost = -1;
    for (int c = 0; c < INV_ENEMY_COLS; c++) {
        for (int r = 0; r < INV_ENEMY_ROWS; r++) {
            if (!id->enemies[r][c])
                continue;
            int ex = id->enemy_x + invaders_enemy_grid_col(c);
            if (ex < leftmost)
                leftmost = ex;
            if (ex > rightmost)
                rightmost = ex;
            break;
        }
    }
    if (rightmost >= 0) {
        int nl = leftmost + id->enemy_dir;
        int nr = rightmost + id->enemy_dir;
        if (nl < 0 || nr >= g->cols) {
            id->enemy_dir *= -1;
            id->enemy_y++;
        } else {
            id->enemy_x += id->enemy_dir;
        }
    }

    /* Enemies reach bottom -> reset */
    int bottom = -1;
    for (int r = INV_ENEMY_ROWS - 1; r >= 0; r--) {
        for (int c = 0; c < INV_ENEMY_COLS; c++) {
            if (id->enemies[r][c]) {
                int ey = id->enemy_y + r;
                if (ey > bottom)
                    bottom = ey;
            }
        }
    }
    if (bottom >= INV_PLAYER_ROW - 1)
        invaders_reset_wave(id);

    /* Move player bullet */
    if (id->bullet_y >= 0) {
        id->bullet_y--;
        if (id->bullet_y >= 0) {
            for (int r = 0; r < INV_ENEMY_ROWS; r++) {
                for (int c = 0; c < INV_ENEMY_COLS; c++) {
                    if (!id->enemies[r][c])
                        continue;
                    int ex = id->enemy_x + invaders_enemy_grid_col(c);
                    int ey = id->enemy_y + r;
                    if (id->bullet_x == ex && id->bullet_y == ey) {
                        id->enemies[r][c] = false;
                        id->enemies_alive--;
                        id->bullet_y = -1;
                        goto bullet_done;
                    }
                }
            }
        }
    }
bullet_done:;

    /* Move enemy bullet */
    if (id->ebullet_y >= 0) {
        id->ebullet_y++;
        if (id->ebullet_y >= g->rows) {
            id->ebullet_y = -1;
        } else if (id->ebullet_y == INV_PLAYER_ROW &&
                   id->ebullet_x >= id->player_x - INV_PLAYER_HALF &&
                   id->ebullet_x <= id->player_x + INV_PLAYER_HALF) {
            id->ebullet_y = -1;
        }
    }

    /* Enemy fires */
    if (id->ebullet_y < 0 && id->enemies_alive > 0 && (rand() % 3) == 0) {
        int tries = 10;
        while (tries-- > 0) {
            int c = rand() % INV_ENEMY_COLS;
            for (int r = INV_ENEMY_ROWS - 1; r >= 0; r--) {
                if (id->enemies[r][c]) {
                    id->ebullet_x = id->enemy_x + invaders_enemy_grid_col(c);
                    id->ebullet_y = id->enemy_y + r + 1;
                    goto fire_done;
                }
            }
        }
    }
fire_done:;

    /* AI player */
    int target = invaders_nearest_col(id);
    if (target >= 0) {
        if (id->player_x < target)
            id->player_x++;
        else if (id->player_x > target)
            id->player_x--;
    }
    if (id->player_x < INV_PLAYER_HALF)
        id->player_x = INV_PLAYER_HALF;
    if (id->player_x > g->cols - 1 - INV_PLAYER_HALF)
        id->player_x = g->cols - 1 - INV_PLAYER_HALF;

    /* Player fires */
    if (id->bullet_y < 0 && (rand() % 2) == 0) {
        id->bullet_x = id->player_x;
        id->bullet_y = INV_PLAYER_ROW - 1;
    }

    /* All dead -> respawn */
    if (id->enemies_alive <= 0)
        invaders_reset_wave(id);

    /* Draw */
    pw_grid_fill_color(g, g->bg_color);

    for (int r = 0; r < INV_ENEMY_ROWS; r++) {
        for (int c = 0; c < INV_ENEMY_COLS; c++) {
            if (!id->enemies[r][c])
                continue;
            int ex = id->enemy_x + invaders_enemy_grid_col(c);
            int ey = id->enemy_y + r;
            if (ex >= 0 && ex < g->cols && ey >= 0 && ey < g->rows)
                pw_grid_set_color(g, (pw_pos_t) {ex, ey}, PW_RED);
        }
    }

    for (int i = -INV_PLAYER_HALF; i <= INV_PLAYER_HALF; i++) {
        int px = id->player_x + i;
        if (px >= 0 && px < g->cols)
            pw_grid_set_color(g, (pw_pos_t) {px, INV_PLAYER_ROW}, PW_GREEN);
    }

    if (id->bullet_y >= 0 && id->bullet_y < g->rows)
        pw_grid_set_color(g, (pw_pos_t) {id->bullet_x, id->bullet_y},
                          PW_YELLOW);

    if (id->ebullet_y >= 0 && id->ebullet_y < g->rows)
        pw_grid_set_color(g, (pw_pos_t) {id->ebullet_x, id->ebullet_y},
                          PW_MAGENTA);
}

static void invaders_destroy(void *data)
{
    free(data);
}

static pw_design_t pw_design_invaders = {"Invaders", invaders_create,
                                         invaders_update, invaders_destroy};

/* =========================================================================
 * Design 5: Pacman March
 * ======================================================================= */

#define PM_SPRITE_W 7
#define PM_SPRITE_H 7
#define PM_NUM_FRAMES 2
#define PM_NUM_GHOSTS 2
#define PM_SPACING 9
#define PM_SPEED 4
#define PM_ANIM_SPEED 1

static const uint8_t pm_pacman_left[PM_NUM_FRAMES][PM_SPRITE_H] = {
    {0x1C, 0x3E, 0x1F, 0x07, 0x1F, 0x3E, 0x1C},
    {0x1C, 0x3E, 0x3F, 0x7F, 0x3F, 0x3E, 0x1C},
};
static const uint8_t pm_pacman_right[PM_NUM_FRAMES][PM_SPRITE_H] = {
    {0x1C, 0x3E, 0x7C, 0x70, 0x7C, 0x3E, 0x1C},
    {0x1C, 0x3E, 0x7E, 0x7F, 0x7E, 0x3E, 0x1C},
};
static const uint8_t pm_ghost_normal[PM_NUM_FRAMES][PM_SPRITE_H] = {
    {0x1C, 0x3E, 0x6B, 0x7F, 0x7F, 0x7F, 0x55},
    {0x1C, 0x3E, 0x6B, 0x7F, 0x7F, 0x7F, 0x2A},
};
static const uint8_t pm_ghost_scared[PM_NUM_FRAMES][PM_SPRITE_H] = {
    {0x1C, 0x3E, 0x55, 0x7F, 0x6B, 0x7F, 0x55},
    {0x1C, 0x3E, 0x55, 0x7F, 0x6B, 0x7F, 0x2A},
};

typedef struct {
    int x;
    int phase;
    int tick;
    int anim_tick;
    int frame;
    int cols;
} PacmanMarchData;

static const Color pm_pacman_color = {255, 235, 59, 255}; /* yellow */
static const Color pm_ghost_colors[PM_NUM_GHOSTS] = {
    {229, 57, 53, 255},  /* red */
    {156, 39, 176, 255}, /* purple */
};
static const Color pm_scared_color = {33, 150, 243, 255}; /* blue */

static void pm_start_phase(PacmanMarchData *pd, int phase)
{
    pd->phase = phase;
    int total = 2 * PM_SPACING + PM_SPRITE_W;
    pd->x = (phase == 0) ? pd->cols : -total;
}

static void pm_draw_sprite(pw_grid_t *g,
                           const uint8_t *sprite,
                           int sx,
                           int start_row,
                           Color color)
{
    for (int c = 0; c < PM_SPRITE_W; c++) {
        int gx = sx + c;
        if (gx < 0 || gx >= g->cols)
            continue;
        for (int r = 0; r < PM_SPRITE_H; r++) {
            int gy = start_row + r;
            if (gy < 0 || gy >= g->rows)
                continue;
            if ((sprite[r] >> (PM_SPRITE_W - 1 - c)) & 1)
                pw_grid_set_color(g, (pw_pos_t) {gx, gy}, color);
        }
    }
}

static void *pacman_march_create(pw_grid_t *g)
{
    PacmanMarchData *pd = calloc(1, sizeof(PacmanMarchData));
    if (!pd)
        return NULL;
    pd->cols = g->cols;
    pm_start_phase(pd, 0);
    g->moveInterval = 0.05f;
    return pd;
}

static void pacman_march_update(pw_grid_t *g, void *data)
{
    PacmanMarchData *pd = (PacmanMarchData *) data;

    pw_grid_fill_color(g, g->bg_color);

    int start_row = (g->rows - PM_SPRITE_H) / 2;
    int f = pd->frame;

    if (pd->phase == 0) {
        /* Ghosts lead, Pacman chases from behind (right side). */
        for (int i = 0; i < PM_NUM_GHOSTS; i++) {
            pm_draw_sprite(g, pm_ghost_normal[f], pd->x + i * PM_SPACING,
                           start_row, pm_ghost_colors[i]);
        }
        pm_draw_sprite(g, pm_pacman_left[f], pd->x + PM_NUM_GHOSTS * PM_SPACING,
                       start_row, pm_pacman_color);
    } else {
        for (int i = 0; i < PM_NUM_GHOSTS; i++) {
            pm_draw_sprite(g, pm_ghost_scared[f], pd->x + i * PM_SPACING,
                           start_row, pm_scared_color);
        }
        pm_draw_sprite(g, pm_pacman_right[f],
                       pd->x + PM_NUM_GHOSTS * PM_SPACING, start_row,
                       pm_pacman_color);
    }

    /* Scroll */
    pd->tick++;
    if (pd->tick >= PM_SPEED) {
        pd->tick = 0;
        int total = 2 * PM_SPACING + PM_SPRITE_W;
        if (pd->phase == 0) {
            pd->x--;
            if (pd->x + total < 0)
                pm_start_phase(pd, 1);
        } else {
            pd->x++;
            if (pd->x > pd->cols)
                pm_start_phase(pd, 0);
        }
    }

    /* Animate */
    pd->anim_tick++;
    if (pd->anim_tick >= PM_ANIM_SPEED) {
        pd->anim_tick = 0;
        pd->frame = (pd->frame + 1) % PM_NUM_FRAMES;
    }
}

static void pacman_march_destroy(void *data)
{
    free(data);
}

static pw_design_t pw_design_pacman_march = {
    "Pacman", pacman_march_create, pacman_march_update, pacman_march_destroy};

/* =========================================================================
 * Design table & pixelwall state
 * ======================================================================= */

static pw_design_t *pw_designs[] = {
    &pw_design_snake,    &pw_design_pong,         &pw_design_tetris,
    &pw_design_invaders, &pw_design_pacman_march,
};
#define PW_DESIGN_COUNT ((int) (sizeof(pw_designs) / sizeof(pw_designs[0])))

/* =========================================================================
 * Rendering
 * ======================================================================= */

static void pw_render_grid(iui_context *ui,
                           iui_port_ctx *port,
                           const pw_grid_t *grid,
                           iui_rect_t area)
{
    (void) ui;
    iui_renderer_t cb = g_iui_port.get_renderer_callbacks(port);

    /* Background */
    cb.draw_box(area, 0, COLOR_TO_IUI_ARGB(grid->bg_color), cb.user);

    float cw = area.width / (float) grid->cols;
    float ch = area.height / (float) grid->rows;

    for (int row = 0; row < grid->rows; row++) {
        for (int col = 0; col < grid->cols; col++) {
            Color c = grid->color[row][col];
            /* Skip background-colored cells */
            if (c.r == grid->bg_color.r && c.g == grid->bg_color.g &&
                c.b == grid->bg_color.b)
                continue;
            iui_rect_t r = {area.x + col * cw, area.y + row * ch, cw - 1.f,
                            ch - 1.f};
            cb.draw_box(r, 0, COLOR_TO_IUI_ARGB(c), cb.user);
        }
    }
}

/* =========================================================================
 * Public API
 * ======================================================================= */

static void pw_switch_design(pixelwall_state_t *pw, int idx)
{
    if (pw->design && pw->design_data)
        pw->design->Destroy(pw->design_data);
    pw->design = pw_designs[idx];
    pw_grid_fill_color(&pw->grid, pw->grid.bg_color);
    pw_grid_fill_data(&pw->grid, 0);
    pw->timer = 0.f;
    pw->design_data = pw->design->Create(&pw->grid);
}

void pixelwall_init(pixelwall_state_t *pw)
{
    memset(pw, 0, sizeof(*pw));
    srand((unsigned) time(NULL)); /* seed once for all designs */
    pw->grid.rows = 16;
    pw->grid.cols = 22;
    pw->grid.bg_color = PW_BLACK;
    pw_grid_fill_color(&pw->grid, pw->grid.bg_color);
    pw_grid_fill_data(&pw->grid, 0);
    pw->design = NULL;
    pw->design_data = NULL;
    pw->timer = 0.f;
    pw->tab = 0;
    pw_switch_design(pw, 0);
}

void draw_pixelwall_window(iui_context *ui,
                           iui_port_ctx *port,
                           pixelwall_state_t *pw,
                           float dt,
                           float win_h)
{
    /* 380 = DEMO_WIN_X (10dp right of control panel at x=30+340),
     * 30  = DEMO_WIN_Y — aligns top edge with the control panel.
     * 420 = 800 (SDL window width) - 380 (fills to the right edge). */
    if (!iui_begin_window(ui, "PixelWall", 380, 30, 420, win_h,
                          IUI_WINDOW_RESIZABLE))
        return;

    static const char *labels[] = {
        "Snake", "Pong", "Tetris", "Invaders", "Pacman",
    };
    uint32_t tab_u = (uint32_t) pw->tab;
    iui_segmented(ui, labels, PW_DESIGN_COUNT, &tab_u);
    int new_tab = (int) tab_u;
    if (new_tab != pw->tab) {
        pw_switch_design(pw, new_tab);
        pw->tab = new_tab;
    }

    /* Tick animation */
    if (pw->design_data) {
        pw->timer += dt;
        if (pw->timer >= pw->grid.moveInterval) {
            pw->timer -= pw->grid.moveInterval;
            pw->design->UpdateFrame(&pw->grid, pw->design_data);
        }
    }

    /* Grid render area — fill all remaining vertical space after the tab bar */
    float avail_h = iui_get_remaining_height(ui);
    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = avail_h});
    iui_rect_t area = iui_box_next(ui);
    pw_render_grid(ui, port, &pw->grid, area);
    iui_box_end(ui);

    iui_end_window(ui);
}
