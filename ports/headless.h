/*
 * ports/headless.h - Headless Backend Test Harness API
 *
 * This header exposes testing utilities for automated UI testing.
 * NOT part of the core libiui API - only for test infrastructure.
 *
 * C Usage:
 *   #include "ports/port.h"
 *   #include "ports/headless.h"
 *
 *   iui_port_ctx *ctx = g_iui_port.init(800, 600, "Test");
 *   iui_headless_set_max_frames(ctx, 10);
 *
 *   // Input injection for interaction testing
 *   iui_headless_inject_click(ctx, 100.0f, 50.0f);
 *   iui_headless_inject_key(ctx, IUI_KEY_TAB);
 *   iui_headless_inject_text(ctx, 'A');
 *
 *   // ... run test frames ...
 *   iui_headless_save_screenshot(ctx, "output.png");
 *
 *   iui_headless_stats_t stats;
 *   iui_headless_get_stats(ctx, &stats);
 *   printf("Draw calls: %u\n", stats.draw_box_calls);
 *
 * Python Integration (three test categories):
 *   python3 scripts/headless-test.py              # Run all tests
 *   python3 scripts/headless-test.py --list       # List available tests
 *   python3 scripts/headless-test.py -t button    # Specific widget test
 *   python3 scripts/headless-test.py -t click_button  # Interaction test
 *   python3 scripts/headless-test.py --interact   # Interaction tests only
 *   python3 scripts/headless-test.py --md3        # MD3 spec validation only
 *   python3 scripts/headless-test.py -s           # Save screenshots
 *   python3 scripts/headless-test.py -v           # Verbose output
 *
 * Test Categories:
 *   - Widget tests: Basic rendering validation
 *   - Interaction tests: Input injection + state validation
 *   - MD3 tests: Material Design 3 compliance from md3-spec.dsl
 *
 * See IMPROVE.md "CI/CD Integration" for GitHub Actions examples.
 */

#ifndef IUI_HEADLESS_H
#define IUI_HEADLESS_H

#include <stdbool.h>
#include <stdint.h>

#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Frame Control API */

/* Set maximum frames before auto-exit (0 = unlimited) */
void iui_headless_set_max_frames(iui_port_ctx *ctx, unsigned long max_frames);

/* Get current frame count */
unsigned long iui_headless_get_frame_count(iui_port_ctx *ctx);

/* Input Injection API */

/* Inject full input state for the next frame */
void iui_headless_inject_input(iui_port_ctx *ctx, const iui_port_input *input);

/* Inject a mouse click at (x, y) */
void iui_headless_inject_click(iui_port_ctx *ctx, float x, float y);

/* Inject a key press */
void iui_headless_inject_key(iui_port_ctx *ctx, int key);

/* Inject text input (Unicode codepoint) */
void iui_headless_inject_text(iui_port_ctx *ctx, uint32_t codepoint);

/* Framebuffer Access API */

/* Get raw framebuffer pointer (ARGB32 format)
 * Returns NULL if framebuffer not enabled.
 */
const uint32_t *iui_headless_get_framebuffer(iui_port_ctx *ctx);

/* Get framebuffer dimensions */
void iui_headless_get_framebuffer_size(const iui_port_ctx *ctx,
                                       int *width,
                                       int *height);

/* Get pixel color at (x, y)
 * Returns 0 if out of bounds.
 */
uint32_t iui_headless_get_pixel(iui_port_ctx *ctx, int x, int y);

/* Clear framebuffer to specified ARGB color */
void iui_headless_clear_framebuffer(iui_port_ctx *ctx, uint32_t color);

/* Screenshot Export API */

/* Save framebuffer as PNG file
 * Returns true on success.
 */
bool iui_headless_save_screenshot(iui_port_ctx *ctx, const char *path);

/* Statistics API */

typedef struct {
    uint32_t draw_box_calls;
    uint32_t draw_line_calls;
    uint32_t draw_circle_calls;
    uint32_t draw_arc_calls;
    uint32_t set_clip_calls;
    uint32_t path_stroke_calls;
    uint64_t total_pixels_drawn;
    unsigned long frame_count;
} iui_headless_stats_t;

/* Get rendering statistics */
void iui_headless_get_stats(iui_port_ctx *ctx, iui_headless_stats_t *stats);

/* Reset statistics counters */
void iui_headless_reset_stats(iui_port_ctx *ctx);

/* Color Helpers (for pixel verification)
 *
 * These are thin wrappers around port-sw.h color functions.
 * Provided for API consistency in test code.
 */
#include "port-sw.h"

#define iui_headless_get_alpha(c) iui_color_alpha(c)
#define iui_headless_get_red(c) iui_color_red(c)
#define iui_headless_get_green(c) iui_color_green(c)
#define iui_headless_get_blue(c) iui_color_blue(c)
#define iui_headless_make_color(r, g, b, a) iui_make_color(r, g, b, a)

#ifdef __cplusplus
}
#endif

#endif /* IUI_HEADLESS_H */
