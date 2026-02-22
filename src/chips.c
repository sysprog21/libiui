/* Chips component implementation */

#include "internal.h"

/* Chip style variants */
typedef enum {
    CHIP_STYLE_ASSIST,     /* Contextual action with optional icon */
    CHIP_STYLE_FILTER,     /* Toggleable selection with checkmark */
    CHIP_STYLE_INPUT,      /* Removable user input with trailing X */
    CHIP_STYLE_SUGGESTION, /* Dynamic suggestion (text only) */
} chip_style;

/* MD3 Chips
 *
 * Chips are compact elements that represent an attribute, text, entity, or
 * action.
 *
 * Four types per MD3:
 * - Assist: Contextual action (icon + label)
 * - Filter: Toggleable selection (checkmark + label)
 * - Input: User-entered data with remove action (label + close)
 * - Suggestion: Dynamic suggestion (label only)
 *
 * Dimensions:
 * - Height: 32dp
 * - Corner radius: 8dp
 * - Icon size: 18dp
 * - Touch target: 48dp minimum
 */

static bool iui_chip_internal(iui_context *ctx,
                              const char *label,
                              const char *icon,
                              chip_style style,
                              bool *selected,
                              bool *removed)
{
    if (!ctx || !label)
        return false;
    if (!ctx->current_window)
        return false;

    /* Calculate chip dimensions */
    float chip_height = IUI_CHIP_HEIGHT;
    float corner_radius = IUI_CHIP_CORNER_RADIUS;
    float icon_size = IUI_CHIP_ICON_SIZE;

    /* Determine if a leading icon exists */
    bool has_leading_icon =
        icon || (style == CHIP_STYLE_FILTER && selected && *selected);

    /* Determine if a trailing icon exists (input chips have close icon) */
    bool has_trailing_icon = (style == CHIP_STYLE_INPUT);

    /* Calculate content width */
    float text_width = iui_get_text_width(ctx, label);
    float content_width = text_width;

    /* Add space for leading icon */
    float padding_start =
        has_leading_icon ? IUI_CHIP_PADDING_H_ICON : IUI_CHIP_PADDING_H;
    float padding_end =
        has_trailing_icon ? IUI_CHIP_PADDING_H_ICON : IUI_CHIP_PADDING_H;

    if (has_leading_icon)
        content_width += icon_size + IUI_CHIP_ICON_LABEL_GAP;
    if (has_trailing_icon)
        content_width += icon_size + IUI_CHIP_ICON_LABEL_GAP;

    float chip_width = padding_start + content_width + padding_end;

    /* Compute chip rect from layout */
    iui_rect_t chip_rect;

    if (ctx->box_depth > 0) {
        /* Box takes priority: left-aligned within box slot */
        chip_rect = (iui_rect_t) {
            .x = ctx->layout.x,
            .y = ctx->layout.y + (ctx->layout.height - chip_height) * 0.5f,
            .width = chip_width,
            .height = chip_height,
        };
    } else if (ctx->in_grid) {
        /* Grid without box: center chip in cell */
        chip_rect = (iui_rect_t) {
            .x = ctx->layout.x + (ctx->layout.width - chip_width) * 0.5f,
            .y = ctx->layout.y + (ctx->layout.height - chip_height) * 0.5f,
            .width = chip_width,
            .height = chip_height,
        };
    } else {
        /* Default: left-aligned in current layout position */
        chip_rect = (iui_rect_t) {
            .x = ctx->layout.x,
            .y = ctx->layout.y,
            .width = chip_width,
            .height = chip_height,
        };
        /* Advance layout cursor */
        ctx->layout.y += chip_height + ctx->padding;
    }

    /* Expand touch target for accessibility (48dp minimum) */
    iui_rect_t touch_rect = chip_rect;
    iui_expand_touch_target_h(&touch_rect, IUI_CHIP_TOUCH_TARGET);

    /* Trailing icon hit area (for input chips) */
    iui_rect_t trailing_icon_rect = {0};
    if (has_trailing_icon) {
        trailing_icon_rect = (iui_rect_t) {
            .x = chip_rect.x + chip_rect.width - padding_end - icon_size,
            .y = chip_rect.y + (chip_height - icon_size) * 0.5f,
            .width = icon_size,
            .height = icon_size,
        };
    }

    /* Check if trailing icon was clicked (for input chips) */
    bool trailing_clicked = false;
    if (has_trailing_icon && (ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
        in_rect(&trailing_icon_rect, ctx->mouse_pos)) {
        trailing_clicked = true;
        if (removed)
            *removed = true;
    }

    /* Get component state using touch target for interaction */
    iui_state_t state = iui_get_component_state(ctx, touch_rect, false);

    /* Determine colors based on style and selected state */
    uint32_t container_color = 0;
    uint32_t text_color = 0;
    uint32_t outline_color = 0;
    uint32_t icon_color = 0;
    bool draw_container = false;
    bool draw_outline = false;

    bool is_selected = (style == CHIP_STYLE_FILTER && selected && *selected);

    switch (style) {
    case CHIP_STYLE_ASSIST:
        /* Assist: surface_container_low background */
        draw_container = true;
        container_color = ctx->colors.surface_container_low;
        text_color = ctx->colors.on_surface;
        icon_color = ctx->colors.primary;
        break;

    case CHIP_STYLE_FILTER:
        if (is_selected) {
            /* Selected: secondary_container fill */
            draw_container = true;
            container_color = ctx->colors.secondary_container;
            text_color = ctx->colors.on_secondary_container;
            icon_color = ctx->colors.on_secondary_container;
        } else {
            /* Unselected: outline only, no fill */
            draw_outline = true;
            outline_color = ctx->colors.outline;
            text_color = ctx->colors.on_surface;
        }
        break;

    case CHIP_STYLE_INPUT:
        /* Input: surface_container_low + outline */
        draw_container = true;
        draw_outline = true;
        container_color = ctx->colors.surface_container_low;
        outline_color = ctx->colors.outline;
        text_color = ctx->colors.on_surface_variant;
        icon_color = ctx->colors.on_surface_variant;
        break;

    case CHIP_STYLE_SUGGESTION:
        /* Suggestion: surface_container_low background */
        draw_container = true;
        container_color = ctx->colors.surface_container_low;
        text_color = ctx->colors.on_surface_variant;
        break;
    }

    /* Draw chip container */
    if (draw_container) {
        ctx->renderer.draw_box(chip_rect, corner_radius, container_color,
                               ctx->renderer.user);
    }

    /* Draw outline */
    if (draw_outline) {
        float outline_width = 1.f;

        if (ctx->renderer.draw_arc) {
            /* Best quality: use arcs for smooth rounded corners */
            /* Top edge */
            iui_rect_t top_edge = {chip_rect.x + corner_radius, chip_rect.y,
                                   chip_rect.width - 2 * corner_radius,
                                   outline_width};
            ctx->renderer.draw_box(top_edge, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Bottom edge */
            iui_rect_t bottom_edge = {
                chip_rect.x + corner_radius,
                chip_rect.y + chip_rect.height - outline_width,
                chip_rect.width - 2 * corner_radius, outline_width};
            ctx->renderer.draw_box(bottom_edge, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Left edge */
            iui_rect_t left_edge = {chip_rect.x, chip_rect.y + corner_radius,
                                    outline_width,
                                    chip_rect.height - 2 * corner_radius};
            ctx->renderer.draw_box(left_edge, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Right edge */
            iui_rect_t right_edge = {
                chip_rect.x + chip_rect.width - outline_width,
                chip_rect.y + corner_radius, outline_width,
                chip_rect.height - 2 * corner_radius};
            ctx->renderer.draw_box(right_edge, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Top-left corner arc */
            ctx->renderer.draw_arc(chip_rect.x + corner_radius,
                                   chip_rect.y + corner_radius, corner_radius,
                                   IUI_PI, 1.5f * IUI_PI, outline_width,
                                   outline_color, ctx->renderer.user);
            /* Top-right corner arc */
            ctx->renderer.draw_arc(
                chip_rect.x + chip_rect.width - corner_radius,
                chip_rect.y + corner_radius, corner_radius, 1.5f * IUI_PI,
                2.f * IUI_PI, outline_width, outline_color, ctx->renderer.user);
            /* Bottom-right corner arc */
            ctx->renderer.draw_arc(
                chip_rect.x + chip_rect.width - corner_radius,
                chip_rect.y + chip_rect.height - corner_radius, corner_radius,
                0.f, 0.5f * IUI_PI, outline_width, outline_color,
                ctx->renderer.user);
            /* Bottom-left corner arc */
            ctx->renderer.draw_arc(
                chip_rect.x + corner_radius,
                chip_rect.y + chip_rect.height - corner_radius, corner_radius,
                0.5f * IUI_PI, IUI_PI, outline_width, outline_color,
                ctx->renderer.user);
        } else if (draw_container) {
            /* Fallback with container: overlay approach creates outline
             * by drawing outer rect with outline color and inner rect with
             * container color to simulate border
             */
            ctx->renderer.draw_box(chip_rect, corner_radius, outline_color,
                                   ctx->renderer.user);
            /* Inner rounded rect (container color) to create outline effect */
            iui_rect_t inner_rect = {chip_rect.x + outline_width,
                                     chip_rect.y + outline_width,
                                     chip_rect.width - 2 * outline_width,
                                     chip_rect.height - 2 * outline_width};
            float inner_corner = corner_radius > outline_width
                                     ? corner_radius - outline_width
                                     : 0.f;
            ctx->renderer.draw_box(inner_rect, inner_corner, container_color,
                                   ctx->renderer.user);
        } else {
            /* Fallback without container: draw edges only (corners have small
             * gaps)
             * Acceptable degraded quality for renderers without draw_arc.
             * Top edge (full width, small corner overlap is acceptable)
             */
            iui_rect_t top_edge_full = {chip_rect.x, chip_rect.y,
                                        chip_rect.width, outline_width};
            ctx->renderer.draw_box(top_edge_full, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Bottom edge */
            iui_rect_t bottom_edge_full = {
                chip_rect.x, chip_rect.y + chip_rect.height - outline_width,
                chip_rect.width, outline_width};
            ctx->renderer.draw_box(bottom_edge_full, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Left edge */
            iui_rect_t left_edge_full = {chip_rect.x, chip_rect.y,
                                         outline_width, chip_rect.height};
            ctx->renderer.draw_box(left_edge_full, 0.f, outline_color,
                                   ctx->renderer.user);
            /* Right edge */
            iui_rect_t right_edge_full = {
                chip_rect.x + chip_rect.width - outline_width, chip_rect.y,
                outline_width, chip_rect.height};
            ctx->renderer.draw_box(right_edge_full, 0.f, outline_color,
                                   ctx->renderer.user);
        }
    }

    /* Draw state layer for hover/press feedback */
    if (!trailing_clicked)
        iui_draw_state_layer(ctx, chip_rect, corner_radius, text_color, state);

    /* Draw content */
    float content_x = chip_rect.x + padding_start;
    float center_y = chip_rect.y + chip_height * 0.5f;

    /* Draw leading icon */
    if (has_leading_icon) {
        float icon_cx = content_x + icon_size * 0.5f;
        float icon_cy = center_y;

        if (style == CHIP_STYLE_FILTER && is_selected) {
            /* Draw checkmark for selected filter chip */
            iui_draw_icon_check(ctx, icon_cx, icon_cy, icon_size, icon_color);
        } else if (icon) {
            /* Draw custom icon */
            iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icon,
                              icon_color);
        }
        content_x += icon_size + IUI_CHIP_ICON_LABEL_GAP;
    }

    /* Draw label */
    float text_y = center_y - ctx->font_height * 0.5f;
    iui_internal_draw_text(ctx, content_x, text_y, label, text_color);

    /* Draw trailing icon (close X for input chips) */
    if (has_trailing_icon) {
        float trailing_cx = trailing_icon_rect.x + icon_size * 0.5f;
        float trailing_cy = center_y;
        iui_draw_fab_icon(ctx, trailing_cx, trailing_cy, icon_size, "close",
                          icon_color);
    }

    /* Handle click */
    bool clicked = (state == IUI_STATE_PRESSED && !trailing_clicked);

    /* Toggle state on click for filter chips */
    if (clicked && style == CHIP_STYLE_FILTER && selected)
        *selected = !*selected;

    /* MD3 runtime validation: track touch target (not visual bounds) */
    IUI_MD3_TRACK_CHIP(touch_rect, corner_radius);

    return clicked;
}

/* Assist Chip - Contextual action */
bool iui_chip_assist(iui_context *ctx, const char *label, const char *icon)
{
    return iui_chip_internal(ctx, label, icon, CHIP_STYLE_ASSIST, NULL, NULL);
}

/* Filter Chip - Toggleable selection */
bool iui_chip_filter(iui_context *ctx, const char *label, bool *selected)
{
    return iui_chip_internal(ctx, label, NULL, CHIP_STYLE_FILTER, selected,
                             NULL);
}

/* Input Chip - User-entered information with remove action */
bool iui_chip_input(iui_context *ctx, const char *label, bool *removed)
{
    if (removed)
        *removed = false; /* Reset removed flag */
    return iui_chip_internal(ctx, label, NULL, CHIP_STYLE_INPUT, NULL, removed);
}

/* Suggestion Chip - Dynamic suggestion */
bool iui_chip_suggestion(iui_context *ctx, const char *label)
{
    return iui_chip_internal(ctx, label, NULL, CHIP_STYLE_SUGGESTION, NULL,
                             NULL);
}
