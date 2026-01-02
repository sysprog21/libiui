/* Search Bar component implementation */

#include "internal.h"

/* MD3 Search Bar
 *
 * Search bar is a text input component with:
 * - Leading icon (default: search)
 * - Trailing icon (default: clear when text present)
 * - Rounded container (full-height corner radius)
 * - Placeholder text support
 *
 * Per MD3:
 * - Height: 56dp
 * - Corner radius: 28dp (fully rounded)
 * - Leading/trailing icon spacing: 16dp from edges
 */

iui_search_bar_result iui_search_bar_ex(iui_context *ctx,
                                        char *buffer,
                                        size_t size,
                                        size_t *cursor,
                                        const char *placeholder,
                                        const char *leading_icon,
                                        const char *trailing_icon)
{
    iui_search_bar_result result = {0};

    if (!ctx || !ctx->current_window || !buffer || !cursor || size == 0)
        return result;

    /* Default icons */
    const char *lead_icon = leading_icon ? leading_icon : "search";
    bool has_text = (buffer[0] != '\0');

    /* Calculate dimensions */
    float bar_height = IUI_SEARCH_BAR_HEIGHT;
    float corner_radius = IUI_SEARCH_BAR_CORNER_RADIUS;
    float icon_size = IUI_SEARCH_BAR_ICON_SIZE;
    float padding_h = IUI_SEARCH_BAR_PADDING_H;
    float icon_gap = IUI_SEARCH_BAR_ICON_GAP;

    /* Calculate bar rect from current layout */
    iui_rect_t bar_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = bar_height,
    };

    /* Calculate icon positions */
    float leading_icon_x = bar_rect.x + padding_h + icon_size * 0.5f;
    float icon_cy = bar_rect.y + bar_height * 0.5f;

    /* Trailing icon area (only active when there's text or custom trailing
     * icon)
     */
    const char *trail_icon = NULL;
    if (trailing_icon) {
        trail_icon = trailing_icon;
    } else if (has_text) {
        trail_icon = "close"; /* Default clear icon when text present */
    }

    float trailing_icon_x =
        bar_rect.x + bar_rect.width - padding_h - icon_size * 0.5f;

    /* Text area start position */
    float text_x_start = bar_rect.x + padding_h + icon_size + icon_gap;

    /* Hit rect for trailing icon click */
    iui_rect_t trailing_hit_rect = {
        .x = trailing_icon_x - icon_size * 0.5f - 4.f,
        .y = bar_rect.y + (bar_height - 48.f) * 0.5f, /* 48dp touch target */
        .width = icon_size + 8.f,
        .height = 48.f,
    };

    /* Check trailing icon click first */
    if (trail_icon && (ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
        in_rect(&trailing_hit_rect, ctx->mouse_pos)) {
        buffer[0] = '\0';
        *cursor = 0;
        result.cleared = true;
    }

    /* Focus handling */
    bool has_focus = (ctx->focused_edit == buffer);
    if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
        in_rect(&bar_rect, ctx->mouse_pos) && !result.cleared) {
        ctx->focused_edit = buffer;
        has_focus = true;
        ctx->cursor_blink = 0.f;
    } else if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
               !in_rect(&bar_rect, ctx->mouse_pos) && has_focus) {
        ctx->focused_edit = NULL;
        has_focus = false;
    }

    /* Handle keyboard input when focused */
    if (has_focus) {
        /* Handle Enter key (submit) first */
        if (ctx->key_pressed == IUI_KEY_ENTER)
            result.submitted = true;

        result.value_changed =
            iui_process_text_input(ctx, buffer, size, cursor, true);

        /* Clear key/char after processing */
        ctx->key_pressed = IUI_KEY_NONE;
        ctx->char_input = 0;
    }

    /* Get component state for hover effects */
    iui_state_t state = iui_get_component_state(ctx, bar_rect, false);

    /* Draw container background (surface_container_high with full round
     * corners)
     */
    ctx->renderer.draw_box(bar_rect, corner_radius,
                           ctx->colors.surface_container_high,
                           ctx->renderer.user);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, bar_rect, corner_radius, ctx->colors.on_surface,
                         state);

    /* Draw leading icon */
    iui_draw_fab_icon(ctx, leading_icon_x, icon_cy, icon_size, lead_icon,
                      ctx->colors.on_surface_variant);

    /* Draw text or placeholder */
    float text_y = bar_rect.y + (bar_height - ctx->font_height) * 0.5f;
    has_text = (buffer[0] != '\0'); /* Update after potential clear */

    if (has_text) {
        /* Draw actual text */
        iui_internal_draw_text(ctx, text_x_start, text_y, buffer,
                               ctx->colors.on_surface);
    } else if (placeholder) {
        /* Draw placeholder */
        iui_internal_draw_text(ctx, text_x_start, text_y, placeholder,
                               ctx->colors.on_surface_variant);
    }

    /* Draw cursor when focused */
    if (has_focus) {
        ctx->cursor_blink += ctx->delta_time;
        if (ctx->cursor_blink > 1.f)
            ctx->cursor_blink -= 1.f;

        if (ctx->cursor_blink < 0.5f) {
            /* Calculate cursor x position */
            size_t pos = *cursor;
            char temp[256];
            size_t copy_len = (pos < sizeof(temp) - 1) ? pos : sizeof(temp) - 1;
            if (copy_len > 0)
                strncpy(temp, buffer, copy_len);
            temp[copy_len] = '\0';

            float cursor_x = text_x_start + iui_get_text_width(ctx, temp);
            float cursor_y = text_y;
            float cursor_height = ctx->font_height;

            /* Clamp cursor to text area */
            float max_cursor_x = bar_rect.x + bar_rect.width - padding_h - 2.f;
            if (trail_icon)
                max_cursor_x -= icon_size + icon_gap;
            if (cursor_x > max_cursor_x)
                cursor_x = max_cursor_x;

            iui_rect_t cursor_rect = {
                cursor_x, cursor_y, IUI_TEXTFIELD_CURSOR_WIDTH, cursor_height};
            ctx->renderer.draw_box(cursor_rect, 0.f, ctx->colors.primary,
                                   ctx->renderer.user);
        }
    }

    /* Draw trailing icon (if present) */
    if (trail_icon) {
        /* Check hover state on trailing icon for visual feedback */
        iui_state_t trail_state =
            iui_get_component_state(ctx, trailing_hit_rect, false);
        uint32_t trail_color = ctx->colors.on_surface_variant;

        /* Draw hover/press state layer on trailing icon area */
        if (trail_state == IUI_STATE_HOVERED ||
            trail_state == IUI_STATE_PRESSED) {
            uint8_t alpha = (trail_state == IUI_STATE_PRESSED)
                                ? IUI_STATE_PRESS_ALPHA
                                : IUI_STATE_HOVER_ALPHA;
            uint32_t layer_color =
                iui_state_layer(ctx->colors.on_surface, alpha);
            iui_rect_t trail_layer_rect = {trailing_icon_x - icon_size * 0.5f,
                                           icon_cy - icon_size * 0.5f,
                                           icon_size, icon_size};
            ctx->renderer.draw_box(trail_layer_rect, icon_size * 0.5f,
                                   layer_color, ctx->renderer.user);
        }

        iui_draw_fab_icon(ctx, trailing_icon_x, icon_cy, icon_size, trail_icon,
                          trail_color);
    }

    /* Advance layout cursor */
    ctx->layout.y += bar_height + ctx->padding;

    return result;
}

/* Search Bar (simplified interface) */
bool iui_search_bar(iui_context *ctx,
                    char *buffer,
                    size_t size,
                    size_t *cursor,
                    const char *placeholder)
{
    iui_search_bar_result result =
        iui_search_bar_ex(ctx, buffer, size, cursor, placeholder, NULL, NULL);
    return result.submitted;
}

/* Search View Implementation
 * Reference: https://m3.material.io/components/search
 * Full-screen modal search experience with suggestions
 */

void iui_search_view_open(iui_search_view_state *search)
{
    if (!search)
        return;

    search->query[0] = '\0';
    search->cursor = 0;
    search->is_open = true;
    search->frames_since_open = 0;
    search->suggestion_count = 0;
    search->suggestion_y = 0.f;
}

void iui_search_view_close(iui_search_view_state *search)
{
    if (!search)
        return;

    search->is_open = false;
    search->frames_since_open = 0;
    search->suggestion_count = 0;
}

bool iui_search_view_is_open(const iui_search_view_state *search)
{
    return search && search->is_open;
}

const char *iui_search_view_get_query(const iui_search_view_state *search)
{
    return search ? search->query : "";
}

bool iui_search_view_begin(iui_context *ctx,
                           iui_search_view_state *search,
                           float screen_width,
                           float screen_height,
                           const char *placeholder)
{
    if (!ctx || !search)
        return false;

    /* If search view was closed externally (e.g., via suggestion click), clean
     * up the orphaned modal state to prevent blocking other modals.
     */
    if (!search->is_open) {
        if (ctx->modal.active &&
            ctx->modal.id == iui_hash_str("search_view_modal")) {
            iui_close_modal(ctx);
        }
        return false;
    }

    /* Note: frame counter is incremented in iui_search_view_end() AFTER
     * all click protection checks, ensuring the first frame after opening
     * (frames_since_open == 0) blocks clicks.
     */

    /* Reset suggestion count for this frame */
    search->suggestion_count = 0;

    /* Begin modal blocking */
    iui_begin_modal(ctx, "search_view_modal");

    /* Full screen bounds */
    iui_rect_t screen_bounds = {0, 0, screen_width, screen_height};
    iui_register_blocking_region(ctx, screen_bounds);

    /* Draw full-screen surface background */
    ctx->renderer.draw_box(screen_bounds, 0.f, ctx->colors.surface,
                           ctx->renderer.user);

    /* Header dimensions */
    float header_h = IUI_SEARCH_VIEW_HEADER_HEIGHT;
    float padding = IUI_SEARCH_VIEW_PADDING;
    float icon_size = IUI_SEARCH_VIEW_ICON_SIZE;
    float touch_target = 48.f;

    /* Draw back arrow button on left */
    iui_rect_t back_rect = {
        .x = padding,
        .y = (header_h - touch_target) * 0.5f,
        .width = touch_target,
        .height = touch_target,
    };

    iui_state_t back_state = iui_get_component_state(ctx, back_rect, false);

    /* Draw state layer for back button */
    iui_draw_state_layer(ctx, back_rect, touch_target * 0.5f,
                         ctx->colors.on_surface_variant, back_state);

    /* Draw back arrow icon */
    float back_cx = back_rect.x + touch_target * 0.5f;
    float back_cy = back_rect.y + touch_target * 0.5f;
    iui_draw_fab_icon(ctx, back_cx, back_cy, icon_size, "arrow_back",
                      ctx->colors.on_surface_variant);

    /* Handle back click */
    if (search->frames_since_open >= 1 && back_state == IUI_STATE_PRESSED) {
        iui_search_view_close(search);
        iui_close_modal(ctx);
        return false;
    }

    /* Search field area */
    float field_x = padding + touch_target + padding * 0.5f;
    float field_w = screen_width - field_x - padding;
    float field_h = IUI_SEARCH_BAR_HEIGHT;
    float field_y = (header_h - field_h) * 0.5f;

    iui_rect_t field_rect = {field_x, field_y, field_w, field_h};

    /* Draw search field background */
    float corner = IUI_SEARCH_BAR_CORNER_RADIUS;
    ctx->renderer.draw_box(field_rect, corner,
                           ctx->colors.surface_container_high,
                           ctx->renderer.user);

    /* Auto-focus the search field */
    ctx->focused_edit = search->query;

    /* Handle text input */
    iui_process_text_input(ctx, search->query, sizeof(search->query),
                           &search->cursor, true);
    ctx->key_pressed = IUI_KEY_NONE;
    ctx->char_input = 0;

    /* Calculate text area */
    float text_x = field_x + padding;
    float text_y = field_y + (field_h - ctx->font_height) * 0.5f;

    /* Draw text or placeholder */
    bool has_text = (search->query[0] != '\0');
    if (has_text) {
        iui_internal_draw_text(ctx, text_x, text_y, search->query,
                               ctx->colors.on_surface);
    } else if (placeholder) {
        iui_internal_draw_text(ctx, text_x, text_y, placeholder,
                               ctx->colors.on_surface_variant);
    }

    /* Draw cursor */
    ctx->cursor_blink += ctx->delta_time;
    if (ctx->cursor_blink > 1.f)
        ctx->cursor_blink -= 1.f;

    if (ctx->cursor_blink < 0.5f) {
        char temp[256];
        size_t pos = search->cursor;
        size_t copy_len = (pos < sizeof(temp) - 1) ? pos : sizeof(temp) - 1;
        if (copy_len > 0)
            strncpy(temp, search->query, copy_len);
        temp[copy_len] = '\0';

        float cursor_x = text_x + iui_get_text_width(ctx, temp);
        iui_rect_t cursor_rect = {cursor_x, text_y, IUI_TEXTFIELD_CURSOR_WIDTH,
                                  ctx->font_height};
        ctx->renderer.draw_box(cursor_rect, 0.f, ctx->colors.primary,
                               ctx->renderer.user);
    }

    /* Draw clear button if text present */
    if (has_text) {
        float clear_x = field_x + field_w - padding - icon_size * 0.5f;
        float clear_cy = field_y + field_h * 0.5f;

        iui_rect_t clear_rect = {clear_x - touch_target * 0.5f,
                                 clear_cy - touch_target * 0.5f, touch_target,
                                 touch_target};

        iui_state_t clear_state =
            iui_get_component_state(ctx, clear_rect, false);

        iui_draw_state_layer(ctx, clear_rect, touch_target * 0.5f,
                             ctx->colors.on_surface_variant, clear_state);

        iui_draw_fab_icon(ctx, clear_x, clear_cy, icon_size, "close",
                          ctx->colors.on_surface_variant);

        /* Handle clear click */
        if (search->frames_since_open >= 1 &&
            clear_state == IUI_STATE_PRESSED) {
            search->query[0] = '\0';
            search->cursor = 0;
        }
    }

    /* Store suggestion start y position */
    search->suggestion_y = header_h;

    return true;
}

bool iui_search_view_suggestion(iui_context *ctx,
                                iui_search_view_state *search,
                                const char *icon,
                                const char *text)
{
    if (!ctx || !search || !search->is_open || !text)
        return false;

    float padding = IUI_SEARCH_VIEW_PADDING;
    float item_h = IUI_SEARCH_VIEW_SUGGESTION_HEIGHT;
    float icon_size = IUI_SEARCH_VIEW_ICON_SIZE;

    float item_y = search->suggestion_y;
    float item_w = ctx->layout.width + padding * 2.f;

    iui_rect_t item_rect = {0, item_y, item_w, item_h};

    /* Get interaction state */
    iui_state_t state = iui_get_component_state(ctx, item_rect, false);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, item_rect, 0.f, ctx->colors.on_surface, state);

    /* Draw icon if provided */
    float content_x = padding;

    if (icon) {
        float icon_cx = padding + icon_size * 0.5f;
        float icon_cy = item_y + item_h * 0.5f;
        iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icon,
                          ctx->colors.on_surface_variant);
        content_x += icon_size + padding;
    }

    /* Draw suggestion text */
    float text_y = item_y + (item_h - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, content_x, text_y, text,
                           ctx->colors.on_surface);

    /* Advance suggestion y for next item */
    search->suggestion_y += item_h;
    search->suggestion_count++;

    /* Handle click */
    if (search->frames_since_open >= 1 && state == IUI_STATE_PRESSED)
        return true;

    return false;
}

void iui_search_view_end(iui_context *ctx, iui_search_view_state *search)
{
    if (!ctx || !search || !search->is_open)
        return;

    /* End modal blocking */
    iui_end_modal(ctx);

    /* Increment frame counter AFTER all protection checks */
    search->frames_since_open++;
}
