/*
 * tests/pixelwall-demo.h - PixelWall pixel-grid demo for libIUI
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../include/iui.h"
#include "../ports/port.h"

/* Grid limits */
#define PW_MAX_ROWS 32
#define PW_MAX_COLS 64

/*
 * RGBA color — no Raylib dependency.
 * Intentionally named 'Color' to match PixelWall source conventions and
 * to avoid type-mismatch errors when the .c file includes this header.
 */
typedef struct {
    unsigned char r, g, b, a;
} Color;

/* 2D integer position */
typedef struct {
    int x, y;
} pw_pos_t;

/* Pixel grid — static storage, no heap */
typedef struct {
    Color color[PW_MAX_ROWS][PW_MAX_COLS];
    uintptr_t data[PW_MAX_ROWS][PW_MAX_COLS];
    int rows, cols;
    Color bg_color;
    float moveInterval;
} pw_grid_t;

/* Design vtable */
typedef struct pw_design_t {
    const char *name;
    void *(*Create)(pw_grid_t *grid);
    void (*UpdateFrame)(pw_grid_t *grid, void *data);
    void (*Destroy)(void *data);
} pw_design_t;

/* Per-window state — embed this in demo_state_t */
typedef struct {
    pw_grid_t grid;
    pw_design_t *design;
    void *design_data;
    float timer;
    int tab; /* active tab index — persists across window open/close */
} pixelwall_state_t;

/* Initialise and start design 0 (Snake). Call once before the first frame. */
void pixelwall_init(pixelwall_state_t *pw);

/* Draw the PixelWall window. win_h should match get_demo_window_height(). */
void draw_pixelwall_window(iui_context *ui,
                           iui_port_ctx *port,
                           pixelwall_state_t *pw,
                           float dt,
                           float win_h);
