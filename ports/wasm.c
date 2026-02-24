/*
 * WebAssembly Backend Implementation for libiui
 *
 * This backend provides direct Canvas 2D API integration without SDL2
 * dependency, reducing binary size. Key features:
 * - Direct Canvas 2D command-based rendering
 * - Browser-native event handling via exported C functions
 * - Single-threaded design (WebAssembly execution model)
 * - Vector font path rendering to Canvas 2D context
 *
 * JavaScript Integration:
 * - IuiCanvas.init(width, height) - Initialize canvas
 * - IuiCanvas.updateCanvas() - Render framebuffer to canvas
 * - iui_wasm_mouse_*() - Inject mouse events from JS
 * - iui_wasm_key() - Inject keyboard events from JS
 */

#include <emscripten.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "port-sw.h"
#include "port.h"

/* WebAssembly port context */
struct iui_port_ctx {
    int width, height; /* Logical width/height */

    /* State */
    bool running;
    bool exit_requested;

    /* Timing */
    double last_frame_time;
    float delta_time;

    /* Queued input (from JavaScript event handlers) */
    iui_port_input queued_input;

    /* DPI scale (for reporting only - rendering uses logical coordinates) */
    float scale;

    /* Rasterizer context (shared with port-sw.h) */
    iui_raster_ctx_t raster;

    /* Vector path state (shared with port-sw.h) */
    iui_path_state_t path;

    /* Callbacks */
    iui_renderer_t render_ops;
    iui_vector_t vector_ops;
};

/* Global context for JavaScript callbacks
 * WebAssembly is single-threaded, so this is safe.
 */
static iui_port_ctx *g_wasm_ctx = NULL;

/* Renderer Callbacks (iui_renderer_t implementation) */

/* clang-format off */

static void wasm_draw_box(iui_rect_t rect,
                          float radius,
                          uint32_t srgb_color,
                          void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.fillStyle = IuiCanvas.parseColor($4);
            ctx.beginPath();
            if ($5 > 0)
                ctx.roundRect($0, $1, $2, $3, $5);
            else
                ctx.rect($0, $1, $2, $3);
            ctx.fill();
    }, rect.x, rect.y, rect.width, rect.height, srgb_color, radius);
}

static void wasm_set_clip_rect(uint16_t min_x,
                               uint16_t min_y,
                               uint16_t max_x,
                               uint16_t max_y,
                               void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.restore();
            ctx.save();
            ctx.beginPath();
            ctx.rect($0, $1, $2 - $0, $3 - $1);
            ctx.clip();
    }, min_x, min_y, max_x, max_y);
}

static void wasm_draw_line(float x0,
                           float y0,
                           float x1,
                           float y1,
                           float width,
                           uint32_t srgb_color,
                           void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.strokeStyle = IuiCanvas.parseColor($5);
            ctx.lineWidth = $4;
            ctx.lineCap = "round";
            ctx.beginPath();
            ctx.moveTo($0, $1);
            ctx.lineTo($2, $3);
            ctx.stroke();
    }, x0, y0, x1, y1, width, srgb_color);
}

static void wasm_draw_circle(float cx,
                             float cy,
                             float radius,
                             uint32_t fill_color,
                             uint32_t stroke_color,
                             float stroke_width,
                             void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.beginPath();
            ctx.arc($0, $1, $2, 0, Math.PI * 2);
            if ($3 !== 0) {
                ctx.fillStyle = IuiCanvas.parseColor($3);
                ctx.fill();
            }
            if ($4 !== 0 && $5 > 0) {
                ctx.strokeStyle = IuiCanvas.parseColor($4);
                ctx.lineWidth = $5;
                ctx.stroke();
            }
    }, cx, cy, radius, fill_color, stroke_color, stroke_width);
}

static void wasm_draw_arc(float cx,
                          float cy,
                          float radius,
                          float start_angle,
                          float end_angle,
                          float width,
                          uint32_t srgb_color,
                          void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.strokeStyle = IuiCanvas.parseColor($6);
            ctx.lineWidth = $5;
            ctx.lineCap = "round";
            ctx.beginPath();
            ctx.arc($0, $1, $2, $3, $4);
            ctx.stroke();
    }, cx, cy, radius, start_angle, end_angle, width, srgb_color);
}

/* Vector Font Callbacks (iui_vector_t implementation) */

static void wasm_path_move(float x, float y, void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.beginPath();
            ctx.moveTo($0, $1);
    }, x, y);
}

static void wasm_path_line(float x, float y, void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.lineTo($0, $1);
    }, x, y);
}

static void wasm_path_curve(float x1,
                            float y1,
                            float x2,
                            float y2,
                            float x3,
                            float y3,
                            void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.bezierCurveTo($0, $1, $2, $3, $4, $5);
    }, x1, y1, x2, y2, x3, y3);
}

static void wasm_path_stroke(float width, uint32_t color, void *user)
{
    EM_ASM({
            const ctx = IuiCanvas.getContext();
            ctx.strokeStyle = IuiCanvas.parseColor($1);
            ctx.lineWidth = $0;
            ctx.lineJoin = "round";
            ctx.lineCap = "round";
            ctx.stroke();
    }, width, color);
}

/* clang-format on */

/* Port Interface Implementation (iui_port_t) */

static iui_port_ctx *wasm_init(int width, int height, const char *title)
{
    (void) title; /* Browser handles window title */

    iui_port_ctx *ctx = (iui_port_ctx *) calloc(1, sizeof(iui_port_ctx));
    if (!ctx)
        return NULL;

    ctx->width = width;
    ctx->height = height;
    ctx->scale = 1.0f;
    ctx->running = true;
    ctx->exit_requested = false;



    /* Initialize path state */
    iui_path_reset(&ctx->path);

    /* Get device pixel ratio from JavaScript */
    ctx->scale =
        (float) EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });

    /* Store global context for JavaScript callbacks */
    g_wasm_ctx = ctx;

    /* Initialize Canvas via JavaScript
     * IuiCanvas is defined in assets/web/iui-wasm.js which must be loaded
     * before the WASM module. This avoids complex EM_ASM with object literals.
     */
    EM_ASM({ IuiCanvas.init($0, $1); }, width, height);

    /* Initialize timing */
    ctx->last_frame_time = emscripten_get_now();

    return ctx;
}

static void wasm_shutdown(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

    /* Cleanup Canvas via JavaScript */
    EM_ASM({ IuiCanvas.destroy(); });



    free(ctx);
    g_wasm_ctx = NULL;
}

static void wasm_configure(iui_port_ctx *ctx)
{
    if (!ctx)
        return;

    /* Initialize renderer callbacks */
    ctx->render_ops.draw_box = wasm_draw_box;
    ctx->render_ops.draw_text = NULL; /* Use vector font */
    ctx->render_ops.text_width = NULL;
    ctx->render_ops.set_clip_rect = wasm_set_clip_rect;
    ctx->render_ops.draw_line = wasm_draw_line;
    ctx->render_ops.draw_circle = wasm_draw_circle;
    ctx->render_ops.draw_arc = wasm_draw_arc;
    ctx->render_ops.user = ctx;

    /* Initialize vector callbacks */
    ctx->vector_ops.path_move = wasm_path_move;
    ctx->vector_ops.path_line = wasm_path_line;
    ctx->vector_ops.path_curve = wasm_path_curve;
    ctx->vector_ops.path_stroke = wasm_path_stroke;
}

static bool wasm_poll_events(iui_port_ctx *ctx)
{
    if (!ctx)
        return false;

    /* Update delta time */
    double now = emscripten_get_now();
    ctx->delta_time = (float) (now - ctx->last_frame_time) / 1000.f;
    ctx->last_frame_time = now;

    /* Cap delta time to prevent jumps */
    if (ctx->delta_time > 0.1f)
        ctx->delta_time = IUI_PORT_FRAME_DT;

    /* Events are injected via exported functions, no polling needed */
    return ctx->running;
}

static bool wasm_should_exit(iui_port_ctx *ctx)
{
    return ctx ? ctx->exit_requested : true;
}

static void wasm_request_exit(iui_port_ctx *ctx)
{
    if (ctx)
        iui_port_request_exit(&ctx->running, &ctx->exit_requested);
}

static void wasm_get_input(iui_port_ctx *ctx, iui_port_input *input)
{
    if (!ctx || !input)
        return;
    iui_port_consume_input(input, &ctx->queued_input);
}

static void wasm_begin_frame(iui_port_ctx *ctx)
{
    if (!ctx)
        return;


    /* clang-format off */
    /* Reset canvas transform, clear with background color, and prepare clip stack */
    EM_ASM({
            const context = IuiCanvas.getContext();
            if (context) {
                const dpr = window.devicePixelRatio || 1;
                context.setTransform(dpr, 0, 0, dpr, 0, 0);
                context.fillStyle = "#282c34";
                context.fillRect(0, 0, $0, $1);
                context.save();
            }
    }, ctx->width, ctx->height);
    /* clang-format on */
}

static void wasm_end_frame(iui_port_ctx *ctx)
{
    if (!ctx)
        return;
}

static iui_renderer_t wasm_get_renderer_callbacks(iui_port_ctx *ctx)
{
    if (!ctx) {
        iui_renderer_t empty = {0};
        return empty;
    }
    return ctx->render_ops;
}

static const iui_vector_t *wasm_get_vector_callbacks(iui_port_ctx *ctx)
{
    if (!ctx)
        return NULL;
    return &ctx->vector_ops;
}

static float wasm_get_delta_time(iui_port_ctx *ctx)
{
    return ctx ? ctx->delta_time : 0.016f;
}

static void wasm_get_window_size(iui_port_ctx *ctx, int *width, int *height)
{
    if (!ctx)
        return;
    if (width)
        *width = ctx->width;
    if (height)
        *height = ctx->height;
}

static void wasm_set_window_size(iui_port_ctx *ctx, int width, int height)
{
    /* Resizing requires framebuffer reallocation - not implemented yet */
    (void) ctx;
    (void) width;
    (void) height;
}

static float wasm_get_dpi_scale(iui_port_ctx *ctx)
{
    return ctx ? ctx->scale : 1.0f;
}

static bool wasm_is_window_focused(iui_port_ctx *ctx)
{
    (void) ctx;
    /* Browser context is always "focused" if running */
    return EM_ASM_INT({ return document.hasFocus() ? 1 : 0; }) != 0;
}

static bool wasm_is_window_visible(iui_port_ctx *ctx)
{
    (void) ctx;
    /* clang-format off */
    return EM_ASM_INT({
               return document.visibilityState === 'visible' ? 1 : 0;
           }) != 0;
    /* clang-format on */
}

static const char *wasm_get_clipboard_text(iui_port_ctx *ctx)
{
    (void) ctx;
    /* Clipboard access in browser requires async API - not implemented */
    return NULL;
}

static void wasm_set_clipboard_text(iui_port_ctx *ctx, const char *text)
{
    (void) ctx;
    (void) text;
    /* Clipboard access in browser requires async API - not implemented */
}



/* Exported Functions for JavaScript Event Injection */

EMSCRIPTEN_KEEPALIVE
void iui_wasm_mouse_motion(int x, int y, int buttons)
{
    if (!g_wasm_ctx)
        return;

    g_wasm_ctx->queued_input.mouse_x = (float) x;
    g_wasm_ctx->queued_input.mouse_y = (float) y;

    /* Convert browser button state to iui format */
    (void) buttons; /* Track for drag operations if needed */
}

EMSCRIPTEN_KEEPALIVE
void iui_wasm_mouse_button(int x, int y, int button, int down)
{
    if (!g_wasm_ctx)
        return;

    g_wasm_ctx->queued_input.mouse_x = (float) x;
    g_wasm_ctx->queued_input.mouse_y = (float) y;

    /* Map browser button to iui_mouse_button enum:
     * Browser: 0=left, 1=middle, 2=right
     * iui: IUI_MOUSE_LEFT=1, IUI_MOUSE_RIGHT=2, IUI_MOUSE_MIDDLE=4
     */
    uint8_t iui_button = 0;
    switch (button) {
    case 0:
        iui_button = IUI_MOUSE_LEFT;
        break;
    case 1:
        iui_button = IUI_MOUSE_MIDDLE;
        break;
    case 2:
        iui_button = IUI_MOUSE_RIGHT;
        break;
    }

    if (down)
        g_wasm_ctx->queued_input.mouse_pressed |= iui_button;
    else
        g_wasm_ctx->queued_input.mouse_released |= iui_button;
}

EMSCRIPTEN_KEEPALIVE
void iui_wasm_scroll(float dx, float dy)
{
    if (!g_wasm_ctx)
        return;

    g_wasm_ctx->queued_input.scroll_x += dx;
    g_wasm_ctx->queued_input.scroll_y += dy;
}

EMSCRIPTEN_KEEPALIVE
void iui_wasm_key(int keycode, int down, int shift)
{
    if (!g_wasm_ctx)
        return;

    g_wasm_ctx->queued_input.shift_down = (shift != 0);

    if (down && g_wasm_ctx->queued_input.key == 0) {
        /* Only store first key event per frame */
        g_wasm_ctx->queued_input.key = keycode;
    }
}

EMSCRIPTEN_KEEPALIVE
void iui_wasm_char(int codepoint)
{
    if (!g_wasm_ctx)
        return;

    if (g_wasm_ctx->queued_input.text == 0) {
        g_wasm_ctx->queued_input.text = (uint32_t) codepoint;
    }
}



EMSCRIPTEN_KEEPALIVE
int iui_wasm_get_width(void)
{
    return g_wasm_ctx ? g_wasm_ctx->width : 0;
}

EMSCRIPTEN_KEEPALIVE
int iui_wasm_get_height(void)
{
    return g_wasm_ctx ? g_wasm_ctx->height : 0;
}

EMSCRIPTEN_KEEPALIVE
void iui_wasm_shutdown(void)
{
    if (!g_wasm_ctx)
        return;
    g_wasm_ctx->running = false;
    g_wasm_ctx->exit_requested = true;
}

/* Global Backend Instance */

const iui_port_t g_iui_port = {
    .init = wasm_init,
    .shutdown = wasm_shutdown,
    .configure = wasm_configure,
    .poll_events = wasm_poll_events,
    .should_exit = wasm_should_exit,
    .request_exit = wasm_request_exit,
    .get_input = wasm_get_input,
    .begin_frame = wasm_begin_frame,
    .end_frame = wasm_end_frame,
    .get_renderer_callbacks = wasm_get_renderer_callbacks,
    .get_vector_callbacks = wasm_get_vector_callbacks,
    .get_delta_time = wasm_get_delta_time,
    .get_window_size = wasm_get_window_size,
    .set_window_size = wasm_set_window_size,
    .get_dpi_scale = wasm_get_dpi_scale,
    .is_window_focused = wasm_is_window_focused,
    .is_window_visible = wasm_is_window_visible,
    .get_clipboard_text = wasm_get_clipboard_text,
    .set_clipboard_text = wasm_set_clipboard_text,
    .get_native_renderer = NULL,
};
