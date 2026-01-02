/* FAB (Floating Action Button) and Icon Button implementation */

#include "internal.h"

/* Icon Button style variants */
typedef enum {
    ICON_BUTTON_STANDARD, /* No container, icon only */
    ICON_BUTTON_FILLED,   /* Primary container */
    ICON_BUTTON_TONAL,    /* Secondary container */
    ICON_BUTTON_OUTLINED, /* Outline border */
} icon_button_style;

/* Floating Action Button (FAB) */

static bool iui_fab_internal(iui_context *ctx,
                             float x,
                             float y,
                             float size,
                             float corner_radius,
                             float icon_size,
                             const char *icon,
                             const char *label)
{
    if (!ctx)
        return false;

    /* Calculate FAB dimensions */
    float fab_w = size;
    float fab_h = size;

    /* For extended FAB, calculate width based on content */
    if (label) {
        float label_width = iui_get_text_width(ctx, label);
        fab_w = IUI_FAB_EXTENDED_PADDING * 2.f + label_width;
        if (icon)
            fab_w += icon_size + IUI_FAB_EXTENDED_GAP;
        fab_h = IUI_FAB_EXTENDED_HEIGHT;
        corner_radius = fab_h * 0.5f; /* Pill shape for extended FAB */
    }

    iui_rect_t fab_rect = {x, y, fab_w, fab_h};

    /* Get component state for interaction */
    iui_state_t state = iui_get_component_state(ctx, fab_rect, false);

    /* MD3 FAB colors */
    uint32_t container_color = ctx->colors.primary_container;
    uint32_t content_color = ctx->colors.on_primary_container;

    /* Draw shadow (elevation_3 for FAB) */
    iui_draw_shadow(ctx, fab_rect, corner_radius, IUI_ELEVATION_3);

    /* Draw container background */
    ctx->renderer.draw_box(fab_rect, corner_radius, container_color,
                           ctx->renderer.user);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, fab_rect, corner_radius, content_color, state);

    /* Calculate content position */
    float center_y = y + fab_h * 0.5f;

    if (label) {
        /* Extended FAB: icon (optional) + label */
        float content_x = x + IUI_FAB_EXTENDED_PADDING;

        if (icon) {
            float icon_cx = content_x + icon_size * 0.5f;
            float icon_cy = center_y;
            iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icon,
                              content_color);
            content_x += icon_size + IUI_FAB_EXTENDED_GAP;
        }

        /* Draw label */
        float label_y = center_y - ctx->font_height * 0.5f;
        iui_internal_draw_text(ctx, content_x, label_y, label, content_color);
    } else {
        /* Standard FAB: centered icon only */
        float icon_cx = x + fab_w * 0.5f;
        float icon_cy = center_y;
        if (icon) {
            iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icon,
                              content_color);
        }
    }

    /* MD3 runtime validation: track rendered FAB dimensions */
    if (label) {
        /* Extended FAB uses different validation */
        IUI_MD3_TRACK_BUTTON(fab_rect, corner_radius);
    } else if (size >= IUI_FAB_LARGE_SIZE - 1.f) {
        IUI_MD3_TRACK_FAB_LARGE(fab_rect, corner_radius);
    } else {
        IUI_MD3_TRACK_FAB(fab_rect, corner_radius);
    }

    /* Handle click */
    if (state == IUI_STATE_PRESSED)
        return true;

    return false;
}

/* Standard FAB (56dp) */
bool iui_fab(iui_context *ctx, float x, float y, const char *icon)
{
    return iui_fab_internal(ctx, x, y, IUI_FAB_SIZE, IUI_FAB_CORNER_RADIUS,
                            IUI_FAB_ICON_SIZE, icon, NULL);
}

/* Large FAB (96dp) */
bool iui_fab_large(iui_context *ctx, float x, float y, const char *icon)
{
    return iui_fab_internal(ctx, x, y, IUI_FAB_LARGE_SIZE,
                            IUI_FAB_LARGE_CORNER_RADIUS,
                            IUI_FAB_LARGE_ICON_SIZE, icon, NULL);
}

/* Extended FAB with label */
bool iui_fab_extended(iui_context *ctx,
                      float x,
                      float y,
                      const char *icon,
                      const char *label)
{
    if (!label)
        return false; /* Extended FAB requires a label */
    return iui_fab_internal(ctx, x, y, 0, 0, IUI_FAB_ICON_SIZE, icon, label);
}

/* Icon buttons */

static bool iui_icon_button_internal(iui_context *ctx,
                                     const char *icon,
                                     icon_button_style style,
                                     bool is_toggle,
                                     bool *selected)
{
    if (!ctx || !icon)
        return false;
    if (!ctx->current_window)
        return false;

    /* Compute button rect from layout.
     * Icon buttons participate in layout unlike FAB.
     */
    float btn_size = IUI_ICON_BUTTON_SIZE;
    iui_rect_t button_rect;

    if (ctx->in_grid) {
        /* In grid mode, use cell dimensions but center the icon button */
        button_rect = (iui_rect_t) {
            .x = ctx->layout.x + (ctx->layout.width - btn_size) * 0.5f,
            .y = ctx->layout.y + (ctx->layout.height - btn_size) * 0.5f,
            .width = btn_size,
            .height = btn_size,
        };
    } else if (ctx->in_flex) {
        /* In flex mode, use layout rect */
        iui_rect_t flex_rect = iui_flex_next(ctx);
        button_rect = (iui_rect_t) {
            .x = flex_rect.x + (flex_rect.width - btn_size) * 0.5f,
            .y = flex_rect.y + (flex_rect.height - btn_size) * 0.5f,
            .width = btn_size,
            .height = btn_size,
        };
    } else if (ctx->in_row) {
        /* In row mode, use layout_next */
        iui_rect_t row_rect = iui_layout_next(ctx);
        button_rect = (iui_rect_t) {
            .x = row_rect.x + (row_rect.width - btn_size) * 0.5f,
            .y = row_rect.y + (row_rect.height - btn_size) * 0.5f,
            .width = btn_size,
            .height = btn_size,
        };
    } else {
        /* Default: left-aligned in current layout position */
        button_rect = (iui_rect_t) {
            .x = ctx->layout.x,
            .y = ctx->layout.y,
            .width = btn_size,
            .height = btn_size,
        };
        /* Advance layout cursor */
        ctx->layout.y += btn_size + ctx->padding;
    }

    /* Expand touch target for accessibility (48dp minimum) */
    iui_rect_t touch_rect = button_rect;
    iui_expand_touch_target(&touch_rect, IUI_ICON_BUTTON_TOUCH_TARGET);

    /* Get component state using touch target for interaction */
    iui_state_t state = iui_get_component_state(ctx, touch_rect, false);

    /* Determine colors based on style and selected state */
    uint32_t container_color = 0;
    uint32_t icon_color = 0;
    uint32_t outline_color = 0;
    float corner_radius = IUI_ICON_BUTTON_CORNER_RADIUS;
    bool draw_container = false, draw_outline = false;

    /* Handle toggle selected state */
    bool is_selected = is_toggle && selected && *selected;

    switch (style) {
    case ICON_BUTTON_FILLED:
        draw_container = true;
        if (is_selected) {
            container_color = ctx->colors.primary;
            icon_color = ctx->colors.on_primary;
        } else {
            container_color = ctx->colors.surface_container_highest;
            icon_color = ctx->colors.primary;
        }
        break;

    case ICON_BUTTON_TONAL:
        draw_container = true;
        if (is_selected) {
            container_color = ctx->colors.secondary_container;
            icon_color = ctx->colors.on_secondary_container;
        } else {
            container_color = ctx->colors.surface_container_highest;
            icon_color = ctx->colors.on_surface_variant;
        }
        break;

    case ICON_BUTTON_OUTLINED:
        draw_outline = true;
        outline_color = ctx->colors.outline;
        if (is_selected) {
            draw_container = true;
            container_color = ctx->colors.inverse_surface;
            icon_color = ctx->colors.inverse_on_surface;
            outline_color = 0; /* No outline when selected */
            draw_outline = false;
        } else {
            icon_color = ctx->colors.on_surface_variant;
        }
        break;

    case ICON_BUTTON_STANDARD:
    default:
        /* No container for standard style */
        if (is_selected) {
            icon_color = ctx->colors.primary;
        } else {
            icon_color = ctx->colors.on_surface_variant;
        }
        break;
    }

    /* Draw container background (if applicable) */
    if (draw_container) {
        ctx->renderer.draw_box(button_rect, corner_radius, container_color,
                               ctx->renderer.user);
    }

    /* Draw outline (for outlined variant) */
    if (draw_outline)
        iui_draw_rect_outline(ctx, button_rect, 1.f, outline_color);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, button_rect, corner_radius, icon_color, state);

    /* Draw icon centered in button */
    float icon_cx = button_rect.x + button_rect.width * 0.5f;
    float icon_cy = button_rect.y + button_rect.height * 0.5f;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, IUI_ICON_BUTTON_ICON_SIZE, icon,
                      icon_color);

    /* Handle click */
    bool clicked = (state == IUI_STATE_PRESSED);

    /* Toggle state on click */
    if (clicked && is_toggle && selected)
        *selected = !*selected;

    return clicked;
}

/* Standard Icon Button (no container) */
bool iui_icon_button(iui_context *ctx, const char *icon)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_STANDARD, false,
                                    NULL);
}

/* Filled Icon Button (primary container) */
bool iui_icon_button_filled(iui_context *ctx, const char *icon)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_FILLED, false, NULL);
}

/* Filled Tonal Icon Button (secondary_container) */
bool iui_icon_button_tonal(iui_context *ctx, const char *icon)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_TONAL, false, NULL);
}

/* Outlined Icon Button (outline border) */
bool iui_icon_button_outlined(iui_context *ctx, const char *icon)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_OUTLINED, false,
                                    NULL);
}

/* Toggle Icon Button (standard style with selected state) */
bool iui_icon_button_toggle(iui_context *ctx, const char *icon, bool *selected)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_STANDARD, true,
                                    selected);
}

/* Toggle Icon Button with Filled style */
bool iui_icon_button_toggle_filled(iui_context *ctx,
                                   const char *icon,
                                   bool *selected)
{
    return iui_icon_button_internal(ctx, icon, ICON_BUTTON_FILLED, true,
                                    selected);
}
