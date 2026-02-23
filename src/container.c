#include <assert.h>
#include "internal.h"

/* Card container */

void iui_card_begin(iui_context *ctx,
                    float x,
                    float y,
                    float w,
                    float h,
                    iui_card_style_t style)
{
    if (!ctx->current_window)
        return;

    iui_rect_t card_rect = {.x = x, .y = y, .width = w, .height = h};

    uint32_t bg_color, border_color = 0;
    float border_width = 0.f;
    iui_elevation_t elevation = IUI_ELEVATION_0;

    switch (style) {
    case IUI_CARD_ELEVATED:
        /* MD3: Elevated cards use surface_container_low with shadow */
        bg_color = ctx->colors.surface_container_low;
        elevation = IUI_ELEVATION_1; /* Subtle shadow for cards */
        break;
    case IUI_CARD_FILLED:
        /* Filled cards use surface_container_highest background (no shadow) */
        bg_color = ctx->colors.surface_container_highest;
        break;
    case IUI_CARD_OUTLINED:
        /* Outlined cards use surface with outline border */
        bg_color = ctx->colors.surface;
        border_color = ctx->colors.outline;
        border_width = 1.f;
        break;
    default:
        /* Default to elevated style */
        bg_color = ctx->colors.surface_container_low;
        elevation = IUI_ELEVATION_1;
        break;
    }

    /* Draw card with elevation (shadow + box) */
    iui_draw_elevated_box(ctx, card_rect, ctx->corner, elevation, bg_color);

    /* Draw border if specified */
    if (border_width > 0.f)
        iui_draw_rect_outline(ctx, card_rect, border_width, border_color);

    /* Save current layout to restore later */
    ctx->layout = (iui_rect_t) {
        .x = card_rect.x + ctx->padding,
        .y = card_rect.y + ctx->padding,
        .width = card_rect.width - 2 * ctx->padding,
        .height = card_rect.height - 2 * ctx->padding,
    };
}

void iui_card_end(iui_context *ctx)
{
    /* Intentional no-op for API symmetry with iui_card_begin().
     * Cards are self-contained visual elements, so no cleanup is needed.
     * This function is retained for a consistent begin/end pairing pattern.
     */
    (void) ctx;
}

/* Progress indicators */

void iui_progress_linear(iui_context *ctx,
                         float value,
                         float max,
                         bool indeterminate)
{
    if (!ctx->current_window || max <= 0)
        return;

    /* Define progress bar dimensions */
    iui_rect_t bar_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y + ctx->layout.height * 0.5f -
             2, /* Center vertically */
        .width = ctx->layout.width,
        .height = 4, /* Standard height for progress bar */
    };

    /* Draw background track */
    ctx->renderer.draw_box(bar_rect,
                           bar_rect.height * 0.5f, /* Pill-shaped corners */
                           ctx->colors.surface_container, ctx->renderer.user);

    /* Draw progress indicator */
    float progress_ratio =
        (max > 0) ? fminf(1.f, fmaxf(0.f, value / max)) : 0.f;
    float filled_width = bar_rect.width * progress_ratio;

    if (indeterminate) {
        /* For indeterminate state, show animated moving bar */
        float bar_width = bar_rect.width * 0.3f; /* 30% of total width */
        float offset = fmodf(ctx->cursor_blink * 3.f, 1.1f) *
                       bar_rect.width; /* Animation based on time */
        float anim_x =
            bar_rect.x + fmodf(offset, bar_rect.width + bar_width) - bar_width;

        /* Draw indeterminate progress bar */
        if (anim_x < bar_rect.x + bar_rect.width &&
            anim_x + bar_width > bar_rect.x) {
            float draw_x = fmaxf(anim_x, bar_rect.x);
            float draw_width =
                fminf(anim_x + bar_width, bar_rect.x + bar_rect.width) - draw_x;
            ctx->renderer.draw_box(
                (iui_rect_t) {draw_x, bar_rect.y, draw_width, bar_rect.height},
                bar_rect.height * 0.5f, ctx->colors.primary,
                ctx->renderer.user);
        }
    } else {
        /* Draw determinate progress */
        if (filled_width > 0) {
            ctx->renderer.draw_box(
                (iui_rect_t) {bar_rect.x, bar_rect.y, filled_width,
                              bar_rect.height},
                bar_rect.height * 0.5f, /* Pill-shaped corners */
                ctx->colors.primary, ctx->renderer.user);
        }
    }

    /* Advance layout for next element */
    iui_newline(ctx);
}

void iui_progress_circular(iui_context *ctx,
                           float value,
                           float max,
                           float size,
                           bool indeterminate)
{
    if (!ctx->current_window || max <= 0 || !iui_has_vector_primitives(ctx))
        return;

    /* Calculate center position */
    float center_x = ctx->layout.x + ctx->layout.width * 0.5f;
    float center_y = ctx->layout.y + size * 0.5f;
    float radius = size * 0.5f - 2; /* Account for stroke width */

    /* Draw background circle (track) */
    iui_draw_circle(ctx, center_x, center_y, radius, 0, /* No fill */
                    iui_state_layer(ctx->colors.surface_container, 0xFF), 1.f);

    /* Draw progress arc */
    float progress_ratio =
        (max > 0) ? fminf(1.f, fmaxf(0.f, value / max)) : 0.f;
    float end_angle;

    if (indeterminate) {
        /* For indeterminate state, show a moving arc segment */
        float time_offset = ctx->cursor_blink * 2.f; /* Speed control */
        float start_angle = fmodf(time_offset, 2.f * IUI_PI); /* Moving start */
        end_angle = start_angle + 1.5f; /* Fixed arc length for indeterminate */
    } else {
        /* For determinate state, go from 0 to calculated angle */
        end_angle = progress_ratio * 2.f * IUI_PI;
    }

    if (end_angle > 0) {
        iui_draw_arc(ctx, center_x, center_y, radius, 0, end_angle, 2.f,
                     ctx->colors.primary);
    }

    /* Advance layout by the size of the circular indicator */
    ctx->layout.y += size;
}

/* MD3 snackbar: 48dp height, 344-672dp width, uses inverse colors for contrast
 */

/* Snackbar */

void iui_snackbar_show(iui_snackbar_state *snackbar,
                       const char *message,
                       float duration,
                       const char *action_label)
{
    if (!snackbar)
        return;
    snackbar->message = message;
    snackbar->action_label = action_label;
    snackbar->duration = duration;
    snackbar->timer = duration;
    snackbar->active = true;
}

void iui_snackbar_hide(iui_snackbar_state *snackbar)
{
    if (!snackbar)
        return;
    snackbar->active = false;
    snackbar->timer = 0.f;
}

bool iui_snackbar(iui_context *ctx,
                  iui_snackbar_state *snackbar,
                  float screen_width,
                  float screen_height)
{
    if (!ctx || !snackbar || !snackbar->active || !snackbar->message)
        return false;

    /* Handle auto-dismiss timer (duration > 0 means timed, 0 = persistent) */
    if (snackbar->duration > 0.f) {
        snackbar->timer -= ctx->delta_time;
        if (snackbar->timer <= 0.f) {
            iui_snackbar_hide(snackbar);
            return false;
        }
    }

    bool action_clicked = false;

    /* MD3 snackbar dimensions (scaled for typical font height) */
    const float min_width = 280.f, max_width = 540.f; /* From 344/672dp */
    const float snackbar_height = 48.f;
    const float corner_radius = 4.f;
    const float padding_h = ctx->spacing.md;      /* 16dp horizontal padding */
    const float margin_bottom = ctx->spacing.md;  /* 16dp bottom margin */
    const float action_padding = ctx->spacing.xs; /* 8dp action padding */

    /* Calculate text width to size snackbar appropriately */
    float message_width = iui_get_text_width(ctx, snackbar->message);
    float action_width = 0.f;
    if (snackbar->action_label) {
        action_width = iui_get_text_width(ctx, snackbar->action_label) +
                       action_padding * 2.f;
    }

    /* Calculate snackbar width */
    float content_width = message_width + padding_h * 2.f;
    if (snackbar->action_label)
        content_width += action_width + padding_h;
    float snackbar_width = fminf(max_width, fmaxf(min_width, content_width));

    /* Position at bottom center of screen */
    float snackbar_x = (screen_width - snackbar_width) * 0.5f,
          snackbar_y = screen_height - snackbar_height - margin_bottom;

    /* Draw snackbar background using inverse colors for contrast */
    ctx->renderer.draw_box(
        (iui_rect_t) {snackbar_x, snackbar_y, snackbar_width, snackbar_height},
        corner_radius, ctx->colors.inverse_surface, ctx->renderer.user);

    /* Draw message text */
    float text_y = snackbar_y + (snackbar_height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, snackbar_x + padding_h, text_y,
                           snackbar->message, ctx->colors.inverse_on_surface);

    /* Draw action button if present */
    if (snackbar->action_label) {
        float action_x = snackbar_x + snackbar_width - action_width - padding_h;
        iui_rect_t action_rect = {
            .x = action_x,
            .y = snackbar_y +
                 (snackbar_height - ctx->font_height - action_padding * 2.f) *
                     0.5f,
            .width = action_width,
            .height = ctx->font_height + action_padding * 2.f,
        };

        /* Check for hover/press on action button */
        bool hovered = in_rect(&action_rect, ctx->mouse_pos);
        bool pressed = hovered && (ctx->mouse_pressed & IUI_MOUSE_LEFT);
        bool released = hovered && (ctx->mouse_released & IUI_MOUSE_LEFT);

        /* Draw hover state layer */
        if (hovered) {
            uint8_t alpha =
                pressed ? IUI_STATE_PRESS_ALPHA : IUI_STATE_HOVER_ALPHA;
            uint32_t layer_color =
                iui_state_layer(ctx->colors.inverse_primary, alpha);
            ctx->renderer.draw_box(action_rect, corner_radius, layer_color,
                                   ctx->renderer.user);
        }

        /* Draw action text */
        float action_text_x = action_x + action_padding;
        float action_text_y =
            snackbar_y + (snackbar_height - ctx->font_height) * 0.5f;
        iui_internal_draw_text(ctx, action_text_x, action_text_y,
                               snackbar->action_label,
                               ctx->colors.inverse_primary);

        /* Handle action click */
        if (released) {
            action_clicked = true;
            iui_snackbar_hide(snackbar);
        }
    }

    return action_clicked;
}

/* Scrollable container: uses clip stack for masking and layout offset for
 * scrolling
 */

iui_rect_t iui_scroll_begin(iui_context *ctx,
                            iui_scroll_state *state,
                            float view_w,
                            float view_h)
{
    iui_rect_t viewport = {0, 0, 0, 0};
    if (!ctx || !state)
        return viewport;

    /* Resolve auto-sizing: 0 or negative means "layout width/height + offset".
     * 0.0f = fill available, -N.0f = fill minus N pixels (e.g. sibling gap).
     */
    if (view_w <= 0.0f)
        view_w = fmaxf(0.0f, ctx->layout.width + view_w);
    if (view_h <= 0.0f)
        view_h = fmaxf(0.0f, ctx->layout.height + view_h);

    assert(
        !ctx->active_scroll &&
        "nested scroll regions are not supported; call iui_scroll_end first");
    assert(view_w <= ctx->layout.width + 0.5f &&
           "scroll view_w exceeds layout width; use 0.f for auto");

    viewport.x = ctx->layout.x;
    viewport.y = ctx->layout.y;
    viewport.width = view_w;
    viewport.height = view_h;

    ctx->scroll_viewport = viewport;
    ctx->active_scroll = state;

    /* Save layout origin for restoration in iui_scroll_end */
    ctx->scroll_content_start_x = ctx->layout.x;
    ctx->scroll_content_start_y = ctx->layout.y;
    ctx->scroll_content_start_width = ctx->layout.width;

    /* Unconditionally reserve scrollbar space to avoid a one-frame width
     * pop when content first exceeds view_h. */
    ctx->layout.width = fmaxf(0.f, view_w - IUI_SCROLLBAR_W);

    /* Clamp scroll to valid range (content size from previous frame) */
    float max_scroll_x = fmaxf(0.f, state->content_w - view_w);
    float max_scroll_y = fmaxf(0.f, state->content_h - view_h);
    state->scroll_x = clamp_float(0.f, max_scroll_x, state->scroll_x);
    state->scroll_y = clamp_float(0.f, max_scroll_y, state->scroll_y);

    /* Apply scroll wheel input if mouse is within viewport */
    if (in_rect(&viewport, ctx->mouse_pos)) {
        state->scroll_x -= ctx->scroll_wheel_dx;
        state->scroll_y -= ctx->scroll_wheel_dy;
        state->scroll_x = clamp_float(0.f, max_scroll_x, state->scroll_x);
        state->scroll_y = clamp_float(0.f, max_scroll_y, state->scroll_y);
        /* Consume delta to prevent overlapping scroll regions from reusing */
        ctx->scroll_wheel_dx = 0;
        ctx->scroll_wheel_dy = 0;
    }

    /* Push clip rect for viewport; rollback on overflow */
    if (!iui_push_clip(ctx, viewport)) {
        ctx->layout.width = ctx->scroll_content_start_width;
        ctx->active_scroll = NULL;
        return (iui_rect_t) {0, 0, 0, 0};
    }

    /* Offset layout by scroll position (content moves opposite to scroll) */
    ctx->layout.x -= state->scroll_x;
    ctx->layout.y -= state->scroll_y;

    return viewport;
}

/* Scroll Container */

bool iui_scroll_end(iui_context *ctx, iui_scroll_state *state)
{
    if (!ctx || !state)
        return false;

    /* Guard against calls when iui_scroll_begin failed (returned empty rect).
     * On failure, active_scroll is NULL; proceeding would pop an unrelated
     * clip and corrupt the stack. */
    if (ctx->active_scroll != state)
        return false;

    /* Measure content size (assumes vertical-only content flow) */
    float content_height =
        (ctx->layout.y + state->scroll_y) - ctx->scroll_content_start_y;
    float content_width =
        ctx->scroll_viewport.width; /* Default to viewport width */

    state->content_h = fmaxf(content_height, 0.f);
    state->content_w = fmaxf(content_width, 0.f);

    /* Restore layout to after the viewport */
    ctx->layout.x = ctx->scroll_content_start_x;
    ctx->layout.width = ctx->scroll_content_start_width;
    ctx->layout.y = ctx->scroll_content_start_y + ctx->scroll_viewport.height +
                    ctx->padding;

    ctx->active_scroll = NULL;

    bool scrollable_x = state->content_w > ctx->scroll_viewport.width;
    bool scrollable_y = state->content_h > ctx->scroll_viewport.height;

    /* Release drag if scrollbar disappears (content shrunk) */
    if (ctx->scroll_dragging == state &&
        (ctx->mouse_released & IUI_MOUSE_LEFT)) {
        ctx->scroll_dragging = NULL;
    }

    if (scrollable_y) {
        float scrollbar_width = IUI_SCROLLBAR_W;
        float track_height = ctx->scroll_viewport.height;
        float thumb_height =
            fmaxf(20.f, track_height *
                            (ctx->scroll_viewport.height / state->content_h));
        float thumb_y_range = track_height - thumb_height;
        float max_scroll = state->content_h - ctx->scroll_viewport.height;
        float scroll_ratio =
            (max_scroll > 0.f) ? state->scroll_y / max_scroll : 0.f;
        float thumb_y = ctx->scroll_viewport.y + thumb_y_range * scroll_ratio;

        iui_rect_t track_rect = {
            .x = ctx->scroll_viewport.x + ctx->scroll_viewport.width -
                 scrollbar_width,
            .y = ctx->scroll_viewport.y,
            .width = scrollbar_width,
            .height = track_height,
        };

        iui_rect_t thumb_rect = {
            .x = track_rect.x,
            .y = thumb_y,
            .width = scrollbar_width,
            .height = thumb_height,
        };

        bool is_dragging = (ctx->scroll_dragging == state);

        /* Left mouse button starts drag */
        if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
            in_rect(&thumb_rect, ctx->mouse_pos)) {
            ctx->scroll_dragging = state;
            ctx->scroll_drag_offset = ctx->mouse_pos.y - thumb_rect.y;
            is_dragging = true;
        }

        if (is_dragging && (ctx->mouse_held & IUI_MOUSE_LEFT)) {
            float new_thumb_y = ctx->mouse_pos.y - ctx->scroll_drag_offset;
            new_thumb_y = clamp_float(
                track_rect.y, track_rect.y + thumb_y_range, new_thumb_y);
            float new_ratio = (thumb_y_range > 0.f)
                                  ? (new_thumb_y - track_rect.y) / thumb_y_range
                                  : 0.f;
            state->scroll_y = new_ratio * max_scroll;
            thumb_y = new_thumb_y;
            thumb_rect.y = thumb_y;
        }

        ctx->renderer.draw_box(track_rect, scrollbar_width * 0.5f,
                               ctx->colors.surface_container_high,
                               ctx->renderer.user);

        uint32_t thumb_color = ctx->colors.outline;
        if (is_dragging) {
            thumb_color = ctx->colors.primary;
        } else if (in_rect(&thumb_rect, ctx->mouse_pos)) {
            thumb_color = ctx->colors.on_surface_variant;
        }
        ctx->renderer.draw_box(thumb_rect, scrollbar_width * 0.5f, thumb_color,
                               ctx->renderer.user);
    }

    /* Pop after scrollbar draw so the thumb stays bounded by the viewport */
    iui_pop_clip(ctx);

    return scrollable_x || scrollable_y;
}

void iui_scroll_by(iui_scroll_state *state, float dx, float dy)
{
    if (!state)
        return;
    state->scroll_x += dx;
    state->scroll_y += dy;
    /* Clamping is deferred to iui_scroll_begin when viewport size is known */
}

void iui_scroll_to(iui_scroll_state *state, float x, float y)
{
    if (!state)
        return;
    state->scroll_x = x;
    state->scroll_y = y;
    /* Clamping is deferred to iui_scroll_begin when viewport size is known */
}

/* Bottom Sheet
 * Reference: https://m3.material.io/components/bottom-sheets
 */

void iui_bottom_sheet_open(iui_bottom_sheet_state *state)
{
    if (state) {
        state->open = true;
        state->target_height = state->height > 0.f ? state->height : 300.f;
    }
}

void iui_bottom_sheet_close(iui_bottom_sheet_state *state)
{
    if (state)
        state->open = false;
}

void iui_bottom_sheet_set_height(iui_bottom_sheet_state *state, float height)
{
    if (state) {
        state->target_height = height;
        if (!state->dragging)
            state->height = height;
    }
}

bool iui_bottom_sheet_begin(iui_context *ctx,
                            iui_bottom_sheet_state *state,
                            float screen_width,
                            float screen_height)
{
    if (!ctx || !state)
        return false;

    /* Initialize defaults if not set */
    if (state->min_height <= 0.f)
        state->min_height = IUI_BOTTOM_SHEET_MIN_HEIGHT;
    if (state->max_height <= 0.f)
        state->max_height = screen_height * 0.9f;
    if (state->target_height <= 0.f)
        state->target_height = 300.f;

    /* Animate open/close */
    float target = state->open ? 1.0f : 0.0f;
    float speed = 8.0f;
    if (state->anim_progress < target) {
        state->anim_progress += ctx->delta_time * speed;
        if (state->anim_progress > target)
            state->anim_progress = target;
    } else if (state->anim_progress > target) {
        state->anim_progress -= ctx->delta_time * speed;
        if (state->anim_progress < target)
            state->anim_progress = target;
    }

    /* Don't render if fully closed */
    if (state->anim_progress <= 0.0f && !state->open)
        return false;

    /* Interpolate height during animation */
    float current_height = state->height * state->anim_progress;
    float sheet_y = screen_height - current_height;

    /* Draw scrim for modal sheets */
    if (state->modal && state->anim_progress > 0.0f) {
        iui_rect_t scrim_rect = {0, 0, screen_width, screen_height};
        uint8_t scrim_alpha =
            (uint8_t) (IUI_SCRIM_ALPHA * state->anim_progress);
        ctx->renderer.draw_box(
            scrim_rect, 0.f,
            (scrim_alpha << 24) | (ctx->colors.scrim & 0x00FFFFFF),
            ctx->renderer.user);

        /* Push modal layer */
        iui_push_layer(ctx, 100);

        /* Click outside to close */
        iui_rect_t sheet_rect = {0, sheet_y, screen_width, current_height};
        if (ctx->mouse_pressed && !in_rect(&sheet_rect, ctx->mouse_pos)) {
            state->open = false;
        }
    }

    /* Draw sheet background with rounded top corners */
    iui_rect_t sheet_rect = {0, sheet_y, screen_width, current_height + 100.f};
    ctx->renderer.draw_box(sheet_rect, IUI_BOTTOM_SHEET_CORNER_RADIUS,
                           ctx->colors.surface_container_low,
                           ctx->renderer.user);

    /* Draw drag handle */
    float handle_x = (screen_width - IUI_BOTTOM_SHEET_DRAG_HANDLE_WIDTH) * 0.5f;
    float handle_y = sheet_y + IUI_BOTTOM_SHEET_DRAG_HANDLE_MARGIN;
    iui_rect_t handle_rect = {handle_x, handle_y,
                              IUI_BOTTOM_SHEET_DRAG_HANDLE_WIDTH,
                              IUI_BOTTOM_SHEET_DRAG_HANDLE_HEIGHT};
    ctx->renderer.draw_box(handle_rect,
                           IUI_BOTTOM_SHEET_DRAG_HANDLE_HEIGHT * 0.5f,
                           ctx->colors.on_surface_variant, ctx->renderer.user);

    /* Handle drag interaction */
    iui_rect_t drag_area = {0, sheet_y, screen_width, 48.f};
    if (in_rect(&drag_area, ctx->mouse_pos) && ctx->mouse_pressed &&
        !state->dragging) {
        state->dragging = true;
        state->drag_start_y = ctx->mouse_pos.y;
        state->drag_start_height = state->height;
    }

    if (state->dragging) {
        if (ctx->mouse_held & IUI_MOUSE_LEFT) {
            float drag_delta = state->drag_start_y - ctx->mouse_pos.y;
            state->height = state->drag_start_height + drag_delta;
            state->height = clamp_float(state->min_height, state->max_height,
                                        state->height);
        } else {
            state->dragging = false;
            /* Snap to open or closed based on velocity/position */
            if (state->height < state->min_height + 50.f) {
                state->open = false;
            }
        }
    }

    /* Register blocking region */
    iui_rect_t block_rect = {0, sheet_y, screen_width, current_height};
    iui_register_blocking_region(ctx, block_rect);

    float content_y = sheet_y + IUI_BOTTOM_SHEET_DRAG_HANDLE_MARGIN * 2.f +
                      IUI_BOTTOM_SHEET_DRAG_HANDLE_HEIGHT;
    iui_rect_t content_rect = {0, content_y, screen_width,
                               current_height - (content_y - sheet_y)};
    if (!iui_push_clip(ctx, content_rect))
        return false; /* clip overflow: abort sheet content */

    return true;
}

void iui_bottom_sheet_end(iui_context *ctx, iui_bottom_sheet_state *state)
{
    if (!ctx || !state)
        return;

    iui_pop_clip(ctx);

    /* Pop modal layer if used */
    if (state->modal && state->anim_progress > 0.0f)
        iui_pop_layer(ctx);
}

/* Tooltip
 * Reference: https://m3.material.io/components/tooltips
 * MD3 plain tooltip: inverse_surface bg, inverse_on_surface text
 */

void iui_tooltip(iui_context *ctx, const char *text)
{
    if (!ctx || !text || !ctx->current_window)
        return;

    /* Only show tooltip when hovering over the anchor widget (previous widget).
     * The anchor rect corresponds to the layout area used by the preceding
     * widget call (typically a button in the same grid cell/row).
     */
    iui_rect_t anchor_rect = {ctx->layout.x, ctx->layout.y, ctx->layout.width,
                              ctx->row_height};
    if (!in_rect(&anchor_rect, ctx->mouse_pos))
        return;

    /* Calculate tooltip dimensions */
    float text_width = iui_get_text_width(ctx, text);
    float width = fminf(
        IUI_TOOLTIP_MAX_WIDTH,
        fmaxf(IUI_TOOLTIP_MIN_WIDTH, text_width + IUI_TOOLTIP_PADDING * 2));
    float height = fmaxf(IUI_TOOLTIP_MIN_HEIGHT,
                         ctx->font_height + IUI_TOOLTIP_PADDING * 2);

    /* Position tooltip below the current layout position */
    float x = ctx->layout.x + (ctx->layout.width - width) * 0.5f;
    float y = ctx->layout.y + ctx->row_height + IUI_TOOLTIP_OFFSET;

    /* Clamp tooltip position to stay within window bounds */
    iui_rect_t win = iui_get_window_rect(ctx);
    if (x < win.x)
        x = win.x;
    if (x + width > win.x + win.width)
        x = win.x + win.width - width;

    /* Smart vertical positioning: check available space above and below */
    if (y + height > win.y + win.height) {
        float space_below =
            (win.y + win.height) - (ctx->layout.y + ctx->row_height);
        float space_above = ctx->layout.y - win.y;
        /* Would overflow bottom - try flipping above if more space there */
        if (space_above > space_below && space_above >= height)
            y = ctx->layout.y - height - IUI_TOOLTIP_OFFSET;
        else
            y = win.y + win.height - height; /* clamp to bottom edge */
    }

    /* Draw tooltip background using inverse colors for contrast */
    iui_rect_t tooltip_rect = {x, y, width, height};
    ctx->renderer.draw_box(tooltip_rect, IUI_TOOLTIP_CORNER_RADIUS,
                           ctx->colors.inverse_surface, ctx->renderer.user);

    /* Draw text centered in tooltip */
    float text_x = x + (width - text_width) * 0.5f;
    float text_y = y + (height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, text,
                           ctx->colors.inverse_on_surface);
}

bool iui_tooltip_rich(iui_context *ctx,
                      const char *title,
                      const char *text,
                      const char *action)
{
    if (!ctx || !text || !ctx->current_window)
        return false;

    /* Only show tooltip when hovering over the anchor widget (previous widget).
     * The anchor rect corresponds to the layout area used by the preceding
     * widget call (typically a button in the same grid cell/row).
     */
    iui_rect_t anchor_rect = {ctx->layout.x, ctx->layout.y, ctx->layout.width,
                              ctx->row_height};
    if (!in_rect(&anchor_rect, ctx->mouse_pos))
        return false;

    bool action_clicked = false;

    /* Calculate dimensions */
    float title_width = title ? iui_get_text_width(ctx, title) : 0.f;
    float text_width = iui_get_text_width(ctx, text);
    float action_width = action ? iui_get_text_width(ctx, action) : 0.f;
    float max_text_width = fmaxf(fmaxf(title_width, text_width), action_width);
    float width = fminf(
        IUI_TOOLTIP_MAX_WIDTH,
        fmaxf(IUI_TOOLTIP_MIN_WIDTH, max_text_width + IUI_TOOLTIP_PADDING * 2));

    /* Calculate height based on content */
    float content_height = ctx->font_height; /* body text */
    if (title)
        content_height += ctx->font_height + 4.f; /* title + gap */
    if (action)
        content_height += ctx->font_height + 8.f; /* action + gap */
    float height = content_height + IUI_TOOLTIP_PADDING * 2;

    /* Position tooltip */
    float x = ctx->layout.x + (ctx->layout.width - width) * 0.5f;
    float y = ctx->layout.y + ctx->row_height + IUI_TOOLTIP_OFFSET;

    /* Clamp tooltip position to stay within window bounds */
    iui_rect_t win = iui_get_window_rect(ctx);
    if (x < win.x)
        x = win.x;
    if (x + width > win.x + win.width)
        x = win.x + win.width - width;

    /* Smart vertical positioning: check available space above and below */
    if (y + height > win.y + win.height) {
        float space_below =
            (win.y + win.height) - (ctx->layout.y + ctx->row_height);
        float space_above = ctx->layout.y - win.y;
        /* Would overflow bottom - try flipping above if more space there */
        if (space_above > space_below && space_above >= height)
            y = ctx->layout.y - height - IUI_TOOLTIP_OFFSET;
        else
            y = win.y + win.height - height; /* clamp to bottom edge */
    }

    /* Draw background */
    iui_rect_t tooltip_rect = {x, y, width, height};
    ctx->renderer.draw_box(tooltip_rect, IUI_TOOLTIP_CORNER_RADIUS,
                           ctx->colors.inverse_surface, ctx->renderer.user);

    /* Draw content */
    float text_y = y + IUI_TOOLTIP_PADDING;

    if (title) {
        iui_internal_draw_text(ctx, x + IUI_TOOLTIP_PADDING, text_y, title,
                               ctx->colors.inverse_on_surface);
        text_y += ctx->font_height + 4.f;
    }

    iui_internal_draw_text(ctx, x + IUI_TOOLTIP_PADDING, text_y, text,
                           ctx->colors.inverse_on_surface);
    text_y += ctx->font_height + 8.f;

    if (action) {
        iui_rect_t action_rect = {x + IUI_TOOLTIP_PADDING, text_y,
                                  action_width + 8.f, ctx->font_height + 4.f};

        /* Check for action click */
        bool hovered = in_rect(&action_rect, ctx->mouse_pos);
        if (hovered && (ctx->mouse_released & IUI_MOUSE_LEFT))
            action_clicked = true;

        /* Draw action with hover state */
        if (hovered) {
            uint32_t hover_color = iui_state_layer(ctx->colors.inverse_primary,
                                                   IUI_STATE_HOVER_ALPHA);
            ctx->renderer.draw_box(action_rect, 4.f, hover_color,
                                   ctx->renderer.user);
        }

        iui_internal_draw_text(ctx, x + IUI_TOOLTIP_PADDING + 4.f, text_y,
                               action, ctx->colors.inverse_primary);
    }

    return action_clicked;
}

/* Badge
 * Reference: https://m3.material.io/components/badges
 * MD3 badge: error color background, on_error text
 */

void iui_badge_dot(iui_context *ctx, float anchor_x, float anchor_y)
{
    if (!ctx || !ctx->current_window)
        return;

    /* Draw a small notification dot at the anchor position */
    float radius = IUI_BADGE_DOT_SIZE * 0.5f;
    float x = anchor_x - IUI_BADGE_OFFSET_X;
    float y = anchor_y + IUI_BADGE_OFFSET_Y;

    ctx->renderer.draw_box(
        (iui_rect_t) {x - radius, y - radius, IUI_BADGE_DOT_SIZE,
                      IUI_BADGE_DOT_SIZE},
        radius, ctx->colors.error, ctx->renderer.user);
}

void iui_badge_number(iui_context *ctx,
                      float anchor_x,
                      float anchor_y,
                      int count,
                      int max_count)
{
    if (!ctx || !ctx->current_window || count <= 0)
        return;

    /* Clamp max_count to 999 to ensure formatted text fits in buffer.
     * Buffer size (8) must hold: max 3 digits + '+' + '\0' = 5 chars minimum.
     * Example outputs: "1", "99", "999", "999+" */
    int effective_max = (max_count > 0 && max_count < 1000) ? max_count : 999;

    char badge_text[8];
    if (count > effective_max)
        snprintf(badge_text, sizeof(badge_text), "%d+", effective_max);
    else
        snprintf(badge_text, sizeof(badge_text), "%d", count);

    /* Calculate badge dimensions */
    float text_width = iui_get_text_width(ctx, badge_text);
    float width =
        fmaxf(IUI_BADGE_LABEL_SIZE, text_width + IUI_BADGE_LABEL_PADDING * 2);
    float height = IUI_BADGE_LABEL_SIZE;
    float radius = height * 0.5f; /* pill shape */

    /* Position badge at anchor with offset */
    float x = anchor_x - IUI_BADGE_OFFSET_X - width * 0.5f;
    float y = anchor_y + IUI_BADGE_OFFSET_Y - height * 0.5f;

    /* Draw badge background */
    ctx->renderer.draw_box((iui_rect_t) {x, y, width, height}, radius,
                           ctx->colors.error, ctx->renderer.user);

    /* Draw badge text centered */
    float text_x = x + (width - text_width) * 0.5f;
    float text_y = y + (height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, badge_text,
                           ctx->colors.on_error);
}

/* Banner
 * Reference: https://m3.material.io/components/banners
 * MD3 banner: surface_container bg, on_surface text, primary actions
 */

/* Helper: Draw a banner action button, returns true if clicked */
static bool draw_banner_action(iui_context *ctx,
                               float *x,
                               float banner_y,
                               float banner_height,
                               const char *label)
{
    float btn_width = iui_get_text_width(ctx, label) + 16.f;
    *x -= btn_width;

    iui_rect_t btn_rect = {
        *x, banner_y + (banner_height - ctx->font_height - 8.f) * 0.5f,
        btn_width, ctx->font_height + 8.f};

    bool clicked = false;
    bool hovered = in_rect(&btn_rect, ctx->mouse_pos);
    if (hovered) {
        uint32_t hover =
            iui_state_layer(ctx->colors.primary, IUI_STATE_HOVER_ALPHA);
        ctx->renderer.draw_box(btn_rect, 4.f, hover, ctx->renderer.user);
        if (ctx->mouse_released & IUI_MOUSE_LEFT)
            clicked = true;
    }

    float text_x = *x + (btn_width - iui_get_text_width(ctx, label)) * 0.5f;
    iui_internal_draw_text(ctx, text_x, btn_rect.y + 4.f, label,
                           ctx->colors.primary);
    return clicked;
}

int iui_banner(iui_context *ctx, const iui_banner_options *options)
{
    if (!ctx || !options || !options->message || !ctx->current_window)
        return 0;

    int result = 0;

    /* Calculate banner height based on content */
    float height = fmaxf(IUI_BANNER_MIN_HEIGHT,
                         ctx->font_height * 2 + IUI_BANNER_PADDING * 2);

    /* Banner rect at current layout position */
    iui_rect_t banner_rect = {ctx->layout.x, ctx->layout.y, ctx->layout.width,
                              height};

    /* Draw banner background */
    ctx->renderer.draw_box(banner_rect, 0.f, ctx->colors.surface_container,
                           ctx->renderer.user);

    /* Draw divider at bottom */
    ctx->renderer.draw_box(
        (iui_rect_t) {banner_rect.x, banner_rect.y + height - 1.f,
                      banner_rect.width, 1.f},
        0.f, ctx->colors.outline_variant, ctx->renderer.user);

    float content_x = banner_rect.x + IUI_BANNER_PADDING;

    /* Draw leading icon if provided */
    if (options->icon) {
        float icon_cy = banner_rect.y + height * 0.5f;
        iui_draw_fab_icon(ctx, content_x + IUI_BANNER_ICON_SIZE * 0.5f, icon_cy,
                          IUI_BANNER_ICON_SIZE, options->icon,
                          ctx->colors.on_surface);
        content_x += IUI_BANNER_ICON_SIZE + IUI_BANNER_PADDING;
    }

    /* Calculate total width needed for action buttons to avoid text overlap.
     * Pre-compute action positions before drawing message text.
     */
    float action_x = banner_rect.x + banner_rect.width - IUI_BANNER_PADDING;
    float actions_width = 0.f;
    if (options->action2) {
        float btn_width = iui_get_text_width(ctx, options->action2) + 16.f;
        actions_width += btn_width + IUI_BANNER_ACTION_GAP;
    }
    if (options->action1) {
        float btn_width = iui_get_text_width(ctx, options->action1) + 16.f;
        actions_width += btn_width;
    }

    /* Draw message text with clipping to prevent overlap with actions.
     * Reserve space for actions plus padding between message and buttons.
     * Always show message (clipped if necessary) rather than dropping it.
     */
    float text_y = banner_rect.y + (height - ctx->font_height) * 0.5f;
    float msg_max_width = banner_rect.width - (content_x - banner_rect.x) -
                          actions_width - IUI_BANNER_PADDING;

    /* Ensure minimum width to show at least partial message */
    if (msg_max_width < ctx->font_height)
        msg_max_width = ctx->font_height;
    iui_rect_t msg_clip = {content_x, banner_rect.y, msg_max_width, height};
    if (iui_push_clip(ctx, msg_clip)) {
        iui_internal_draw_text(ctx, content_x, text_y, options->message,
                               ctx->colors.on_surface);
        iui_pop_clip(ctx);
    }

    if (options->action2) {
        if (draw_banner_action(ctx, &action_x, banner_rect.y, height,
                               options->action2))
            result = 2;
        action_x -= IUI_BANNER_ACTION_GAP;
    }

    if (options->action1) {
        if (draw_banner_action(ctx, &action_x, banner_rect.y, height,
                               options->action1))
            result = 1;
    }

    /* Advance layout past banner */
    ctx->layout.y += height + ctx->padding;

    return result;
}

/* Data Table
 * Reference: https://m3.material.io/components/data-tables
 *
 * User-provided iui_table_state enables:
 * - Multiple tables rendered simultaneously
 * - Nested tables
 * - Thread-safe rendering (each thread uses its own state)
 */

void iui_table_begin(iui_context *ctx,
                     iui_table_state *state,
                     int cols,
                     const float *widths)
{
    if (!ctx || !ctx->current_window || !state || cols <= 0 ||
        cols > IUI_MAX_TABLE_COLS || !widths)
        return;

    state->cols = cols;
    state->start_x = ctx->layout.x;
    state->row_y = ctx->layout.y;
    state->current_col = 0;
    state->in_header = true;
    state->row_index = -1;

    /* Compute column widths (similar to flex layout) */
    float total_fixed = 0.f, total_flex = 0.f;
    int zero_count = 0;
    for (int i = 0; i < cols; i++) {
        if (widths[i] > 0)
            total_fixed += widths[i];
        else if (widths[i] < 0)
            total_flex += -widths[i];
        else
            zero_count++;
    }

    float available = ctx->layout.width - total_fixed;
    if (available < 0)
        available = 0;

    float flex_unit = (total_flex > 0) ? available / total_flex : 0.f;
    /* Zero-width columns share remaining space equally if no flex cols */
    float zero_width = 0.f;
    if (zero_count > 0 && total_flex == 0.f && available > 0)
        zero_width = available / zero_count;

    for (int i = 0; i < cols; i++) {
        if (widths[i] > 0)
            state->widths[i] = widths[i];
        else if (widths[i] < 0)
            state->widths[i] = -widths[i] * flex_unit;
        else
            state->widths[i] = zero_width; /* share remaining space */
    }

    state->current_x = state->start_x;
}

void iui_table_header(iui_context *ctx,
                      iui_table_state *state,
                      const char *text)
{
    if (!ctx || !ctx->current_window || !state || !text ||
        state->current_col >= state->cols)
        return;

    float width = state->widths[state->current_col];
    float height = IUI_TABLE_HEADER_HEIGHT;

    /* Draw header cell background */
    iui_rect_t cell_rect = {state->current_x, state->row_y, width, height};
    ctx->renderer.draw_box(cell_rect, 0.f, ctx->colors.surface_container,
                           ctx->renderer.user);

    /* Draw header text with label_large style (bold) */
    float text_x = state->current_x + IUI_TABLE_CELL_PADDING;
    float text_y = state->row_y + (height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, text,
                           ctx->colors.on_surface_variant);

    state->current_x += width;
    state->current_col++;

    /* If all headers done, draw header divider and move to data rows */
    if (state->current_col >= state->cols) {
        state->row_y += height;
        /* Draw divider below header */
        ctx->renderer.draw_box(
            (iui_rect_t) {state->start_x,
                          state->row_y - IUI_TABLE_DIVIDER_HEIGHT,
                          ctx->layout.width, IUI_TABLE_DIVIDER_HEIGHT},
            0.f, ctx->colors.outline_variant, ctx->renderer.user);
        state->in_header = false;
    }
}

void iui_table_row_begin(iui_context *ctx, iui_table_state *state)
{
    if (!ctx || !ctx->current_window || !state)
        return;

    state->current_col = 0;
    state->current_x = state->start_x;
    state->row_index++;
}

void iui_table_cell(iui_context *ctx,
                    iui_table_state *state,
                    const char *text,
                    ...)
{
    if (!ctx || !ctx->current_window || !state || !text ||
        state->current_col >= state->cols)
        return;

    /* Format text with error handling */
    ctx->string_buffer[0] = '\0'; /* Initialize buffer before formatting */
    va_list args;
    va_start(args, text);
    int written =
        vsnprintf(ctx->string_buffer, IUI_STRING_BUFFER_SIZE, text, args);
    va_end(args);
    if (written < 0) {
        ctx->string_buffer[0] = '\0'; /* vsnprintf error, use empty string */
        return;
    }
    if (written >= IUI_STRING_BUFFER_SIZE)
        ctx->string_buffer[IUI_STRING_BUFFER_SIZE - 1] = '\0';

    float width = state->widths[state->current_col];
    float height = IUI_TABLE_ROW_HEIGHT;

    /* Alternate row background for zebra striping */
    if (state->row_index % 2 == 1) {
        iui_rect_t cell_rect = {state->current_x, state->row_y, width, height};
        ctx->renderer.draw_box(cell_rect, 0.f,
                               ctx->colors.surface_container_low,
                               ctx->renderer.user);
    }

    /* Draw cell text */
    float text_x = state->current_x + IUI_TABLE_CELL_PADDING;
    float text_y = state->row_y + (height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, ctx->string_buffer,
                           ctx->colors.on_surface);

    state->current_x += width;
    state->current_col++;
}

void iui_table_row_end(iui_context *ctx, iui_table_state *state)
{
    if (!ctx || !ctx->current_window || !state)
        return;

    state->row_y += IUI_TABLE_ROW_HEIGHT;

    /* Draw row divider */
    ctx->renderer.draw_box(
        (iui_rect_t) {state->start_x, state->row_y - IUI_TABLE_DIVIDER_HEIGHT,
                      ctx->layout.width, IUI_TABLE_DIVIDER_HEIGHT},
        0.f, ctx->colors.outline_variant, ctx->renderer.user);
}

void iui_table_end(iui_context *ctx, iui_table_state *state)
{
    if (!ctx || !ctx->current_window || !state)
        return;

    /* Update layout position to after table */
    ctx->layout.y = state->row_y + ctx->padding;
}
