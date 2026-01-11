#include "internal.h"

/* MD3 Runtime Validation - global state (when enabled) */
#ifdef IUI_MD3_RUNTIME_VALIDATION
iui_md3_validation_state_t g_md3_validation = {0};
#endif

/* MD3 Motion System - const declarations */

#ifdef CONFIG_FEATURE_ANIMATION
static const iui_motion_config iui_motion_standard = {
    .enter_duration = IUI_DURATION_SHORT_4, /* 200ms */
    .exit_duration = IUI_DURATION_SHORT_4,  /* 200ms */
    .enter_easing = IUI_EASING_STANDARD_DECELERATE,
    .exit_easing = IUI_EASING_STANDARD_ACCELERATE,
};

static const iui_motion_config iui_motion_emphasized = {
    .enter_duration = IUI_DURATION_MEDIUM_2, /* 300ms */
    .exit_duration = IUI_DURATION_SHORT_4,   /* 200ms */
    .enter_easing = IUI_EASING_EMPHASIZED_DECELERATE,
    .exit_easing = IUI_EASING_EMPHASIZED_ACCELERATE,
};

static const iui_motion_config iui_motion_quick = {
    .enter_duration = IUI_DURATION_SHORT_2, /* 100ms */
    .exit_duration = IUI_DURATION_SHORT_1,  /* 50ms */
    .enter_easing = IUI_EASING_STANDARD_DECELERATE,
    .exit_easing = IUI_EASING_STANDARD_ACCELERATE,
};

static const iui_motion_config iui_motion_dialog = {
    .enter_duration = IUI_DURATION_MEDIUM_3, /* 350ms */
    .exit_duration = IUI_DURATION_MEDIUM_1,  /* 250ms */
    .enter_easing = IUI_EASING_EMPHASIZED_DECELERATE,
    .exit_easing = IUI_EASING_STANDARD_ACCELERATE,
};

static const iui_motion_config iui_motion_menu = {
    .enter_duration = IUI_DURATION_MEDIUM_2, /* 300ms */
    .exit_duration = IUI_DURATION_SHORT_4,   /* 200ms */
    .enter_easing = IUI_EASING_STANDARD_DECELERATE,
    .exit_easing = IUI_EASING_STANDARD_ACCELERATE,
};

/* Getter functions for predefined motion configurations */
const iui_motion_config *iui_motion_get_standard(void)
{
    return &iui_motion_standard;
}

const iui_motion_config *iui_motion_get_emphasized(void)
{
    return &iui_motion_emphasized;
}

const iui_motion_config *iui_motion_get_quick(void)
{
    return &iui_motion_quick;
}

const iui_motion_config *iui_motion_get_dialog(void)
{
    return &iui_motion_dialog;
}

const iui_motion_config *iui_motion_get_menu(void)
{
    return &iui_motion_menu;
}

/* Cubic Bezier Easing Functions
 * Calculate y coordinate of cubic bezier curve at parameter t
 * @t  : Parameter value [0,1]
 * @y1 : Y coordinate of first control point
 * @y2 : Y coordinate of second control point
 */
static float cubic_bezier_y(float t, float y1, float y2)
{
    float u = 1.f - t;
    float tt = t * t, uu = u * u;
    float ttt = tt * t;

    /* B(t) = (1-t)^3*P0.y + 3*(1-t)^2*t*P1.y + 3*(1-t)*t^2*P2.y + t^3*P3.y
     * With P0.y=0 and P3.y=1:
     */
    return 3.f * uu * t * y1 + 3.f * u * tt * y2 + ttt;
}

/* Solve for t given x using Newton-Raphson iteration
 * @target_x : Target x-coordinate value [0,1]
 * @x1       : X coordinate of first control point
 * @x2       : X coordinate of second control point
 *
 * Returns the t value where the bezier x-coordinate equals target_x
 */
static float bezier_solve_t(float target_x, float x1, float x2)
{
    if (target_x <= 0.f)
        return 0.f;
    if (target_x >= 1.f)
        return 1.f;

    float t = target_x; /* Initial guess */

    /* Newton-Raphson iterations (5 is usually enough for convergence) */
    for (int i = 0; i < 5; i++) {
        float u = 1.f - t, tt = t * t, uu = u * u;

        /* x(t) = 3*(1-t)^2*t*x1 + 3*(1-t)*t^2*x2 + t^3 */
        float x = 3.f * uu * t * x1 + 3.f * u * tt * x2 + tt * t,
              dx = 3.f * uu * x1 + 6.f * u * t * (x2 - x1) +
                   3.f * tt * (1.f - x2),
              error = x - target_x;

        if (fabsf(dx) < 1e-6f)
            break;

        if (fabsf(error) < 1e-6f)
            break;

        t -= error / dx;
        t = fmaxf(0.f, fminf(1.f, t));
    }

    return t;
}

/* Apply cubic bezier easing curve.
 * @x:  Input value [0,1]
 * @x1: X coordinate of first control point (P1)
 * @y1: Y coordinate of first control point (P1)
 * @x2: X coordinate of second control point (P2)
 * @y2: Y coordinate of second control point (P2)
 *
 * Returns output value after applying cubic bezier easing.
 */
static float ease_cubic_bezier(float x, float x1, float y1, float x2, float y2)
{
    if (x <= 0.f)
        return 0.f;
    if (x >= 1.f)
        return 1.f;

    float t = bezier_solve_t(x, x1, x2);
    return cubic_bezier_y(t, y1, y2);
}

/* MD3 Easing and Motion Functions */

float iui_ease(float t, iui_easing_t easing)
{
    if (t <= 0.f)
        return 0.f;
    if (t >= 1.f)
        return 1.f;

    switch (easing) {
    case IUI_EASING_LINEAR:
        return t;

    case IUI_EASING_STANDARD:
        /* MD3 Standard: cubic-bezier(0.2, 0.0, 0, 1.0) */
        return ease_cubic_bezier(t, 0.2f, 0.0f, 0.0f, 1.0f);

    case IUI_EASING_STANDARD_DECELERATE:
        /* MD3 Standard Decelerate: cubic-bezier(0.0, 0.0, 0, 1.0) */
        return ease_cubic_bezier(t, 0.0f, 0.0f, 0.0f, 1.0f);

    case IUI_EASING_STANDARD_ACCELERATE:
        /* MD3 Standard Accelerate: cubic-bezier(0.3, 0.0, 1.0, 1.0) */
        return ease_cubic_bezier(t, 0.3f, 0.0f, 1.0f, 1.0f);

    case IUI_EASING_EMPHASIZED:
        /* MD3 Emphasized: cubic-bezier(0.2, 0.0, 0, 1.0) (same as standard) */
        return ease_cubic_bezier(t, 0.2f, 0.0f, 0.0f, 1.0f);

    case IUI_EASING_EMPHASIZED_DECELERATE:
        /* MD3 Emphasized Decelerate: cubic-bezier(0.05, 0.7, 0.1, 1.0) */
        return ease_cubic_bezier(t, 0.05f, 0.7f, 0.1f, 1.0f);

    case IUI_EASING_EMPHASIZED_ACCELERATE:
        /* MD3 Emphasized Accelerate: cubic-bezier(0.3, 0.0, 0.8, 0.15) */
        return ease_cubic_bezier(t, 0.3f, 0.0f, 0.8f, 0.15f);

    default:
        return t;
    }
}

/* Apply motion configuration for asymmetric enter/exit animations */
float iui_motion_apply(float t,
                       bool is_entering,
                       const iui_motion_config *config)
{
    if (!config)
        return t;
    iui_easing_t easing =
        is_entering ? config->enter_easing : config->exit_easing;
    return iui_ease(t, easing);
}

/* Get appropriate duration from motion config */
float iui_motion_get_duration(bool is_entering, const iui_motion_config *config)
{
    if (!config)
        return IUI_DURATION_SHORT_4; /* Default 200ms */
    return is_entering ? config->enter_duration : config->exit_duration;
}

/* Calculate animation progress from elapsed time using motion config */
float iui_motion_progress(float elapsed,
                          bool is_entering,
                          const iui_motion_config *config)
{
    float duration = iui_motion_get_duration(is_entering, config);
    if (duration <= 0.f)
        return 1.f;

    float t = elapsed / duration;
    t = fminf(1.f, fmaxf(0.f, t));

    return iui_motion_apply(t, is_entering, config);
}
#endif /* CONFIG_FEATURE_ANIMATION */

/* MD3 theme presets */

const iui_theme_t g_theme_light = {
    /* Primary color group */
    .primary = 0xFF6750A4,
    .on_primary = 0xFFFFFFFF,
    .primary_container = 0xFFE0D0F8,
    .on_primary_container = 0xFF21005D,

    /* Secondary color group */
    .secondary = 0xFF625B71,
    .on_secondary = 0xFFFFFFFF,
    .secondary_container = 0xFFE8DEF8,
    .on_secondary_container = 0xFF1D192B,

    /* Tertiary color group (complementary rose/pink) */
    .tertiary = 0xFF7D5260,
    .on_tertiary = 0xFFFFFFFF,
    .tertiary_container = 0xFFFFD8E4,
    .on_tertiary_container = 0xFF31111D,

    /* Surface color group (5-level elevation hierarchy) */
    .surface = 0xFFFEF7FF,
    .on_surface = 0xFF1D1B20,
    .surface_variant = 0xFFE7E0EC,
    .on_surface_variant = 0xFF49454F,
    .surface_container_lowest = 0xFFFFFFFF,
    .surface_container_low = 0xFFF7F2FA,
    .surface_container = 0xFFF3EDF7,
    .surface_container_high = 0xFFECE6F0,
    .surface_container_highest = 0xFFE6E0E9,

    /* Outline group */
    .outline = 0xFF79747E,
    .outline_variant = 0xFFCAC4D0,

    /* Error color group */
    .error = 0xFFB3261E,
    .on_error = 0xFFFFFFFF,
    .error_container = 0xFFF9DEDC,
    .on_error_container = 0xFF410E0B,

    /* Utility colors */
    .shadow = 0xFF000000,
    .scrim = 0xFF000000,
    .inverse_surface = 0xFF313033,
    .inverse_on_surface = 0xFFF4EFF4,
    .inverse_primary = 0xFFD0BCFF,
};

/* MD3 Dark Theme (Purple seed #6750A4 from Material Theme Builder) */
const iui_theme_t g_theme_dark = {
    /* Primary color group */
    .primary = 0xFFD0BCFF,
    .on_primary = 0xFF381E72,
    .primary_container = 0xFF4F378B,
    .on_primary_container = 0xFFEADDFF,

    /* Secondary color group */
    .secondary = 0xFFCCC2DC,
    .on_secondary = 0xFF332D41,
    .secondary_container = 0xFF4A4458,
    .on_secondary_container = 0xFFE8DEF8,

    /* Tertiary color group (complementary rose/pink) */
    .tertiary = 0xFFEFB8C8,
    .on_tertiary = 0xFF492532,
    .tertiary_container = 0xFF633B48,
    .on_tertiary_container = 0xFFFFD8E4,

    /* Surface color group (5-level elevation hierarchy) */
    .surface = 0xFF141218,
    .on_surface = 0xFFE6E0E9,
    .surface_variant = 0xFF49454F,
    .on_surface_variant = 0xFFCAC4D0,
    .surface_container_lowest = 0xFF0F0D13,
    .surface_container_low = 0xFF1D1B20,
    .surface_container = 0xFF211F26,
    .surface_container_high = 0xFF2B2930,
    .surface_container_highest = 0xFF36343B,

    /* Outline group */
    .outline = 0xFF938F99,
    .outline_variant = 0xFF49454F,

    /* Error color group */
    .error = 0xFFF2B8B5,
    .on_error = 0xFF601410,
    .error_container = 0xFF8C1D18,
    .on_error_container = 0xFFF9DEDC,

    /* Utility colors */
    .shadow = 0xFF000000,
    .scrim = 0xFF000000,
    .inverse_surface = 0xFFE6E0E9,
    .inverse_on_surface = 0xFF313033,
    .inverse_primary = 0xFF6750A4,
};

/* MD3 Shape Token Definitions */

const iui_shape_tokens iui_shape_tokens_default = {
    .none = 0.f,
    .extra_small = 2.f,
    .small = 4.f,
    .medium = 8.f,
    .large = 12.f,
    .extra_large = 16.f,
    .full = 0.f, /* This will be calculated per element */
};

const iui_shape_tokens iui_shape_tokens_compact = {
    .none = 0.f,
    .extra_small = 1.f,
    .small = 2.f,
    .medium = 4.f,
    .large = 8.f,
    .extra_large = 12.f,
    .full = 0.f, /* This will be calculated per element */
};

/* MD3 Typography Scale Definitions */

const iui_typography_scale iui_typography_scale_default = {
    .headline_small = 24.f,
    .title_large = 22.f,
    .title_medium = 16.f,
    .title_small = 14.f,
    .body_large = 16.f,
    .body_medium = 14.f,
    .body_small = 12.f,
    .label_large = 14.f,
    .label_medium = 12.f,
    .label_small = 11.f,
    /* The rest are set to 0 as they are not commonly used */
    .display_large = 0.f,
    .display_medium = 0.f,
    .display_small = 0.f,
    .headline_large = 0.f,
    .headline_medium = 0.f,
};

const iui_typography_scale iui_typography_scale_dense = {
    .headline_small = 20.f,
    .title_large = 18.f,
    .title_medium = 14.f,
    .title_small = 12.f,
    .body_large = 14.f,
    .body_medium = 12.f,
    .body_small = 11.f,
    .label_large = 12.f,
    .label_medium = 11.f,
    .label_small = 10.f,
    /* The rest are set to 0 as they are not commonly used */
    .display_large = 0.f,
    .display_medium = 0.f,
    .display_small = 0.f,
    .headline_large = 0.f,
    .headline_medium = 0.f,
};

/* MD3 spacing token definitions */

static const iui_spacing_tokens iui_spacing_tokens_default = {
    .none = 0.f,
    .xxs = 4.f,  /* micro gaps, icon internal padding */
    .xs = 8.f,   /* standard component gaps, divider margins */
    .sm = 12.f,  /* menu padding, content spacing */
    .md = 16.f,  /* container padding, FAB gaps, search bar padding */
    .lg = 24.f,  /* dialog padding, section spacing */
    .xl = 32.f,  /* large section margins */
    .xxl = 48.f, /* touch targets, major section breaks */
};

/* Vector Font Rendering */

void iui_compute_vector_metrics(float font_height,
                                float *out_ascent_px,
                                float *out_descent_px)
{
    if (font_height <= 0.f) {
        *out_ascent_px = 0.f;
        *out_descent_px = 0.f;
        return;
    }
    float scale = font_height / IUI_FONT_UNITS_PER_EM;
    int max_ascent = 0, max_descent = 0;
    for (int c = 32; c < 127; ++c) {
        const signed char *g = iui_glyph_table + iui_glyph_offsets[c];
        if (IUI_GLYPH_ASCENT(g) > max_ascent)
            max_ascent = IUI_GLYPH_ASCENT(g);
        if (IUI_GLYPH_DESCENT(g) > max_descent)
            max_descent = IUI_GLYPH_DESCENT(g);
    }
    *out_ascent_px = max_ascent * scale;
    *out_descent_px = max_descent * scale;
}

/* Keep spacing tied to stroke weight so glyphs have consistent side bearings.
 */
float iui_vector_pen_for_height(float font_height)
{
    if (font_height <= 0.f)
        return 0.f;
    return fmaxf(font_height / IUI_FONT_PEN_WIDTH_DIVISOR,
                 IUI_FONT_PEN_WIDTH_MIN);
}

/* Calculate side bearing for vector text (space beyond glyph bounds)
 * Accounts for pen width to prevent clipping at edges
 * @scale: Font scale factor
 * @pen_w: Pen width for vector drawing
 *
 * Returns calculated side bearing value
 */
static float iui_vector_side_bearing(float scale, float pen_w)
{
    /* Leave breathing room even when strokes get thin at small sizes */
    const float min_margin = scale * IUI_FONT_SIDE_BEARING_MIN;
    return fmaxf(pen_w + scale * IUI_FONT_SIDE_BEARING_EXTRA, min_margin);
}

float iui_text_width_vec(const char *text, float font_height)
{
    if (font_height <= 0.f)
        return 0.f;

    float w = 0.f, scale = font_height / IUI_FONT_UNITS_PER_EM;
    const float pen_w = iui_vector_pen_for_height(font_height);
    const float side = iui_vector_side_bearing(scale, pen_w);
    for (; *text; ++text) {
        unsigned char c = (unsigned char) *text;
        const signed char *g = iui_get_glyph(c);
        float glyph_w =
            (float) (IUI_GLYPH_RIGHT(g) - IUI_GLYPH_LEFT(g)) * scale;
        w += glyph_w + side * 2.f;
    }
    return w;
}

/* Codepoint width measurement for built-in vector font.
 * Returns the width of a Unicode codepoint at the given font height.
 * Codepoints outside ASCII range (0x20-0x7E) use the fallback box glyph.
 */
float iui_codepoint_width_vec(uint32_t cp, float font_height)
{
    if (font_height <= 0.f)
        return 0.f;

    float scale = font_height / IUI_FONT_UNITS_PER_EM;
    const float pen_w = iui_vector_pen_for_height(font_height);
    const float side = iui_vector_side_bearing(scale, pen_w);
    /* iui_get_glyph handles out-of-range codepoints by returning box glyph */
    const signed char *g = iui_get_glyph((unsigned char) (cp > 0x7F ? 0 : cp));
    float glyph_w = (float) (IUI_GLYPH_RIGHT(g) - IUI_GLYPH_LEFT(g)) * scale;
    return glyph_w + side * 2.f;
}

/* Emit vector commands for drawing a glyph.
 * @ctx:     Current UI context
 * @g:       Pointer to glyph data
 * @base_x:  X coordinate for glyph placement
 * @base_y:  Y coordinate for glyph placement
 * @color:   Color to draw the glyph with
 */
static void iui_emit_glyph(iui_context *ctx,
                           const signed char *g,
                           float base_x,
                           float base_y,
                           uint32_t color)
{
    float scale = ctx->font_height / IUI_FONT_UNITS_PER_EM;
    float pen_w = ctx->pen_width;
    const signed char *it = IUI_GLYPH_DRAW(g);
    float ox = base_x - IUI_GLYPH_LEFT(g) * scale;
    /* Glyph's local coordinate system has baseline at y=0. */
    /* To align glyph's baseline with text baseline (base_y), start at base_y.
     */
    float oy = base_y;
    float x = ox, y = oy;
    bool in_path = false; /* track if path content exists to stroke */

    for (;;) {
        switch (*it++) {
        case 'm':
            /* Stroke any existing sub-path before starting a new one */
            if (ctx->vector && in_path) {
                ctx->vector->path_stroke(pen_w, color, ctx->renderer.user);
                in_path = false;
            }
            x = ox + it[0] * scale;
            y = oy + it[1] * scale;
            if (ctx->vector) {
                float sx = floorf(x + 0.5f);
                float sy = floorf(y + 0.5f);
                ctx->vector->path_move(sx, sy, ctx->renderer.user);
                x = sx;
                y = sy;
            }
            it += 2;
            break;
        case 'l': {
            float nx = ox + it[0] * scale;
            float ny = oy + it[1] * scale;
            if (ctx->vector) {
                float sx = floorf(nx + 0.5f);
                float sy = floorf(ny + 0.5f);
                ctx->vector->path_line(sx, sy, ctx->renderer.user);
                nx = sx, ny = sy;
                in_path = true;
            } else {
                /* Fallback: draw line as thin box */
                float dx = nx - x, dy = ny - y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.5f) {
                    float mx = (x + nx) * 0.5f, my = (y + ny) * 0.5f;
                    float w = fmaxf(len, 1.f), h = fmaxf(pen_w, 1.f);
                    if (fabsf(dx) > fabsf(dy))
                        ctx->renderer.draw_box(
                            (iui_rect_t) {fminf(x, nx), my - h * 0.5f, w, h}, 0,
                            color, ctx->renderer.user);
                    else
                        ctx->renderer.draw_box(
                            (iui_rect_t) {mx - h * 0.5f, fminf(y, ny), h, len},
                            0, color, ctx->renderer.user);
                }
            }
            x = nx, y = ny;
            it += 2;
            break;
        }
        case 'c':
            if (ctx->vector) {
                float cx1 = floorf((ox + it[0] * scale) + 0.5f);
                float cy1 = floorf((oy + it[1] * scale) + 0.5f);
                float cx2 = floorf((ox + it[2] * scale) + 0.5f);
                float cy2 = floorf((oy + it[3] * scale) + 0.5f);
                float cx3 = floorf((ox + it[4] * scale) + 0.5f);
                float cy3 = floorf((oy + it[5] * scale) + 0.5f);
                ctx->vector->path_curve(cx1, cy1, cx2, cy2, cx3, cy3,
                                        ctx->renderer.user);
                in_path = true;
            } else {
                /* Fallback: approximate curve with adaptive subdivision */
                float x0 = x, y0 = y;
                float x1 = ox + it[0] * scale, y1 = oy + it[1] * scale;
                float x2 = ox + it[2] * scale, y2 = oy + it[3] * scale;
                float x3 = ox + it[4] * scale, y3 = oy + it[5] * scale;
                /* Use Manhattan distance for cheap length estimate */
                float approx_len = fabsf(x1 - x0) + fabsf(y1 - y0) +
                                   fabsf(x2 - x1) + fabsf(y2 - y1) +
                                   fabsf(x3 - x2) + fabsf(y3 - y2);
                /* Adaptive segments: 4-12 based on curve size (avoid sqrtf) */
                int segments =
                    (int) fminf(fmaxf(approx_len * 0.25f, 4.f), 12.f);
                float inv_seg = 1.f / (float) segments;
                for (int i = 1; i <= segments; i++) {
                    float t = (float) i * inv_seg;
                    float u = 1.f - t;
                    float nx = u * u * u * x0 + 3 * u * u * t * x1 +
                               3 * u * t * t * x2 + t * t * t * x3;
                    float ny = u * u * u * y0 + 3 * u * u * t * y1 +
                               3 * u * t * t * y2 + t * t * t * y3;
                    float dx = nx - x, dy = ny - y;
                    float len = sqrtf(dx * dx + dy * dy);
                    if (len > 0.5f) {
                        float w = fmaxf(len, 1.f), h = fmaxf(pen_w, 1.f);
                        if (fabsf(dx) > fabsf(dy))
                            ctx->renderer.draw_box(
                                (iui_rect_t) {fminf(x, nx),
                                              (y + ny) * 0.5f - h * 0.5f, w, h},
                                0, color, ctx->renderer.user);
                        else
                            ctx->renderer.draw_box(
                                (iui_rect_t) {(x + nx) * 0.5f - h * 0.5f,
                                              fminf(y, ny), h, len},
                                0, color, ctx->renderer.user);
                    }
                    x = nx;
                    y = ny;
                }
            }
            x = ox + it[4] * scale;
            y = oy + it[5] * scale;
            it += 6;
            break;
        case 'e':
            if (ctx->vector)
                ctx->vector->path_stroke(pen_w, color, ctx->renderer.user);
            return;
        default:
            return; /* safety */
        }
    }
}

void iui_draw_text_vec(iui_context *ctx,
                       float x,
                       float y,
                       const char *text,
                       uint32_t color)
{
    if (ctx->font_height <= 0.f)
        return;
    float scale = ctx->font_height / IUI_FONT_UNITS_PER_EM;
    const float side = iui_vector_side_bearing(scale, ctx->pen_width);
    /* Use measured ascent/descent for stable baseline placement */
    float baseline_y = y + ctx->font_ascent_px;
    /* Snap to pixel grid for crisper strokes */
    baseline_y = floorf(baseline_y + 0.5f);
    float cursor_x = floorf(x + 0.5f);

    for (; *text; ++text) {
        unsigned char c = (unsigned char) *text;
        const signed char *g = iui_get_glyph(c);
        float glyph_w =
            (float) (IUI_GLYPH_RIGHT(g) - IUI_GLYPH_LEFT(g)) * scale;
        iui_emit_glyph(ctx, g, cursor_x + side, baseline_y, color);
        cursor_x += glyph_w + side * 2.f;
    }
}

/* Core Initialization and Input Handling */

size_t iui_min_memory_size(void)
{
    return sizeof(iui_context);
}

iui_config_t iui_make_config(void *buffer,
                             iui_renderer_t renderer,
                             float font_height,
                             const iui_vector_t *vector)
{
    iui_config_t config = {
        .buffer = buffer,
        .renderer = renderer,
        .font_height = font_height,
        .vector = vector,
    };
    return config;
}

bool iui_config_is_valid(const iui_config_t *config)
{
    if (!config)
        return false;

    /* Guard: buffer must not be NULL */
    if (!config->buffer)
        return false;

    /* Guard: either draw_text or vector callbacks must be provided */
    bool has_raster = config->renderer.draw_text;
    bool has_vector = config->vector;
    if (!has_raster && !has_vector)
        return false;

    /* Guard: other required callbacks */
    if (!config->renderer.draw_box || !config->renderer.set_clip_rect)
        return false;

    /* text_width only required if no vector font */
    if (!has_vector && !config->renderer.text_width)
        return false;

    /* Guard: buffer must be properly aligned */
    if (((uintptr_t) config->buffer) % sizeof(uintptr_t) != 0)
        return false;

    /* Guard: invalid font height would collapse layout */
    if (config->font_height <= 0.f)
        return false;

    return true;
}

/* Initialize UI context from configuration.
 *
 * Subsystem Initialization Order:
 *   1. Core state: input, font metrics, padding, theme
 *   2. Clip stack: scissor region management
 *   3. Modal system: overlay blocking and z-order
 *   4. Input layer: double-buffered blocking regions
 *   5. Focus system: keyboard navigation and trapping
 *   6. Input capture: drag operation ownership
 *   7. IME state: international text composition
 *   8. Clipboard: copy/paste callbacks
 *   9. Scroll state: container scroll tracking
 *  10. Token systems: typography, shape, spacing
 *  11. Vector font: pen width and metrics
 *  12. Performance: batching, dirty rects, text cache
 *
 * Returns NULL if config validation fails.
 */
iui_context *iui_init(const iui_config_t *config)
{
    if (!iui_config_is_valid(config))
        return NULL;

    iui_context *ctx = (iui_context *) config->buffer;
    *ctx = (iui_context) {
        .mouse_pressed = 0,
        .mouse_held = 0,
        .mouse_released = 0,
        .font_height = config->font_height,
        .renderer = config->renderer,
        /* MD3 spacing: Use 8dp as standard padding (snapped to 4dp grid)
         * This ensures consistent spacing regardless of font size
         */
        .padding = 8.f,
        .corner = fmaxf(config->font_height / 2.f, 2.f),
        .colors = g_theme_light,
    };
    ctx->row_height = ctx->font_height * 1.5f;
    ctx->vector = config->vector;

    /* Initialize clip stack */
    ctx->clip.depth = 0;
    ctx->current_clip = (iui_clip_rect) {0, 0, UINT16_MAX, UINT16_MAX};

    /* Initialize selection */
    ctx->selection_start = 0;

    /* Initialize modal state */
    ctx->modal.active = false, ctx->modal.rendering = false;
    ctx->modal.clicked_inside = false, ctx->modal.frames_since_open = 0;
    ctx->modal.id = 0, ctx->modal.layer_id = 0;

    /* Initialize input layer system */
    ctx->input_layer.current_buffer = 0, ctx->input_layer.region_count[0] = 0;
    ctx->input_layer.region_count[1] = 0, ctx->input_layer.next_reg_order = 0;
    ctx->input_layer.layer_depth = 0, ctx->input_layer.next_layer_id = 0;
    ctx->input_layer.current_layer_id = 0;

    /* Initialize focus trap state */
    ctx->focus_trap.layer_id = 0, ctx->focus_trap.first_focusable_idx = 0;
    ctx->focus_trap.last_focusable_idx = 0, ctx->focus_trap.active = false;

    /* Initialize input capture state */
    ctx->input_capture.owner_id = 0, ctx->input_capture.bounds_x = 0;
    ctx->input_capture.bounds_y = 0, ctx->input_capture.bounds_w = 0;
    ctx->input_capture.bounds_h = 0, ctx->input_capture.active = false;
    ctx->input_capture.require_start = false;

    /* Initialize IME state */
    ctx->ime.composing_text[0] = '\0';
    ctx->ime.composing_cursor = 0;
    ctx->ime.is_composing = false;

    /* Initialize clipboard - NULL callbacks by default */
    memset(&ctx->clipboard, 0, sizeof(ctx->clipboard));

    /* Initialize scroll state */
    ctx->active_scroll = NULL;
    ctx->scroll_viewport = (iui_rect_t) {0, 0, 0, 0};
    ctx->scroll_content_start_y = 0;
    ctx->scroll_content_start_x = 0;
    ctx->scroll_wheel_dx = 0;
    ctx->scroll_wheel_dy = 0;
    ctx->scroll_dragging = NULL;
    ctx->scroll_drag_offset = 0;

    /* Initialize typography scale */
    ctx->typography = iui_typography_scale_default;

    /* Initialize shape tokens */
    ctx->shapes = iui_shape_tokens_default;

    /* Initialize spacing tokens */
    ctx->spacing = iui_spacing_tokens_default;

    /* Slightly thinner default stroke improves legibility and spacing */
    ctx->pen_width = iui_vector_pen_for_height(config->font_height);
    iui_compute_vector_metrics(ctx->font_height, &ctx->font_ascent_px,
                               &ctx->font_descent_px);

    /* Initialize performance systems (disabled by default) */
    iui_batch_init(ctx);
    iui_dirty_init(ctx);
    iui_text_cache_init(ctx);
    return ctx;
}

/* Input event functions moved to event.c */

const iui_theme_t *iui_get_theme(iui_context *ctx)
{
    return &ctx->colors;
}

const iui_theme_t *iui_theme_light(void)
{
    return &g_theme_light;
}

const iui_theme_t *iui_theme_dark(void)
{
    return &g_theme_dark;
}

void iui_set_theme(iui_context *ctx, const iui_theme_t *theme)
{
    if (!theme)
        return;
    ctx->colors = *theme;
}

bool iui_push_id(iui_context *ctx, const void *data, size_t size)
{
    if (ctx->id_stack_index >= IUI_ID_STACK_SIZE)
        return false;
    ctx->id_stack[ctx->id_stack_index++] = iui_hash(data, size);
    return true;
}

void iui_pop_id(iui_context *ctx)
{
    if (ctx->id_stack_index > 0)
        ctx->id_stack_index--;
}

/* MD3 keyboard focus system */

bool iui_register_focusable(iui_context *ctx,
                            uint32_t id,
                            iui_rect_t bounds,
                            float corner)
{
    if (ctx->focus_count >= IUI_MAX_FOCUSABLE_WIDGETS)
        return false;

    /* Don't register if modal is blocking this widget */
    if (ctx->modal.active && !ctx->modal.rendering)
        return false;

    ctx->focus_order[ctx->focus_count] = id;
    ctx->focus_rects[ctx->focus_count] = bounds;
    ctx->focus_corners[ctx->focus_count] = corner;
    ctx->focus_count++;

    /* Track current focus index */
    if (id == ctx->focused_widget_id)
        ctx->focus_index = ctx->focus_count - 1;
    return true;
}

/* Set focus to a specific widget by its string identifier */
void iui_set_focus(iui_context *ctx, const char *id)
{
    if (!id) {
        ctx->focused_widget_id = 0;
        ctx->focus_index = -1;
        return;
    }
    ctx->focused_widget_id = iui_hash_str(id);
    /* focus_index will be updated during registration */
}

/* Check if a widget currently has keyboard focus */
bool iui_has_focus(const iui_context *ctx, const char *id)
{
    if (!id || ctx->focused_widget_id == 0)
        return false;
    return ctx->focused_widget_id == iui_hash_str(id);
}

/* Internal: Check if widget with given ID is focused */
bool iui_widget_is_focused(const iui_context *ctx, uint32_t id)
{
    return ctx->focused_widget_id == id && id != 0;
}

/* Clear all keyboard focus */
void iui_clear_focus(iui_context *ctx)
{
    ctx->focused_widget_id = 0;
    ctx->focus_index = -1;
}

/* Move focus to the next focusable widget (Tab key) */
void iui_focus_next(iui_context *ctx)
{
    ctx->focus_navigation_pending = true;
    ctx->focus_navigation_direction = 1;
}

/* Move focus to the previous focusable widget (Shift+Tab) */
void iui_focus_prev(iui_context *ctx)
{
    ctx->focus_navigation_pending = true;
    ctx->focus_navigation_direction = -1;
}

/* Get the currently focused widget ID (0 if none) */
uint32_t iui_get_focused_id(const iui_context *ctx)
{
    return ctx->focused_widget_id;
}

/* Check if any widget currently has focus */
bool iui_has_any_focus(const iui_context *ctx)
{
    return ctx->focused_widget_id != 0;
}

/* Internal: Process pending focus navigation (called at end of frame).
 * Respects focus trap boundaries when active.
 */
void iui_process_focus_navigation(iui_context *ctx)
{
    if (!ctx->focus_navigation_pending || ctx->focus_count == 0)
        return;

    ctx->focus_navigation_pending = false;

    /* Determine navigation bounds based on focus trap */
    int min_index = 0;
    int max_index = ctx->focus_count - 1;

    if (ctx->focus_trap.active && ctx->focus_trap.first_focusable_idx <=
                                      ctx->focus_trap.last_focusable_idx) {
        min_index = ctx->focus_trap.first_focusable_idx;
        max_index = ctx->focus_trap.last_focusable_idx;
    }

    /* If no current focus, start from beginning or end of valid range */
    if (ctx->focus_index < min_index || ctx->focus_index > max_index) {
        ctx->focus_index =
            (ctx->focus_navigation_direction > 0) ? min_index : max_index;
    } else {
        /* Move in direction with wrap-around within bounds */
        ctx->focus_index += ctx->focus_navigation_direction;
        if (ctx->focus_index > max_index)
            ctx->focus_index = min_index;
        else if (ctx->focus_index < min_index)
            ctx->focus_index = max_index;
    }

    /* Update focused widget ID */
    if (ctx->focus_index >= 0 && ctx->focus_index < ctx->focus_count)
        ctx->focused_widget_id = ctx->focus_order[ctx->focus_index];
}

#ifdef CONFIG_FEATURE_FOCUS
/* Draw focus ring around a rect (MD3 spec: 3dp width, 2dp offset, primary
 * color)
 */
void iui_draw_focus_ring(iui_context *ctx,
                         iui_rect_t bounds,
                         float corner_radius)
{
    if (!ctx->renderer.draw_box)
        return;

    /* MD3 focus ring: 3dp stroke, 2dp offset from component edge */
    float offset = IUI_FOCUS_RING_OFFSET;
    float stroke = IUI_FOCUS_RING_WIDTH;

    /* Expand bounds by offset */
    iui_rect_t ring = {
        .x = bounds.x - offset - stroke,
        .y = bounds.y - offset - stroke,
        .width = bounds.width + 2.f * (offset + stroke),
        .height = bounds.height + 2.f * (offset + stroke),
    };

    /* Adjust corner radius for outer ring */
    float outer_corner = corner_radius + offset + stroke;

    /* Use primary color for focus ring (MD3 spec) */
    uint32_t ring_color = ctx->colors.primary;

    /* Draw outer ring (full box) */
    ctx->renderer.draw_box(ring, outer_corner, ring_color, ctx->renderer.user);

    /* Draw inner cutout with surface color to create ring effect */
    iui_rect_t inner = {
        .x = bounds.x - offset,
        .y = bounds.y - offset,
        .width = bounds.width + 2.f * offset,
        .height = bounds.height + 2.f * offset,
    };
    float inner_corner = corner_radius + offset;

    /* Use surface color to "cut out" the inner area */
    ctx->renderer.draw_box(inner, inner_corner, ctx->colors.surface,
                           ctx->renderer.user);
}
#else
/* Stub: Focus ring disabled - draw nothing */
void iui_draw_focus_ring(iui_context *ctx,
                         iui_rect_t bounds,
                         float corner_radius)
{
    (void) ctx;
    (void) bounds;
    (void) corner_radius;
}
#endif /* CONFIG_FEATURE_FOCUS */

/* WCAG accessibility - Color contrast utilities */

#ifdef CONFIG_FEATURE_ACCESSIBILITY
/* Apply sRGB gamma correction to a single channel (0-255 -> 0.0-1.0 linear) */
static float srgb_to_linear(uint8_t channel)
{
    float c = (float) channel / 255.f;
    /* WCAG 2.1 formula: threshold at 0.04045 (updated May 2021; was 0.03928) */
    if (c <= 0.04045f)
        return c / 12.92f;
    return powf((c + 0.055f) / 1.055f, 2.4f);
}

float iui_relative_luminance(uint32_t color)
{
    /* Extract RGB from 0xAARRGGBB format */
    uint8_t r = (color >> 16) & 0xFF, g = (color >> 8) & 0xFF, b = color & 0xFF;

    /* Convert to linear RGB */
    float r_lin = srgb_to_linear(r), g_lin = srgb_to_linear(g),
          b_lin = srgb_to_linear(b);

    /* WCAG relative luminance formula
     * Coefficients based on human perception (green most sensitive)
     */
    return 0.2126f * r_lin + 0.7152f * g_lin + 0.0722f * b_lin;
}

float iui_contrast_ratio(uint32_t color1, uint32_t color2)
{
    float l1 = iui_relative_luminance(color1);
    float l2 = iui_relative_luminance(color2);

    /* Ensure l1 is the lighter color (higher luminance) */
    if (l2 > l1) {
        float tmp = l1;
        l1 = l2;
        l2 = tmp;
    }

    /* WCAG contrast ratio formula */
    return (l1 + 0.05f) / (l2 + 0.05f);
}

bool iui_wcag_aa_normal(uint32_t foreground, uint32_t background)
{
    return iui_contrast_ratio(foreground, background) >= IUI_WCAG_AA_NORMAL;
}

bool iui_wcag_aa_large(uint32_t foreground, uint32_t background)
{
    return iui_contrast_ratio(foreground, background) >= IUI_WCAG_AA_LARGE;
}

bool iui_wcag_aaa_normal(uint32_t foreground, uint32_t background)
{
    return iui_contrast_ratio(foreground, background) >= IUI_WCAG_AAA_NORMAL;
}

bool iui_wcag_aaa_large(uint32_t foreground, uint32_t background)
{
    return iui_contrast_ratio(foreground, background) >= IUI_WCAG_AAA_LARGE;
}

int iui_theme_validate_contrast(const iui_theme_t *theme)
{
    if (!theme)
        return -1;

    int failures = 0;

    /* Critical text/background combinations for WCAG AA normal text (4.5:1)
     * Primary text on surfaces
     */
    if (!iui_wcag_aa_normal(theme->on_surface, theme->surface))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_surface_variant, theme->surface_variant))
        failures++;

    /* Primary colors */
    if (!iui_wcag_aa_normal(theme->on_primary, theme->primary))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_primary_container,
                            theme->primary_container))
        failures++;

    /* Secondary colors */
    if (!iui_wcag_aa_normal(theme->on_secondary, theme->secondary))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_secondary_container,
                            theme->secondary_container))
        failures++;

    /* Tertiary colors */
    if (!iui_wcag_aa_normal(theme->on_tertiary, theme->tertiary))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_tertiary_container,
                            theme->tertiary_container))
        failures++;

    /* Error colors */
    if (!iui_wcag_aa_normal(theme->on_error, theme->error))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_error_container, theme->error_container))
        failures++;

    /* Inverse colors */
    if (!iui_wcag_aa_normal(theme->inverse_on_surface, theme->inverse_surface))
        failures++;

    /* Surface containers (text uses on_surface) */
    if (!iui_wcag_aa_normal(theme->on_surface, theme->surface_container))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_surface, theme->surface_container_high))
        failures++;
    if (!iui_wcag_aa_normal(theme->on_surface,
                            theme->surface_container_highest))
        failures++;

    return failures;
}

/* Accessibility - Screen reader support */

void iui_set_a11y_callbacks(iui_context *ctx, const iui_a11y_callbacks *cbs)
{
    if (!ctx)
        return;
    if (cbs) {
        ctx->a11y_callbacks = *cbs;
        ctx->a11y_enabled =
            (cbs->announce || cbs->on_focus || cbs->on_state || cbs->on_value);
    } else {
        memset(&ctx->a11y_callbacks, 0, sizeof(ctx->a11y_callbacks));
        ctx->a11y_enabled = false;
    }
}

const iui_a11y_callbacks *iui_get_a11y_callbacks(const iui_context *ctx)
{
    if (!ctx || !ctx->a11y_enabled)
        return NULL;
    return &ctx->a11y_callbacks;
}

bool iui_a11y_enabled(const iui_context *ctx)
{
    return ctx && ctx->a11y_enabled;
}

void iui_announce(iui_context *ctx, const char *text, iui_a11y_live_t priority)
{
    if (!ctx || !ctx->a11y_enabled || !ctx->a11y_callbacks.announce || !text)
        return;
    ctx->a11y_callbacks.announce(text, priority, ctx->a11y_callbacks.user);
}

void iui_announcef(iui_context *ctx,
                   iui_a11y_live_t priority,
                   const char *fmt,
                   ...)
{
    if (!ctx || !ctx->a11y_enabled || !ctx->a11y_callbacks.announce || !fmt)
        return;

    va_list args;
    va_start(args, fmt);
    int written =
        vsnprintf(ctx->string_buffer, IUI_STRING_BUFFER_SIZE, fmt, args);
    if (written >= IUI_STRING_BUFFER_SIZE)
        ctx->string_buffer[IUI_STRING_BUFFER_SIZE - 1] = '\0';
    va_end(args);

    ctx->a11y_callbacks.announce(ctx->string_buffer, priority,
                                 ctx->a11y_callbacks.user);
}

iui_a11y_hint iui_a11y_make_hint(const char *label, iui_a11y_role_t role)
{
    iui_a11y_hint hint = {
        .label = label,
        .role = role,
    };
    return hint;
}

iui_a11y_hint iui_a11y_make_slider_hint(const char *label,
                                        float value,
                                        float min,
                                        float max)
{
    iui_a11y_hint hint = {
        .label = label,
        .role = IUI_A11Y_ROLE_SLIDER,
        .value_now = value,
        .value_min = min,
        .value_max = max,
    };
    return hint;
}

iui_a11y_hint iui_a11y_make_set_hint(const char *label,
                                     iui_a11y_role_t role,
                                     int position,
                                     int total)
{
    iui_a11y_hint hint = {
        .label = label,
        .role = role,
        .position_in_set = position,
        .set_size = total,
    };
    return hint;
}

void iui_a11y_push(iui_context *ctx, const iui_a11y_hint *hint)
{
    if (!ctx || !hint || ctx->a11y_stack_depth >= 8)
        return;
    ctx->a11y_stack[ctx->a11y_stack_depth++] = *hint;
}

void iui_a11y_pop(iui_context *ctx)
{
    if (!ctx || ctx->a11y_stack_depth <= 0)
        return;
    ctx->a11y_stack_depth--;
}

void iui_a11y_notify_state(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           uint32_t old_state,
                           uint32_t new_state)
{
    if (!ctx || !ctx->a11y_enabled || !ctx->a11y_callbacks.on_state || !widget)
        return;
    if (old_state == new_state)
        return;
    ctx->a11y_callbacks.on_state(widget, old_state, new_state,
                                 ctx->a11y_callbacks.user);
}

void iui_a11y_notify_value(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           float old_value,
                           float new_value)
{
    if (!ctx || !ctx->a11y_enabled || !ctx->a11y_callbacks.on_value || !widget)
        return;
    if (old_value == new_value)
        return;
    ctx->a11y_callbacks.on_value(widget, old_value, new_value,
                                 ctx->a11y_callbacks.user);
}

void iui_a11y_notify_focus(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           iui_rect_t bounds)
{
    if (!ctx || !ctx->a11y_enabled || !ctx->a11y_callbacks.on_focus || !widget)
        return;
    ctx->a11y_callbacks.on_focus(widget, bounds, ctx->a11y_callbacks.user);
}

const char *iui_a11y_role_name(iui_a11y_role_t role)
{
    static const char *role_names[] = {
        "none",        /* a11y_role_none */
        "button",      /* a11y_role_button */
        "checkbox",    /* a11y_role_checkbox */
        "radio",       /* a11y_role_radio */
        "switch",      /* a11y_role_switch */
        "slider",      /* a11y_role_slider */
        "textfield",   /* a11y_role_textfield */
        "combobox",    /* a11y_role_combobox */
        "menu",        /* a11y_role_menu */
        "menuitem",    /* a11y_role_menuitem */
        "tab",         /* a11y_role_tab */
        "tabpanel",    /* a11y_role_tabpanel */
        "dialog",      /* a11y_role_dialog */
        "alertdialog", /* a11y_role_alertdialog */
        "alert",       /* a11y_role_alert */
        "status",      /* a11y_role_status */
        "progressbar", /* a11y_role_progressbar */
        "link",        /* a11y_role_link */
        "heading",     /* a11y_role_heading */
        "list",        /* a11y_role_list */
        "listitem",    /* a11y_role_listitem */
        "img",         /* a11y_role_img */
        "search",      /* a11y_role_search */
        "scrollbar",   /* a11y_role_scrollbar */
    };
    int idx = (int) role;
    if (idx < 0 || idx >= (int) (sizeof(role_names) / sizeof(role_names[0])))
        return "unknown";
    return role_names[idx];
}

const char *iui_a11y_state_description(uint32_t state)
{
    static char buf[256];
    char *p = buf;
    size_t remaining = sizeof(buf);
    int count = 0;

    if (state == IUI_A11Y_STATE_NONE)
        return "";

#define APPEND_STATE(flag, name)                                             \
    if ((state & (flag)) && remaining > 1) {                                 \
        int n = snprintf(p, remaining, "%s%s", count > 0 ? ", " : "", name); \
        if (n > 0 && (size_t) n < remaining) {                               \
            p += n;                                                          \
            remaining -= n;                                                  \
            count++;                                                         \
        }                                                                    \
    }

    APPEND_STATE(IUI_A11Y_STATE_CHECKED, "checked")
    APPEND_STATE(IUI_A11Y_STATE_SELECTED, "selected")
    APPEND_STATE(IUI_A11Y_STATE_DISABLED, "disabled")
    APPEND_STATE(IUI_A11Y_STATE_EXPANDED, "expanded")
    APPEND_STATE(IUI_A11Y_STATE_COLLAPSED, "collapsed")
    APPEND_STATE(IUI_A11Y_STATE_PRESSED, "pressed")
    APPEND_STATE(IUI_A11Y_STATE_FOCUSED, "focused")
    APPEND_STATE(IUI_A11Y_STATE_BUSY, "busy")
    APPEND_STATE(IUI_A11Y_STATE_INVALID, "invalid")
    APPEND_STATE(IUI_A11Y_STATE_REQUIRED, "required")
    APPEND_STATE(IUI_A11Y_STATE_READONLY, "read-only")
    APPEND_STATE(IUI_A11Y_STATE_MULTISELECTABLE, "multi-selectable")
    APPEND_STATE(IUI_A11Y_STATE_HASPOPUP, "has popup")

#undef APPEND_STATE
    (void) count; /* suppress unused-after-final-increment warning */
    return buf;
}

int iui_a11y_describe(const iui_a11y_hint *hint, char *buf, size_t buf_size)
{
    if (!hint || !buf || buf_size == 0)
        return 0;

    char *p = buf;
    size_t remaining = buf_size;
    int written = 0;

    /* Label first */
    if (hint->label && hint->label[0]) {
        int n = snprintf(p, remaining, "%s", hint->label);
        if (n > 0 && (size_t) n < remaining) {
            p += n;
            remaining -= n;
            written += n;
        }
    }

    /* Role */
    if (hint->role != IUI_A11Y_ROLE_NONE && remaining > 2) {
        const char *role_name = iui_a11y_role_name(hint->role);
        int n =
            snprintf(p, remaining, "%s%s", written > 0 ? ", " : "", role_name);
        if (n > 0 && (size_t) n < remaining) {
            p += n;
            remaining -= n;
            written += n;
        }
    }

    /* Value for sliders/progress */
    if ((hint->role == IUI_A11Y_ROLE_SLIDER ||
         hint->role == IUI_A11Y_ROLE_PROGRESSBAR) &&
        remaining > 2) {
        if (hint->value_text && hint->value_text[0]) {
            int n = snprintf(p, remaining, ", %s", hint->value_text);
            if (n > 0 && (size_t) n < remaining) {
                p += n;
                remaining -= n;
                written += n;
            }
        } else {
            int n = snprintf(p, remaining, ", %.0f of %.0f", hint->value_now,
                             hint->value_max);
            if (n > 0 && (size_t) n < remaining) {
                p += n;
                remaining -= n;
                written += n;
            }
        }
    }

    /* Position in set */
    if (hint->set_size > 0 && hint->position_in_set > 0 && remaining > 2) {
        int n = snprintf(p, remaining, ", %d of %d", hint->position_in_set,
                         hint->set_size);
        if (n > 0 && (size_t) n < remaining) {
            p += n;
            remaining -= n;
            written += n;
        }
    }

    /* State */
    if (hint->state != IUI_A11Y_STATE_NONE && remaining > 2) {
        const char *state_desc = iui_a11y_state_description(hint->state);
        if (state_desc && state_desc[0]) {
            int n = snprintf(p, remaining, ", %s", state_desc);
            if (n > 0 && (size_t) n < remaining) {
                p += n;
                remaining -= n;
                written += n;
            }
        }
    }

    /* Description */
    if (hint->description && hint->description[0] && remaining > 2) {
        int n = snprintf(p, remaining, ". %s", hint->description);
        if (n > 0 && (size_t) n < remaining) {
            p += n;
            remaining -= n;
            written += n;
        }
    }

    /* Hint */
    if (hint->hint && hint->hint[0] && remaining > 2) {
        int n = snprintf(p, remaining, ". %s", hint->hint);
        if (n > 0 && (size_t) n < remaining) {
            written += n;
        }
    }

    return written;
}
#endif /* CONFIG_FEATURE_ACCESSIBILITY */

/* Input layer system */

/* Internal: Swap double buffers at frame start */
void iui_input_layer_frame_begin(iui_context *ctx)
{
    /* Swap buffers: previous write buffer becomes read buffer */
    ctx->input_layer.current_buffer = 1 - ctx->input_layer.current_buffer;
    /* Clear the new write buffer for this frame's registrations */
    ctx->input_layer.region_count[ctx->input_layer.current_buffer] = 0;
    ctx->input_layer.next_reg_order = 0;
}

/* Internal: Check if two rects overlap. */
static bool iui_rect_ts_overlap(const iui_rect_t *a, const iui_rect_t *b)
{
    return !(a->x + a->width < b->x || b->x + b->width < a->x ||
             a->y + a->height < b->y || b->y + b->height < a->y);
}

int iui_push_layer(iui_context *ctx, int z_order)
{
    if (ctx->input_layer.layer_depth >= IUI_MAX_INPUT_LAYERS)
        return 0; /* Stack overflow */

    /* Generate unique layer ID */
    ctx->input_layer.next_layer_id++;
    int layer_id = ctx->input_layer.next_layer_id;

    /* Push onto stack with both ID and z_order for proper restoration */
    iui_layer_entry_t *entry =
        &ctx->input_layer.layer_stack[ctx->input_layer.layer_depth];
    entry->layer_id = layer_id;
    entry->z_order = z_order;

    ctx->input_layer.layer_depth++;
    ctx->input_layer.current_layer_id = layer_id;
    ctx->input_layer.current_z_order = z_order;

    return layer_id;
}

void iui_pop_layer(iui_context *ctx)
{
    if (ctx->input_layer.layer_depth <= 0)
        return;

    ctx->input_layer.layer_depth--;

    /* Restore previous layer's ID and z_order from stack */
    if (ctx->input_layer.layer_depth == 0) {
        ctx->input_layer.current_layer_id = 0;
        ctx->input_layer.current_z_order = 0;
    } else {
        /* Restore to previous stack entry */
        iui_layer_entry_t *prev =
            &ctx->input_layer.layer_stack[ctx->input_layer.layer_depth - 1];
        ctx->input_layer.current_layer_id = prev->layer_id;
        ctx->input_layer.current_z_order = prev->z_order;
    }
}

bool iui_register_blocking_region(iui_context *ctx, iui_rect_t bounds)
{
    int buf = ctx->input_layer.current_buffer;
    if (ctx->input_layer.region_count[buf] >= IUI_MAX_BLOCKING_REGIONS)
        return false; /* Region limit reached */

    iui_block_region_t *reg =
        &ctx->input_layer.regions[buf][ctx->input_layer.region_count[buf]];
    reg->bounds = bounds;
    reg->layer_id = ctx->input_layer.current_layer_id;
    reg->z_order = ctx->input_layer.current_z_order;
    reg->registration_order = ctx->input_layer.next_reg_order++;
    reg->blocks_input = true;

    ctx->input_layer.region_count[buf]++;
    return true;
}

bool iui_should_process_input(iui_context *ctx, iui_rect_t bounds)
{
    /* Fast path - no overlays active (common case) */
    if (ctx->input_layer.layer_depth == 0 && !ctx->modal.active) {
        /* Also check input capture */
        if (ctx->input_capture.active) {
            uint32_t widget_id = iui_hash(&bounds, sizeof(bounds));
            return widget_id == ctx->input_capture.owner_id;
        }
        return true;
    }

    /* Backward compatibility: also check legacy modal state */
    if (ctx->modal.active && !ctx->modal.rendering) {
        return false;
    }

    /* No layers active = process all input */
    if (ctx->input_layer.layer_depth == 0) {
        return true;
    }

    /* Read from previous frame's buffer (double-buffering) */
    int read_buf = 1 - ctx->input_layer.current_buffer;
    int region_count = ctx->input_layer.region_count[read_buf];

    if (region_count == 0) {
        return true;
    }

    /* Find highest z_order blocking region ONLY among overlapping regions.
     * Critical: must scan all overlapping regions before deciding - early
     * return would miss higher z-order overlays that should block input. Use
     * z_order for priority (higher = on top), registration_order as tie-breaker
     */
    int highest_z_order = -1;
    int highest_reg_order = -1;
    int highest_layer_id = -1;

    for (int i = 0; i < region_count; i++) {
        const iui_block_region_t *reg = &ctx->input_layer.regions[read_buf][i];
        if (!reg->blocks_input)
            continue;

        /* ONLY consider regions that actually overlap with the widget bounds.
         * Non-overlapping regions (e.g., dialog in corner) should not globally
         * block unrelated widgets elsewhere on screen.
         */
        if (!iui_rect_ts_overlap(&bounds, &reg->bounds))
            continue;

        /* Track highest blocking region among overlapping only */
        if (reg->z_order > highest_z_order ||
            (reg->z_order == highest_z_order &&
             reg->registration_order > highest_reg_order)) {
            highest_z_order = reg->z_order;
            highest_reg_order = reg->registration_order;
            highest_layer_id = reg->layer_id;
        }
    }

    /* No overlapping blocking regions = allow input */
    if (highest_layer_id < 0)
        return true;

    /* Allow input only if current layer is the highest overlapping layer.
     * This ensures that lower layers cannot process input when a higher z-order
     * overlay is covering them.
     */
    return (ctx->input_layer.current_layer_id == highest_layer_id);
}

int iui_get_current_layer(const iui_context *ctx)
{
    return ctx->input_layer.current_layer_id;
}

bool iui_has_active_layer(const iui_context *ctx)
{
    return ctx->input_layer.layer_depth > 0;
}

int iui_get_layer_depth(const iui_context *ctx)
{
    return ctx->input_layer.layer_depth;
}

/* Per-Frame Field ID Tracking
 * Prevents stale state bugs when widgets are conditionally hidden or their
 * render order changes. Text fields and sliders register themselves each frame.
 * State is cleared for fields not seen during the current frame.
 */

void iui_field_tracking_frame_begin(iui_context *ctx)
{
    memset(ctx->field_tracking.textfield_ids, 0,
           sizeof(ctx->field_tracking.textfield_ids));
    memset(ctx->field_tracking.slider_ids, 0,
           sizeof(ctx->field_tracking.slider_ids));
    ctx->field_tracking.textfield_count = 0;
    ctx->field_tracking.slider_count = 0;
    ctx->field_tracking.frame_number++;
}

/* Hash pointer to table index using multiplicative hash.
 * Simple >> 3 shift causes collisions for page-aligned allocations (4KB apart).
 * Knuth's multiplicative hash (golden ratio) provides better bit mixing.
 */
static inline uint32_t iui_ptr_hash_idx(const void *ptr, uint32_t mask)
{
    uint32_t h = (uint32_t) ((uintptr_t) ptr >> 3);
    h *= 2654435769u; /* 2^32 / phi (golden ratio) */
    return h & mask;
}

void iui_register_textfield(iui_context *ctx, void *buffer)
{
    if (!buffer)
        return;

    /* Hash index with linear probing for collisions */
    uint32_t idx = iui_ptr_hash_idx(buffer, IUI_MAX_TRACKED_TEXTFIELDS - 1);

    /* Probe entire table until finding empty slot or duplicate */
    for (int probe = 0; probe < IUI_MAX_TRACKED_TEXTFIELDS; probe++) {
        void *cached = ctx->field_tracking.textfield_ids[idx];
        if (!cached) {
            /* Found empty slot, insert here */
            ctx->field_tracking.textfield_ids[idx] = buffer;
            ctx->field_tracking.textfield_count++;
            return;
        }
        if (cached == buffer) /* Already registered, done */
            return;

        /* Collision, continue probing */
        idx = (idx + 1) & (IUI_MAX_TRACKED_TEXTFIELDS - 1);
    }
}

void iui_register_slider(iui_context *ctx, uint32_t slider_id)
{
    if (slider_id == 0)
        return;

    /* Direct hash index with linear probing for collisions */
    uint32_t idx = slider_id & (IUI_MAX_TRACKED_SLIDERS - 1);

    /* Probe entire table until finding empty slot or duplicate */
    for (int probe = 0; probe < IUI_MAX_TRACKED_SLIDERS; probe++) {
        uint32_t cached = ctx->field_tracking.slider_ids[idx];
        if (!cached) {
            /* Found empty slot, insert here */
            ctx->field_tracking.slider_ids[idx] = slider_id;
            ctx->field_tracking.slider_count++;
            return;
        }
        if (cached == slider_id) {
            /* Already registered, done */
            return;
        }
        /* Collision, continue probing */
        idx = (idx + 1) & (IUI_MAX_TRACKED_SLIDERS - 1);
    }
}

bool iui_textfield_is_registered(const iui_context *ctx, const void *buffer)
{
    if (!buffer)
        return false;

    uint32_t idx = iui_ptr_hash_idx(buffer, IUI_MAX_TRACKED_TEXTFIELDS - 1);

    /* Probe entire table looking for match or empty slot */
    for (int probe = 0; probe < IUI_MAX_TRACKED_TEXTFIELDS; probe++) {
        void *cached = ctx->field_tracking.textfield_ids[idx];
        if (cached == buffer)
            return true;
        if (!cached)
            return false;
        idx = (idx + 1) & (IUI_MAX_TRACKED_TEXTFIELDS - 1);
    }
    return false;
}

bool iui_slider_is_registered(const iui_context *ctx, uint32_t slider_id)
{
    if (slider_id == 0)
        return false;

    uint32_t idx = slider_id & (IUI_MAX_TRACKED_SLIDERS - 1);

    /* Probe entire table looking for match or empty slot */
    for (int probe = 0; probe < IUI_MAX_TRACKED_SLIDERS; probe++) {
        uint32_t cached = ctx->field_tracking.slider_ids[idx];
        if (cached == slider_id)
            return true;
        if (!cached)
            return false;
        idx = (idx + 1) & (IUI_MAX_TRACKED_SLIDERS - 1);
    }
    return false;
}

void iui_field_tracking_frame_end(iui_context *ctx)
{
    /* Clear focused_edit if the text field was not rendered this frame */
    if (ctx->focused_edit &&
        !iui_textfield_is_registered(ctx, ctx->focused_edit))
        ctx->focused_edit = NULL;

    /* Clear slider state if the active slider was not rendered this frame.
     * Use IUI_SLIDER_ID_MASK to extract the 31-bit slider ID (ignoring
     * the animation flag in bit 31) for comparison against tracked sliders.
     */
    uint32_t active_slider = ctx->slider.active_id & IUI_SLIDER_ID_MASK;
    if (active_slider != 0 && !iui_slider_is_registered(ctx, active_slider)) {
        ctx->slider.active_id = 0;
        ctx->slider.drag_offset = 0.f;
        ctx->slider.anim_t = 0.f;
    }
}

/* Public API: manual field ID reset
 * Called automatically by iui_begin_frame(). Exposed for advanced use cases
 * such as mid-frame layout changes or conditional widget hiding.
 */
void iui_reset_field_ids(iui_context *ctx)
{
    iui_field_tracking_frame_begin(ctx);
}

/* Focus Trapping for Modal Layers
 * Reference: https://www.w3.org/WAI/ARIA/apg/patterns/dialog-modal/
 *
 * When a modal is active, focus navigation must be constrained to that layer.
 * This is required for accessibility compliance (ARIA modal pattern).
 */

void iui_focus_trap_begin(iui_context *ctx, int layer_id)
{
    if (!ctx || layer_id <= 0)
        return;

    ctx->focus_trap.layer_id = layer_id;
    ctx->focus_trap.active = true;
    /* Track the starting index for focusable widgets in this trap */
    ctx->focus_trap.first_focusable_idx = ctx->focus_count;
    ctx->focus_trap.last_focusable_idx = ctx->focus_count;
}

void iui_focus_trap_end(iui_context *ctx)
{
    if (!ctx || !ctx->focus_trap.active)
        return;

    /* Record the last focusable widget index in this trap */
    ctx->focus_trap.last_focusable_idx = ctx->focus_count - 1;

    /* If focus is outside the trap, move it to the first widget in trap */
    if (ctx->focus_trap.first_focusable_idx <=
        ctx->focus_trap.last_focusable_idx) {
        if (ctx->focus_index < ctx->focus_trap.first_focusable_idx ||
            ctx->focus_index > ctx->focus_trap.last_focusable_idx) {
            /* Focus is outside trap bounds - move to first widget in trap */
            ctx->focus_index = ctx->focus_trap.first_focusable_idx;
            if (ctx->focus_index < ctx->focus_count)
                ctx->focused_widget_id = ctx->focus_order[ctx->focus_index];
        }
    }

    ctx->focus_trap.active = false;
    ctx->focus_trap.layer_id = 0;
}

bool iui_layer_is_focused(const iui_context *ctx, int layer_id)
{
    if (!ctx)
        return false;
    return ctx->focus_trap.active && ctx->focus_trap.layer_id == layer_id;
}

/* Input Capture for Drag Operations
 * Prevents accidental activation when mouse enters component mid-drag.
 */

bool iui_begin_input_capture(iui_context *ctx,
                             iui_rect_t bounds,
                             bool require_start_in_bounds)
{
    if (!ctx)
        return false;

    uint32_t widget_id = iui_hash(&bounds, sizeof(bounds));

    /* If capture is already active, check if this widget owns it */
    if (ctx->input_capture.active)
        return ctx->input_capture.owner_id == widget_id;

    /* Start capture on mouse press */
    if (ctx->mouse_pressed & IUI_MOUSE_LEFT) {
        bool in_bounds = in_rect(&bounds, ctx->mouse_pos);

        if (require_start_in_bounds && !in_bounds) {
            /* Mouse press wasn't in widget bounds - don't capture */
            return false;
        }

        if (in_bounds) {
            ctx->input_capture.active = true;
            ctx->input_capture.owner_id = widget_id;
            ctx->input_capture.bounds_x = bounds.x;
            ctx->input_capture.bounds_y = bounds.y;
            ctx->input_capture.bounds_w = bounds.width;
            ctx->input_capture.bounds_h = bounds.height;
            ctx->input_capture.require_start = require_start_in_bounds;
            return true;
        }
    }

    /* If mouse is held and start in bounds is not required, input might
     * still be processed but a new capture should not start
     */
    return false;
}

bool iui_is_input_captured(const iui_context *ctx)
{
    return ctx && ctx->input_capture.active;
}

void iui_release_capture(iui_context *ctx)
{
    if (!ctx)
        return;

    ctx->input_capture.active = false;
    ctx->input_capture.owner_id = 0;
    ctx->input_capture.bounds_x = 0;
    ctx->input_capture.bounds_y = 0;
    ctx->input_capture.bounds_w = 0;
    ctx->input_capture.bounds_h = 0;
}

/* IME (Input Method Editor) Support */

void iui_update_composition(iui_context *ctx, const char *text, int cursor)
{
    if (!ctx)
        return;

    if (text) {
        size_t len = strlen(text);
        if (len >= sizeof(ctx->ime.composing_text))
            len = sizeof(ctx->ime.composing_text) - 1;
        memcpy(ctx->ime.composing_text, text, len);
        ctx->ime.composing_text[len] = '\0';
        ctx->ime.composing_cursor = cursor;
        ctx->ime.is_composing = true;
    } else {
        ctx->ime.composing_text[0] = '\0';
        ctx->ime.composing_cursor = 0;
        ctx->ime.is_composing = false;
    }
}

void iui_commit_composition(iui_context *ctx, const char *text)
{
    if (!ctx)
        return;

    /* Insert the committed text as if it were typed */
    if (text) {
        size_t len = strlen(text);
        for (size_t i = 0; i < len; i++) {
            /* Queue each character for input processing */
            ctx->char_input = (unsigned char) text[i];
            /* Note: The text field will process this in the next frame.
             * For immediate processing, the platform layer should handle
             * multi-character insertion directly.
             */
        }
    }

    /* Clear composition state */
    ctx->ime.composing_text[0] = '\0';
    ctx->ime.composing_cursor = 0;
    ctx->ime.is_composing = false;
}

bool iui_ime_is_composing(const iui_context *ctx)
{
    return ctx && ctx->ime.is_composing;
}

const char *iui_ime_get_text(const iui_context *ctx)
{
    if (!ctx || !ctx->ime.is_composing)
        return NULL;
    return ctx->ime.composing_text;
}

/* Clipboard Support */

void iui_set_clipboard_callbacks(iui_context *ctx, const iui_clipboard_t *cb)
{
    if (!ctx)
        return;

    if (cb) {
        ctx->clipboard = *cb;
    } else {
        memset(&ctx->clipboard, 0, sizeof(ctx->clipboard));
    }
}

bool iui_clipboard_copy(iui_context *ctx, const char *text, size_t len)
{
    if (!ctx || !text || len == 0)
        return false;

    if (!ctx->clipboard.set)
        return false;

    ctx->clipboard.set(text, len, ctx->clipboard.user);
    return true;
}

size_t iui_clipboard_paste(iui_context *ctx, char *buffer, size_t buffer_size)
{
    if (!ctx || !buffer || buffer_size == 0)
        return 0;

    if (!ctx->clipboard.get)
        return 0;

    const char *clipboard_text = ctx->clipboard.get(ctx->clipboard.user);
    if (!clipboard_text)
        return 0;

    size_t text_len = strlen(clipboard_text);
    size_t copy_len = (text_len < buffer_size - 1) ? text_len : buffer_size - 1;
    memcpy(buffer, clipboard_text, copy_len);
    buffer[copy_len] = '\0';

    return copy_len;
}
