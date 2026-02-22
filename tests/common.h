/*
 * Test Infrastructure
 *
 * Shared test macros, mock renderers, and utilities.
 * Include this header in all test-*.c files.
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/internal.h"
#include "iui.h"
#include "iui_config.h"

/* Test Counters (extern declarations) */
extern int g_tests_run;
extern int g_tests_passed;
extern int g_tests_failed;
extern int g_verbose;

/* Renderer callback counters */
extern int g_draw_box_calls;
extern int g_draw_text_calls;
extern int g_set_clip_calls;
extern int g_draw_line_calls;
extern int g_draw_circle_calls;
extern int g_draw_arc_calls;

/* Last call parameters for verification */
extern float g_last_box_x, g_last_box_y, g_last_box_w, g_last_box_h;
extern float g_last_box_radius;
extern uint32_t g_last_box_color;

/* Extended last call parameters for text */
extern float g_last_text_x, g_last_text_y;
extern char g_last_text_content[256];
extern uint32_t g_last_text_color;

/* Extended last call parameters for clip */
extern uint16_t g_last_clip_min_x, g_last_clip_min_y;
extern uint16_t g_last_clip_max_x, g_last_clip_max_y;

/* Extended last call parameters for primitives */
extern float g_last_line_x0, g_last_line_y0, g_last_line_x1, g_last_line_y1;
extern float g_last_line_width;
extern uint32_t g_last_line_color;

extern float g_last_circle_cx, g_last_circle_cy, g_last_circle_radius;
extern uint32_t g_last_circle_fill, g_last_circle_stroke;
extern float g_last_circle_stroke_w;

extern float g_last_arc_cx, g_last_arc_cy, g_last_arc_radius;
extern float g_last_arc_start, g_last_arc_end, g_last_arc_width;
extern uint32_t g_last_arc_color;

/* Test Macros */

#define TEST(name)                             \
    do {                                       \
        g_tests_run++;                         \
        if (g_verbose)                         \
            printf("  [TEST] %s ... ", #name); \
    } while (0)

#define PASS()                \
    do {                      \
        g_tests_passed++;     \
        if (g_verbose)        \
            printf("PASS\n"); \
    } while (0)

#define FAIL(msg)                                   \
    do {                                            \
        g_tests_failed++;                           \
        printf("  [FAIL] %s: %s\n", __func__, msg); \
    } while (0)

#define ASSERT_TRUE(cond)            \
    do {                             \
        if (!(cond)) {               \
            FAIL(#cond " is false"); \
            return;                  \
        }                            \
    } while (0)

#define ASSERT_FALSE(cond)          \
    do {                            \
        if (cond) {                 \
            FAIL(#cond " is true"); \
            return;                 \
        }                           \
    } while (0)

#define ASSERT_EQ(a, b)         \
    do {                        \
        if ((a) != (b)) {       \
            FAIL(#a " != " #b); \
            return;             \
        }                       \
    } while (0)

#define ASSERT_NEAR(a, b, eps)         \
    do {                               \
        if (fabs((a) - (b)) > (eps)) { \
            FAIL(#a " not near " #b);  \
            return;                    \
        }                              \
    } while (0)

#define ASSERT_NOT_NULL(ptr)       \
    do {                           \
        if ((ptr) == NULL) {       \
            FAIL(#ptr " is NULL"); \
            return;                \
        }                          \
    } while (0)

#define ASSERT_NULL(ptr)               \
    do {                               \
        if ((ptr) != NULL) {           \
            FAIL(#ptr " is not NULL"); \
            return;                    \
        }                              \
    } while (0)

#define ASSERT_STR_EQ(a, b)            \
    do {                               \
        if (strcmp((a), (b)) != 0) {   \
            FAIL(#a " string != " #b); \
            return;                    \
        }                              \
    } while (0)

/* Test Window Setup/Teardown Macros
 *
 * Reduces boilerplate for tests that need a window context.
 *
 * Usage:
 *   static void test_foo(void) {
 *       BEGIN_TEST_WINDOW(foo);
 *       // ... test code using 'ctx' ...
 *       END_TEST_WINDOW();
 *   }
 *
 * Notes:
 * - Injects local variables: _test_buffer (void*), ctx (iui_context*)
 * - Do NOT use early 'return' in test body; it leaks _test_buffer
 * - For assertions that may fail, use ASSERT_* macros (they call return)
 *   only before any allocations, or accept the leak in failure paths
 */

/* Internal helper - do not use directly */
#define BEGIN_TEST_WINDOW_IMPL_(name, use_vec)                     \
    TEST(name);                                                    \
    void *_test_buffer = malloc(iui_min_memory_size());            \
    if (!_test_buffer) {                                           \
        FAIL("malloc failed");                                     \
        return;                                                    \
    }                                                              \
    iui_context *ctx = create_test_context(_test_buffer, use_vec); \
    if (!ctx) {                                                    \
        FAIL("create_test_context returned NULL");                 \
        free(_test_buffer);                                        \
        return;                                                    \
    }                                                              \
    iui_begin_frame(ctx, 1.0f / 60.0f);                            \
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0)

#define BEGIN_TEST_WINDOW(name) BEGIN_TEST_WINDOW_IMPL_(name, false)
#define BEGIN_TEST_WINDOW_VEC(name) BEGIN_TEST_WINDOW_IMPL_(name, true)

#define END_TEST_WINDOW() \
    iui_end_window(ctx);  \
    iui_end_frame(ctx);   \
    free(_test_buffer);   \
    PASS()

/* ANSI color codes for terminal output */
#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"

/* Section tracking for grouped test output */
extern int g_section_failed;
extern const char *g_section_name;

#define SECTION_BEGIN(name)                \
    do {                                   \
        g_section_failed = g_tests_failed; \
        g_section_name = (name);           \
    } while (0)

#define SECTION_END()                                                \
    do {                                                             \
        if (g_tests_failed == g_section_failed)                      \
            printf("Test %-40s[ " ANSI_GREEN "OK" ANSI_RESET " ]\n", \
                   g_section_name);                                  \
        else                                                         \
            printf("Test %-40s[ " ANSI_RED "FAIL" ANSI_RESET " ]\n", \
                   g_section_name);                                  \
    } while (0)

/* Mock Renderer Functions (declarations) */

void mock_draw_box(iui_rect_t rect, float r, uint32_t color, void *user);
void mock_draw_text(float x,
                    float y,
                    const char *text,
                    uint32_t color,
                    void *user);
void mock_set_clip(uint16_t min_x,
                   uint16_t min_y,
                   uint16_t max_x,
                   uint16_t max_y,
                   void *user);
float mock_text_width(const char *text, void *user);
void mock_draw_line(float x0,
                    float y0,
                    float x1,
                    float y1,
                    float width,
                    uint32_t color,
                    void *user);
void mock_draw_circle(float cx,
                      float cy,
                      float radius,
                      uint32_t fill,
                      uint32_t stroke,
                      float stroke_w,
                      void *user);
void mock_draw_arc(float cx,
                   float cy,
                   float radius,
                   float start,
                   float end,
                   float width,
                   uint32_t color,
                   void *user);
void reset_counters(void);

/* Test Context Factory (declaration) */

iui_context *create_test_context(void *buffer, bool with_vector_prims);

/* Quick test context initialization (uses static buffer) */
static inline iui_context *test_init_context(void)
{
    /* Use union to force alignment to void* (C99 compliant) */
    static union {
        uint8_t buffer[65536];
        void *align_force;
    } u;

    memset(u.buffer, 0, sizeof(u.buffer));
    return create_test_context(u.buffer, false);
}

/* Interaction Simulation Helpers */

/* Simulate a mouse click at position (x, y) within a frame */
void test_simulate_click(iui_context *ctx, float x, float y);

/* Simulate a mouse click with press and release in separate frames */
void test_simulate_click_frames(iui_context *ctx,
                                float x,
                                float y,
                                float delta_time);

/* Simulate mouse drag from (x0, y0) to (x1, y1) */
void test_simulate_drag(iui_context *ctx,
                        float x0,
                        float y0,
                        float x1,
                        float y1,
                        float delta_time);

/* Get the bounds of the last widget rendered (approximated from last box) */
static inline iui_rect_t test_get_last_widget_bounds(void)
{
    return (iui_rect_t) {g_last_box_x, g_last_box_y, g_last_box_w,
                         g_last_box_h};
}

/* Check if point is inside rect */
static inline bool test_point_in_rect(float x, float y, iui_rect_t r)
{
    return x >= r.x && x < r.x + r.width && y >= r.y && y < r.y + r.height;
}

/* Test Suite Runners (declarations) */
void run_demo_tests(iui_context *ctx);
void run_init_tests(void);
void run_bounds_tests(void);
void run_layout_tests(void);
void run_widget_tests(void);
void run_input_tests(void);
void run_animation_tests(void);
void run_theme_tests(void);
void run_vector_tests(void);
void run_string_tests(void);
void run_state_machine_tests(void);
void run_new_component_tests(void);
void run_modal_tests(void);
void run_snackbar_tests(void);
void run_elevation_tests(void);
void run_scroll_tests(void);
void run_textfield_icon_tests(void);
void run_menu_tests(void);
void run_dialog_tests(void);
void run_slider_ex_tests(void);
void run_chip_tests(void);
void run_spec_tests(void);
void run_focus_tests(void);
void run_input_layer_tests(void);
void run_clip_tests(void);
void run_field_tracking_tests(void);
void run_overflow_tests(void);
void run_navigation_tests(void);
void run_bottom_sheet_tests(void);
void run_box_tests(void);

#endif /* TEST_COMMON_H */
