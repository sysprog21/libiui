/*
 * tests/example.c - Comprehensive Platform-Agnostic Example
 *
 * This example uses the port abstraction layer (ports/port.h) for platform
 * independence while demonstrating all visual features.
 *
 * Build: make example
 * Run: ./libiui_example
 *
 * Features demonstrated:
 *   - Calculator with keyboard support
 *   - Analog clock with smooth second hand
 *   - Vector primitives (gauges, charts, progress rings)
 *   - Nyan Cat animation (RLE-compressed frames)
 *   - MD3 color scheme display
 *   - UI components (buttons, chips, sliders, progress)
 *   - Motion system (easing curves)
 *   - Accessibility (WCAG contrast, screen reader hints)
 *   - TextField variants
 *   - Menus, dialogs, date/time pickers, snackbar
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/iui.h"
#include "../ports/port.h"
#include "../src/font.h"
#include "../src/internal.h"
#include "../src/iui_config.h"

/* Platform-specific includes */
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <sys/time.h>

#ifdef CONFIG_DEMO_NYANCAT
/* Include the generated nyancat data */
#include "nyancat-data.h"
#endif

#ifdef CONFIG_DEMO_PIXELWALL
#include "pixelwall-demo.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef CONFIG_DEMO_CLOCK
/* Clock colors */
#define CLOCK_BACKGROUND 0xff3b80ae
#define CLOCK_HOUR 0xffdedede
#define CLOCK_MINUTE 0xffffffff
#define CLOCK_SECOND 0xfff5a97f
#define CLOCK_TICK 0xffbababa
#define CLOCK_NUMBER 0xffdedede
#define CLOCK_BORDER 0xffbababa
#endif /* CONFIG_DEMO_CLOCK */

#ifdef CONFIG_DEMO_CALCULATOR
/* Calculator layout constants */
#define CALC_MARGIN 6
#define CALC_BTN_W 46
#define CALC_BTN_H 40
#define CALC_BTN_PAD 6
#define CALC_COLS 4
#define CALC_ROWS 5
#define CALC_DISPLAY_H 32
#define CALC_TITLE_H 38
#define CALC_SEP_H 16
#define CALC_GRID_W \
    ((CALC_BTN_W * CALC_COLS) + (CALC_BTN_PAD * (CALC_COLS - 1)))
#define CALC_GRID_H \
    ((CALC_BTN_H * CALC_ROWS) + (CALC_BTN_PAD * (CALC_ROWS - 1)))
#define CALC_WIN_W (CALC_GRID_W + CALC_MARGIN * 2 + 20)
#define CALC_WIN_H                                              \
    (CALC_TITLE_H + CALC_DISPLAY_H + CALC_SEP_H + CALC_GRID_H + \
     CALC_MARGIN * 2 + 12)

/* Calculator state */
typedef struct {
    char display[64];
    size_t cursor;
    double accumulator, operand; /* calculation values */
    char operation;
    bool new_input;
} calculator_state_t;
#endif /* CONFIG_DEMO_CALCULATOR */

#ifdef CONFIG_DEMO_NYANCAT
/* Nyancat animation state */
#define NYANCAT_PALETTE_SIZE 14
#define NYANCAT_FRAME_WIDTH 64
#define NYANCAT_FRAME_HEIGHT 64
#define NYANCAT_TOTAL_FRAMES 12
#define NYANCAT_PIXEL_COUNT (NYANCAT_FRAME_WIDTH * NYANCAT_FRAME_HEIGHT)

static const uint32_t nyancat_palette_colors[14] = {
    0xFF010157, 0xFFFFFFFF, 0xFF000000, 0xFFE0B482, 0xFFFF69B4,
    0xFFFC0FC0, 0xFFFF0000, 0xFFFFA500, 0xFFFFFF00, 0xFF00C800,
    0xFF0B86F3, 0xFF9370DB, 0xFFAAAAAA, 0xFFFFDEAD,
};

static int nyancat_current_frame = 0;
static float nyancat_frame_time = 0.0f;
static const float NYANCAT_FRAME_INTERVAL = 0.08f;
static uint8_t nyancat_all_frames[NYANCAT_TOTAL_FRAMES][4096];
static bool nyancat_frames_initialized = false;
#endif /* CONFIG_DEMO_NYANCAT */

#ifdef CONFIG_DEMO_CALCULATOR
/* Forward declarations */
static void calc_init(calculator_state_t *calc);
static void calc_input_digit(calculator_state_t *calc, char digit);
static void calc_input_decimal(calculator_state_t *calc);
static void calc_negate(calculator_state_t *calc);
static void calc_clear(calculator_state_t *calc);
static void calc_clear_all(calculator_state_t *calc);
static void calc_operation(calculator_state_t *calc, char op);
static void calc_equals(calculator_state_t *calc);
#endif /* CONFIG_DEMO_CALCULATOR */

#ifdef CONFIG_DEMO_CALCULATOR
/* Calculator Implementation */

static void calc_init(calculator_state_t *calc)
{
    memset(calc, 0, sizeof(*calc));
    calc->display[0] = '0';
    calc->display[1] = '\0';
    calc->cursor = 1;
    calc->new_input = true;
}

static void calc_input_digit(calculator_state_t *calc, char digit)
{
    if (calc->new_input || !strcmp(calc->display, "0")) {
        calc->display[0] = digit;
        calc->display[1] = '\0';
        calc->cursor = 1;
        calc->new_input = false;
        return;
    }
    size_t len = strlen(calc->display);
    if (len < sizeof(calc->display) - 1) {
        calc->display[len] = digit;
        calc->display[len + 1] = '\0';
        calc->cursor = len + 1;
    }
}

static void calc_input_decimal(calculator_state_t *calc)
{
    if (!strchr(calc->display, '.')) {
        size_t len = strlen(calc->display);
        if (len < sizeof(calc->display) - 1) {
            calc->display[len] = '.';
            calc->display[len + 1] = '\0';
            calc->cursor = len + 1;
            calc->new_input = false;
        }
    }
}

static void calc_negate(calculator_state_t *calc)
{
    if (calc->display[0] == '-') {
        memmove(calc->display, calc->display + 1, strlen(calc->display));
    } else {
        size_t len = strlen(calc->display);
        if (len + 1 < sizeof(calc->display)) {
            memmove(calc->display + 1, calc->display, len + 1);
            calc->display[0] = '-';
        }
    }
}

static void calc_clear(calculator_state_t *calc)
{
    calc->display[0] = '0';
    calc->display[1] = '\0';
    calc->cursor = 1;
    calc->new_input = true;
}

static void calc_clear_all(calculator_state_t *calc)
{
    calc_init(calc);
}

static void calc_operation(calculator_state_t *calc, char op)
{
    calc->accumulator = atof(calc->display);
    calc->operation = op;
    calc->new_input = true;
}

static void calc_equals(calculator_state_t *calc)
{
    calc->operand = atof(calc->display);
    double result = calc->accumulator;
    switch (calc->operation) {
    case '+':
        result += calc->operand;
        break;
    case '-':
        result -= calc->operand;
        break;
    case '*':
        result *= calc->operand;
        break;
    case '/':
        result = (calc->operand != 0.0) ? result / calc->operand : 0.0;
        break;
    default:
        result = calc->operand;
        break;
    }
    snprintf(calc->display, sizeof(calc->display), "%.10g", result);
    calc->cursor = strlen(calc->display);
    calc->new_input = true;
}

/* Standard demo window position: 10dp gap right of libIUI Demo (x=30+340=370).
 * y=30 keeps all windows top-aligned with the control panel. */
#define DEMO_WIN_X 380
#define DEMO_WIN_Y 30

/* Forward declaration so all draw functions can call get_demo_window_height()
 * without knowing the full implementation, which lives after the constants. */
static float get_demo_window_height(void);

static void draw_calculator_window(iui_context *ui, calculator_state_t *calc)
{
    iui_begin_window(ui, "Calculator", DEMO_WIN_X, DEMO_WIN_Y, CALC_WIN_W,
                     get_demo_window_height(), 0);

    iui_text(ui, IUI_ALIGN_RIGHT, "%s", calc->display);
    iui_newline(ui);
    iui_divider(ui);

    iui_grid_begin(ui, CALC_COLS, CALC_BTN_W, CALC_BTN_H, CALC_BTN_PAD);

    if (iui_button(ui, "C", IUI_ALIGN_CENTER))
        calc_clear(calc);
    iui_grid_next(ui);
    if (iui_button(ui, "+/-", IUI_ALIGN_CENTER))
        calc_negate(calc);
    iui_grid_next(ui);
    if (iui_button(ui, "AC", IUI_ALIGN_CENTER))
        calc_clear_all(calc);
    iui_grid_next(ui);
    if (iui_button(ui, "/", IUI_ALIGN_CENTER))
        calc_operation(calc, '/');
    iui_grid_next(ui);

    if (iui_button(ui, "7", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '7');
    iui_grid_next(ui);
    if (iui_button(ui, "8", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '8');
    iui_grid_next(ui);
    if (iui_button(ui, "9", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '9');
    iui_grid_next(ui);
    if (iui_button(ui, "*", IUI_ALIGN_CENTER))
        calc_operation(calc, '*');
    iui_grid_next(ui);

    if (iui_button(ui, "4", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '4');
    iui_grid_next(ui);
    if (iui_button(ui, "5", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '5');
    iui_grid_next(ui);
    if (iui_button(ui, "6", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '6');
    iui_grid_next(ui);
    if (iui_button(ui, "-", IUI_ALIGN_CENTER))
        calc_operation(calc, '-');
    iui_grid_next(ui);

    if (iui_button(ui, "1", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '1');
    iui_grid_next(ui);
    if (iui_button(ui, "2", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '2');
    iui_grid_next(ui);
    if (iui_button(ui, "3", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '3');
    iui_grid_next(ui);
    if (iui_button(ui, "+", IUI_ALIGN_CENTER))
        calc_operation(calc, '+');
    iui_grid_next(ui);

    if (iui_button(ui, "0", IUI_ALIGN_CENTER))
        calc_input_digit(calc, '0');
    iui_grid_next(ui);
    if (iui_button(ui, "00", IUI_ALIGN_CENTER)) {
        calc_input_digit(calc, '0');
        calc_input_digit(calc, '0');
    }
    iui_grid_next(ui);
    if (iui_button(ui, ".", IUI_ALIGN_CENTER))
        calc_input_decimal(calc);
    iui_grid_next(ui);
    if (iui_button(ui, "=", IUI_ALIGN_CENTER))
        calc_equals(calc);

    iui_grid_end(ui);
    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_CALCULATOR */

#ifdef CONFIG_DEMO_CLOCK
/* Analog Clock Implementation */

static void draw_clock_window(iui_context *ui, iui_port_ctx *port)
{
    iui_begin_window(ui, "Analog Clock", DEMO_WIN_X, DEMO_WIN_Y, 320,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);
    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = 200.f});
    iui_rect_t rect = iui_box_next(ui);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm now_tm;
    localtime_r(&tv.tv_sec, &now_tm);

    float size = fminf(rect.width, rect.height), radius = size * 0.45f,
          cx = rect.x + rect.width * 0.5f, cy = rect.y + rect.height * 0.5f;

    /* Draw clock face using libiui primitives */
    iui_draw_circle(ui, cx, cy, radius + 4.f, CLOCK_BORDER, 0, 0);
    iui_draw_circle(ui, cx, cy, radius, CLOCK_BACKGROUND, 0, 0);

    /* Tick marks */
    for (int i = 0; i < 60; ++i) {
        float a = ((float) M_PI * 2.f * (float) i) / 60.f - (float) M_PI / 2.f;
        float inner = (i % 5 == 0) ? radius * 0.72f : radius * 0.82f,
              outer = radius * 0.92f, x0 = cx + cosf(a) * inner,
              y0 = cy + sinf(a) * inner, x1 = cx + cosf(a) * outer,
              y1 = cy + sinf(a) * outer;
        iui_draw_line(ui, x0, y0, x1, y1, 1.f,
                      (i % 5 == 0) ? CLOCK_NUMBER : CLOCK_TICK);
    }

    float seconds = (float) now_tm.tm_sec + (float) tv.tv_usec / 1000000.f;
    float minutes = (float) now_tm.tm_min + seconds / 60.f;
    float hours = (float) (now_tm.tm_hour % 12) + minutes / 60.f;

    float second_angle = seconds * (float) M_PI / 30.f - (float) M_PI / 2.f;
    float minute_angle = minutes * (float) M_PI / 30.f - (float) M_PI / 2.f;
    float hour_angle = hours * (float) M_PI / 6.f - (float) M_PI / 2.f;

    /* Hour hand */
    float hx = cx + cosf(hour_angle) * radius * 0.55f;
    float hy = cy + sinf(hour_angle) * radius * 0.55f;
    iui_draw_line(ui, cx, cy, hx, hy, 6.f, CLOCK_HOUR);

    /* Minute hand */
    float mx = cx + cosf(minute_angle) * radius * 0.75f;
    float my = cy + sinf(minute_angle) * radius * 0.75f;
    iui_draw_line(ui, cx, cy, mx, my, 4.f, CLOCK_MINUTE);

    /* Second hand */
    float sx = cx + cosf(second_angle) * radius * 0.85f;
    float sy = cy + sinf(second_angle) * radius * 0.85f;
    iui_draw_line(ui, cx, cy, sx, sy, 2.f, CLOCK_SECOND);

    /* Center pivot */
    iui_draw_circle(ui, cx, cy, 6.f, CLOCK_BORDER, 0, 0);
    iui_draw_circle(ui, cx, cy, 4.f, CLOCK_SECOND, 0, 0);

    iui_box_end(ui);
    iui_end_window(ui);
    (void) port;
}
#endif /* CONFIG_DEMO_CLOCK */

#ifdef CONFIG_DEMO_VECTOR
/* Vector Primitives Demo */

static void draw_vector_demo_window(iui_context *ui, float progress_value)
{
    iui_begin_window(ui, "Vector Primitives", DEMO_WIN_X, DEMO_WIN_Y, 390,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    const uint32_t COLOR_RED = 0xFFE53935;
    const uint32_t COLOR_ORANGE = 0xFFFB8C00;
    const uint32_t COLOR_YELLOW = 0xFFFFEB3B;
    const uint32_t COLOR_GREEN = 0xFF43A047;
    const uint32_t COLOR_CYAN = 0xFF00BCD4;
    const uint32_t COLOR_BLUE = 0xFF1E88E5;
    const uint32_t COLOR_PURPLE = 0xFF8E24AA;
    const uint32_t COLOR_PINK = 0xFFE91E63;
    const uint32_t COLOR_GRAY = 0xFF455A64;
    const uint32_t COLOR_WHITE = 0xFFFFFFFF;

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = 220.f});
    iui_rect_t area = iui_box_next(ui);

    /* Multi-ring progress indicator */
    float cx1 = area.x + 55.f;
    float cy1 = area.y + 55.f;

    iui_draw_arc(ui, cx1, cy1, 45.f, 0, (float) M_PI * 2.f, 5.f, COLOR_GRAY);
    iui_draw_arc(ui, cx1, cy1, 45.f, -(float) M_PI / 2.f,
                 -(float) M_PI / 2.f + progress_value * (float) M_PI * 2.f, 5.f,
                 COLOR_CYAN);

    float mid_progress = fmodf(progress_value + 0.33f, 1.f);
    iui_draw_arc(ui, cx1, cy1, 35.f, 0, (float) M_PI * 2.f, 5.f, COLOR_GRAY);
    iui_draw_arc(ui, cx1, cy1, 35.f, -(float) M_PI / 2.f,
                 -(float) M_PI / 2.f + mid_progress * (float) M_PI * 2.f, 5.f,
                 COLOR_PINK);

    float inner_progress = fmodf(progress_value + 0.66f, 1.f);
    iui_draw_arc(ui, cx1, cy1, 25.f, 0, (float) M_PI * 2.f, 5.f, COLOR_GRAY);
    iui_draw_arc(ui, cx1, cy1, 25.f, -(float) M_PI / 2.f,
                 -(float) M_PI / 2.f + inner_progress * (float) M_PI * 2.f, 5.f,
                 COLOR_YELLOW);
    iui_draw_circle(ui, cx1, cy1, 8.f, COLOR_WHITE, 0, 0);

    /* Speedometer gauge */
    float cx2 = area.x + 180.f;
    float cy2 = area.y + 70.f;
    float radius2 = 55.f;

    iui_draw_arc(ui, cx2, cy2, radius2, (float) M_PI, (float) M_PI * 0.7f, 10.f,
                 COLOR_GREEN);
    iui_draw_arc(ui, cx2, cy2, radius2, (float) M_PI * 0.7f,
                 (float) M_PI * 0.4f, 10.f, COLOR_YELLOW);
    iui_draw_arc(ui, cx2, cy2, radius2, (float) M_PI * 0.4f,
                 (float) M_PI * 0.15f, 10.f, COLOR_ORANGE);
    iui_draw_arc(ui, cx2, cy2, radius2, (float) M_PI * 0.15f, 0, 10.f,
                 COLOR_RED);

    float needle_angle = (float) M_PI - progress_value * (float) M_PI;
    float needle_x = cx2 + cosf(needle_angle) * (radius2 - 18.f);
    float needle_y = cy2 + sinf(needle_angle) * (radius2 - 18.f);
    iui_draw_line(ui, cx2, cy2, needle_x, needle_y, 3.f, COLOR_WHITE);
    iui_draw_circle(ui, cx2, cy2, 8.f, COLOR_BLUE, COLOR_WHITE, 2.f);

    for (int i = 0; i <= 10; i++) {
        float tick_angle = (float) M_PI - (float) i * (float) M_PI / 10.f;
        float tick_inner = radius2 - 25.f;
        float tick_outer = radius2 - 15.f;
        float tx0 = cx2 + cosf(tick_angle) * tick_inner;
        float ty0 = cy2 + sinf(tick_angle) * tick_inner;
        float tx1 = cx2 + cosf(tick_angle) * tick_outer;
        float ty1 = cy2 + sinf(tick_angle) * tick_outer;
        iui_draw_line(ui, tx0, ty0, tx1, ty1, 2.f, COLOR_WHITE);
    }

    /* Concentric circles with rotating dots */
    float cx3 = area.x + 310.f;
    float cy3 = area.y + 55.f;
    uint32_t ring_colors[] = {COLOR_PURPLE, COLOR_BLUE, COLOR_CYAN};

    for (int ring = 0; ring < 3; ring++) {
        float r = 20.f + ring * 15.f;
        iui_draw_circle(ui, cx3, cy3, r, 0, ring_colors[ring], 2.f);
        float dot_angle =
            progress_value * (float) M_PI * 2.f * (ring % 2 ? -1.f : 1.f) +
            ring * (float) M_PI / 3.f;
        float dot_x = cx3 + cosf(dot_angle) * r;
        float dot_y = cy3 + sinf(dot_angle) * r;
        iui_draw_circle(ui, dot_x, dot_y, 5.f, ring_colors[ring], 0, 0);
    }

    /* Multi-line chart */
    float chart_x = area.x + 10.f;
    float chart_y = area.y + 125.f;
    float chart_w = 100.f;
    float chart_h = 70.f;

    for (int i = 0; i <= 4; i++) {
        float gy = chart_y + (float) i * chart_h / 4.f;
        iui_draw_line(ui, chart_x, gy, chart_x + chart_w, gy, 1.f, COLOR_GRAY);
    }
    iui_draw_line(ui, chart_x, chart_y, chart_x, chart_y + chart_h, 2.f,
                  COLOR_WHITE);
    iui_draw_line(ui, chart_x, chart_y + chart_h, chart_x + chart_w,
                  chart_y + chart_h, 2.f, COLOR_WHITE);

    float prev_x = chart_x;
    float prev_y = chart_y + chart_h / 2.f;
    for (int i = 1; i <= 10; i++) {
        float t = (float) i / 10.f;
        float x = chart_x + t * chart_w;
        float y =
            chart_y + chart_h / 2.f -
            sinf((t + progress_value) * (float) M_PI * 2.f) * chart_h * 0.35f;
        iui_draw_line(ui, prev_x, prev_y, x, y, 2.f, COLOR_CYAN);
        prev_x = x;
        prev_y = y;
    }

    prev_x = chart_x;
    prev_y = chart_y + chart_h / 2.f;
    for (int i = 1; i <= 10; i++) {
        float t = (float) i / 10.f;
        float x = chart_x + t * chart_w;
        float y =
            chart_y + chart_h / 2.f -
            cosf((t + progress_value) * (float) M_PI * 2.f) * chart_h * 0.35f;
        iui_draw_line(ui, prev_x, prev_y, x, y, 2.f, COLOR_PINK);
        prev_x = x;
        prev_y = y;
    }

    /* Starburst */
    float cx5 = area.x + 180.f;
    float cy5 = area.y + 160.f;
    uint32_t star_colors[] = {COLOR_RED,    COLOR_ORANGE, COLOR_YELLOW,
                              COLOR_GREEN,  COLOR_CYAN,   COLOR_BLUE,
                              COLOR_PURPLE, COLOR_PINK};
    for (int i = 0; i < 16; i++) {
        float angle =
            (float) i * (float) M_PI / 8.f + progress_value * (float) M_PI;
        float len = 25.f + 15.f * sinf(progress_value * (float) M_PI * 4.f +
                                       (float) i * 0.5f);
        float ex = cx5 + cosf(angle) * len;
        float ey = cy5 + sinf(angle) * len;
        iui_draw_line(ui, cx5, cy5, ex, ey, 2.f, star_colors[i % 8]);
    }
    iui_draw_circle(ui, cx5, cy5, 8.f, COLOR_WHITE, 0, 0);

    /* Bezier curve with dots */
    float bx = area.x + 250.f, by = area.y + 125.f;
    float bw = 100.f, bh = 70.f;

    prev_x = bx;
    prev_y = by + bh;
    uint32_t colors[] = {COLOR_PURPLE, COLOR_BLUE, COLOR_CYAN, COLOR_GREEN};
    for (int i = 1; i <= 12; i++) {
        float t = (float) i / 12.f;
        float x = bx + t * bw;
        float ctrl_y =
            by - 20.f + sinf(progress_value * (float) M_PI * 2.f) * 30.f;
        float y = (1.f - t) * (1.f - t) * (by + bh) +
                  2.f * (1.f - t) * t * ctrl_y + t * t * (by + bh * 0.3f);
        iui_draw_line(ui, prev_x, prev_y, x, y, 3.f, colors[i % 4]);
        iui_draw_circle(ui, x, y, 4.f, colors[i % 4], 0, 0);
        prev_x = x;
        prev_y = y;
    }

    iui_box_end(ui);
    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_VECTOR */

#ifdef CONFIG_DEMO_NYANCAT
/* Nyancat Animation */

static void decompress_baseline_frame(const uint8_t *opcodes,
                                      const uint8_t *data_end,
                                      uint8_t *output)
{
    int output_pos = 0, current_color = 0;
    const uint8_t *p = opcodes;

    for (int j = 0; j < NYANCAT_PIXEL_COUNT; j++)
        output[j] = 0;

    while (output_pos < NYANCAT_PIXEL_COUNT && p < data_end) {
        uint8_t opcode = *p++;
        if (opcode == 0xFF) {
            break;
        } else if ((opcode & 0xF0) == 0x00) {
            current_color = opcode & 0x0F;
        } else if ((opcode & 0xF0) == 0x20) {
            int count = (opcode & 0x0F) + 1;
            for (int j = 0; j < count && output_pos < NYANCAT_PIXEL_COUNT; j++)
                output[output_pos++] = current_color;
        } else if ((opcode & 0xF0) == 0x30) {
            int count = ((opcode & 0x0F) + 1) * 16;
            for (int j = 0; j < count && output_pos < NYANCAT_PIXEL_COUNT; j++)
                output[output_pos++] = current_color;
        } else {
            break;
        }
    }
}

static void decompress_delta_frame(const uint8_t *prev_frame,
                                   const uint8_t *opcodes,
                                   const uint8_t *data_end,
                                   uint8_t *output)
{
    for (int i = 0; i < NYANCAT_PIXEL_COUNT; i++)
        output[i] = prev_frame[i];

    int pos = 0, current_color = 0;
    const uint8_t *p = opcodes;

    while (pos < NYANCAT_PIXEL_COUNT && p < data_end) {
        uint8_t opcode = *p++;
        if (opcode == 0xFF) {
            break;
        } else if ((opcode & 0xF0) == 0x00) {
            current_color = opcode & 0x0F;
        } else if ((opcode & 0xF0) == 0x10) {
            pos += (opcode & 0x0F) + 1;
        } else if ((opcode & 0xF0) == 0x20) {
            int count = (opcode & 0x0F) + 1;
            for (int j = 0; j < count && pos < NYANCAT_PIXEL_COUNT; j++)
                output[pos++] = current_color;
        } else if ((opcode & 0xF0) == 0x30) {
            pos += ((opcode & 0x0F) + 1) * 16;
        } else if ((opcode & 0xF0) == 0x40) {
            int count = ((opcode & 0x0F) + 1) * 16;
            for (int j = 0; j < count && pos < NYANCAT_PIXEL_COUNT; j++)
                output[pos++] = current_color;
        } else if ((opcode & 0xF0) == 0x50) {
            pos += ((opcode & 0x0F) + 1) * 64;
        } else {
            break;
        }
    }
}

static void nyancat_init_frames(void)
{
    if (nyancat_frames_initialized)
        return;

    /* Frame 0: baseline decompression */
    uint16_t offset = nyancat_frame_offsets[0];
    uint16_t next_offset = nyancat_frame_offsets[1];
    const uint8_t *frame_data = &nyancat_compressed_data[offset];
    const uint8_t *data_end = &nyancat_compressed_data[next_offset];
    decompress_baseline_frame(frame_data, data_end, nyancat_all_frames[0]);

    /* Frames 1-11: delta decompression */
    for (int i = 1; i < NYANCAT_TOTAL_FRAMES; i++) {
        offset = nyancat_frame_offsets[i];
        next_offset = (i < NYANCAT_TOTAL_FRAMES - 1)
                          ? nyancat_frame_offsets[i + 1]
                          : sizeof(nyancat_compressed_data);
        frame_data = &nyancat_compressed_data[offset];
        data_end = &nyancat_compressed_data[next_offset];
        decompress_delta_frame(nyancat_all_frames[i - 1], frame_data, data_end,
                               nyancat_all_frames[i]);
    }

    nyancat_frames_initialized = true;
}

static void update_nyancat_animation(float delta_time)
{
    nyancat_frame_time += delta_time;
    if (nyancat_frame_time >= NYANCAT_FRAME_INTERVAL) {
        nyancat_current_frame =
            (nyancat_current_frame + 1) % NYANCAT_TOTAL_FRAMES;
        nyancat_frame_time = 0.0f;
    }
}

static void draw_nyancat_window(iui_context *ui, iui_port_ctx *port)
{
    nyancat_init_frames();

    iui_begin_window(ui, "Nyan Cat", DEMO_WIN_X, DEMO_WIN_Y, 390,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);
    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = 280.f});
    iui_rect_t area = iui_box_next(ui);

    float pixel_width = area.width / NYANCAT_FRAME_WIDTH;
    float pixel_height = area.height / NYANCAT_FRAME_HEIGHT;
    float pixel_size =
        (pixel_width < pixel_height) ? pixel_width : pixel_height;

    float total_width = pixel_size * NYANCAT_FRAME_WIDTH;
    float total_height = pixel_size * NYANCAT_FRAME_HEIGHT;
    float start_x = area.x + (area.width - total_width) * 0.5f;
    float start_y = area.y + (area.height - total_height) * 0.5f;

    const uint8_t *frame_pixels = nyancat_all_frames[nyancat_current_frame];

    /* Get renderer callbacks for portable rendering */
    iui_renderer_t callbacks = g_iui_port.get_renderer_callbacks(port);

    /* Fill background using draw_box callback */
    uint32_t bg_color = nyancat_palette_colors[0];
    iui_rect_t bg_rect = {start_x, start_y, total_width, total_height};
    callbacks.draw_box(bg_rect, 0, bg_color, callbacks.user);

    /* Draw pixels using draw_box callback (portable across all backends) */
    for (int row = 0; row < NYANCAT_FRAME_HEIGHT; row++) {
        for (int col = 0; col < NYANCAT_FRAME_WIDTH; col++) {
            uint8_t palette_idx = frame_pixels[row * NYANCAT_FRAME_WIDTH + col];
            if (palette_idx >= NYANCAT_PALETTE_SIZE)
                palette_idx = 0;
            if (palette_idx == 0)
                continue;

            uint32_t color = nyancat_palette_colors[palette_idx];
            float px = start_x + col * pixel_size;
            float py = start_y + row * pixel_size;

            iui_rect_t pixel_rect = {px, py, pixel_size, pixel_size};
            callbacks.draw_box(pixel_rect, 0, color, callbacks.user);
        }
    }

    iui_box_end(ui);
    (void) port; /* Suppress unused parameter warning */
    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_NYANCAT */

/* Color Scheme Demo */

#ifdef CONFIG_DEMO_THEME
static void draw_color_scheme_window(iui_context *ui, iui_port_ctx *port)
{
    iui_begin_window(ui, "Color Scheme", DEMO_WIN_X, DEMO_WIN_Y, 300,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    const iui_theme_t *theme = iui_get_theme(ui);
    const int sw = 65, sh = 32;

    /* Primary group */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Primary");
    iui_rect_t r = iui_grid_begin(ui, 4, sw, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->primary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->on_primary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->primary_container);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->on_primary_container);
    iui_grid_end(ui);

    /* Secondary group */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Secondary");
    r = iui_grid_begin(ui, 4, sw, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->secondary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->on_secondary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->secondary_container);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->on_secondary_container);
    iui_grid_end(ui);

    /* Tertiary group */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Tertiary");
    r = iui_grid_begin(ui, 4, sw, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->tertiary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->on_tertiary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->tertiary_container);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->on_tertiary_container);
    iui_grid_end(ui);

    /* Surface elevation */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Surface Elevation");
    r = iui_grid_begin(ui, 5, 52, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->surface_container_lowest);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->surface_container_low);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->surface_container);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->surface_container_high);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->surface_container_highest);
    iui_grid_end(ui);

    /* Surface */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Surface");
    r = iui_grid_begin(ui, 4, sw, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->surface);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->on_surface);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->surface_variant);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->on_surface_variant);
    iui_grid_end(ui);

    /* Error group */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Error");
    r = iui_grid_begin(ui, 4, sw, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->error);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->on_error);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->error_container);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->on_error_container);
    iui_grid_end(ui);

    /* Outline */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Outline");
    r = iui_grid_begin(ui, 2, sw * 2, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->outline);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->outline_variant);
    iui_grid_end(ui);

    /* Inverse & Utility */
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "Inverse & Utility");
    r = iui_grid_begin(ui, 5, 52, sh, 2);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->inverse_surface);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0,
                          theme->inverse_on_surface);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->inverse_primary);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->shadow);
    r = iui_grid_next(ui);
    iui_draw_elevated_box(ui, r, 4.f, IUI_ELEVATION_0, theme->scrim);
    iui_grid_end(ui);

    iui_end_window(ui);
    (void) port;
}
#endif /* CONFIG_DEMO_THEME */

#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT) && \
    defined(CONFIG_MODULE_CONTAINER)
/* UI Components Demo (Buttons, Widgets, Elevation, Progress) */

static void draw_iui_components_window(iui_context *ui, float delta_time)
{
    iui_begin_window(ui, "UI Components", DEMO_WIN_X, DEMO_WIN_Y, 420,
                     get_demo_window_height(),
                     IUI_WINDOW_RESIZABLE | IUI_WINDOW_AUTO_SIZE);

    static int tab_sel = 0;
#ifdef CONFIG_MODULE_NAVIGATION
    static const char *tab_labels[] = {"Buttons", "Widgets", "Elevation",
                                       "Progress", "Extras"};
    tab_sel = iui_tabs(ui, tab_sel, 5, tab_labels);
    iui_newline(ui);
#endif

    if (tab_sel == 0) {
        /* Buttons Tab */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "BUTTON STYLES");
        iui_grid_begin(ui, 2, 170, 30, 4);
        iui_filled_button(ui, "Filled", IUI_ALIGN_CENTER);
        iui_grid_next(ui);
        iui_tonal_button(ui, "Tonal", IUI_ALIGN_CENTER);
        iui_grid_next(ui);
        iui_elevated_button(ui, "Elevated", IUI_ALIGN_CENTER);
        iui_grid_next(ui);
        iui_text_button(ui, "Text", IUI_ALIGN_CENTER);
        iui_grid_end(ui);

#ifdef CONFIG_MODULE_ACTION
        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "ICON BUTTONS");
        iui_grid_begin(ui, 6, 60, 44, 4);
        iui_icon_button(ui, "settings");
        iui_grid_next(ui);
        iui_icon_button(ui, "search");
        iui_grid_next(ui);
        iui_icon_button(ui, "menu");
        iui_grid_next(ui);
        iui_icon_button_filled(ui, "favorite");
        iui_grid_next(ui);
        iui_icon_button_tonal(ui, "share");
        iui_grid_next(ui);
        iui_icon_button_outlined(ui, "add");
        iui_grid_end(ui);

        static bool icon_toggle_1 = false;
        static bool icon_toggle_2 = true;
        iui_grid_begin(ui, 2, 60, 44, 4);
        iui_icon_button_toggle(ui, "favorite", &icon_toggle_1);
        iui_grid_next(ui);
        iui_icon_button_toggle_filled(ui, "bookmark", &icon_toggle_2);
        iui_grid_end(ui);

        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "FAB");
        iui_rect_t win_rect = iui_get_window_rect(ui);
        iui_rect_t layout = iui_get_layout_rect(ui);
        float fab_y = layout.y + 8.f;
        float x = win_rect.x + 12.f;

        iui_fab(ui, x, fab_y, "add");
        x += 64.f;
        iui_fab_extended(ui, x, fab_y, "compose", "New");

        iui_box_begin(ui,
                      &(iui_box_config_t) {.child_count = 1, .cross = 64.f});
        iui_box_next(ui);
        iui_box_end(ui);
#endif /* CONFIG_MODULE_ACTION */

    } else if (tab_sel == 1) {
        /* Widgets Tab */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "SELECTION");

        static uint32_t seg_sel = 0;
        const char *seg_opts[] = {"Day", "Week", "Month"};
        iui_segmented(ui, seg_opts, 3, &seg_sel);

        static bool chk1 = true;
        iui_checkbox(ui, "Remember me", &chk1);

        static int radio_val = 1;
        iui_grid_begin(ui, 3, 115, 24, 4);
        iui_radio(ui, "Small", &radio_val, 0);
        iui_grid_next(ui);
        iui_radio(ui, "Medium", &radio_val, 1);
        iui_grid_next(ui);
        iui_radio(ui, "Large", &radio_val, 2);
        iui_grid_end(ui);

        iui_divider(ui);

        static bool sw1 = true;
        iui_switch(ui, "Notifications", &sw1, "+", "-");

#ifdef CONFIG_MODULE_SELECTION
        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "CHIPS");

        iui_box_begin(
            ui, &(iui_box_config_t) {
                    .child_count = 3, .cross = IUI_CHIP_HEIGHT + 8, .gap = 8});
        iui_box_next(ui);
        iui_chip_assist(ui, "Add event", "add");
        iui_box_next(ui);
        iui_chip_assist(ui, "Settings", "settings");
        iui_box_next(ui);
        iui_chip_assist(ui, "Search", "search");
        iui_box_end(ui);

        static bool filter1 = false, filter2 = true, filter3 = false;
        iui_box_begin(
            ui, &(iui_box_config_t) {
                    .child_count = 3, .cross = IUI_CHIP_HEIGHT + 8, .gap = 8});
        iui_box_next(ui);
        iui_chip_filter(ui, "Active", &filter1);
        iui_box_next(ui);
        iui_chip_filter(ui, "Pending", &filter2);
        iui_box_next(ui);
        iui_chip_filter(ui, "Archived", &filter3);
        iui_box_end(ui);

        static bool input1_vis = true, input2_vis = true;
        bool input1_removed = false, input2_removed = false;
        iui_box_begin(
            ui, &(iui_box_config_t) {
                    .child_count = 2, .cross = IUI_CHIP_HEIGHT + 8, .gap = 8});
        iui_box_next(ui);
        if (input1_vis) {
            iui_chip_input(ui, "user@mail.com", &input1_removed);
            if (input1_removed)
                input1_vis = false;
        }
        iui_box_next(ui);
        if (input2_vis) {
            iui_chip_input(ui, "tag:important", &input2_removed);
            if (input2_removed)
                input2_vis = false;
        }
        iui_box_end(ui);

        iui_box_begin(
            ui, &(iui_box_config_t) {
                    .child_count = 3, .cross = IUI_CHIP_HEIGHT + 8, .gap = 8});
        iui_box_next(ui);
        iui_chip_suggestion(ui, "Try this");
        iui_box_next(ui);
        iui_chip_suggestion(ui, "Popular");
        iui_box_next(ui);
        iui_chip_suggestion(ui, "Recent");
        iui_box_end(ui);
#endif /* CONFIG_MODULE_SELECTION */

        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "SCROLLABLE");

        static iui_scroll_state widget_scroll = {0};
        iui_scroll_begin(ui, &widget_scroll, 0.f, 60.f);
        for (int i = 0; i < 20; i++) {
            iui_text(ui, IUI_ALIGN_LEFT, "Scroll item %d", i + 1);
            iui_newline(ui);
        }
        iui_scroll_end(ui, &widget_scroll);

    } else if (tab_sel == 2) {
        /* Elevation Tab */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "ELEVATION LEVELS");
        iui_text_body_small(ui, IUI_ALIGN_LEFT,
                            "Shadow depth increases with level");

        uint32_t panel_bg = 0xFFE8E4E0;
        iui_box_begin(ui,
                      &(iui_box_config_t) {.child_count = 1, .cross = 200.f});
        iui_rect_t panel = iui_box_next(ui);
        iui_draw_elevated_box(ui, panel, 12.f, IUI_ELEVATION_0, panel_bg);

        float gx = panel.x + 12.f;
        float gy = panel.y + 12.f;
        float cw = 165.f, ch = 48.f, gapx = 12.f, gapy = 12.f;

        for (int i = 0; i <= 5; i++) {
            int col = i % 2, row = i / 2;
            iui_rect_t box = {.x = gx + col * (cw + gapx),
                              .y = gy + row * (ch + gapy),
                              .width = cw,
                              .height = ch};
            iui_draw_elevated_box(ui, box, 8.f, (enum iui_elevation) i,
                                  0xFFFFFFFF);
        }
        iui_box_end(ui);

        iui_grid_begin(ui, 3, 115, 20, 4);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "0: Flat");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "1: Cards");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "2: Buttons");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "3: Dialogs");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "4: Drawers");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "5: Modals");
        iui_grid_end(ui);

        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "CARD STYLES");
        iui_box_begin(ui,
                      &(iui_box_config_t) {.child_count = 1, .cross = 80.f});
        iui_rect_t cp = iui_box_next(ui);
        iui_draw_elevated_box(ui, cp, 12.f, IUI_ELEVATION_0, panel_bg);

        float cardw = 100.f, cardh = 45.f, gap = 12.f;
        float sx = cp.x + 16.f, sy = cp.y + 10.f;

        iui_rect_t c1 = {sx, sy, cardw, cardh};
        iui_draw_elevated_box(ui, c1, 8.f, IUI_ELEVATION_2, 0xFFFFFFFF);

        iui_rect_t c2 = {sx + cardw + gap, sy, cardw, cardh};
        iui_draw_elevated_box(ui, c2, 8.f, IUI_ELEVATION_0, 0xFFD0D0D0);

        iui_rect_t c3 = {sx + 2 * (cardw + gap), sy, cardw, cardh};
        iui_draw_elevated_box(ui, c3, 8.f, IUI_ELEVATION_0, 0xFFFFFFFF);
        iui_box_end(ui);

        iui_grid_begin(ui, 3, 115, 20, 4);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "Elevated");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "Filled");
        iui_grid_next(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER, "Outlined");
        iui_grid_end(ui);

    } else if (tab_sel == 3) {
        /* Progress Tab */
        static float animated_progress = 0.f;
        animated_progress = fmodf(animated_progress + delta_time * 20.f, 100.f);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "SLIDERS");

        static float slider_val = 50.f;
        iui_slider(ui, "Progress", 0, 100, 1, &slider_val, "%.0f%%");

        static float slider_ex1 = 70.f;
        iui_slider_options opts1 = {
            .start_text = "Min",
            .end_text = "Max",
            .show_value_indicator = true,
            .value_format = "%.0f%%",
        };
        slider_ex1 = iui_slider_ex(ui, slider_ex1, 0.f, 100.f, 5.f, &opts1);

        static float slider_ex2 = 30.f;
        iui_slider_options opts2 = {
            .active_track_color = 0x4CAF50FF,
            .inactive_track_color = 0xE0E0E0FF,
            .handle_color = 0x388E3CFF,
            .show_value_indicator = true,
            .value_format = "%.1f",
        };
        slider_ex2 = iui_slider_ex(ui, slider_ex2, 0.f, 100.f, 0.5f, &opts2);

        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "LINEAR PROGRESS");
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Slider-controlled:");
        iui_progress_linear(ui, slider_val, 100, false);
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Animated:");
        iui_progress_linear(ui, animated_progress, 100, false);

        iui_divider(ui);

        iui_text_label_small(ui, IUI_ALIGN_LEFT, "CIRCULAR PROGRESS");
        if (iui_has_vector_primitives(ui)) {
            iui_grid_begin(ui, 4, 85, 50, 4);
            iui_progress_circular(ui, slider_val, 100, 40, false);
            iui_grid_next(ui);
            iui_progress_circular(ui, animated_progress, 100, 40, false);
            iui_grid_next(ui);
            iui_progress_circular(ui, 100 - animated_progress, 100, 40, false);
            iui_grid_next(ui);
            iui_progress_circular(ui, 0, 100, 40, true);
            iui_grid_end(ui);
        } else {
            iui_text_body_small(ui, IUI_ALIGN_LEFT,
                                "Requires vector primitives");
        }

    } else if (tab_sel == 4) {
        /* Extras Tab - Tooltips, Badges, Banner, Data Table */
        static bool show_banner = true;
        static int banner_action = 0;

        /* Tooltip demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "TOOLTIPS");
        iui_grid_begin(ui, 2, 170, 36, 8);
        if (iui_filled_button(ui, "Hover for tip", IUI_ALIGN_CENTER))
            (void) 0;
        iui_tooltip(ui, "This is a plain tooltip");
        iui_grid_next(ui);
        if (iui_tonal_button(ui, "Rich tooltip", IUI_ALIGN_CENTER))
            (void) 0;
        if (iui_tooltip_rich(ui, "Rich Tooltip", "Detailed explanation here",
                             "Learn more"))
            printf("Tooltip action clicked\n");
        iui_grid_end(ui);

        iui_divider(ui);

        /* Badge demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "BADGES");
        iui_box_begin(ui, &(iui_box_config_t) {
                              .child_count = 4, .cross = 50.f, .gap = 8});

        /* Badge dot */
        iui_rect_t notif_slot = iui_box_next(ui);
        float notif_cx = notif_slot.x + notif_slot.width * 0.5f;
        float notif_cy = notif_slot.y + notif_slot.height * 0.5f;
        if (iui_icon_button(ui, "notifications"))
            (void) 0;
        iui_badge_dot(ui, notif_cx + 12.f, notif_cy - 12.f);

        /* Badge with count */
        iui_rect_t mail_slot = iui_box_next(ui);
        float mail_cx = mail_slot.x + mail_slot.width * 0.5f;
        float mail_cy = mail_slot.y + mail_slot.height * 0.5f;
        if (iui_icon_button(ui, "mail"))
            (void) 0;
        iui_badge_number(ui, mail_cx + 12.f, mail_cy - 12.f, 7, 99);

        /* Badge with large count (clamped) */
        iui_rect_t chat_slot = iui_box_next(ui);
        float chat_cx = chat_slot.x + chat_slot.width * 0.5f;
        float chat_cy = chat_slot.y + chat_slot.height * 0.5f;
        if (iui_icon_button(ui, "chat"))
            (void) 0;
        iui_badge_number(ui, chat_cx + 12.f, chat_cy - 12.f, 150, 99);

        /* Badge with count 0 (hidden) */
        iui_rect_t inbox_slot = iui_box_next(ui);
        float inbox_cx = inbox_slot.x + inbox_slot.width * 0.5f;
        float inbox_cy = inbox_slot.y + inbox_slot.height * 0.5f;
        if (iui_icon_button(ui, "inbox"))
            (void) 0;
        iui_badge_number(ui, inbox_cx + 12.f, inbox_cy - 12.f, 0, 99);

        iui_box_end(ui);

        iui_divider(ui);

        /* Banner demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "BANNER");
        if (show_banner) {
            iui_banner_options banner_opts = {
                .message = "Network connection lost",
                .action1 = "Retry",
                .action2 = "Dismiss",
                .icon = "warning",
            };
            banner_action = iui_banner(ui, &banner_opts);
            if (banner_action == 1) {
                printf("Banner: Retry clicked\n");
            } else if (banner_action == 2) {
                show_banner = false;
            }
        } else {
            if (iui_text_button(ui, "Show Banner", IUI_ALIGN_LEFT))
                show_banner = true;
            iui_newline(ui); /* Advance layout to match banner height */
        }

        iui_divider(ui);

        /* Data Table demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "DATA TABLE");
        static iui_table_state table = {0}; /* user-provided state */
        float col_widths[] = {-1.f, 80.f, 60.f};
        iui_table_begin(ui, &table, 3, col_widths);

        iui_table_header(ui, &table, "Name");
        iui_table_header(ui, &table, "Role");
        iui_table_header(ui, &table, "ID");

        iui_table_row_begin(ui, &table);
        iui_table_cell(ui, &table, "Alice");
        iui_table_cell(ui, &table, "Admin");
        iui_table_cell(ui, &table, "001");
        iui_table_row_end(ui, &table);

        iui_table_row_begin(ui, &table);
        iui_table_cell(ui, &table, "Bob");
        iui_table_cell(ui, &table, "User");
        iui_table_cell(ui, &table, "002");
        iui_table_row_end(ui, &table);

        iui_table_row_begin(ui, &table);
        iui_table_cell(ui, &table, "Carol");
        iui_table_cell(ui, &table, "Guest");
        iui_table_cell(ui, &table, "003");
        iui_table_row_end(ui, &table);

        iui_table_end(ui, &table);
    }

    iui_end_window(ui);
}
#endif /* CONFIG_MODULE_BASIC && MODULE_INPUT && MODULE_CONTAINER */

/* Accessibility Demo */

#ifdef CONFIG_DEMO_ACCESSIBILITY
static void draw_accessibility_window(iui_context *ui)
{
    iui_begin_window(ui, "Accessibility", DEMO_WIN_X, DEMO_WIN_Y, 340,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    const iui_theme_t *theme = iui_get_theme(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "THEME CONTRAST (WCAG 2.1)");
    int failures = iui_theme_validate_contrast(theme);
    if (failures == 0) {
        iui_text_body_medium(ui, IUI_ALIGN_LEFT,
                             "All color pairs pass WCAG AA");
    } else {
        iui_text_body_medium(ui, IUI_ALIGN_LEFT,
                             "Warning: %d pairs fail WCAG AA", failures);
    }

    iui_divider(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "KEY CONTRAST RATIOS");
    float surface_ratio = iui_contrast_ratio(theme->on_surface, theme->surface);
    float primary_ratio = iui_contrast_ratio(theme->on_primary, theme->primary);
    float error_ratio = iui_contrast_ratio(theme->on_error, theme->error);

    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Text/Surface: %.1f:1 %s",
                        surface_ratio,
                        surface_ratio >= 4.5f ? "(AA)" : "(FAIL)");
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Primary btn: %.1f:1 %s",
                        primary_ratio,
                        primary_ratio >= 3.0f ? "(AA-lg)" : "(FAIL)");
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Error btn: %.1f:1 %s", error_ratio,
                        error_ratio >= 3.0f ? "(AA-lg)" : "(FAIL)");

    iui_divider(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "KEYBOARD FOCUS (Tab/Shift+Tab)");
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Press Tab to cycle focus");

    static bool a11y_check = false;
    iui_checkbox(ui, "Checkbox (Enter to toggle)", &a11y_check);

    static int a11y_radio = 0;
    iui_radio(ui, "Option A (Enter to select)", &a11y_radio, 0);
    iui_radio(ui, "Option B", &a11y_radio, 1);
    iui_radio(ui, "Option C", &a11y_radio, 2);

    iui_divider(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "SCREEN READER HINTS");
    iui_text_body_small(ui, IUI_ALIGN_LEFT,
                        "Semantic descriptions for widgets:");

    static char desc_buf[256];

    iui_a11y_hint cb_hint =
        iui_a11y_make_hint("Enable notifications", IUI_A11Y_ROLE_CHECKBOX);
    cb_hint.state = a11y_check ? IUI_A11Y_STATE_CHECKED : IUI_A11Y_STATE_NONE;
    iui_a11y_describe(&cb_hint, desc_buf, sizeof(desc_buf));
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "CB: \"%s\"", desc_buf);

    iui_a11y_hint radio_hint =
        iui_a11y_make_set_hint(a11y_radio == 0   ? "Option A"
                               : a11y_radio == 1 ? "Option B"
                                                 : "Option C",
                               IUI_A11Y_ROLE_RADIO, a11y_radio + 1, 3);
    radio_hint.state = IUI_A11Y_STATE_SELECTED;
    iui_a11y_describe(&radio_hint, desc_buf, sizeof(desc_buf));
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Radio: \"%s\"", desc_buf);

    static float a11y_slider = 75.f;
    iui_a11y_hint slider_hint =
        iui_a11y_make_slider_hint("Volume", a11y_slider, 0.f, 100.f);
    iui_a11y_describe(&slider_hint, desc_buf, sizeof(desc_buf));
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Slider: \"%s\"", desc_buf);

    iui_divider(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "RELATIVE LUMINANCE");
    float surf_lum = iui_relative_luminance(theme->surface);
    float on_surf_lum = iui_relative_luminance(theme->on_surface);
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Surface: %.3f", surf_lum);
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "On-Surface: %.3f", on_surf_lum);

    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_ACCESSIBILITY */

#ifdef CONFIG_DEMO_FONT_EDITOR

/* Font Editor: interactive vector glyph editor with canvas, control points,
 * mouse selection, keyboard movement, and undo. Mimics
 * externals/mado/tools/font-edit but uses zero heap allocations.
 */

#define FE_MAX_OPS 64
#define FE_MAX_POINTS 192
#define FE_UNDO_DEPTH 8

typedef enum {
    FE_OP_MOVE,
    FE_OP_LINE,
    FE_OP_CURVE,
} fe_op_type_t;

typedef struct {
    fe_op_type_t type;
    int n_pts;    /* 1 for move/line, 3 for curve */
    int pt_start; /* index into points[] */
} fe_op_t;

typedef struct {
    float x, y;
} fe_point_t;

typedef struct {
    fe_op_t ops[FE_MAX_OPS];
    int n_ops;
    fe_point_t points[FE_MAX_POINTS];
    int n_points;
    int left, right, ascent, descent;
} fe_glyph_t;

typedef struct {
    int op_idx; /* -1 = none */
    int pt_idx; /* index into glyph points[], -1 = none */
} fe_selection_t;

typedef struct {
    int selected_char;
    float zoom;
    fe_glyph_t glyph;
    fe_glyph_t undo_stack[FE_UNDO_DEPTH];
    int undo_top;   /* next write slot (circular) */
    int undo_count; /* number of valid entries */
    fe_selection_t sel_first;
    fe_selection_t sel_last;
    iui_rect_t canvas_rect;
    int prev_char; /* track character changes to re-parse */
    bool moving;   /* true during arrow-key moves for undo coalescing */
} fe_state_t;

/* Parse glyph bytecode into editable fe_glyph_t */
static void fe_parse_glyph(fe_glyph_t *g, int char_code)
{
    const signed char *raw = iui_get_glyph((unsigned char) char_code);

    g->left = IUI_GLYPH_LEFT(raw);
    g->right = IUI_GLYPH_RIGHT(raw);
    g->ascent = IUI_GLYPH_ASCENT(raw);
    g->descent = IUI_GLYPH_DESCENT(raw);
    g->n_ops = 0;
    g->n_points = 0;

    const signed char *it = IUI_GLYPH_DRAW(raw);
    for (;;) {
        signed char op = *it++;
        if (g->n_ops >= FE_MAX_OPS)
            break;
        switch (op) {
        case 'm':
            if (g->n_points + 1 > FE_MAX_POINTS)
                return;
            g->ops[g->n_ops].type = FE_OP_MOVE;
            g->ops[g->n_ops].n_pts = 1;
            g->ops[g->n_ops].pt_start = g->n_points;
            g->points[g->n_points].x = (float) it[0];
            g->points[g->n_points].y = (float) it[1];
            g->n_points++;
            g->n_ops++;
            it += 2;
            break;
        case 'l':
            if (g->n_points + 1 > FE_MAX_POINTS)
                return;
            g->ops[g->n_ops].type = FE_OP_LINE;
            g->ops[g->n_ops].n_pts = 1;
            g->ops[g->n_ops].pt_start = g->n_points;
            g->points[g->n_points].x = (float) it[0];
            g->points[g->n_points].y = (float) it[1];
            g->n_points++;
            g->n_ops++;
            it += 2;
            break;
        case 'c':
            if (g->n_points + 3 > FE_MAX_POINTS)
                return;
            g->ops[g->n_ops].type = FE_OP_CURVE;
            g->ops[g->n_ops].n_pts = 3;
            g->ops[g->n_ops].pt_start = g->n_points;
            g->points[g->n_points].x = (float) it[0];
            g->points[g->n_points].y = (float) it[1];
            g->points[g->n_points + 1].x = (float) it[2];
            g->points[g->n_points + 1].y = (float) it[3];
            g->points[g->n_points + 2].x = (float) it[4];
            g->points[g->n_points + 2].y = (float) it[5];
            g->n_points += 3;
            g->n_ops++;
            it += 6;
            break;
        case 'e':
            return;
        default:
            return;
        }
    }
}

static void fe_push_undo(fe_state_t *st)
{
    memcpy(&st->undo_stack[st->undo_top], &st->glyph, sizeof(fe_glyph_t));
    st->undo_top = (st->undo_top + 1) % FE_UNDO_DEPTH;
    if (st->undo_count < FE_UNDO_DEPTH)
        st->undo_count++;
}

static void fe_pop_undo(fe_state_t *st)
{
    if (st->undo_count <= 0)
        return;
    st->undo_top = (st->undo_top - 1 + FE_UNDO_DEPTH) % FE_UNDO_DEPTH;
    st->undo_count--;
    memcpy(&st->glyph, &st->undo_stack[st->undo_top], sizeof(fe_glyph_t));
    st->sel_first.op_idx = -1;
    st->sel_first.pt_idx = -1;
    st->sel_last.op_idx = -1;
    st->sel_last.pt_idx = -1;
}

/* Compute canvas origin: baseline sits so that the full glyph
 * (ascent above, descent below) is centered vertically in the canvas.
 * Horizontally, center the glyph width (left..right) in the canvas.
 */
static void fe_canvas_origin(const fe_state_t *st, float *ox, float *oy)
{
    float cw = st->canvas_rect.width;
    float ch = st->canvas_rect.height;
    float asc = (float) st->glyph.ascent;
    float desc = (float) st->glyph.descent;
    float left = (float) st->glyph.left;
    float right = (float) st->glyph.right;
    float glyph_h = (asc - desc) * st->zoom; /* total pixel height */
    float glyph_w = (right - left) * st->zoom;
    float margin = 20.0f;

    /* Baseline Y: position so ascent..descent block is centered */
    float top_y = (ch - glyph_h) * 0.5f + margin * 0.5f;
    if (top_y < margin)
        top_y = margin;
    *oy = st->canvas_rect.y + top_y + asc * st->zoom;

    /* Origin X: center glyph width */
    *ox = st->canvas_rect.x + (cw - glyph_w) * 0.5f - left * st->zoom;
}

/* Screen coords to glyph coords */
static void fe_screen_to_glyph(const fe_state_t *st,
                               float sx,
                               float sy,
                               float *gx,
                               float *gy)
{
    float origin_x, origin_y;
    fe_canvas_origin(st, &origin_x, &origin_y);
    *gx = (sx - origin_x) / st->zoom;
    *gy = (sy - origin_y) / st->zoom;
}

/* Find nearest point to given glyph coordinates; returns point index or -1 */
static int fe_hit_test(const fe_state_t *st, float gx, float gy, int *out_op)
{
    float threshold = 5.0f; /* glyph units */
    float best_dist = threshold * threshold;
    int best_pt = -1;
    int best_op = -1;

    for (int i = 0; i < st->glyph.n_ops; i++) {
        const fe_op_t *op = &st->glyph.ops[i];
        for (int j = 0; j < op->n_pts; j++) {
            int pi = op->pt_start + j;
            float dx = st->glyph.points[pi].x - gx;
            float dy = st->glyph.points[pi].y - gy;
            float d2 = dx * dx + dy * dy;
            if (d2 < best_dist) {
                best_dist = d2;
                best_pt = pi;
                best_op = i;
            }
        }
    }
    if (out_op)
        *out_op = best_op;
    return best_pt;
}

/* Draw grid lines on canvas background */
static void fe_draw_canvas_bg(iui_context *ui, const fe_state_t *st)
{
    const iui_theme_t *theme = iui_get_theme(ui);
    iui_rect_t cr = st->canvas_rect;
    float ox, oy;
    fe_canvas_origin(st, &ox, &oy);
    float z = st->zoom;

    /* Canvas background */
    ui->renderer.draw_box(cr, 0, theme->surface_container, ui->renderer.user);

    /* Pre-compute screen-space bounds for grid extent */
    float sy_top = oy + -64.0f * z;
    float sy_bot = oy + 20.0f * z;
    float sx_left = ox + -80.0f * z;
    float sx_right = ox + 80.0f * z;
    float cr_right = cr.x + cr.width;
    float cr_bottom = cr.y + cr.height;

    /* Grid lines at 10-unit glyph intervals */
    uint32_t grid_color = (theme->outline_variant & 0x00FFFFFF) | 0x30000000;
    float step = 10.0f;
    /* Vertical grid lines */
    for (float gx = -80.0f; gx <= 80.0f; gx += step) {
        float sx = ox + gx * z;
        if (sx >= cr.x && sx <= cr_right)
            iui_draw_line(ui, sx, fmaxf(sy_top, cr.y), sx,
                          fminf(sy_bot, cr_bottom), 1.0f, grid_color);
    }
    /* Horizontal grid lines */
    for (float gy = -60.0f; gy <= 20.0f; gy += step) {
        float sy = oy + gy * z;
        if (sy >= cr.y && sy <= cr_bottom)
            iui_draw_line(ui, fmaxf(sx_left, cr.x), sy,
                          fminf(sx_right, cr_right), sy, 1.0f, grid_color);
    }

    /* Baseline (y=0) in red */
    if (oy >= cr.y && oy <= cr_bottom)
        iui_draw_line(ui, fmaxf(sx_left, cr.x), oy, fminf(sx_right, cr_right),
                      oy, 1.5f, 0xFFCC3333);

    /* Ascent line in blue */
    if (st->glyph.ascent != 0) {
        float sy = oy + (float) -st->glyph.ascent * z;
        if (sy >= cr.y && sy <= cr_bottom)
            iui_draw_line(ui, fmaxf(sx_left, cr.x), sy,
                          fminf(sx_right, cr_right), sy, 1.0f, 0xFF3366CC);
    }

    /* Descent line in blue */
    if (st->glyph.descent != 0) {
        float sy = oy + (float) -st->glyph.descent * z;
        if (sy >= cr.y && sy <= cr_bottom)
            iui_draw_line(ui, fmaxf(sx_left, cr.x), sy,
                          fminf(sx_right, cr_right), sy, 1.0f, 0xFF3366CC);
    }

    /* Left bearing in green */
    {
        float sx = ox + (float) st->glyph.left * z;
        if (sx >= cr.x && sx <= cr_right)
            iui_draw_line(ui, sx, fmaxf(sy_top, cr.y), sx,
                          fminf(sy_bot, cr_bottom), 1.0f, 0xFF33AA33);
    }

    /* Right bearing in green */
    {
        float sx = ox + (float) st->glyph.right * z;
        if (sx >= cr.x && sx <= cr_right)
            iui_draw_line(ui, sx, fmaxf(sy_top, cr.y), sx,
                          fminf(sy_bot, cr_bottom), 1.0f, 0xFF33AA33);
    }
}

/* Render the glyph path using line segments (flatten beziers) */
static void fe_draw_glyph_path(iui_context *ui, const fe_state_t *st)
{
    const fe_glyph_t *g = &st->glyph;
    const iui_theme_t *theme = iui_get_theme(ui);
    uint32_t path_color = theme->on_surface;
    float ox, oy;
    fe_canvas_origin(st, &ox, &oy);
    float z = st->zoom;
    float pen_x = 0, pen_y = 0;

    for (int i = 0; i < g->n_ops; i++) {
        const fe_op_t *op = &g->ops[i];
        const fe_point_t *pts = &g->points[op->pt_start];

        switch (op->type) {
        case FE_OP_MOVE:
            pen_x = pts[0].x;
            pen_y = pts[0].y;
            break;
        case FE_OP_LINE: {
            float sx0 = ox + pen_x * z, sy0 = oy + pen_y * z;
            float sx1 = ox + pts[0].x * z, sy1 = oy + pts[0].y * z;
            iui_draw_line(ui, sx0, sy0, sx1, sy1, 1.5f, path_color);
            pen_x = pts[0].x;
            pen_y = pts[0].y;
            break;
        }
        case FE_OP_CURVE: {
            float x0 = pen_x, y0 = pen_y;
            float x1 = pts[0].x, y1 = pts[0].y;
            float x2 = pts[1].x, y2 = pts[1].y;
            float x3 = pts[2].x, y3 = pts[2].y;
            int segments = 16;
            float inv = 1.0f / (float) segments;
            float prev_sx = ox + x0 * z, prev_sy = oy + y0 * z;
            for (int s = 1; s <= segments; s++) {
                float t = (float) s * inv;
                float u = 1.0f - t;
                float bx = u * u * u * x0 + 3 * u * u * t * x1 +
                           3 * u * t * t * x2 + t * t * t * x3;
                float by = u * u * u * y0 + 3 * u * u * t * y1 +
                           3 * u * t * t * y2 + t * t * t * y3;
                float cur_sx = ox + bx * z, cur_sy = oy + by * z;
                iui_draw_line(ui, prev_sx, prev_sy, cur_sx, cur_sy, 1.5f,
                              path_color);
                prev_sx = cur_sx;
                prev_sy = cur_sy;
            }
            pen_x = x3;
            pen_y = y3;
            break;
        }
        }
    }
}

/* Draw a control point; enlarged with white ring when selected */
static void fe_draw_point(iui_context *ui,
                          const fe_state_t *st,
                          float sx,
                          float sy,
                          int pt_idx,
                          uint32_t color)
{
    bool selected =
        (st->sel_first.pt_idx == pt_idx) || (st->sel_last.pt_idx == pt_idx);
    if (selected)
        iui_draw_circle(ui, sx, sy, 6.0f, color, 0xFFFFFFFF, 2.0f);
    else
        iui_draw_circle(ui, sx, sy, 4.0f, color, 0, 0);
}

/* Draw control points with color coding */
static void fe_draw_control_points(iui_context *ui, const fe_state_t *st)
{
    const fe_glyph_t *g = &st->glyph;
    float ox, oy;
    fe_canvas_origin(st, &ox, &oy);
    float z = st->zoom;

    /* Colors: move=yellow, line=red, curve ctrl=blue, curve end=green */
    uint32_t col_move = 0xFFCCCC00;
    uint32_t col_line = 0xFFCC3333;
    uint32_t col_ctrl = 0xFF3366CC;
    uint32_t col_end = 0xFF33AA33;
    uint32_t col_handle = 0x80888888;

    for (int i = 0; i < g->n_ops; i++) {
        const fe_op_t *op = &g->ops[i];
        const fe_point_t *pts = &g->points[op->pt_start];

        if (op->type == FE_OP_CURVE) {
            /* Find pen position from previous op's endpoint */
            float pen_gx = 0, pen_gy = 0;
            if (i > 0) {
                const fe_op_t *prev = &g->ops[i - 1];
                int last_pt = prev->pt_start + prev->n_pts - 1;
                pen_gx = g->points[last_pt].x;
                pen_gy = g->points[last_pt].y;
            }
            float sx_pen = ox + pen_gx * z, sy_pen = oy + pen_gy * z;
            float sx_c0 = ox + pts[0].x * z, sy_c0 = oy + pts[0].y * z;
            float sx_c1 = ox + pts[1].x * z, sy_c1 = oy + pts[1].y * z;
            float sx_end = ox + pts[2].x * z, sy_end = oy + pts[2].y * z;

            /* Handle lines */
            iui_draw_line(ui, sx_pen, sy_pen, sx_c0, sy_c0, 1.0f, col_handle);
            iui_draw_line(ui, sx_end, sy_end, sx_c1, sy_c1, 1.0f, col_handle);

            fe_draw_point(ui, st, sx_c0, sy_c0, op->pt_start, col_ctrl);
            fe_draw_point(ui, st, sx_c1, sy_c1, op->pt_start + 1, col_ctrl);
            fe_draw_point(ui, st, sx_end, sy_end, op->pt_start + 2, col_end);
        } else {
            uint32_t col = (op->type == FE_OP_MOVE) ? col_move : col_line;
            float sx = ox + pts[0].x * z, sy = oy + pts[0].y * z;
            fe_draw_point(ui, st, sx, sy, op->pt_start, col);
        }
    }
}

/* Draw clickable ASCII grid (0x20..0x7E).
 * Dynamically fits columns to window width for consistent layout.
 */
static void fe_draw_char_grid(iui_context *ui, fe_state_t *st)
{
    const iui_theme_t *theme = iui_get_theme(ui);
    int start_char = 0x20;
    int end_char = 0x7E;
    int total_chars = end_char - start_char + 1; /* 95 printable chars */
    float cell_h = 14.0f;

    /* Determine column count from current layout width */
    float avail_w = ui->layout.width;
    if (avail_w < 40.0f)
        avail_w = 300.0f;
    float cell_w = 13.0f;
    int cols = (int) (avail_w / cell_w);
    if (cols < 10)
        cols = 10;
    if (cols > 32)
        cols = 32;

    int rows = (total_chars + cols - 1) / cols;
    float total_h = (float) rows * cell_h;

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = total_h});
    iui_rect_t area = iui_box_next(ui);

    /* Recompute cell_w to fill the allocated width evenly */
    cell_w = area.width / (float) cols;

    for (int ch = start_char; ch <= end_char; ch++) {
        int idx = ch - start_char;
        int col = idx % cols;
        int row = idx / cols;
        iui_rect_t cell = {area.x + (float) col * cell_w,
                           area.y + (float) row * cell_h, cell_w, cell_h};

        /* Highlight selected character */
        if (ch == st->selected_char)
            ui->renderer.draw_box(cell, 2.0f, theme->primary_container,
                                  ui->renderer.user);

        /* Draw character label */
        char label[2] = {(char) ch, '\0'};
        float tw = iui_text_width_vec(label, ui->font_height);
        float tx = cell.x + (cell_w - tw) * 0.5f;
        float ty = cell.y + (cell_h - ui->font_height) * 0.5f;
        iui_draw_text_vec(ui, tx, ty, label,
                          ch == st->selected_char ? theme->on_primary_container
                                                  : theme->on_surface);

        /* Hit test for mouse click */
        float mx = ui->mouse_pos.x, my = ui->mouse_pos.y;
        if ((ui->mouse_pressed & IUI_MOUSE_LEFT) && mx >= cell.x &&
            mx < cell.x + cell.width && my >= cell.y &&
            my < cell.y + cell.height) {
            st->selected_char = ch;
        }
    }
    iui_box_end(ui);
}

/* Draw compact operation list -- show selected op's neighborhood */
static void fe_draw_op_list(iui_context *ui, const fe_state_t *st)
{
    const fe_glyph_t *g = &st->glyph;
    static const char *tn[] = {"m", "l", "c"};
    int max_show = 4;

    /* Center the view around the selected op if possible */
    int center = st->sel_first.op_idx >= 0 ? st->sel_first.op_idx : 0;
    int start = center - max_show / 2;
    if (start < 0)
        start = 0;
    int end = start + max_show;
    if (end > g->n_ops)
        end = g->n_ops;

    for (int i = start; i < end; i++) {
        const fe_op_t *op = &g->ops[i];
        const fe_point_t *pts = &g->points[op->pt_start];
        bool is_sel = (st->sel_first.op_idx == i) || (st->sel_last.op_idx == i);
        const char *type_str = (unsigned) op->type < 3 ? tn[op->type] : "?";

        if (op->type == FE_OP_CURVE) {
            iui_text_body_small(
                ui, IUI_ALIGN_LEFT, "%s%d:%s(%.0f,%.0f %.0f,%.0f %.0f,%.0f)",
                is_sel ? ">" : " ", i, type_str, pts[0].x, pts[0].y, pts[1].x,
                pts[1].y, pts[2].x, pts[2].y);
        } else {
            iui_text_body_small(ui, IUI_ALIGN_LEFT, "%s%d:%s(%.0f,%.0f)",
                                is_sel ? ">" : " ", i, type_str, pts[0].x,
                                pts[0].y);
        }
    }
    if (g->n_ops > max_show)
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "  [%d/%d ops]", end, g->n_ops);
}

/* Handle mouse clicks on the canvas for point selection */
static void fe_handle_mouse(iui_context *ui, fe_state_t *st)
{
    float mx = ui->mouse_pos.x, my = ui->mouse_pos.y;
    iui_rect_t cr = st->canvas_rect;

    /* Check mouse within canvas */
    if (mx < cr.x || mx > cr.x + cr.width || my < cr.y || my > cr.y + cr.height)
        return;

    if (ui->mouse_pressed & IUI_MOUSE_LEFT) {
        st->moving = false;
        float gx, gy;
        fe_screen_to_glyph(st, mx, my, &gx, &gy);
        int op_idx = -1;
        int pt_idx = fe_hit_test(st, gx, gy, &op_idx);
        st->sel_first.op_idx = op_idx;
        st->sel_first.pt_idx = pt_idx;
    }

    if (ui->mouse_pressed & IUI_MOUSE_RIGHT) {
        st->moving = false;
        float gx, gy;
        fe_screen_to_glyph(st, mx, my, &gx, &gy);
        int op_idx = -1;
        int pt_idx = fe_hit_test(st, gx, gy, &op_idx);
        st->sel_last.op_idx = op_idx;
        st->sel_last.pt_idx = pt_idx;
    }
}

/* Handle keyboard input for point movement and undo */
static void fe_handle_keys(iui_context *ui, fe_state_t *st)
{
    int key = ui->key_pressed;
    if (key == 0) {
        st->moving = false;
        return;
    }

    /* 'u' = undo */
    if (key == 'u' || key == 'U') {
        st->moving = false;
        fe_pop_undo(st);
        return;
    }

    /* Escape = clear selection */
    if (key == IUI_KEY_ESCAPE) {
        st->moving = false;
        st->sel_first.op_idx = -1;
        st->sel_first.pt_idx = -1;
        st->sel_last.op_idx = -1;
        st->sel_last.pt_idx = -1;
        return;
    }

    /* Arrow keys move selected point */
    if (st->sel_first.pt_idx < 0)
        return;

    float dx = 0, dy = 0;
    if (key == IUI_KEY_LEFT)
        dx = -1.0f;
    else if (key == IUI_KEY_RIGHT)
        dx = 1.0f;
    else if (key == IUI_KEY_UP)
        dy = -1.0f;
    else if (key == IUI_KEY_DOWN)
        dy = 1.0f;
    else
        return;

    /* Coalesce consecutive arrow moves into a single undo entry */
    if (!st->moving)
        fe_push_undo(st);
    st->moving = true;
    int pi = st->sel_first.pt_idx;
    st->glyph.points[pi].x += dx;
    st->glyph.points[pi].y += dy;

    /* Shift + arrow: also move paired control point in curves */
    if (ui->modifiers & IUI_MOD_SHIFT) {
        int oi = st->sel_first.op_idx;
        if (oi >= 0 && st->glyph.ops[oi].type == FE_OP_CURVE) {
            int base = st->glyph.ops[oi].pt_start;
            int offset = pi - base;
            /* cp1 (offset 0) pairs with cp2 (offset 1) */
            int pair = -1;
            if (offset == 0)
                pair = base + 1;
            else if (offset == 1)
                pair = base;
            if (pair >= 0 && pair < st->glyph.n_points) {
                st->glyph.points[pair].x += dx;
                st->glyph.points[pair].y += dy;
            }
        }
    }
}

static void draw_font_editor_window(iui_context *ui)
{
    static fe_state_t st = {
        .selected_char = 'A',
        .zoom = 4.0f,
        .undo_top = 0,
        .undo_count = 0,
        .sel_first = {-1, -1},
        .sel_last = {-1, -1},
        .prev_char = -1,
        .moving = false,
    };

    /* Re-parse glyph when character changes */
    if (st.selected_char != st.prev_char) {
        fe_parse_glyph(&st.glyph, st.selected_char);
        st.prev_char = st.selected_char;
        st.sel_first.op_idx = -1;
        st.sel_first.pt_idx = -1;
        st.sel_last.op_idx = -1;
        st.sel_last.pt_idx = -1;
        st.undo_top = 0;
        st.undo_count = 0;
        st.moving = false;
    }

    iui_begin_window(ui, "Font Editor", DEMO_WIN_X, DEMO_WIN_Y, 460,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    /* Row 1: Prev/Next + character info */
    iui_grid_begin(ui, 3, 50.f, 24, 4.f);
    if (iui_button(ui, "Prev", IUI_ALIGN_CENTER)) {
        st.selected_char--;
        if (st.selected_char < 0x20)
            st.selected_char = 0x7E;
    }
    iui_grid_next(ui);
    if (iui_button(ui, "Next", IUI_ALIGN_CENTER)) {
        st.selected_char++;
        if (st.selected_char > 0x7E)
            st.selected_char = 0x20;
    }
    iui_grid_next(ui);
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "'%c' 0x%02X L:%d R:%d",
                        st.selected_char >= 32 && st.selected_char <= 126
                            ? (char) st.selected_char
                            : '?',
                        st.selected_char, st.glyph.left, st.glyph.right);
    iui_grid_end(ui);

    /* Row 2: ASCII character grid */
    fe_draw_char_grid(ui, &st);

    /* Row 3: Zoom slider */
    iui_slider(ui, "Zoom", 2.0f, 12.0f, 0.5f, &st.zoom, "%.1f");

    /* Row 4: Canvas -- dominant element, fills remaining space */
    float canvas_h = 280.0f;
    iui_box_begin(ui,
                  &(iui_box_config_t) {.child_count = 1, .cross = canvas_h});
    st.canvas_rect = iui_box_next(ui);

    iui_push_clip(ui, st.canvas_rect);
    fe_draw_canvas_bg(ui, &st);
    fe_draw_glyph_path(ui, &st);
    fe_draw_control_points(ui, &st);
    iui_pop_clip(ui);
    iui_box_end(ui);

    /* Handle mouse and keyboard interaction */
    fe_handle_mouse(ui, &st);
    fe_handle_keys(ui, &st);

    /* Row 5: Compact info bar */
    iui_divider(ui);
    fe_draw_op_list(ui, &st);
    iui_text_body_small(ui, IUI_ALIGN_LEFT,
                        "A:%d D:%d  Click=sel  Arrows=move  U=undo",
                        st.glyph.ascent, st.glyph.descent);

    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_FONT_EDITOR */

/* Motion System Demo */

#ifdef CONFIG_DEMO_MOTION
static void draw_motion_window(iui_context *ui, float delta_time)
{
    iui_begin_window(ui, "Motion", DEMO_WIN_X, DEMO_WIN_Y, 360,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    const iui_theme_t *theme = iui_get_theme(ui);

    static float motion_timer = 0.f;
    static bool playing = true;
    static const float duration = 1.2f;

    if (playing) {
        motion_timer += delta_time;
        if (motion_timer >= duration)
            motion_timer = 0.f;
    }

    float t = motion_timer / duration;

    static const char *easing_names[] = {"Linear", "Standard", "Decel",
                                         "Accel"};
    static uint32_t easing_sel = 1;
    iui_segmented(ui, easing_names, 4, &easing_sel);
    iui_newline(ui);

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = 50.f});
    iui_rect_t track_area = iui_box_next(ui);

    float track_y = track_area.y + track_area.height * 0.5f;
    float track_left = track_area.x + 24.f;
    float track_right = track_area.x + track_area.width - 24.f;
    float track_w = track_right - track_left;

    float eased = iui_ease(t, (enum iui_easing) easing_sel);

    iui_draw_line(ui, track_left, track_y, track_left + track_w, track_y, 4.f,
                  theme->surface_container_highest);

    if (eased > 0.01f) {
        iui_draw_line(ui, track_left, track_y, track_left + track_w * eased,
                      track_y, 4.f, theme->primary);
    }

    float ball_x = track_left + eased * track_w;
    iui_draw_circle(ui, ball_x, track_y, 12.f, theme->primary, 0, 0);
    iui_draw_circle(ui, ball_x, track_y, 5.f, theme->on_primary, 0, 0);

    iui_draw_circle(ui, track_left, track_y, 4.f, theme->outline, 0, 0);
    iui_draw_circle(ui, track_right, track_y, 4.f, theme->outline, 0, 0);
    iui_box_end(ui);

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 1, .cross = 160.f});
    iui_rect_t graph_area = iui_box_next(ui);

    float gx = graph_area.x + 20.f;
    float gy = graph_area.y + graph_area.height - 20.f;
    float gw = graph_area.width - 40.f;
    float gh = graph_area.height - 40.f;

    iui_draw_line(ui, gx + gw * 0.5f, gy, gx + gw * 0.5f, gy - gh, 1.f,
                  theme->surface_container_high);
    iui_draw_line(ui, gx, gy - gh * 0.5f, gx + gw, gy - gh * 0.5f, 1.f,
                  theme->surface_container_high);

    iui_draw_line(ui, gx, gy, gx + gw, gy, 1.f, theme->outline);
    iui_draw_line(ui, gx, gy, gx, gy - gh, 1.f, theme->outline);
    iui_draw_line(ui, gx, gy, gx + gw, gy - gh, 1.f, theme->outline_variant);

    float px = gx, py = gy;
    for (int i = 1; i <= 40; i++) {
        float st = (float) i / 40.f;
        float se = iui_ease(st, (enum iui_easing) easing_sel);
        float nx = gx + st * gw;
        float ny = gy - se * gh;
        iui_draw_line(ui, px, py, nx, ny, 2.f, theme->primary);
        px = nx;
        py = ny;
    }

    float dot_x = gx + t * gw;
    float dot_y = gy - eased * gh;
    iui_draw_circle(ui, dot_x, dot_y, 6.f, theme->tertiary, 0, 0);
    iui_box_end(ui);

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 2, .gap = 8});
    iui_box_next(ui);
    iui_text_body_medium(ui, IUI_ALIGN_CENTER, "t = %.2f", t);
    iui_box_next(ui);
    iui_text_body_medium(ui, IUI_ALIGN_CENTER, "y = %.2f", eased);
    iui_box_end(ui);

    iui_newline(ui);

    iui_box_begin(ui, &(iui_box_config_t) {.child_count = 2, .gap = 8});
    iui_box_next(ui);
    if (iui_filled_button(ui, playing ? "Pause" : "Play", IUI_ALIGN_CENTER)) {
        playing = !playing;
    }
    iui_box_next(ui);
    if (iui_outlined_button(ui, "Reset", IUI_ALIGN_CENTER)) {
        motion_timer = 0.f;
    }
    iui_box_end(ui);

    iui_end_window(ui);
}
#endif /* CONFIG_DEMO_MOTION */

#ifdef CONFIG_MODULE_INPUT
/* TextField Demo */

static void draw_textfield_window(iui_context *ui)
{
    static char basic_text[64] = "";
    static size_t basic_cursor = 0;
    static char password_text[64] = "";
    static size_t password_cursor = 0;
    static bool password_visible = false;
    static char readonly_text[64] = "Read-only content";
    static size_t readonly_cursor = 0;
    static char disabled_text[64] = "Disabled field";
    static size_t disabled_cursor = 0;
    static char validated_text[64] = "";
    static size_t validated_cursor = 0;
    /* Text selection demo state */
    static char selection_text[128] = "Select me with mouse drag";
    static iui_edit_state selection_state = {0};
    /* Scroll state for the window content */
    static iui_scroll_state scroll = {0};

    iui_begin_window(ui, "TextField", DEMO_WIN_X, DEMO_WIN_Y, 390,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    /* Scrollable content area */
    iui_scroll_begin(ui, &scroll, 0.f, 360.f);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "BASIC (FILLED)");
    {
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_FILLED,
            .placeholder = "Enter text...",
        };
        iui_textfield(ui, basic_text, sizeof(basic_text), &basic_cursor, &opts);
    }
    iui_newline(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "WITH SELECTION");
    {
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_OUTLINED,
            .placeholder = "Select with mouse or Shift+arrows",
        };
        iui_textfield_with_selection(ui, selection_text, sizeof(selection_text),
                                     &selection_state, &opts);
    }
    iui_newline(ui);

#ifdef CONFIG_MODULE_SEARCH
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "SEARCH BAR");
    {
        static char searchbar_text[256] = "";
        static size_t searchbar_cursor = 0;
        iui_search_bar_result r = iui_search_bar_ex(
            ui, searchbar_text, sizeof(searchbar_text), &searchbar_cursor,
            "Search anything...", NULL, NULL);
        if (r.submitted) {
            printf("Search submitted: %s\n", searchbar_text);
        }
        if (r.cleared) {
            printf("Search cleared\n");
        }
    }
    iui_newline(ui);
#endif /* CONFIG_MODULE_SEARCH */

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "PASSWORD FIELD");
    {
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_OUTLINED,
            .placeholder = "Enter password...",
            .trailing_icon = password_visible
                                 ? IUI_TEXTFIELD_ICON_VISIBILITY
                                 : IUI_TEXTFIELD_ICON_VISIBILITY_OFF,
            .password_mode = !password_visible,
        };
        iui_textfield_result r = iui_textfield(
            ui, password_text, sizeof(password_text), &password_cursor, &opts);
        if (r.trailing_icon_clicked) {
            password_visible = !password_visible;
        }
    }
    iui_newline(ui);

    iui_text_label_small(ui, IUI_ALIGN_LEFT, "VALIDATED FIELD");
    {
        size_t len = strlen(validated_text);
        enum iui_textfield_icon trailing = IUI_TEXTFIELD_ICON_NONE;
        if (len >= 3) {
            trailing = IUI_TEXTFIELD_ICON_CHECK;
        } else if (len > 0) {
            trailing = IUI_TEXTFIELD_ICON_ERROR;
        }
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_FILLED,
            .placeholder = "Min 3 characters...",
            .trailing_icon = trailing,
        };
        iui_textfield(ui, validated_text, sizeof(validated_text),
                      &validated_cursor, &opts);
    }
    iui_newline(ui);

    iui_divider(ui);
    iui_text_label_small(ui, IUI_ALIGN_LEFT, "STATES");

    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Read-only:");
    {
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_OUTLINED,
            .read_only = true,
        };
        iui_textfield(ui, readonly_text, sizeof(readonly_text),
                      &readonly_cursor, &opts);
    }
    iui_newline(ui);

    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Disabled:");
    {
        iui_textfield_options opts = {
            .style = IUI_TEXTFIELD_FILLED,
            .disabled = true,
        };
        iui_textfield(ui, disabled_text, sizeof(disabled_text),
                      &disabled_cursor, &opts);
    }

    iui_scroll_end(ui, &scroll);
    iui_end_window(ui);
}
#endif /* CONFIG_MODULE_INPUT - TextField demo */

#ifdef CONFIG_MODULE_NAVIGATION
/* Top App Bar Demo */

static void draw_appbar_demo_window(iui_context *ui)
{
    iui_begin_window(ui, "Top App Bar", DEMO_WIN_X, DEMO_WIN_Y, 420,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    static int appbar_tab = 0;
    static const char *appbar_tabs[] = {"Small", "Center", "Medium", "Large"};
    appbar_tab = iui_tabs(ui, appbar_tab, 4, appbar_tabs);
    iui_newline(ui);

    /* Scroll state for collapsible variants */
    static iui_scroll_state appbar_scroll = {0};

    /* Get scroll offset for collapse animation */
    float scroll_offset = appbar_scroll.scroll_y;

    if (appbar_tab == 0) {
        /* Small App Bar (64dp, fixed) */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "SMALL APP BAR (64dp)");
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Fixed height, title on left");
        iui_newline(ui);

        /* Draw app bar - returns true if nav icon clicked */
        if (iui_top_app_bar(ui, "Small Title", IUI_APPBAR_SMALL, 0))
            printf("Navigation icon clicked (Small)\n");
        /* Add action icons */
        if (iui_top_app_bar_action(ui, "search"))
            printf("Search action clicked\n");
        if (iui_top_app_bar_action(ui, "settings"))
            printf("Settings action clicked\n");

        iui_newline(ui);
        iui_text_body_small(ui, IUI_ALIGN_LEFT,
                            "Click hamburger menu or action icons");

    } else if (appbar_tab == 1) {
        /* Center-aligned App Bar (64dp, fixed) */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "CENTER-ALIGNED (64dp)");
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Fixed height, centered title");
        iui_newline(ui);

        if (iui_top_app_bar(ui, "Centered", IUI_APPBAR_CENTER, 0))
            printf("Navigation icon clicked (Center)\n");
        if (iui_top_app_bar_action(ui, "favorite"))
            printf("Favorite action clicked\n");

        iui_newline(ui);
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Title is centered in the bar");

    } else if (appbar_tab == 2) {
        /* Medium App Bar (112dp -> 64dp on scroll) */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "MEDIUM APP BAR (112dp)");
        iui_text_body_small(ui, IUI_ALIGN_LEFT,
                            "Collapses to 64dp on scroll, title at bottom");
        iui_newline(ui);

        if (iui_top_app_bar(ui, "Medium Bar", IUI_APPBAR_MEDIUM,
                            scroll_offset)) {
            printf("Navigation icon clicked (Medium)\n");
        }
        if (iui_top_app_bar_action(ui, "share"))
            printf("Share action clicked\n");
        if (iui_top_app_bar_action(ui, "bookmark"))
            printf("Bookmark action clicked\n");

        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Scroll content below:");
        iui_scroll_begin(ui, &appbar_scroll, 0.f, 180.f);
        for (int i = 0; i < 30; i++) {
            iui_text(ui, IUI_ALIGN_LEFT,
                     "Scrollable item %d - scroll to collapse", i + 1);
            iui_newline(ui);
        }
        iui_scroll_end(ui, &appbar_scroll);

    } else if (appbar_tab == 3) {
        /* Large App Bar (152dp -> 64dp on scroll) */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "LARGE APP BAR (152dp)");
        iui_text_body_small(ui, IUI_ALIGN_LEFT,
                            "Collapses to 64dp, prominent title");
        iui_newline(ui);

        if (iui_top_app_bar(ui, "Large Title", IUI_APPBAR_LARGE,
                            scroll_offset)) {
            printf("Navigation icon clicked (Large)\n");
        }
        if (iui_top_app_bar_action(ui, "add"))
            printf("Add action clicked\n");
        if (iui_top_app_bar_action(ui, "edit"))
            printf("Edit action clicked\n");
        if (iui_top_app_bar_action(ui, "delete"))
            printf("Delete action clicked\n");

        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Scroll content below:");
        iui_scroll_begin(ui, &appbar_scroll, 0.f, 150.f);
        for (int i = 0; i < 30; i++) {
            iui_text(ui, IUI_ALIGN_LEFT,
                     "Large bar item %d - scroll to see collapse", i + 1);
            iui_newline(ui);
        }
        iui_scroll_end(ui, &appbar_scroll);
    }

    iui_divider(ui);
    iui_text_body_small(ui, IUI_ALIGN_LEFT, "Scroll offset: %.0f px",
                        scroll_offset);

    iui_end_window(ui);
}
#endif /* CONFIG_MODULE_NAVIGATION */

#ifdef CONFIG_MODULE_LIST
/* List Component Demo */

static void draw_list_demo_window(iui_context *ui)
{
    iui_begin_window(ui, "Lists Demo", DEMO_WIN_X, DEMO_WIN_Y, 380,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    static int list_tab = 0;
    static const char *list_tabs[] = {"Simple", "Two-line", "Controls"};
    list_tab = iui_tabs(ui, list_tab, 3, list_tabs);
    iui_newline(ui);

    static iui_scroll_state list_scroll = {0};
    iui_scroll_begin(ui, &list_scroll, 0.f, 380.f);

    if (list_tab == 0) {
        /* Simple one-line lists */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "ONE-LINE LISTS");
        iui_newline(ui);

        if (iui_list_item_simple(ui, "Inbox", "mail"))
            printf("Inbox clicked\n");
        if (iui_list_item_simple(ui, "Starred", "star"))
            printf("Starred clicked\n");
        if (iui_list_item_simple(ui, "Sent", "send"))
            printf("Sent clicked\n");
        if (iui_list_item_simple(ui, "Drafts", "edit"))
            printf("Drafts clicked\n");

        iui_list_divider(ui);

        if (iui_list_item_simple(ui, "Settings", "settings"))
            printf("Settings clicked\n");
        if (iui_list_item_simple(ui, "Help & Feedback", "help"))
            printf("Help clicked\n");

    } else if (list_tab == 1) {
        /* Two-line lists with supporting text */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "TWO-LINE LISTS");
        iui_newline(ui);

        if (iui_list_item_two_line(ui, "John Doe", "Hey, are you free today?",
                                   "person"))
            printf("John clicked\n");
        if (iui_list_item_two_line(ui, "Jane Smith",
                                   "Meeting rescheduled to 3pm", "person"))
            printf("Jane clicked\n");
        if (iui_list_item_two_line(ui, "Project Update",
                                   "Sprint review tomorrow", "folder"))
            printf("Project clicked\n");

        iui_list_divider(ui);

        /* With trailing meta text */
        iui_list_item item = {
            .headline = "System Update",
            .supporting = "New version available",
            .leading_type = IUI_LIST_LEADING_ICON,
            .leading_icon = "refresh",
            .trailing_type = IUI_LIST_TRAILING_TEXT,
            .trailing_text = "2 min",
            .show_divider = true,
        };
        if (iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item))
            printf("Update clicked\n");

        item.headline = "Storage Alert";
        item.supporting = "Running low on space";
        item.leading_icon = "warning";
        item.trailing_text = "5 min";
        item.show_divider = false;
        if (iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item))
            printf("Storage clicked\n");

    } else if (list_tab == 2) {
        /* Lists with interactive controls */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "LISTS WITH CONTROLS");
        iui_newline(ui);

        /* Checkbox lists */
        static bool check1 = false, check2 = true, check3 = false;

        iui_list_item item = {
            .headline = "Enable notifications",
            .supporting = "Get alerts for new messages",
            .leading_type = IUI_LIST_LEADING_CHECKBOX,
            .checkbox_value = &check1,
            .show_divider = true,
        };
        iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item);

        item.headline = "Auto-sync";
        item.supporting = "Sync data automatically";
        item.checkbox_value = &check2;
        iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item);

        item.headline = "Dark mode";
        item.supporting = "Use dark color scheme";
        item.checkbox_value = &check3;
        item.show_divider = false;
        iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item);

        iui_list_divider(ui);

        /* Switch lists */
        static bool switch1 = true, switch2 = false;

        item.headline = "Wi-Fi";
        item.supporting = "Connected to HomeNetwork";
        item.leading_type = IUI_LIST_LEADING_ICON;
        item.leading_icon = "wifi";
        item.trailing_type = IUI_LIST_TRAILING_SWITCH;
        item.checkbox_value = &switch1;
        item.show_divider = true;
        iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item);

        item.headline = "Bluetooth";
        item.supporting = "Not connected";
        item.leading_icon = "bluetooth";
        item.checkbox_value = &switch2;
        item.show_divider = false;
        iui_list_item_ex(ui, IUI_LIST_TWO_LINE, &item);

        iui_list_divider(ui);

        /* Radio button lists */
        static int sort_option = 0;
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "Sort by:");
        iui_newline(ui);

        iui_list_item radio_item = {
            .headline = "Name",
            .leading_type = IUI_LIST_LEADING_RADIO,
            .radio_value = &sort_option,
            .radio_option = 0,
            .show_divider = true,
        };
        iui_list_item_ex(ui, IUI_LIST_ONE_LINE, &radio_item);

        radio_item.headline = "Date";
        radio_item.radio_option = 1;
        iui_list_item_ex(ui, IUI_LIST_ONE_LINE, &radio_item);

        radio_item.headline = "Size";
        radio_item.radio_option = 2;
        radio_item.show_divider = false;
        iui_list_item_ex(ui, IUI_LIST_ONE_LINE, &radio_item);
    }

    iui_scroll_end(ui, &list_scroll);
    iui_end_window(ui);
}
#endif /* CONFIG_MODULE_LIST */

#ifdef CONFIG_MODULE_NAVIGATION
/* Navigation Components Demo */

static void draw_navigation_demo_window(iui_context *ui)
{
    iui_begin_window(ui, "Navigation Demo", DEMO_WIN_X, DEMO_WIN_Y, 410,
                     get_demo_window_height(), IUI_WINDOW_RESIZABLE);

    static int nav_tab = 0;
    static const char *nav_tabs[] = {"Rail", "Bar", "Drawer"};
    nav_tab = iui_tabs(ui, nav_tab, 3, nav_tabs);
    iui_newline(ui);

    if (nav_tab == 0) {
        /* Navigation Rail Demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "NAVIGATION RAIL");
        iui_text_body_small(ui, IUI_ALIGN_LEFT,
                            "Vertical navigation for tablet/desktop");
        iui_newline(ui);

        static iui_nav_rail_state rail_state = {0};

#ifdef CONFIG_MODULE_BASIC
        /* Toggle button for expanded state */
        if (iui_button(ui, rail_state.expanded ? "Collapse" : "Expand",
                       IUI_ALIGN_LEFT)) {
            iui_nav_rail_toggle(&rail_state);
        }
#else
        iui_text(ui, IUI_ALIGN_LEFT,
                 "Rail: %s (MODULE_BASIC required for toggle)",
                 rail_state.expanded ? "Expanded" : "Collapsed");
#endif
        iui_newline(ui);

        /* Get available area.
         * iui_get_remaining_height() reads the live window clip rect so it
         * correctly accounts for title bar, padding, and decorations —
         * unlike (win.y + win.height - layout.y) which uses the outer frame.
         *
         * Snap rail_height to the nearest row_h multiple so skip_rows
         * advances the cursor exactly to the rail bottom with 2 rows
         * guaranteed for the status text below: given
         *   avail_rows = floor((avail - 8) / row_h)
         *   rail_height = (avail_rows - 2) * row_h - 8
         * the cursor after skip_rows equals (avail_rows-2)*row_h, leaving
         * at least 2*row_h + remainder between cursor and clip_bottom. */
        iui_rect_t layout = iui_get_layout_rect(ui);
        float rail_x = layout.x;
        float rail_y = layout.y + 8.f;
        float row_h = fmaxf(1.f, layout.height); /* guard against uninit ctx */
        float avail = iui_get_remaining_height(ui);
        int avail_rows = (int) floorf((avail - 8.f) / row_h);
        float rail_height = fmaxf(80.f, (float) (avail_rows - 2) * row_h - 8.f);

        iui_nav_rail_begin(ui, &rail_state, rail_x, rail_y, rail_height);

        /* FAB at top */
        if (iui_nav_rail_fab(ui, &rail_state, "edit")) {
            printf("Rail FAB clicked\n");
        }

        /* Navigation items */
        if (iui_nav_rail_item(ui, &rail_state, "home", "Home", 0))
            printf("Selected Home\n");
        if (iui_nav_rail_item(ui, &rail_state, "search", "Search", 1))
            printf("Selected Search\n");
        if (iui_nav_rail_item(ui, &rail_state, "favorite", "Favorites", 2))
            printf("Selected Favorites\n");
        if (iui_nav_rail_item(ui, &rail_state, "settings", "Settings", 3))
            printf("Selected Settings\n");

        iui_nav_rail_end(ui, &rail_state);

        /* Advance layout cursor past the rail.
         * rail_y starts 8dp below layout.y, so total vertical span from
         * layout.y is (8 + rail_height).  Use ceilf to avoid landing
         * inside the rail's last row. */
        /* Subtract a small epsilon before ceiling to prevent FP rounding
         * (e.g. 8.f + (N*row_h - 8.f) may compute as N*row_h + epsilon)
         * from silently adding an extra row.  Scale with row_h: at large
         * values (row_h~100, N~1000) the ULP at N*row_h is ~0.008f, so a
         * fixed 0.01f would be unreliable; 1e-4*row_h stays below 1 ULP. */
        float eps = fmaxf(0.01f, row_h * 1e-4f);
        int skip_rows = (int) ceilf((8.f + rail_height - eps) / row_h);
        for (int i = 0; i < skip_rows; i++)
            iui_newline(ui);
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Selected: %d",
                            rail_state.selected);
        iui_newline(ui);
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "State: %s",
                            rail_state.expanded ? "Expanded" : "Collapsed");

    } else if (nav_tab == 1) {
        /* Navigation Bar Demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "NAVIGATION BAR");
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Bottom navigation for mobile");
        iui_newline(ui);

        static iui_nav_bar_state bar_state = {0};

        /* Content area simulation */
        iui_text_body_medium(ui, IUI_ALIGN_CENTER, "Content Area");
        for (int i = 0; i < 6; i++)
            iui_newline(ui);

        const char *section_names[] = {"Home", "Search", "Favorites", "Profile",
                                       "Settings"};
        iui_text_body_large(ui, IUI_ALIGN_CENTER, "Current: %s",
                            section_names[bar_state.selected]);
        iui_newline(ui);
        iui_text_body_small(ui, IUI_ALIGN_CENTER,
                            "Tap items below to navigate");

        /* Push down to bottom */
        for (int i = 0; i < 5; i++)
            iui_newline(ui);

        /* Get position for bottom bar */
        iui_rect_t bar_layout = iui_get_layout_rect(ui);
        float bar_x = bar_layout.x;
        float bar_y = bar_layout.y + 60.f;
        float bar_width = bar_layout.width;

        iui_nav_bar_begin(ui, &bar_state, bar_x, bar_y, bar_width, 5);

        iui_nav_bar_item(ui, &bar_state, "home", "Home", 0);
        iui_nav_bar_item(ui, &bar_state, "search", "Search", 1);
        iui_nav_bar_item(ui, &bar_state, "favorite", "Favorites", 2);
        iui_nav_bar_item(ui, &bar_state, "person", "Profile", 3);
        iui_nav_bar_item(ui, &bar_state, "settings", "Settings", 4);

        iui_nav_bar_end(ui, &bar_state);

    } else if (nav_tab == 2) {
        /* Navigation Drawer Demo */
        iui_text_label_small(ui, IUI_ALIGN_LEFT, "NAVIGATION DRAWER");
        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Slide-out side panel");
        iui_newline(ui);

        static iui_nav_drawer_state drawer_state = {.modal = true};

#ifdef CONFIG_MODULE_BASIC
        /* Button to open drawer */
        if (iui_button(ui, "Open Drawer", IUI_ALIGN_LEFT))
            iui_nav_drawer_open(&drawer_state);
#else
        iui_text(ui, IUI_ALIGN_LEFT,
                 "Drawer: %s (MODULE_BASIC required for button)",
                 drawer_state.open ? "Open" : "Closed");
#endif
        iui_newline(ui);

        /* Show current selection */
        const char *drawer_items[] = {"Inbox", "Starred", "Sent", "Drafts",
                                      "Trash"};
        iui_text_body_medium(ui, IUI_ALIGN_LEFT, "Selected: %s",
                             drawer_items[drawer_state.selected]);
        iui_newline(ui);

        /* Modal toggle */
        static bool drawer_modal = true;
        if (iui_switch(ui, "Modal (click outside to close)", &drawer_modal,
                       NULL, NULL)) {
            drawer_state.modal = drawer_modal;
        }
        iui_newline(ui);

        iui_text_body_small(ui, IUI_ALIGN_LEFT, "Animation progress: %.2f",
                            drawer_state.anim_progress);

        /* Get position for drawer (left edge of window) */
        iui_rect_t drawer_layout = iui_get_layout_rect(ui);
        float drawer_x = drawer_layout.x - 8.f;
        float drawer_y = drawer_layout.y - 200.f;
        float drawer_height = 400.f;

        if (iui_nav_drawer_begin(ui, &drawer_state, drawer_x, drawer_y,
                                 drawer_height)) {
            iui_nav_drawer_item(ui, &drawer_state, "inbox", "Inbox", 0);
            iui_nav_drawer_item(ui, &drawer_state, "star", "Starred", 1);
            iui_nav_drawer_item(ui, &drawer_state, "send", "Sent", 2);
            iui_nav_drawer_item(ui, &drawer_state, "drafts", "Drafts", 3);
            iui_nav_drawer_item(ui, &drawer_state, "delete", "Trash", 4);

            iui_nav_drawer_end(ui, &drawer_state);
        }
    }

    iui_end_window(ui);
}
#endif /* CONFIG_MODULE_NAVIGATION navigation demo */

/* Demo Window Layout Constants
 *
 * These constants mirror libiui's internal layout metrics.
 */
/* Derived from : font_height = 21, row_height = font_height * 1.5,
                padding = 8 */

#define DEMO_FONT_HEIGHT 21.f
#define DEMO_ROW_HEIGHT (DEMO_FONT_HEIGHT * 1.5f) /* 31.5 */
#define DEMO_PADDING 8.f
#define DEMO_DIVIDER_HEIGHT 4.f

/* Grid layout (matches iui_grid_begin parameters in main loop) */
#define DEMO_TOGGLE_CELL_HEIGHT 26.f
#define DEMO_TOGGLE_GRID_PAD 4.f
#define DEMO_TOGGLE_COLS 2

#define DEMO_ACTION_CELL_HEIGHT 26.f
#define DEMO_ACTION_GRID_PAD 4.f
#define DEMO_ACTION_COLS 2

/* Demo Window Height Calculation
 *
 * The Demo window height depends on the number of
 * enabled feature toggles.
 * Each toggle occupies a grid cell,
 * arranged in 2 columns.
 */

/* Count enabled demo toggles at runtime.
 * Must mirror the toggle grid in example_frame exactly.
 * The entire grid is gated on CONFIG_MODULE_INPUT; entries within follow
 * the same preprocessor conditions as their iui_switch() calls. */
static int get_demo_toggle_count(void)
{
    int count = 0;
#ifdef CONFIG_MODULE_INPUT
#ifdef CONFIG_DEMO_CALCULATOR
    count++;
#endif
#ifdef CONFIG_DEMO_CLOCK
    count++;
#endif
#ifdef CONFIG_DEMO_VECTOR
    count++;
#endif
#ifdef CONFIG_DEMO_NYANCAT
    count++;
#endif
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_CONTAINER)
    count++; /* Components */
#endif
#ifdef CONFIG_DEMO_THEME
    count++; /* Colors */
#endif
    count++; /* TextField - unconditional within MODULE_INPUT */
#ifdef CONFIG_MODULE_NAVIGATION
    count += 2; /* App Bar + Nav */
#endif
#ifdef CONFIG_MODULE_LIST
    count++; /* Lists */
#endif
#ifdef CONFIG_DEMO_MOTION
    count++;
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    count++;
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    count++; /* Font Editor */
#endif
#ifdef CONFIG_FEATURE_THEME
    count++; /* Dark mode */
#endif
#endif /* CONFIG_MODULE_INPUT */
    return count;
}

/* Count action buttons based on enabled modules */
static int get_demo_action_count(void)
{
    int count = 1; /* Snackbar - always present */
#ifdef CONFIG_MODULE_OVERLAY
    count += 3; /* Dialog Demo, Search, Fullscreen */
#endif
#ifdef CONFIG_MODULE_PICKER
    count += 2; /* Date Picker, Time Picker */
#endif
    return count;
}

/* Calculate Demo window height based on enabled toggles and actions */
static float get_demo_window_height(void)
{
    /* Window structure (top to bottom):
     * 1. Title bar area: padding + row_height + padding
     * 2. "Welcome..." text: row_height
     * 3. newline: row_height
     * 4. Divider
     * 5. Menu bar (File, Edit): row_height
     * 6. newline: row_height
     * 7. Toggle grid: DYNAMIC (based on enabled demos)
     * 8. Action buttons grid: DYNAMIC (based on enabled modules)
     * 9. FPS text: row_height
     * 10. Bottom padding
     */
    const float title_area = DEMO_PADDING + DEMO_ROW_HEIGHT + DEMO_PADDING;
    const float content_rows = DEMO_ROW_HEIGHT * 5; /* items 2-6, 9 */
    const float divider = DEMO_DIVIDER_HEIGHT;
    const float bottom_pad = DEMO_PADDING;

    /* Dynamic toggle grid height */
    int toggle_count = get_demo_toggle_count();
    int toggle_rows =
        (toggle_count + DEMO_TOGGLE_COLS - 1) / DEMO_TOGGLE_COLS; /* ceil div */
    float toggle_grid =
        toggle_rows * (DEMO_TOGGLE_CELL_HEIGHT + DEMO_TOGGLE_GRID_PAD);

    /* Dynamic action grid height */
    int action_count = get_demo_action_count();
    int action_rows =
        (action_count + DEMO_ACTION_COLS - 1) / DEMO_ACTION_COLS; /* ceil div */
    float action_grid =
        action_rows * (DEMO_ACTION_CELL_HEIGHT + DEMO_ACTION_GRID_PAD);

    return title_area + content_rows + divider + toggle_grid + action_grid +
           bottom_pad;
}

/* Main loop state structure.
 * For Emscripten builds, emscripten_set_main_loop_arg() requires all state
 * to be passed via a void pointer. This struct holds everything that was
 * previously local to main(). */

typedef struct {
    /* Core handles */
    iui_port_ctx *port;
    iui_context *ui;
    void *iui_buffer;

    /* Window dimensions */
    int window_w, window_h;

    /* Demo visibility toggles */
#ifdef CONFIG_DEMO_CALCULATOR
    bool show_calculator;
#endif
#ifdef CONFIG_DEMO_CLOCK
    bool show_clock;
#endif
#ifdef CONFIG_DEMO_VECTOR
    bool show_vector_demo;
    float vector_progress;
#endif
#ifdef CONFIG_DEMO_NYANCAT
    bool show_nyancat;
#endif
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT) && \
    defined(CONFIG_MODULE_CONTAINER)
    bool show_iui_components;
#endif
#ifdef CONFIG_DEMO_THEME
    bool show_color_scheme;
#endif
#ifdef CONFIG_MODULE_INPUT
    bool show_textfield_demo;
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    bool show_appbar_demo;
    bool show_navigation_demo;
#endif
#ifdef CONFIG_MODULE_LIST
    bool show_list_demo;
#endif
#ifdef CONFIG_DEMO_MOTION
    bool show_motion;
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    bool show_accessibility;
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    bool show_font_editor;
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    pixelwall_state_t pixelwall;
    bool show_pixelwall;
#endif
#ifdef CONFIG_FEATURE_THEME
    bool dark_mode;
#endif
#ifdef CONFIG_DEMO_CALCULATOR
    calculator_state_t calc;
#endif

    /* Snackbar state */
    iui_snackbar_state snackbar;
    int snackbar_count;
    char snackbar_msg[64];

#ifdef CONFIG_MODULE_OVERLAY
    /* Menu and dialog state */
    iui_menu_state file_menu;
    iui_menu_state edit_menu;
    iui_dialog_state confirm_dialog;
    iui_fullscreen_dialog_state fullscreen_dialog;
    iui_search_view_state search_view;
    int dropdown_selected;
#endif
#ifdef CONFIG_MODULE_PICKER
    /* Picker state */
    iui_date_picker_state date_picker;
    iui_time_picker_state time_picker;
#endif

    /* FPS tracking */
    float fps_display;
    float fps_accumulator;
    int fps_frame_count;

    /* Running flag (for native loop exit) */
    bool running;
} demo_state_t;

/* Dropdown options (constant, not per-state) */
#ifdef CONFIG_MODULE_OVERLAY
static const char *dropdown_options[] = {"Option 1", "Option 2", "Option 3",
                                         "Option 4"};
#endif

#ifdef CONFIG_MODULE_BASIC
/* Helper to make demo windows exclusive (only one visible at a time).
 * Clears all show_* flags except the one being activated.
 * Only used when control panel is active (requires MODULE_BASIC).
 */
static void demo_close_other_windows(demo_state_t *state, bool *keep_open)
{
#ifdef CONFIG_DEMO_CALCULATOR
    if (&state->show_calculator != keep_open)
        state->show_calculator = false;
#endif
#ifdef CONFIG_DEMO_CLOCK
    if (&state->show_clock != keep_open)
        state->show_clock = false;
#endif
#ifdef CONFIG_DEMO_VECTOR
    if (&state->show_vector_demo != keep_open)
        state->show_vector_demo = false;
#endif
#ifdef CONFIG_DEMO_NYANCAT
    if (&state->show_nyancat != keep_open)
        state->show_nyancat = false;
#endif
#if defined(CONFIG_MODULE_INPUT) && defined(CONFIG_MODULE_CONTAINER)
    if (&state->show_iui_components != keep_open)
        state->show_iui_components = false;
#endif
#ifdef CONFIG_DEMO_THEME
    if (&state->show_color_scheme != keep_open)
        state->show_color_scheme = false;
#endif
#ifdef CONFIG_MODULE_INPUT
    if (&state->show_textfield_demo != keep_open)
        state->show_textfield_demo = false;
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    if (&state->show_appbar_demo != keep_open)
        state->show_appbar_demo = false;
    if (&state->show_navigation_demo != keep_open)
        state->show_navigation_demo = false;
#endif
#ifdef CONFIG_MODULE_LIST
    if (&state->show_list_demo != keep_open)
        state->show_list_demo = false;
#endif
#ifdef CONFIG_DEMO_MOTION
    if (&state->show_motion != keep_open)
        state->show_motion = false;
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    if (&state->show_accessibility != keep_open)
        state->show_accessibility = false;
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    if (&state->show_font_editor != keep_open)
        state->show_font_editor = false;
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    if (&state->show_pixelwall != keep_open)
        state->show_pixelwall = false;
#endif
}
#endif /* CONFIG_MODULE_BASIC */

/* Frame callback.
 * Processes a single frame. Called from native while loop in main()
 * or from emscripten_set_main_loop_arg(). */

static void example_frame(void *arg)
{
    demo_state_t *state = (demo_state_t *) arg;
    iui_port_ctx *port = state->port;
    iui_context *ui = state->ui;

    /* Poll events - for Emscripten this always returns true */
    if (!g_iui_port.poll_events(port)) {
        state->running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    /* Get input from port and apply to UI */
    iui_port_input input;
    g_iui_port.get_input(port, &input);
    iui_port_apply_input(ui, &input);

#ifdef CONFIG_DEMO_CALCULATOR
    /* Calculator keyboard handling - simplified, key events handled by port */
    (void) (state->show_calculator && input.key != 0);
#endif

    /* Get timing */
    float delta_time = g_iui_port.get_delta_time(port);

    /* Update FPS counter */
    state->fps_accumulator += delta_time;
    state->fps_frame_count++;
    if (state->fps_accumulator >= 1.f) {
        state->fps_display =
            (float) state->fps_frame_count / state->fps_accumulator;
        state->fps_accumulator = 0.f;
        state->fps_frame_count = 0;
    }

    /* Update animations */
#ifdef CONFIG_DEMO_NYANCAT
    update_nyancat_animation(delta_time);
#endif
#ifdef CONFIG_DEMO_VECTOR
    state->vector_progress += delta_time * 0.2f;
    if (state->vector_progress > 1.f)
        state->vector_progress -= 1.f;
#endif

    /* Update window size */
    g_iui_port.get_window_size(port, &state->window_w, &state->window_h);

    /* Begin rendering */
    g_iui_port.begin_frame(port);
    iui_begin_frame(ui, delta_time);

    /* Main Demo Window - requires MODULE_BASIC for control panel */
#ifdef CONFIG_MODULE_BASIC
    iui_begin_window(ui, "libIUI Demo", 30, 30, 340, get_demo_window_height(),
                     IUI_WINDOW_RESIZABLE);

    iui_text(ui, IUI_ALIGN_CENTER, "Welcome to immediate-mode UI");
    iui_newline(ui);
    iui_divider(ui);

#ifdef CONFIG_MODULE_OVERLAY
    /* Menu bar */
    iui_sizing_t menu_sizes[] = {IUI_FIXED(60), IUI_FIXED(60)};
    iui_box_begin(ui, &(iui_box_config_t) {
                          .child_count = 2, .sizes = menu_sizes, .gap = 4});
    iui_rect_t file_btn_slot = iui_box_next(ui);
    if (iui_text_button(ui, "File", IUI_ALIGN_CENTER)) {
        iui_menu_close(&state->edit_menu);
        iui_menu_open(&state->file_menu, "file_menu", file_btn_slot.x,
                      file_btn_slot.y + file_btn_slot.height);
    }
    iui_rect_t edit_btn_slot = iui_box_next(ui);
    if (iui_text_button(ui, "Edit", IUI_ALIGN_CENTER)) {
        iui_menu_close(&state->file_menu);
        iui_menu_open(&state->edit_menu, "edit_menu", edit_btn_slot.x,
                      edit_btn_slot.y + edit_btn_slot.height);
    }
    iui_box_end(ui);
#endif /* CONFIG_MODULE_OVERLAY */
    iui_newline(ui);

#ifdef CONFIG_MODULE_INPUT
    /* Toggle switches - requires MODULE_INPUT
     * Windows are exclusive: only one demo window visible at a time.
     */
    const float toggle_grid_pad = 4.f;
    const float toggle_cell_w =
        (iui_get_layout_rect(ui).width - toggle_grid_pad) * 0.5f;
    iui_grid_begin(ui, 2, toggle_cell_w, 26, toggle_grid_pad);
#ifdef CONFIG_DEMO_CALCULATOR
    if (iui_switch(ui, "Calculator", &state->show_calculator, NULL, NULL) &&
        state->show_calculator)
        demo_close_other_windows(state, &state->show_calculator);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_CLOCK
    if (iui_switch(ui, "Analog Clock", &state->show_clock, NULL, NULL) &&
        state->show_clock)
        demo_close_other_windows(state, &state->show_clock);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_VECTOR
    if (iui_switch(ui, "Vector", &state->show_vector_demo, NULL, NULL) &&
        state->show_vector_demo)
        demo_close_other_windows(state, &state->show_vector_demo);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_NYANCAT
    if (iui_switch(ui, "Nyan Cat", &state->show_nyancat, NULL, NULL) &&
        state->show_nyancat)
        demo_close_other_windows(state, &state->show_nyancat);
    iui_grid_next(ui);
#endif
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT) && \
    defined(CONFIG_MODULE_CONTAINER)
    if (iui_switch(ui, "Components", &state->show_iui_components, NULL, NULL) &&
        state->show_iui_components)
        demo_close_other_windows(state, &state->show_iui_components);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_THEME
    if (iui_switch(ui, "Colors", &state->show_color_scheme, NULL, NULL) &&
        state->show_color_scheme)
        demo_close_other_windows(state, &state->show_color_scheme);
    iui_grid_next(ui);
#endif
    if (iui_switch(ui, "TextField", &state->show_textfield_demo, NULL, NULL) &&
        state->show_textfield_demo)
        demo_close_other_windows(state, &state->show_textfield_demo);
    iui_grid_next(ui);
#ifdef CONFIG_MODULE_NAVIGATION
    if (iui_switch(ui, "App Bar", &state->show_appbar_demo, NULL, NULL) &&
        state->show_appbar_demo)
        demo_close_other_windows(state, &state->show_appbar_demo);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_MODULE_LIST
    if (iui_switch(ui, "Lists", &state->show_list_demo, NULL, NULL) &&
        state->show_list_demo)
        demo_close_other_windows(state, &state->show_list_demo);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    if (iui_switch(ui, "Nav", &state->show_navigation_demo, NULL, NULL) &&
        state->show_navigation_demo)
        demo_close_other_windows(state, &state->show_navigation_demo);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_MOTION
    if (iui_switch(ui, "Motion", &state->show_motion, NULL, NULL) &&
        state->show_motion)
        demo_close_other_windows(state, &state->show_motion);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    if (iui_switch(ui, "A11y", &state->show_accessibility, NULL, NULL) &&
        state->show_accessibility)
        demo_close_other_windows(state, &state->show_accessibility);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    if (iui_switch(ui, "Font Editor", &state->show_font_editor, NULL, NULL) &&
        state->show_font_editor)
        demo_close_other_windows(state, &state->show_font_editor);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    if (iui_switch(ui, "Pixel wall", &state->show_pixelwall, NULL, NULL) &&
        state->show_pixelwall)
        demo_close_other_windows(state, &state->show_pixelwall);
    iui_grid_next(ui);
#endif
#ifdef CONFIG_FEATURE_THEME
    if (iui_switch(ui, "Dark mode", &state->dark_mode, NULL, NULL)) {
        iui_set_theme(ui,
                      state->dark_mode ? iui_theme_dark() : iui_theme_light());
    }
#endif
    iui_grid_end(ui);
#endif /* CONFIG_MODULE_INPUT */

    /* Action buttons */
    const float action_grid_pad = 4.f;
    const float action_cell_w =
        (iui_get_layout_rect(ui).width - action_grid_pad) * 0.5f;
    iui_grid_begin(ui, 2, action_cell_w, 26, action_grid_pad);
#ifdef CONFIG_MODULE_OVERLAY
    if (iui_filled_button(ui, "Dialog Demo", IUI_ALIGN_CENTER)) {
        iui_dialog_show(&state->confirm_dialog, "Confirm Action",
                        "Are you sure you want to proceed?", "Confirm");
    }
#endif
    iui_grid_next(ui);
    if (iui_outlined_button(ui, "Snackbar", IUI_ALIGN_CENTER)) {
        state->snackbar_count++;
        snprintf(state->snackbar_msg, sizeof(state->snackbar_msg),
                 "Message #%d shown!", state->snackbar_count);
        iui_snackbar_show(&state->snackbar, state->snackbar_msg, 3.f, NULL);
    }
    iui_grid_next(ui);
#ifdef CONFIG_MODULE_PICKER
    if (iui_tonal_button(ui, "Date Picker", IUI_ALIGN_CENTER)) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        iui_date_picker_show(&state->date_picker, t->tm_year + 1900,
                             t->tm_mon + 1, t->tm_mday);
    }
    iui_grid_next(ui);
    if (iui_tonal_button(ui, "Time Picker", IUI_ALIGN_CENTER)) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        iui_time_picker_show(&state->time_picker, t->tm_hour, t->tm_min, false);
    }
#endif
#ifdef CONFIG_MODULE_OVERLAY
    iui_grid_next(ui);
    if (iui_text_button(ui, "Search", IUI_ALIGN_CENTER))
        iui_search_view_open(&state->search_view);
    iui_grid_next(ui);
    if (iui_text_button(ui, "Fullscreen", IUI_ALIGN_CENTER)) {
        iui_fullscreen_dialog_open(&state->fullscreen_dialog,
                                   "Full-Screen Dialog");
    }
#endif
    iui_grid_end(ui);

    iui_text(ui, IUI_ALIGN_RIGHT, "FPS: %.1f", state->fps_display);

    iui_end_window(ui);
#endif /* CONFIG_MODULE_BASIC - end main control panel */

    /* Demo windows - each guarded by its own config */
#ifdef CONFIG_DEMO_CALCULATOR
    if (state->show_calculator)
        draw_calculator_window(ui, &state->calc);
#endif
#ifdef CONFIG_DEMO_CLOCK
    if (state->show_clock)
        draw_clock_window(ui, port);
#endif
#ifdef CONFIG_DEMO_VECTOR
    if (state->show_vector_demo)
        draw_vector_demo_window(ui, state->vector_progress);
#endif
#ifdef CONFIG_DEMO_NYANCAT
    if (state->show_nyancat)
        draw_nyancat_window(ui, port);
#endif
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT) && \
    defined(CONFIG_MODULE_CONTAINER)
    if (state->show_iui_components)
        draw_iui_components_window(ui, delta_time);
#endif
#ifdef CONFIG_DEMO_THEME
    if (state->show_color_scheme)
        draw_color_scheme_window(ui, port);
#endif
#ifdef CONFIG_DEMO_MOTION
    if (state->show_motion)
        draw_motion_window(ui, delta_time);
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    if (state->show_accessibility)
        draw_accessibility_window(ui);
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    if (state->show_font_editor)
        draw_font_editor_window(ui);
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    if (state->show_pixelwall)
        draw_pixelwall_window(ui, port, &state->pixelwall, delta_time,
                              get_demo_window_height());
#endif
#ifdef CONFIG_MODULE_INPUT
    if (state->show_textfield_demo)
        draw_textfield_window(ui);
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    if (state->show_appbar_demo)
        draw_appbar_demo_window(ui);
#endif
#ifdef CONFIG_MODULE_LIST
    if (state->show_list_demo)
        draw_list_demo_window(ui);
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    if (state->show_navigation_demo)
        draw_navigation_demo_window(ui);
#endif

#ifdef CONFIG_MODULE_OVERLAY
    /* Menus */
    if (iui_menu_begin(ui, &state->file_menu, NULL)) {
        if (iui_menu_add_item(ui, &state->file_menu,
                              &(iui_menu_item) {
                                  .text = "New",
                                  .leading_icon = "doc",
                                  .trailing_text = "Ctrl+N",
                              })) {
            printf("File > New clicked\n");
            iui_menu_close(&state->file_menu);
        }
        if (iui_menu_add_item(ui, &state->file_menu,
                              &(iui_menu_item) {
                                  .text = "Open...",
                                  .leading_icon = "folder",
                                  .trailing_text = "Ctrl+O",
                              })) {
            printf("File > Open clicked\n");
            iui_menu_close(&state->file_menu);
        }
        if (iui_menu_add_item(ui, &state->file_menu,
                              &(iui_menu_item) {
                                  .text = "Save",
                                  .leading_icon = "save",
                                  .trailing_text = "Ctrl+S",
                              })) {
            printf("File > Save clicked\n");
            iui_menu_close(&state->file_menu);
        }
        iui_menu_add_item(ui, &state->file_menu,
                          &(iui_menu_item) {.is_divider = true});
        iui_menu_add_item(
            ui, &state->file_menu,
            &(iui_menu_item) {.text = "Export", .disabled = true});
        iui_menu_add_item(ui, &state->file_menu,
                          &(iui_menu_item) {.is_divider = true});
        if (iui_menu_add_item(ui, &state->file_menu,
                              &(iui_menu_item) {
                                  .text = "Exit",
                                  .trailing_text = "Alt+F4",
                              })) {
            printf("File > Exit clicked\n");
            iui_menu_close(&state->file_menu);
        }
        iui_menu_end(ui, &state->file_menu);
    }

    if (iui_menu_begin(ui, &state->edit_menu, NULL)) {
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Undo",
                                  .leading_icon = "undo",
                                  .trailing_text = "Ctrl+Z",
                              })) {
            printf("Edit > Undo clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Redo",
                                  .leading_icon = "redo",
                                  .trailing_text = "Ctrl+Y",
                              })) {
            printf("Edit > Redo clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        iui_menu_add_item(ui, &state->edit_menu,
                          &(iui_menu_item) {.is_divider = true});
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Cut",
                                  .trailing_text = "Ctrl+X",
                              })) {
            printf("Edit > Cut clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Copy",
                                  .trailing_text = "Ctrl+C",
                              })) {
            printf("Edit > Copy clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Paste",
                                  .trailing_text = "Ctrl+V",
                              })) {
            printf("Edit > Paste clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        iui_menu_add_item(ui, &state->edit_menu,
                          &(iui_menu_item) {.is_gap = true});
        if (iui_menu_add_item(ui, &state->edit_menu,
                              &(iui_menu_item) {
                                  .text = "Select All",
                                  .trailing_text = "Ctrl+A",
                              })) {
            printf("Edit > Select All clicked\n");
            iui_menu_close(&state->edit_menu);
        }
        iui_menu_end(ui, &state->edit_menu);
    }

    /* Dialog */
    {
        int result =
            iui_dialog(ui, &state->confirm_dialog, (float) state->window_w,
                       (float) state->window_h);
        if (result >= 0) {
            if (result == 0) {
                printf("Dialog: Cancel clicked\n");
            } else if (result == 1) {
                printf("Dialog: Confirm clicked\n");
            }
        }
    }

    /* Full-Screen Dialog */
    if (iui_fullscreen_dialog_begin(ui, &state->fullscreen_dialog,
                                    (float) state->window_w,
                                    (float) state->window_h)) {
        if (iui_fullscreen_dialog_action(ui, &state->fullscreen_dialog,
                                         "Save")) {
            printf("Full-Screen Dialog: Save clicked\n");
            iui_fullscreen_dialog_close(&state->fullscreen_dialog);
        }
        /* Content area - demonstrate dropdown */
        iui_dropdown_options opts = {
            .options = dropdown_options,
            .option_count = 4,
            .selected_index = &state->dropdown_selected,
            .label = "Select Option",
            .helper_text = "Choose one of the options above",
            .disabled = false,
        };
        if (iui_dropdown(ui, &opts)) {
            printf("Dropdown selected: %s\n",
                   dropdown_options[state->dropdown_selected]);
        }
        iui_fullscreen_dialog_end(ui, &state->fullscreen_dialog);
    }

    /* Search View */
    if (iui_search_view_begin(ui, &state->search_view, (float) state->window_w,
                              (float) state->window_h, "Search...")) {
        /* Add search suggestions */
        if (iui_search_view_suggestion(ui, &state->search_view, "search",
                                       "Recent search 1")) {
            printf("Search suggestion 1 selected\n");
            iui_search_view_close(&state->search_view);
        }
        if (iui_search_view_suggestion(ui, &state->search_view, "search",
                                       "Recent search 2")) {
            printf("Search suggestion 2 selected\n");
            iui_search_view_close(&state->search_view);
        }
        if (iui_search_view_suggestion(ui, &state->search_view, "star",
                                       "Saved item")) {
            printf("Saved item selected\n");
            iui_search_view_close(&state->search_view);
        }
        iui_search_view_end(ui, &state->search_view);
    }
#endif /* CONFIG_MODULE_OVERLAY */

#ifdef CONFIG_MODULE_PICKER
    /* Pickers */
    if (iui_date_picker(ui, &state->date_picker, (float) state->window_w,
                        (float) state->window_h)) {
        printf("Date selected: %04d-%02d-%02d\n", state->date_picker.year,
               state->date_picker.month, state->date_picker.day);
    }

    if (iui_time_picker(ui, &state->time_picker, (float) state->window_w,
                        (float) state->window_h)) {
        if (state->time_picker.use_24h) {
            printf("Time selected: %02d:%02d\n", state->time_picker.hour,
                   state->time_picker.minute);
        } else {
            printf("Time selected: %02d:%02d %s\n", state->time_picker.hour,
                   state->time_picker.minute,
                   state->time_picker.is_pm ? "PM" : "AM");
        }
    }
#endif /* CONFIG_MODULE_PICKER */

    if (iui_snackbar(ui, &state->snackbar, (float) state->window_w,
                     (float) state->window_h)) {
        printf("Snackbar action clicked!\n");
    }

    /* End rendering */
    iui_end_frame(ui);
    g_iui_port.end_frame(port);
}

/* Main Entry Point */

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    /* Demo visibility default based on module availability.
     * Auto-display demos when control panel unavailable:
     * - MODULE_BASIC: required for control panel window
     * - MODULE_INPUT: required for toggle switches
     */
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT)
#define DEMO_DEFAULT_VIS false
#else
#define DEMO_DEFAULT_VIS true
#endif

    /* Initialize demo state */
    static demo_state_t state = {0};

    /* Initialize platform backend */
    state.port = g_iui_port.init(800, 600, "libiui Example (Port)");
    if (!state.port) {
        fprintf(stderr, "Failed to initialize port\n");
        return 1;
    }

    g_iui_port.configure(state.port);

    /* Allocate UI buffer */
    state.iui_buffer = malloc(iui_min_memory_size());
    if (!state.iui_buffer) {
        fprintf(stderr, "Failed to allocate UI buffer\n");
        g_iui_port.shutdown(state.port);
        return 1;
    }

    /* Get renderer callbacks from port */
    iui_renderer_t renderer = g_iui_port.get_renderer_callbacks(state.port);
    const iui_vector_t *vector = g_iui_port.get_vector_callbacks(state.port);

    /* Initialize libiui */
    iui_config_t config = {
        .buffer = state.iui_buffer,
        .font_height = DEMO_FONT_HEIGHT,
        .renderer = renderer,
        .vector = vector,
    };

    state.ui = iui_init(&config);
    if (!state.ui) {
        fprintf(stderr, "Failed to initialize libiui\n");
        free(state.iui_buffer);
        g_iui_port.shutdown(state.port);
        return 1;
    }

    /* Get window dimensions */
    g_iui_port.get_window_size(state.port, &state.window_w, &state.window_h);

    /* Initialize demo visibility toggles */
#ifdef CONFIG_DEMO_CALCULATOR
    state.show_calculator = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_CLOCK
    state.show_clock = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_VECTOR
    state.show_vector_demo = DEMO_DEFAULT_VIS;
    state.vector_progress = 0.f;
#endif
#ifdef CONFIG_DEMO_NYANCAT
    state.show_nyancat = DEMO_DEFAULT_VIS;
#endif
#if defined(CONFIG_MODULE_BASIC) && defined(CONFIG_MODULE_INPUT) && \
    defined(CONFIG_MODULE_CONTAINER)
    state.show_iui_components = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_THEME
    state.show_color_scheme = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_MODULE_INPUT
    state.show_textfield_demo = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_MODULE_NAVIGATION
    state.show_appbar_demo = DEMO_DEFAULT_VIS;
    state.show_navigation_demo = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_MODULE_LIST
    state.show_list_demo = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_MOTION
    state.show_motion = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_ACCESSIBILITY
    state.show_accessibility = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_FONT_EDITOR
    state.show_font_editor = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    state.show_pixelwall = DEMO_DEFAULT_VIS;
#endif
#ifdef CONFIG_FEATURE_THEME
    state.dark_mode = true;
#endif
#ifdef CONFIG_DEMO_CALCULATOR
    calc_init(&state.calc);
#endif
#ifdef CONFIG_DEMO_PIXELWALL
    pixelwall_init(&state.pixelwall);
#endif

#ifdef CONFIG_FEATURE_THEME
    /* Set initial theme */
    iui_set_theme(state.ui, iui_theme_dark());
#endif

    /* Initialize FPS tracking */
    state.fps_display = 0.f;
    state.fps_accumulator = 0.f;
    state.fps_frame_count = 0;

    /* Mark as running */
    state.running = true;

#ifdef __EMSCRIPTEN__
    /* For Emscripten, use the browser's animation frame callback.
     * 0 = use requestAnimationFrame (best for browsers)
     * 1 = simulate infinite loop (required for ASYNCIFY) */
    emscripten_set_main_loop_arg(example_frame, &state, 0, 1);
#else
    /* Native main loop */
    while (state.running)
        example_frame(&state);

    /* Cleanup (only reached on native, not Emscripten) */
    free(state.iui_buffer);
    g_iui_port.shutdown(state.port);
#endif

    return 0;
}
