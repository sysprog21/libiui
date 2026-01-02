/*
 * Headless Backend Implementation
 *
 * This file implements the iui_port_t interface without any display output.
 * Useful for:
 *   - Automated testing (CI/CD pipelines)
 *   - Sanitizer runs (ASan, UBSan, MSan)
 *   - Benchmarking without GPU overhead
 *   - Headless servers
 *   - Visual regression testing (screenshot comparison)
 *
 * Features:
 *   - Software rasterizer for draw_box, draw_line, draw_circle, draw_arc
 *   - Framebuffer capture for screenshot export
 *   - Statistics tracking for performance analysis
 *   - Shared memory mode for external tool control (HEAD3)
 *
 * Build: No external dependencies required
 */

/* Enable POSIX features (clock_gettime, ftruncate, struct timespec) */
#define _POSIX_C_SOURCE 200809L

#include "headless.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "headless-shm.h"
#include "port-sw.h"
#include "port.h"

/* Platform-specific includes for shared memory */
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

/* Framebuffer rendering configuration */
#define HEADLESS_ENABLE_FRAMEBUFFER 1

/* Headless port context - state for testing and framebuffer rendering. */
struct iui_port_ctx {
    int width, height;
    float font_height;

    /* Timing (fixed for deterministic testing) */
    float delta_time;

    /* Exit state */
    bool running;
    bool exit_requested;

    /* Frame counter for automated testing */
    unsigned long frame_count;
    unsigned long max_frames; /* 0 = unlimited */

    /* Simulated input for testing */
    iui_port_input queued_input;
    iui_port_input pending_input; /* Set by test harness */
    bool has_pending_mouse; /* True if mouse position was explicitly set */

    /* Callbacks (stored for get_renderer_callbacks) */
    iui_renderer_t render_ops;
    iui_vector_t vector_ops;

    /* Clipboard simulation */
    char *clipboard_text;

#if HEADLESS_ENABLE_FRAMEBUFFER
    /* Framebuffer for software rendering (ARGB32) */
    uint32_t *framebuffer;
    size_t fb_size; /* width * height */

    /* Rasterizer context (shared with port-sw.h) */
    iui_raster_ctx_t raster;

    /* Vector path state (shared with port-sw.h) */
    iui_path_state_t path;
#endif

    /* Statistics tracking */
    struct {
        uint32_t draw_box_calls;
        uint32_t draw_line_calls;
        uint32_t draw_circle_calls;
        uint32_t draw_arc_calls;
        uint32_t set_clip_calls;
        uint32_t path_stroke_calls;
        uint64_t total_pixels_drawn;
    } stats;

    /* Shared memory state (HEAD3) */
    struct {
        bool enabled;
        char name[64];         /* SHM name (e.g., "/libiui_shm") */
        void *base;            /* Mapped memory base pointer */
        size_t size;           /* Total mapped size */
        int fd;                /* File descriptor (POSIX) */
        uint32_t last_cmd_seq; /* Last processed command sequence */
    } shm;
};

/* Color manipulation - map to public API from headless.h
 * Note: These macros bridge headless.h API to port-common.h implementation
 */
#define get_alpha(c) iui_color_alpha(c)
#define get_red(c) iui_color_red(c)
#define get_green(c) iui_color_green(c)
#define get_blue(c) iui_color_blue(c)
#define make_color(r, g, b, a) iui_make_color(r, g, b, a)

/* NOTE: Drawing primitives (fill_rounded_rect, draw_line, fill_circle,
 * stroke_circle, draw_arc) have been consolidated into port-sw.h as shared
 * inline functions: iui_raster_rounded_rect, iui_raster_line,
 * iui_raster_circle_fill, iui_raster_circle_stroke, iui_raster_arc
 */

/* Renderer Callbacks */

static void headless_draw_box(iui_rect_t rect,
                              float radius,
                              uint32_t srgb_color,
                              void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.draw_box_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (ctx->framebuffer)
        iui_raster_rounded_rect(&ctx->raster, rect.x, rect.y, rect.width,
                                rect.height, radius, srgb_color);
#else
    (void) rect;
    (void) radius;
    (void) srgb_color;
#endif
}

static void headless_set_clip_rect(uint16_t min_x,
                                   uint16_t min_y,
                                   uint16_t max_x,
                                   uint16_t max_y,
                                   void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.set_clip_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (min_x == 0 && min_y == 0 && max_x == UINT16_MAX &&
        max_y == UINT16_MAX) {
        iui_raster_reset_clip(&ctx->raster);
    } else {
        iui_raster_set_clip(&ctx->raster, min_x, min_y, max_x, max_y);
    }
#else
    (void) min_x;
    (void) min_y;
    (void) max_x;
    (void) max_y;
#endif
}

static void headless_draw_line(float x0,
                               float y0,
                               float x1,
                               float y1,
                               float width,
                               uint32_t srgb_color,
                               void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.draw_line_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (ctx->framebuffer)
        iui_raster_line(&ctx->raster, x0, y0, x1, y1, width, srgb_color);
#else
    (void) x0;
    (void) y0;
    (void) x1;
    (void) y1;
    (void) width;
    (void) srgb_color;
#endif
}

static void headless_draw_circle(float cx,
                                 float cy,
                                 float radius,
                                 uint32_t fill_color,
                                 uint32_t stroke_color,
                                 float stroke_width,
                                 void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.draw_circle_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (ctx->framebuffer) {
        if (fill_color != 0)
            iui_raster_circle_fill(&ctx->raster, cx, cy, radius, fill_color);
        if (stroke_color != 0 && stroke_width > 0.f)
            iui_raster_circle_stroke(&ctx->raster, cx, cy, radius, stroke_width,
                                     stroke_color);
    }
#else
    (void) cx;
    (void) cy;
    (void) radius;
    (void) fill_color;
    (void) stroke_color;
    (void) stroke_width;
#endif
}

static void headless_draw_arc(float cx,
                              float cy,
                              float radius,
                              float start_angle,
                              float end_angle,
                              float width,
                              uint32_t srgb_color,
                              void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.draw_arc_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (ctx->framebuffer)
        iui_raster_arc(&ctx->raster, cx, cy, radius, start_angle, end_angle,
                       width, srgb_color);
#else
    (void) cx;
    (void) cy;
    (void) radius;
    (void) start_angle;
    (void) end_angle;
    (void) width;
    (void) srgb_color;
#endif
}

/* Vector Font Callbacks */

static void headless_path_move(float x, float y, void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
#if HEADLESS_ENABLE_FRAMEBUFFER
    iui_path_move_to(&ctx->path, x, y);
#else
    (void) ctx;
    (void) x;
    (void) y;
#endif
}

static void headless_path_line(float x, float y, void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
#if HEADLESS_ENABLE_FRAMEBUFFER
    iui_path_line_to(&ctx->path, x, y);
#else
    (void) ctx;
    (void) x;
    (void) y;
#endif
}

static void headless_path_curve(float x1,
                                float y1,
                                float x2,
                                float y2,
                                float x3,
                                float y3,
                                void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
#if HEADLESS_ENABLE_FRAMEBUFFER
    iui_path_curve_to(&ctx->path, x1, y1, x2, y2, x3, y3);
#else
    (void) ctx;
    (void) x1;
    (void) y1;
    (void) x2;
    (void) y2;
    (void) x3;
    (void) y3;
#endif
}

static void headless_path_stroke(float width, uint32_t color, void *user)
{
    iui_port_ctx *ctx = (iui_port_ctx *) user;
    ctx->stats.path_stroke_calls++;

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx->framebuffer || ctx->path.count < 2) {
        iui_path_reset(&ctx->path);
        return;
    }

    /* Use SDL2-compatible path stroke with round caps and consistent AA */
    iui_raster_path_stroke(&ctx->raster, &ctx->path, width, color);
    iui_path_reset(&ctx->path);
#else
    (void) width;
    (void) color;
#endif
}

/* Port Interface Implementation (iui_port_t) */

static iui_port_ctx *headless_init(int width, int height, const char *title)
{
    (void) title;

    iui_port_ctx *ctx = (iui_port_ctx *) calloc(1, sizeof(iui_port_ctx));
    if (!ctx)
        return NULL;

    ctx->width = width > 0 ? width : 800;
    ctx->height = height > 0 ? height : 600;
    ctx->running = true;
    ctx->exit_requested = false;
    ctx->frame_count = 0;
    ctx->max_frames = 0;                 /* Unlimited by default */
    ctx->delta_time = IUI_PORT_FRAME_DT; /* Fixed ~60fps */
    ctx->clipboard_text = NULL;

#if HEADLESS_ENABLE_FRAMEBUFFER
    /* Allocate framebuffer */
    ctx->fb_size = (size_t) ctx->width * (size_t) ctx->height;
    ctx->framebuffer = (uint32_t *) calloc(ctx->fb_size, sizeof(uint32_t));
    if (!ctx->framebuffer) {
        free(ctx);
        return NULL;
    }

    /* Initialize raster context with framebuffer */
    iui_raster_init(&ctx->raster, ctx->framebuffer, ctx->width, ctx->height);

    /* Initialize path state */
    iui_path_reset(&ctx->path);
#endif

    return ctx;
}

static void headless_shutdown(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

    if (ctx->clipboard_text) {
        free(ctx->clipboard_text);
        ctx->clipboard_text = NULL;
    }

#if HEADLESS_ENABLE_FRAMEBUFFER
    if (ctx->framebuffer) {
        free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
#endif

    free(ctx);
}

static void headless_configure(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

    /* Set default font height */
    float logical_font_size = 14.0f;
    ctx->font_height = logical_font_size * 1.5f;

    /* Initialize renderer callbacks with port context as user data */
    ctx->render_ops.draw_box = headless_draw_box;
    ctx->render_ops.draw_text = NULL;
    ctx->render_ops.text_width = NULL;
    ctx->render_ops.set_clip_rect = headless_set_clip_rect;
    ctx->render_ops.draw_line = headless_draw_line;
    ctx->render_ops.draw_circle = headless_draw_circle;
    ctx->render_ops.draw_arc = headless_draw_arc;
    ctx->render_ops.user = ctx;

    /* Initialize vector callbacks */
    ctx->vector_ops.path_move = headless_path_move;
    ctx->vector_ops.path_line = headless_path_line;
    ctx->vector_ops.path_curve = headless_path_curve;
    ctx->vector_ops.path_stroke = headless_path_stroke;
}

static bool headless_poll_events(iui_port_ctx *ctx)
{
    if (!ctx)
        return false;

    /*
     * Preserve mouse position across frames (like SDL2).
     * Only update position if explicitly set via inject functions.
     */
    if (ctx->has_pending_mouse) {
        ctx->queued_input.mouse_x = ctx->pending_input.mouse_x;
        ctx->queued_input.mouse_y = ctx->pending_input.mouse_y;
        ctx->has_pending_mouse = false;
    }

    /* Copy per-event fields from pending input */
    ctx->queued_input.mouse_pressed = ctx->pending_input.mouse_pressed;
    ctx->queued_input.mouse_released = ctx->pending_input.mouse_released;
    ctx->queued_input.key = ctx->pending_input.key;
    ctx->queued_input.text = ctx->pending_input.text;
    ctx->queued_input.scroll_x = ctx->pending_input.scroll_x;
    ctx->queued_input.scroll_y = ctx->pending_input.scroll_y;
    ctx->queued_input.shift_down = ctx->pending_input.shift_down;

    /* Reset per-event fields for next frame (keep mouse position) */
    ctx->pending_input.mouse_pressed = 0;
    ctx->pending_input.mouse_released = 0;
    ctx->pending_input.key = 0;
    ctx->pending_input.text = 0;
    ctx->pending_input.scroll_x = 0;
    ctx->pending_input.scroll_y = 0;
    ctx->pending_input.shift_down = false;

    /* Update delta time.
     * Note: clock() measures CPU time, not wall time. For headless testing,
     * a fixed frame time ensures consistent, deterministic behavior regardless
     * of actual elapsed time. This matches expected 60fps.
     */
    ctx->delta_time =
        IUI_PORT_FRAME_DT; /* Fixed ~60fps for deterministic testing */

    /* Increment frame counter */
    ctx->frame_count++;

    /* Auto-exit if max_frames is set */
    if (ctx->max_frames > 0 && ctx->frame_count >= ctx->max_frames)
        iui_port_request_exit(&ctx->running, &ctx->exit_requested);

    /* Process shared memory events and commands if SHM mode is enabled */
    if (ctx->shm.enabled) {
        iui_headless_process_shm_events(ctx);
        iui_headless_process_shm_commands(ctx);
    }

    return ctx->running;
}

static bool headless_should_exit(iui_port_ctx *ctx)
{
    return ctx ? ctx->exit_requested : true;
}

static void headless_request_exit(iui_port_ctx *ctx)
{
    if (ctx)
        iui_port_request_exit(&ctx->running, &ctx->exit_requested);
}

static void headless_get_input(iui_port_ctx *ctx, iui_port_input *input)
{
    if (!ctx || !input)
        return;
    *input = ctx->queued_input;
}

static void headless_begin_frame(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

#if HEADLESS_ENABLE_FRAMEBUFFER
    /* Clear framebuffer with dark background (matches SDL2 backend) */
    if (ctx->framebuffer) {
        uint32_t bg_color = iui_make_color(40, 44, 52, 255);
        iui_raster_clear(&ctx->raster, bg_color);
    }

    /* Reset clip to full framebuffer */
    iui_raster_reset_clip(&ctx->raster);
#endif
}

static void headless_end_frame(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

#if HEADLESS_ENABLE_FRAMEBUFFER
    /* Update pixel count from rasterizer */
    ctx->stats.total_pixels_drawn = ctx->raster.pixels_drawn;
#endif

    /* Sync shared memory if SHM mode is enabled */
    if (ctx->shm.enabled) {
        iui_headless_update_shm_stats(ctx);
        iui_headless_sync_shm_framebuffer(ctx);
    }
}

static iui_renderer_t headless_get_renderer_callbacks(iui_port_ctx *ctx)
{
    if (!ctx) {
        iui_renderer_t empty = {0};
        return empty;
    }
    return ctx->render_ops;
}

static const iui_vector_t *headless_get_vector_callbacks(iui_port_ctx *ctx)
{
    if (!ctx)
        return NULL;
    return &ctx->vector_ops;
}

static float headless_get_delta_time(iui_port_ctx *ctx)
{
    return ctx ? ctx->delta_time : IUI_PORT_FRAME_DT;
}

static void headless_get_window_size(iui_port_ctx *ctx, int *width, int *height)
{
    if (!ctx)
        return;
    if (width)
        *width = ctx->width;
    if (height)
        *height = ctx->height;
}

static void headless_set_window_size(iui_port_ctx *ctx, int width, int height)
{
    if (!ctx)
        return;

    int new_width = width > 0 ? width : ctx->width;
    int new_height = height > 0 ? height : ctx->height;

    if (new_width == ctx->width && new_height == ctx->height)
        return;

#if HEADLESS_ENABLE_FRAMEBUFFER
    /* Reallocate framebuffer for new size */
    size_t new_size = (size_t) new_width * (size_t) new_height;
    uint32_t *new_fb = (uint32_t *) calloc(new_size, sizeof(uint32_t));
    if (!new_fb)
        return;

    if (ctx->framebuffer)
        free(ctx->framebuffer);

    ctx->framebuffer = new_fb;
    ctx->fb_size = new_size;

    /* Reinitialize raster context with new framebuffer */
    iui_raster_init(&ctx->raster, ctx->framebuffer, new_width, new_height);
#endif

    ctx->width = new_width;
    ctx->height = new_height;
}

static float headless_get_dpi_scale(iui_port_ctx *ctx)
{
    (void) ctx;
    return 1.0f; /* No HiDPI in headless mode */
}

static bool headless_is_window_focused(iui_port_ctx *ctx)
{
    (void) ctx;
    return true; /* Always focused in headless mode */
}

static bool headless_is_window_visible(iui_port_ctx *ctx)
{
    (void) ctx;
    return true; /* Always visible in headless mode */
}

static const char *headless_get_clipboard_text(iui_port_ctx *ctx)
{
    if (!ctx)
        return NULL;
    return ctx->clipboard_text;
}

static void headless_set_clipboard_text(iui_port_ctx *ctx, const char *text)
{
    if (!ctx)
        return;

    if (ctx->clipboard_text) {
        free(ctx->clipboard_text);
        ctx->clipboard_text = NULL;
    }

    if (text) {
        size_t len = strlen(text);
        ctx->clipboard_text = (char *) malloc(len + 1);
        if (ctx->clipboard_text) {
            memcpy(ctx->clipboard_text, text, len + 1);
        }
    }
}

static void *headless_get_native_renderer(iui_port_ctx *ctx)
{
    (void) ctx;
    return NULL; /* No native renderer in headless mode */
}

/* Global Backend Instance */
const iui_port_t g_iui_port = {
    .init = headless_init,
    .shutdown = headless_shutdown,
    .configure = headless_configure,
    .poll_events = headless_poll_events,
    .should_exit = headless_should_exit,
    .request_exit = headless_request_exit,
    .get_input = headless_get_input,
    .begin_frame = headless_begin_frame,
    .end_frame = headless_end_frame,
    .get_renderer_callbacks = headless_get_renderer_callbacks,
    .get_vector_callbacks = headless_get_vector_callbacks,
    .get_delta_time = headless_get_delta_time,
    .get_window_size = headless_get_window_size,
    .set_window_size = headless_set_window_size,
    .get_dpi_scale = headless_get_dpi_scale,
    .is_window_focused = headless_is_window_focused,
    .is_window_visible = headless_is_window_visible,
    .get_clipboard_text = headless_get_clipboard_text,
    .set_clipboard_text = headless_set_clipboard_text,
    .get_native_renderer = headless_get_native_renderer,
};

/* Test Harness API - Additional functions for automated testing.
 * These are not part of the standard port interface.
 */

/* Set max frames before auto-exit (0 = unlimited) */
void iui_headless_set_max_frames(iui_port_ctx *ctx, unsigned long max_frames)
{
    if (ctx)
        ctx->max_frames = max_frames;
}

unsigned long iui_headless_get_frame_count(iui_port_ctx *ctx)
{
    return ctx ? ctx->frame_count : 0;
}

/* Inject full input state for next frame */
void iui_headless_inject_input(iui_port_ctx *ctx, const iui_port_input *input)
{
    if (!ctx || !input)
        return;
    ctx->pending_input = *input;
    ctx->has_pending_mouse = true;
}

/* Inject left-click at position */
void iui_headless_inject_click(iui_port_ctx *ctx, float x, float y)
{
    if (!ctx)
        return;
    ctx->pending_input.mouse_x = x;
    ctx->pending_input.mouse_y = y;
    ctx->pending_input.mouse_pressed = IUI_MOUSE_LEFT;
    ctx->pending_input.mouse_released = IUI_MOUSE_LEFT;
    ctx->has_pending_mouse = true;
}

void iui_headless_inject_key(iui_port_ctx *ctx, int key)
{
    if (!ctx)
        return;
    ctx->pending_input.key = key;
}

void iui_headless_inject_text(iui_port_ctx *ctx, uint32_t codepoint)
{
    if (!ctx)
        return;
    ctx->pending_input.text = codepoint;
}

/* Returns NULL if framebuffer not enabled */
const uint32_t *iui_headless_get_framebuffer(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    return ctx ? ctx->framebuffer : NULL;
#else
    (void) ctx;
    return NULL;
#endif
}

/*
 * Get framebuffer dimensions.
 */
void iui_headless_get_framebuffer_size(iui_port_ctx *ctx,
                                       int *width,
                                       int *height)
{
    if (!ctx)
        return;
    if (width)
        *width = ctx->width;
    if (height)
        *height = ctx->height;
}

void iui_headless_get_stats(iui_port_ctx *ctx, iui_headless_stats_t *stats)
{
    if (!ctx || !stats)
        return;

    stats->draw_box_calls = ctx->stats.draw_box_calls;
    stats->draw_line_calls = ctx->stats.draw_line_calls;
    stats->draw_circle_calls = ctx->stats.draw_circle_calls;
    stats->draw_arc_calls = ctx->stats.draw_arc_calls;
    stats->set_clip_calls = ctx->stats.set_clip_calls;
    stats->path_stroke_calls = ctx->stats.path_stroke_calls;
    stats->total_pixels_drawn = ctx->stats.total_pixels_drawn;
    stats->frame_count = ctx->frame_count;
}

void iui_headless_reset_stats(iui_port_ctx *ctx)
{
    if (!ctx)
        return;
    memset(&ctx->stats, 0, sizeof(ctx->stats));
}

/* Minimal PNG encoder - uncompressed DEFLATE (store mode) */

/* CRC-32 table for PNG */
static uint32_t crc_table[256];
static bool crc_table_computed = false;

static void make_crc_table(void)
{
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xEDB88320 ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = true;
}

static uint32_t update_crc(uint32_t crc, const uint8_t *buf, size_t len)
{
    uint32_t c = crc;
    for (size_t n = 0; n < len; n++)
        c = crc_table[(c ^ buf[n]) & 0xFF] ^ (c >> 8);
    return c;
}

static uint32_t png_crc(const uint8_t *buf, size_t len)
{
    return update_crc(0xFFFFFFFF, buf, len) ^ 0xFFFFFFFF;
}

/* Adler-32 checksum for zlib */
static uint32_t adler32(const uint8_t *data, size_t len)
{
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

/* Write big-endian 32-bit value */
static void write_be32(uint8_t *p, uint32_t v)
{
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

/* Write little-endian 16-bit value */
static void write_le16(uint8_t *p, uint16_t v)
{
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
}

/* Save framebuffer as PNG (uncompressed). Returns true on success. */
bool iui_headless_save_screenshot(iui_port_ctx *ctx, const char *path)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->framebuffer || !path)
        return false;

    if (!crc_table_computed)
        make_crc_table();

    FILE *fp = fopen(path, "wb");
    if (!fp)
        return false;

    int width = ctx->width;
    int height = ctx->height;

    /* PNG signature */
    static const uint8_t png_sig[8] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    };
    fwrite(png_sig, 1, 8, fp);

    /* IHDR chunk */
    uint8_t ihdr[25];
    write_be32(ihdr, 13); /* Length */
    ihdr[4] = 'I';
    ihdr[5] = 'H';
    ihdr[6] = 'D';
    ihdr[7] = 'R';
    write_be32(ihdr + 8, (uint32_t) width);
    write_be32(ihdr + 12, (uint32_t) height);
    ihdr[16] = 8; /* Bit depth */
    ihdr[17] = 6; /* Color type: RGBA */
    ihdr[18] = 0; /* Compression */
    ihdr[19] = 0; /* Filter */
    ihdr[20] = 0; /* Interlace */
    write_be32(ihdr + 21, png_crc(ihdr + 4, 17));
    fwrite(ihdr, 1, 25, fp);

    /* Prepare raw image data (filter byte + RGBA per row) */
    size_t row_bytes = 1 + (size_t) width * 4;
    size_t raw_size = row_bytes * (size_t) height;
    uint8_t *raw_data = (uint8_t *) malloc(raw_size);
    if (!raw_data) {
        fclose(fp);
        return false;
    }

    /* Convert ARGB to RGBA with filter byte */
    for (int y = 0; y < height; y++) {
        size_t row_offset = (size_t) y * row_bytes;
        raw_data[row_offset] = 0; /* Filter: None */
        for (int x = 0; x < width; x++) {
            uint32_t pixel = ctx->framebuffer[(size_t) y * (size_t) width + x];
            size_t idx = row_offset + 1 + (size_t) x * 4;
            raw_data[idx + 0] = get_red(pixel);
            raw_data[idx + 1] = get_green(pixel);
            raw_data[idx + 2] = get_blue(pixel);
            raw_data[idx + 3] = get_alpha(pixel);
        }
    }

    /* Create uncompressed zlib stream (DEFLATE store mode).
     * Format: zlib header (2 bytes) + deflate blocks + adler32 (4 bytes)
     *
     * Each deflate store block:
     * - 1 byte: BFINAL (1 bit) + BTYPE=00 (2 bits) + padding
     * - 2 bytes: LEN (little-endian)
     * - 2 bytes: NLEN (~LEN)
     * - LEN bytes: literal data
     *
     * Max block size is 65535 bytes, so we split into multiple blocks.
     */
    size_t max_block_size = 65535;
    size_t num_blocks = (raw_size + max_block_size - 1) / max_block_size;

    /* Calculate zlib stream size */
    size_t zlib_size = 2; /* zlib header */
    for (size_t i = 0; i < num_blocks; i++) {
        size_t block_start = i * max_block_size;
        size_t block_len = (block_start + max_block_size <= raw_size)
                               ? max_block_size
                               : (raw_size - block_start);
        zlib_size += 5 + block_len; /* header + data */
    }
    zlib_size += 4; /* adler32 */

    uint8_t *zlib_data = (uint8_t *) malloc(zlib_size);
    if (!zlib_data) {
        free(raw_data);
        fclose(fp);
        return false;
    }

    size_t zlib_pos = 0;

    /* zlib header: CMF=0x78 (deflate, 32K window), FLG=0x01 (no dict, check) */
    zlib_data[zlib_pos++] = 0x78;
    zlib_data[zlib_pos++] = 0x01;

    /* Write deflate blocks */
    for (size_t i = 0; i < num_blocks; i++) {
        size_t block_start = i * max_block_size;
        size_t block_len = (block_start + max_block_size <= raw_size)
                               ? max_block_size
                               : (raw_size - block_start);
        bool is_final = (i == num_blocks - 1);

        /* Block header */
        zlib_data[zlib_pos++] = is_final ? 0x01 : 0x00; /* BFINAL + BTYPE=00 */
        write_le16(zlib_data + zlib_pos, (uint16_t) block_len);
        zlib_pos += 2;
        write_le16(zlib_data + zlib_pos, (uint16_t) ~block_len);
        zlib_pos += 2;

        /* Block data */
        memcpy(zlib_data + zlib_pos, raw_data + block_start, block_len);
        zlib_pos += block_len;
    }

    /* Adler-32 checksum */
    uint32_t adler = adler32(raw_data, raw_size);
    write_be32(zlib_data + zlib_pos, adler);

    free(raw_data);

    /* IDAT chunk */
    size_t idat_chunk_size = 4 + zlib_size + 4; /* type + data + crc */
    uint8_t *idat = (uint8_t *) malloc(4 + idat_chunk_size);
    if (!idat) {
        free(zlib_data);
        fclose(fp);
        return false;
    }

    write_be32(idat, (uint32_t) zlib_size);
    idat[4] = 'I';
    idat[5] = 'D';
    idat[6] = 'A';
    idat[7] = 'T';
    memcpy(idat + 8, zlib_data, zlib_size);
    write_be32(idat + 8 + zlib_size, png_crc(idat + 4, 4 + zlib_size));
    fwrite(idat, 1, 4 + idat_chunk_size, fp);

    free(zlib_data);
    free(idat);

    /* IEND chunk */
    uint8_t iend[12];
    write_be32(iend, 0);
    iend[4] = 'I';
    iend[5] = 'E';
    iend[6] = 'N';
    iend[7] = 'D';
    write_be32(iend + 8, png_crc(iend + 4, 4));
    fwrite(iend, 1, 12, fp);

    fclose(fp);
    return true;
#else
    (void) ctx;
    (void) path;
    return false;
#endif
}

/* Get pixel color at specified position.
 * Returns 0 if position is out of bounds or framebuffer not enabled.
 */
uint32_t iui_headless_get_pixel(iui_port_ctx *ctx, int x, int y)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->framebuffer)
        return 0;
    if (x < 0 || x >= ctx->width || y < 0 || y >= ctx->height)
        return 0;
    return ctx->framebuffer[(size_t) y * (size_t) ctx->width + x];
#else
    (void) ctx;
    (void) x;
    (void) y;
    return 0;
#endif
}

/* Clear framebuffer to specified color. */
void iui_headless_clear_framebuffer(iui_port_ctx *ctx, uint32_t color)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->framebuffer)
        return;
    iui_raster_clear(&ctx->raster, color);
#else
    (void) ctx;
    (void) color;
#endif
}

/* Shared Memory API Implementation (HEAD3)
 * Enables external tools to interact with the headless backend via IPC.
 */

/* Get current timestamp in nanoseconds */
static uint64_t get_timestamp_ns(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t) (count.QuadPart * 1000000000ULL / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec;
#endif
}

/* Enable shared memory mode.
 * Creates a shared memory region that external tools can access.
 */
bool iui_headless_enable_shm(iui_port_ctx *ctx, const char *shm_name)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !shm_name || ctx->shm.enabled)
        return false;

    /* Store SHM name */
    size_t name_len = strlen(shm_name);
    if (name_len >= sizeof(ctx->shm.name))
        return false;
    memcpy(ctx->shm.name, shm_name, name_len + 1);

    /* Calculate total size */
    ctx->shm.size = iui_shm_total_size(ctx->width, ctx->height);

#ifdef _WIN32
    /* Windows: CreateFileMapping */
    HANDLE hMapFile =
        CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                           (DWORD) ctx->shm.size, shm_name + 1);
    if (!hMapFile)
        return false;

    ctx->shm.base =
        MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, ctx->shm.size);
    if (!ctx->shm.base) {
        CloseHandle(hMapFile);
        return false;
    }
    ctx->shm.fd = (int) (intptr_t) hMapFile;
#else
    /* POSIX: shm_open + mmap */
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        return false;

    if (ftruncate(fd, (off_t) ctx->shm.size) < 0) {
        close(fd);
        shm_unlink(shm_name);
        return false;
    }

    ctx->shm.base =
        mmap(NULL, ctx->shm.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ctx->shm.base == MAP_FAILED) {
        close(fd);
        shm_unlink(shm_name);
        ctx->shm.base = NULL;
        return false;
    }
    ctx->shm.fd = fd;
#endif

    /* Initialize header */
    iui_shm_header_t *hdr = (iui_shm_header_t *) ctx->shm.base;
    memset(hdr, 0, sizeof(*hdr));
    hdr->magic = IUI_SHM_MAGIC;
    hdr->version = IUI_SHM_VERSION;
    hdr->width = ctx->width;
    hdr->height = ctx->height;
    hdr->running = 1;
    hdr->timestamp_ns = get_timestamp_ns();

    /* Initialize ring buffer indices */
    hdr->event_write_idx = 0;
    hdr->event_read_idx = 0;

    ctx->shm.enabled = true;
    ctx->shm.last_cmd_seq = 0;

    return true;
#else
    (void) ctx;
    (void) shm_name;
    return false;
#endif
}

/* Disable shared memory mode. */
void iui_headless_disable_shm(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->shm.enabled)
        return;

    /* Mark as not running before cleanup */
    if (ctx->shm.base) {
        iui_shm_header_t *hdr = (iui_shm_header_t *) ctx->shm.base;
        hdr->running = 0;
    }

#ifdef _WIN32
    if (ctx->shm.base)
        UnmapViewOfFile(ctx->shm.base);
    if (ctx->shm.fd)
        CloseHandle((HANDLE) (intptr_t) ctx->shm.fd);
#else
    if (ctx->shm.base && ctx->shm.base != MAP_FAILED)
        munmap(ctx->shm.base, ctx->shm.size);
    if (ctx->shm.fd >= 0)
        close(ctx->shm.fd);
    shm_unlink(ctx->shm.name);
#endif

    ctx->shm.base = NULL;
    ctx->shm.fd = -1;
    ctx->shm.enabled = false;
#else
    (void) ctx;
#endif
}

/* Check if shared memory mode is enabled. */
bool iui_headless_shm_enabled(iui_port_ctx *ctx)
{
    return ctx && ctx->shm.enabled;
}

/* Get the shared memory header. */
iui_shm_header_t *iui_headless_get_shm_header(iui_port_ctx *ctx)
{
    if (!ctx || !ctx->shm.enabled || !ctx->shm.base)
        return NULL;
    return (iui_shm_header_t *) ctx->shm.base;
}

/* Process pending events from the shared memory ring buffer. */
void iui_headless_process_shm_events(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->shm.enabled || !ctx->shm.base)
        return;

    iui_shm_header_t *hdr = (iui_shm_header_t *) ctx->shm.base;
    iui_shm_event_t *events = iui_shm_get_events(ctx->shm.base);

    /* Process all pending events */
    while (hdr->event_read_idx != hdr->event_write_idx) {
        uint32_t idx = hdr->event_read_idx % IUI_SHM_EVENT_RING_SIZE;
        iui_shm_event_t *ev = &events[idx];

        switch (ev->type) {
        case IUI_SHM_EVENT_MOUSE_MOVE:
            ctx->pending_input.mouse_x = ev->x;
            ctx->pending_input.mouse_y = ev->y;
            ctx->has_pending_mouse = true;
            break;

        case IUI_SHM_EVENT_MOUSE_CLICK:
            ctx->pending_input.mouse_x = ev->x;
            ctx->pending_input.mouse_y = ev->y;
            ctx->pending_input.mouse_pressed = ev->key;
            ctx->pending_input.mouse_released = ev->key;
            ctx->has_pending_mouse = true;
            break;

        case IUI_SHM_EVENT_MOUSE_DOWN:
            ctx->pending_input.mouse_x = ev->x;
            ctx->pending_input.mouse_y = ev->y;
            ctx->pending_input.mouse_pressed = ev->key;
            ctx->has_pending_mouse = true;
            break;

        case IUI_SHM_EVENT_MOUSE_UP:
            ctx->pending_input.mouse_x = ev->x;
            ctx->pending_input.mouse_y = ev->y;
            ctx->pending_input.mouse_released = ev->key;
            ctx->has_pending_mouse = true;
            break;

        case IUI_SHM_EVENT_KEY_PRESS:
            ctx->pending_input.key = (int) ev->key;
            break;

        case IUI_SHM_EVENT_TEXT_INPUT:
            ctx->pending_input.text = ev->text;
            break;

        case IUI_SHM_EVENT_SCROLL:
            ctx->pending_input.scroll_x = ev->x;
            ctx->pending_input.scroll_y = ev->y;
            break;

        default:
            break;
        }

        /* Advance read index */
        hdr->event_read_idx++;
    }
#else
    (void) ctx;
#endif
}

/* Process pending commands from external tools. */
void iui_headless_process_shm_commands(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->shm.enabled || !ctx->shm.base)
        return;

    iui_shm_header_t *hdr = (iui_shm_header_t *) ctx->shm.base;

    /* Check for new command */
    if (hdr->command_seq == ctx->shm.last_cmd_seq)
        return;

    ctx->shm.last_cmd_seq = hdr->command_seq;
    hdr->response_status = IUI_SHM_STATUS_PENDING;

    switch (hdr->command) {
    case IUI_SHM_CMD_SCREENSHOT:
        if (iui_headless_save_screenshot(ctx, hdr->command_path))
            hdr->response_status = IUI_SHM_STATUS_OK;
        else
            hdr->response_status = IUI_SHM_STATUS_ERROR;
        break;

    case IUI_SHM_CMD_RESET_STATS:
        iui_headless_reset_stats(ctx);
        hdr->response_status = IUI_SHM_STATUS_OK;
        break;

    case IUI_SHM_CMD_EXIT:
        iui_port_request_exit(&ctx->running, &ctx->exit_requested);
        hdr->response_status = IUI_SHM_STATUS_OK;
        break;

    case IUI_SHM_CMD_GET_STATS:
        /* Stats are already in the header, just acknowledge */
        hdr->response_status = IUI_SHM_STATUS_OK;
        break;

    default:
        hdr->response_status = IUI_SHM_STATUS_ERROR;
        break;
    }

    /* Mark response as complete */
    hdr->response_seq = hdr->command_seq;
    hdr->command = IUI_SHM_CMD_NONE;
#else
    (void) ctx;
#endif
}

/* Update shared memory statistics. */
void iui_headless_update_shm_stats(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->shm.enabled || !ctx->shm.base)
        return;

    iui_shm_header_t *hdr = (iui_shm_header_t *) ctx->shm.base;
    hdr->running = ctx->running ? 1 : 0;
    hdr->frame_count = (uint32_t) ctx->frame_count;
    hdr->timestamp_ns = get_timestamp_ns();
    hdr->draw_box_calls = ctx->stats.draw_box_calls;
    hdr->draw_line_calls = ctx->stats.draw_line_calls;
    hdr->draw_circle_calls = ctx->stats.draw_circle_calls;
    hdr->draw_arc_calls = ctx->stats.draw_arc_calls;
    hdr->set_clip_calls = ctx->stats.set_clip_calls;
    hdr->path_stroke_calls = ctx->stats.path_stroke_calls;
    hdr->total_pixels_drawn = ctx->stats.total_pixels_drawn;
#else
    (void) ctx;
#endif
}

/* Copy framebuffer to shared memory. */
void iui_headless_sync_shm_framebuffer(iui_port_ctx *ctx)
{
#if HEADLESS_ENABLE_FRAMEBUFFER
    if (!ctx || !ctx->shm.enabled || !ctx->shm.base || !ctx->framebuffer)
        return;

    uint32_t *shm_fb = iui_shm_get_framebuffer(ctx->shm.base);
    memcpy(shm_fb, ctx->framebuffer, ctx->fb_size * sizeof(uint32_t));
#else
    (void) ctx;
#endif
}
