#include "internal.h"

/* Text Edit Widgets */

/* Textfield color state for consistent rendering */
typedef struct {
    uint32_t bg, border, text, icon;
    float indicator_height;
} textfield_colors_t;

/* Calculate textfield colors based on state (focus, hover, disabled) */
static textfield_colors_t textfield_calc_colors(iui_context *ctx,
                                                bool has_focus,
                                                bool hovered,
                                                bool disabled)
{
    textfield_colors_t c = {
        .bg = ctx->colors.surface_container,
        .border = has_focus ? ctx->colors.primary : ctx->colors.outline,
        .text = ctx->colors.on_surface,
        .icon = ctx->colors.on_surface_variant,
        .indicator_height = has_focus ? 2.f : 1.f,
    };

    if (disabled) {
        c.bg = iui_blend_color(
            ctx->colors.surface_container,
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_DISABLE_ALPHA));
        c.text = iui_state_layer(ctx->colors.on_surface, IUI_PLACEHOLDER_ALPHA);
        c.icon = iui_state_layer(ctx->colors.on_surface_variant,
                                 IUI_PLACEHOLDER_ALPHA);
        c.border = ctx->colors.outline_variant;
    } else if (hovered && !has_focus) {
        uint32_t hover_layer =
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_HOVER_ALPHA);
        c.bg = iui_blend_color(ctx->colors.surface_container, hover_layer);
    }
    return c;
}

/* Draw textfield background based on style (filled or outlined) */
static void textfield_draw_background(iui_context *ctx,
                                      iui_rect_t rect,
                                      iui_textfield_style_t style,
                                      const textfield_colors_t *c)
{
    if (style == IUI_TEXTFIELD_OUTLINED) {
        ctx->renderer.draw_box(rect, ctx->corner, c->border,
                               ctx->renderer.user);
        ctx->renderer.draw_box((iui_rect_t) {rect.x + 1, rect.y + 1,
                                             rect.width - 2, rect.height - 2},
                               ctx->corner - 1,
                               ctx->colors.surface_container_high,
                               ctx->renderer.user);
    } else {
        ctx->renderer.draw_box(rect, ctx->corner, c->bg, ctx->renderer.user);
        ctx->renderer.draw_box(
            (iui_rect_t) {rect.x, rect.y + rect.height - c->indicator_height,
                          rect.width, c->indicator_height},
            0.f, c->border, ctx->renderer.user);
    }
}

/* Draw textfield icons (leading and trailing) */
static void textfield_draw_icons(iui_context *ctx,
                                 const iui_textfield_options *opts,
                                 iui_rect_t leading_rect,
                                 iui_rect_t trailing_rect,
                                 float icon_size,
                                 uint32_t icon_color)
{
    if (opts->leading_icon != IUI_TEXTFIELD_ICON_NONE) {
        float cx = leading_rect.x + leading_rect.width * 0.5f;
        float cy = leading_rect.y + leading_rect.height * 0.5f;
        iui_draw_textfield_icon(ctx, opts->leading_icon, cx, cy,
                                icon_size * 0.8f, icon_color);
    }

    if (opts->trailing_icon != IUI_TEXTFIELD_ICON_NONE) {
        float cx = trailing_rect.x + trailing_rect.width * 0.5f;
        float cy = trailing_rect.y + trailing_rect.height * 0.5f;

        /* Trailing icon hover effect */
        if (!opts->disabled && in_rect(&trailing_rect, ctx->mouse_pos)) {
            uint32_t hover = iui_state_layer(icon_color, IUI_STATE_HOVER_ALPHA);
            ctx->renderer.draw_box(trailing_rect, icon_size * 0.5f, hover,
                                   ctx->renderer.user);
        }
        iui_draw_textfield_icon(ctx, opts->trailing_icon, cx, cy,
                                icon_size * 0.8f, icon_color);
    }
}

/* Get text width up to position, handling password masking */
static float textfield_get_width_to_pos(iui_context *ctx,
                                        const char *buffer,
                                        size_t pos,
                                        bool password_mode)
{
    if (pos == 0)
        return 0.f;

    char tmp[IUI_STRING_BUFFER_SIZE];
    size_t buf_len = strlen(buffer);
    /* Clamp pos to actual buffer length to prevent OOB read */
    if (pos > buf_len)
        pos = buf_len;
    size_t len = pos < sizeof(tmp) - 1 ? pos : sizeof(tmp) - 1;

    if (password_mode) {
        for (size_t i = 0; i < len; i++)
            tmp[i] = '*';
    } else {
        memcpy(tmp, buffer, len);
    }
    tmp[len] = '\0';
    return iui_get_text_width(ctx, tmp);
}

iui_textfield_result iui_textfield(iui_context *ctx,
                                   char *buffer,
                                   size_t size,
                                   size_t *cursor,
                                   const iui_textfield_options *options)
{
    iui_textfield_result result = {0};

    if (!ctx->current_window || !buffer || !cursor || size == 0)
        return result;

    /* Register this text field for per-frame tracking */
    iui_register_textfield(ctx, buffer);

    /* Default options if NULL */
    iui_textfield_options opts = {0};
    if (options)
        opts = *options;

    /* Calculate icon dimensions */
    float icon_size = ctx->font_height, icon_padding = ctx->padding;
    float leading_icon_width = (opts.leading_icon != IUI_TEXTFIELD_ICON_NONE)
                                   ? (icon_size + icon_padding)
                                   : 0.f;

    /* MD3 spec: TextField height = 56dp */
    iui_rect_t edit_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = IUI_TEXTFIELD_HEIGHT,
    };

    /* Icon hit areas */
    iui_rect_t leading_icon_rect = {
        .x = edit_rect.x + icon_padding * 0.5f,
        .y = edit_rect.y + (edit_rect.height - icon_size) * 0.5f,
        .width = icon_size,
        .height = icon_size,
    };

    iui_rect_t trailing_icon_rect = {
        .x = edit_rect.x + edit_rect.width - icon_size - icon_padding * 0.5f,
        .y = edit_rect.y + (edit_rect.height - icon_size) * 0.5f,
        .width = icon_size,
        .height = icon_size,
    };

    /* Text area start position (between icons) */
    float text_x_start = edit_rect.x + ctx->padding + leading_icon_width;

    /* Check icon clicks first (only if not disabled) */
    if (!opts.disabled && (ctx->mouse_pressed & IUI_MOUSE_LEFT)) {
        if (opts.leading_icon != IUI_TEXTFIELD_ICON_NONE &&
            in_rect(&leading_icon_rect, ctx->mouse_pos)) {
            result.leading_icon_clicked = true;
        }
        if (opts.trailing_icon != IUI_TEXTFIELD_ICON_NONE &&
            in_rect(&trailing_icon_rect, ctx->mouse_pos)) {
            result.trailing_icon_clicked = true;
        }
    }

    /* Focus handling (skip if disabled or read-only) */
    bool has_focus = (ctx->focused_edit == buffer);
    if (!opts.disabled && !opts.read_only) {
        if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
            in_rect(&edit_rect, ctx->mouse_pos) &&
            !result.leading_icon_clicked && !result.trailing_icon_clicked) {
            ctx->focused_edit = buffer;
            has_focus = true;
            ctx->cursor_blink = 0.f;
        } else if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
                   !in_rect(&edit_rect, ctx->mouse_pos) && has_focus) {
            ctx->focused_edit = NULL;
            has_focus = false;
        }
    } else if ((opts.read_only || opts.disabled) && has_focus) {
        /* Clear focus if field became read-only or disabled while focused */
        ctx->focused_edit = NULL;
        has_focus = false;
    }

    /* Handle keyboard input when focused (skip if read_only or disabled) */
    if (has_focus && !opts.read_only && !opts.disabled) {
        /* Handle Enter key (submit) first */
        if (ctx->key_pressed == IUI_KEY_ENTER)
            result.submitted = true;

        result.value_changed =
            iui_process_text_input(ctx, buffer, size, cursor, true);

        /* Reset cursor blink on navigation */
        if (ctx->key_pressed == IUI_KEY_LEFT ||
            ctx->key_pressed == IUI_KEY_RIGHT ||
            ctx->key_pressed == IUI_KEY_HOME || ctx->key_pressed == IUI_KEY_END)
            ctx->cursor_blink = 0.f;
    }

    bool hovered = in_rect(&edit_rect, ctx->mouse_pos);

    /* Calculate colors and draw background */
    textfield_colors_t colors =
        textfield_calc_colors(ctx, has_focus, hovered, opts.disabled);
    textfield_draw_background(ctx, edit_rect, opts.style, &colors);
    textfield_draw_icons(ctx, &opts, leading_icon_rect, trailing_icon_rect,
                         icon_size, colors.icon);

    /* Draw text or placeholder */
    float text_x = text_x_start,
          text_y = edit_rect.y + (edit_rect.height - ctx->font_height) * 0.5f;

    if (buffer[0] == '\0' && opts.placeholder) {
        /* Placeholder text */
        iui_internal_draw_text(
            ctx, text_x, text_y, opts.placeholder,
            iui_state_layer(ctx->colors.on_surface, IUI_PLACEHOLDER_ALPHA));
    } else {
        /* Actual text (or masked for password mode) */
        if (opts.password_mode && buffer[0] != '\0') {
            size_t len = strlen(buffer);
            char masked[IUI_STRING_BUFFER_SIZE];
            size_t mask_len =
                len < sizeof(masked) - 1 ? len : sizeof(masked) - 1;
            for (size_t i = 0; i < mask_len; i++)
                masked[i] = '*';
            masked[mask_len] = '\0';
            iui_internal_draw_text(ctx, text_x, text_y, masked, colors.text);
        } else {
            iui_internal_draw_text(ctx, text_x, text_y, buffer, colors.text);
        }
    }

    /* Draw cursor when focused */
    if (has_focus && !opts.disabled && ctx->cursor_blink < 0.5f) {
        size_t len = strlen(buffer), pos = *cursor;
        if (pos > len)
            pos = len;
        float cursor_x = text_x + textfield_get_width_to_pos(
                                      ctx, buffer, pos, opts.password_mode);
        ctx->renderer.draw_box(
            (iui_rect_t) {cursor_x, text_y, IUI_TEXTFIELD_CURSOR_WIDTH,
                          ctx->font_height},
            0.f, ctx->colors.primary, ctx->renderer.user);
    }

    iui_newline(ctx);
    /* Only clear input if this field consumed it (has focus and is editable) */
    if (has_focus && !opts.read_only && !opts.disabled) {
        ctx->key_pressed = 0;
        ctx->char_input = 0;
    }

    /* MD3 runtime validation: track rendered textfield dimensions */
    IUI_MD3_TRACK_TEXTFIELD(edit_rect, ctx->corner);

    return result;
}

/* Text Selection Helper Functions */

/* Find cursor position from x coordinate (nearest character snapping).
 * @ctx:          Current UI context
 * @buffer:       Text buffer to measure
 * @text_x_start: Starting x position of text
 * @click_x:      X coordinate of mouse click
 *
 * Returns byte position in buffer closest to click_x.
 * Uses incremental width computation for O(n) complexity.
 * Iterates by UTF-8 codepoint to ensure cursor lands on valid boundaries.
 */
static size_t iui_find_cursor_from_x(iui_context *ctx,
                                     const char *buffer,
                                     float text_x_start,
                                     float click_x)
{
    size_t len = strlen(buffer), best_pos = 0;
    if (len == 0)
        return 0;

    float best_dist = fabsf(click_x - text_x_start);
    float cumulative_x = text_x_start;
    size_t pos = 0;

    while (pos < len) {
        /* Decode codepoint at current position */
        uint32_t cp = iui_utf8_decode(buffer, pos, len);
        size_t next_pos = iui_utf8_next(buffer, pos, len);

        /* Accumulate width for this codepoint */
        cumulative_x += iui_get_codepoint_width(ctx, cp);

        /* Check if this boundary is closer to click position */
        float dist = fabsf(click_x - cumulative_x);
        if (dist < best_dist) {
            best_dist = dist;
            best_pos = next_pos;
        }

        pos = next_pos;
    }
    return best_pos;
}

/* Find word boundaries around a position (UTF-8 aware).
 * Returns byte positions for start and end of word containing pos.
 */
static void iui_find_word_boundaries(const char *buffer,
                                     size_t pos,
                                     size_t *start,
                                     size_t *end)
{
    size_t len = strlen(buffer);
    if (len == 0) {
        *start = *end = 0;
        return;
    }

    /* Clamp pos to valid range and ensure at code point boundary */
    if (pos >= len)
        pos = len - 1;
    while (pos > 0 && iui_utf8_is_continuation((unsigned char) buffer[pos]))
        pos--;

    /* Find start: scan backward to find word boundary (UTF-8 aware) */
    *start = pos;
    while (*start > 0) {
        size_t prev = iui_utf8_prev(buffer, *start);
        uint32_t cp = iui_utf8_decode(buffer, prev, len);
        if (!iui_utf8_is_word_char(cp))
            break;
        *start = prev;
    }

    /* Find end: scan forward to find word boundary (UTF-8 aware) */
    *end = pos;
    while (*end < len) {
        uint32_t cp = iui_utf8_decode(buffer, *end, len);
        if (!iui_utf8_is_word_char(cp))
            break;
        *end = iui_utf8_next(buffer, *end, len);
    }

    /* If landed on non-word char, select the single code point */
    if (*start == *end && len > 0)
        *end = iui_utf8_next(buffer, *start, len);
}

/* Normalize selection: ensure start <= end */
static void iui_normalize_selection(iui_edit_state *state)
{
    if (state->selection_start > state->selection_end) {
        size_t tmp = state->selection_start;
        state->selection_start = state->selection_end;
        state->selection_end = tmp;
    }
}

/* Delete selected text, returns true if text was deleted */
static bool iui_delete_selection(char *buffer, iui_edit_state *state)
{
    iui_normalize_selection(state);
    if (state->selection_start == state->selection_end)
        return false;

    size_t len = strlen(buffer);

    /* Move text after selection to fill gap */
    memmove(buffer + state->selection_start, buffer + state->selection_end,
            len - state->selection_end + 1);

    /* Update cursor to selection start */
    state->cursor = state->selection_start;
    state->selection_end = state->selection_start;

    return true;
}

/* Check if there's an active selection */
static bool iui_has_selection(const iui_edit_state *state)
{
    return state->selection_start != state->selection_end;
}

/* Clamp cursor and selection to valid UTF-8 boundaries and buffer length */
static void textfield_clamp_state(const char *buffer,
                                  iui_edit_state *state,
                                  size_t len)
{
    if (!buffer)
        return;
    if (state->cursor > len)
        state->cursor = len;
    while (state->cursor > 0 &&
           iui_utf8_is_continuation((unsigned char) buffer[state->cursor]))
        state->cursor--;
    if (state->selection_start > len)
        state->selection_start = len;
    if (state->selection_end > len)
        state->selection_end = len;
}

/* Handle UTF-8 character insertion at cursor position */
static bool textfield_insert_char(iui_context *ctx,
                                  char *buffer,
                                  size_t buffer_size,
                                  size_t len,
                                  iui_edit_state *state)
{
    if (!buffer || !state)
        return false;
    char utf8_buf[4];
    size_t cp_len = iui_utf8_encode(ctx->char_input, utf8_buf);
    if (len + cp_len < buffer_size && state->cursor + cp_len < buffer_size) {
        memmove(buffer + state->cursor + cp_len, buffer + state->cursor,
                len - state->cursor + 1);
        memcpy(buffer + state->cursor, utf8_buf, cp_len);
        state->cursor += cp_len;
        state->selection_start = state->selection_end = state->cursor;
        return true;
    }
    return false;
}

/* Move cursor left with Ctrl+word skip support */
static void textfield_move_left(const char *buffer,
                                size_t len,
                                iui_edit_state *state,
                                bool ctrl_held)
{
    if (!buffer || !state)
        return;
    if (ctrl_held) {
        size_t prev = state->cursor;
        while (prev > 0) {
            prev = iui_utf8_prev(buffer, prev);
            uint32_t cp = iui_utf8_decode(buffer, prev, len);
            if (cp != ' ' && cp != '\t')
                break;
            state->cursor = prev;
        }
        while (state->cursor > 0) {
            size_t prev = iui_utf8_prev(buffer, state->cursor);
            uint32_t cp = iui_utf8_decode(buffer, prev, len);
            if (!iui_utf8_is_word_char(cp))
                break;
            state->cursor = prev;
        }
    } else {
        state->cursor = iui_utf8_prev(buffer, state->cursor);
    }
}

/* Move cursor right with Ctrl+word skip support */
static void textfield_move_right(const char *buffer,
                                 size_t len,
                                 iui_edit_state *state,
                                 bool ctrl_held)
{
    if (!buffer || !state)
        return;
    if (ctrl_held) {
        while (state->cursor < len) {
            uint32_t cp = iui_utf8_decode(buffer, state->cursor, len);
            if (!iui_utf8_is_word_char(cp))
                break;
            state->cursor = iui_utf8_next(buffer, state->cursor, len);
        }
        while (state->cursor < len) {
            uint32_t cp = iui_utf8_decode(buffer, state->cursor, len);
            if (cp != ' ' && cp != '\t')
                break;
            state->cursor = iui_utf8_next(buffer, state->cursor, len);
        }
    } else {
        state->cursor = iui_utf8_next(buffer, state->cursor, len);
    }
}

/* Handle cursor movement with selection extension */
static void textfield_handle_cursor_movement(const char *buffer,
                                             size_t len,
                                             iui_edit_state *state,
                                             bool shift_held,
                                             bool ctrl_held,
                                             enum iui_key_code key)
{
    if (!buffer || !state)
        return;
    if (key == IUI_KEY_LEFT) {
        if (shift_held) {
            if (state->cursor > 0) {
                if (!iui_has_selection(state))
                    state->selection_start = state->selection_end =
                        state->cursor;
                textfield_move_left(buffer, len, state, ctrl_held);
                if (state->cursor < state->selection_start)
                    state->selection_start = state->cursor;
                else
                    state->selection_end = state->cursor;
            }
        } else {
            if (iui_has_selection(state)) {
                iui_normalize_selection(state);
                state->cursor = state->selection_start;
            } else if (state->cursor > 0) {
                textfield_move_left(buffer, len, state, ctrl_held);
            }
            state->selection_start = state->selection_end = state->cursor;
        }
    } else if (key == IUI_KEY_RIGHT) {
        if (shift_held) {
            if (state->cursor < len) {
                if (!iui_has_selection(state))
                    state->selection_start = state->selection_end =
                        state->cursor;
                textfield_move_right(buffer, len, state, ctrl_held);
                if (state->cursor > state->selection_end)
                    state->selection_end = state->cursor;
                else
                    state->selection_start = state->cursor;
            }
        } else {
            if (iui_has_selection(state)) {
                iui_normalize_selection(state);
                state->cursor = state->selection_end;
            } else if (state->cursor < len) {
                textfield_move_right(buffer, len, state, ctrl_held);
            }
            state->selection_start = state->selection_end = state->cursor;
        }
    } else if (key == IUI_KEY_HOME) {
        if (shift_held) {
            if (!iui_has_selection(state))
                state->selection_end = state->cursor;
            state->cursor = 0;
            state->selection_start = 0;
        } else {
            state->cursor = 0;
            state->selection_start = state->selection_end = 0;
        }
    } else if (key == IUI_KEY_END) {
        if (shift_held) {
            if (!iui_has_selection(state))
                state->selection_start = state->cursor;
            state->cursor = len;
            state->selection_end = len;
        } else {
            state->cursor = len;
            state->selection_start = state->selection_end = len;
        }
    }
}

/* Process text input with selection support (UTF-8 aware) */
static bool iui_process_text_input_selection(iui_context *ctx,
                                             char *buffer,
                                             size_t buffer_size,
                                             iui_edit_state *state)
{
    bool modified = false;
    size_t len = strlen(buffer);

    textfield_clamp_state(buffer, state, len);

    bool shift_held = (ctx->modifiers & IUI_MOD_SHIFT) != 0;
    bool ctrl_held = (ctx->modifiers & IUI_MOD_CTRL) != 0;

    /* Character input - replace selection if active (UTF-8 aware) */
    if (ctx->char_input >= 32) {
        if (iui_has_selection(state)) {
            iui_delete_selection(buffer, state);
            len = strlen(buffer);
            modified = true;
        }

        if (textfield_insert_char(ctx, buffer, buffer_size, len, state))
            modified = true;
    }

    /* Key handling with UTF-8 awareness */
    switch (ctx->key_pressed) {
    case IUI_KEY_LEFT:
    case IUI_KEY_RIGHT:
    case IUI_KEY_HOME:
    case IUI_KEY_END:
        textfield_handle_cursor_movement(buffer, len, state, shift_held,
                                         ctrl_held, ctx->key_pressed);
        break;

    case IUI_KEY_BACKSPACE:
        if (iui_has_selection(state)) {
            modified = iui_delete_selection(buffer, state);
        } else if (state->cursor > 0 && len > 0) {
            /* Delete previous UTF-8 code point */
            size_t prev_pos = iui_utf8_prev(buffer, state->cursor);
            memmove(buffer + prev_pos, buffer + state->cursor,
                    len - state->cursor + 1);
            state->cursor = prev_pos;
            modified = true;
        }
        state->selection_start = state->selection_end = state->cursor;
        break;

    case IUI_KEY_DELETE:
        if (iui_has_selection(state)) {
            modified = iui_delete_selection(buffer, state);
        } else if (state->cursor < len) {
            /* Delete current UTF-8 code point */
            size_t next_pos = iui_utf8_next(buffer, state->cursor, len);
            memmove(buffer + state->cursor, buffer + next_pos,
                    len - next_pos + 1);
            modified = true;
        }
        state->selection_start = state->selection_end = state->cursor;
        break;

    default:
        break;
    }

    return modified;
}

/* Text Edit with Selection Support Implementation */

bool iui_edit_with_selection(iui_context *ctx,
                             char *buffer,
                             size_t buffer_size,
                             iui_edit_state *state)
{
    if (!ctx->current_window || !buffer || !state || buffer_size == 0)
        return false;

    /* Register this text field for per-frame tracking */
    iui_register_textfield(ctx, buffer);

    bool modified = false;
    size_t len = strlen(buffer);

    /* Clamp state values to valid range */
    if (state->cursor > len)
        state->cursor = len;
    if (state->selection_start > len)
        state->selection_start = len;
    if (state->selection_end > len)
        state->selection_end = len;

    /* Track time since last click for multi-click detection */
    state->last_click_time += ctx->delta_time;
    if (state->last_click_time > IUI_DOUBLE_CLICK_TIME)
        state->last_click_count = 0; /* Reset if too much time elapsed */

    iui_rect_t edit_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = ctx->row_height,
    };

    /* Check for focus */
    bool has_focus = (ctx->focused_edit == buffer);
    bool hovered = in_rect(&edit_rect, ctx->mouse_pos);

    /* Calculate text area */
    float text_x = edit_rect.x + ctx->padding,
          text_y = edit_rect.y + (edit_rect.height - ctx->font_height) * 0.5f,
          text_area_width = edit_rect.width - ctx->padding * 2.f;

    /* Calculate total text width for scroll */
    float total_text_width = iui_get_text_width(ctx, buffer),
          max_scroll = fmaxf(0.f, total_text_width - text_area_width);

    /* Clamp scroll offset */
    if (state->scroll_offset > max_scroll)
        state->scroll_offset = max_scroll;
    if (state->scroll_offset < 0.f)
        state->scroll_offset = 0.f;

    /* Handle mouse input for focus and selection */
    if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) && hovered) {
        ctx->focused_edit = buffer;
        has_focus = true;
        ctx->cursor_blink = 0.f;

        /* Calculate click position in text */
        float click_x = ctx->mouse_pos.x + state->scroll_offset;
        size_t click_pos = iui_find_cursor_from_x(ctx, buffer, text_x, click_x);

        /* Double/triple click detection based on click count and position */
        if (state->last_click_count >= 2 &&
            fabsf((float) click_pos - (float) state->last_click_pos) < 3) {
            /* Triple click: select all */
            state->selection_start = 0;
            state->selection_end = len;
            state->cursor = len;
            state->last_click_count = 0;
        } else if (state->last_click_count == 1 &&
                   fabsf((float) click_pos - (float) state->last_click_pos) <
                       3) {
            /* Double click: select word */
            iui_find_word_boundaries(buffer, click_pos, &state->selection_start,
                                     &state->selection_end);
            state->cursor = state->selection_end;
            state->last_click_count = 2;
        } else {
            /* Single click: position cursor */
            state->cursor = click_pos;
            state->selection_start = click_pos;
            state->selection_end = click_pos;
            state->last_click_count = 1;
            state->last_click_pos = click_pos;
            state->is_dragging = true;
        }
        /* Reset timing for next click detection */
        state->last_click_time = 0.f;
    } else if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) && !hovered && has_focus) {
        /* Click outside: lose focus */
        ctx->focused_edit = NULL;
        has_focus = false;
        state->is_dragging = false;
    }

    /* Handle drag selection */
    if (state->is_dragging && (ctx->mouse_held & IUI_MOUSE_LEFT)) {
        float drag_x = ctx->mouse_pos.x + state->scroll_offset;
        size_t drag_pos = iui_find_cursor_from_x(ctx, buffer, text_x, drag_x);
        state->cursor = drag_pos;

        /* Update selection range */
        if (drag_pos < state->last_click_pos) {
            state->selection_start = drag_pos;
            state->selection_end = state->last_click_pos;
        } else {
            state->selection_start = state->last_click_pos;
            state->selection_end = drag_pos;
        }
    }

    /* End drag on mouse release */
    if (ctx->mouse_released & IUI_MOUSE_LEFT) {
        state->is_dragging = false;
    }

    /* Handle keyboard input when focused and auto-scroll to keep cursor visible
     */
    if (has_focus) {
        modified =
            iui_process_text_input_selection(ctx, buffer, buffer_size, state);
        /* Reset cursor blink on any key activity */
        if (ctx->key_pressed != IUI_KEY_NONE || ctx->char_input != 0)
            ctx->cursor_blink = 0.f;

        /* Auto-scroll to keep cursor visible */
        float cursor_x_in_text =
            textfield_get_width_to_pos(ctx, buffer, state->cursor, false);
        if (cursor_x_in_text < state->scroll_offset)
            state->scroll_offset = cursor_x_in_text;
        if (cursor_x_in_text > state->scroll_offset + text_area_width)
            state->scroll_offset = cursor_x_in_text - text_area_width;
    }

    /* Drawing - MD3 TextField: Filled variant */
    textfield_colors_t colors =
        textfield_calc_colors(ctx, has_focus, hovered, false);
    textfield_draw_background(ctx, edit_rect, IUI_TEXTFIELD_FILLED, &colors);

    /* Set clip region for text area */
    iui_rect_t text_clip = {
        .x = edit_rect.x + ctx->padding,
        .y = edit_rect.y,
        .width = text_area_width,
        .height = edit_rect.height,
    };

    /* Draw selection highlight (MD3: primary @ 40% focused, 20% unfocused) */
    float draw_text_x = text_x - state->scroll_offset;
    iui_normalize_selection(state);
    if (state->selection_start != state->selection_end) {
        float sel_start_x =
            draw_text_x + textfield_get_width_to_pos(
                              ctx, buffer, state->selection_start, false);
        float sel_end_x =
            draw_text_x + textfield_get_width_to_pos(
                              ctx, buffer, state->selection_end, false);
        float visible_start = fmaxf(sel_start_x, text_clip.x);
        float visible_end = fminf(sel_end_x, text_clip.x + text_clip.width);

        if (visible_end > visible_start) {
            uint8_t sel_alpha =
                has_focus ? IUI_SELECTION_ALPHA : (IUI_SELECTION_ALPHA / 2);
            uint32_t selection_color =
                iui_state_layer(ctx->colors.primary, sel_alpha);
            ctx->renderer.draw_box(
                (iui_rect_t) {visible_start, text_y,
                              visible_end - visible_start, ctx->font_height},
                0.f, selection_color, ctx->renderer.user);
        }
    }

    /* Draw text (with scroll offset) */
    iui_internal_draw_text(ctx, draw_text_x, text_y, buffer,
                           ctx->colors.on_surface);

    /* Draw cursor when focused */
    if (has_focus && ctx->cursor_blink < 0.5f) {
        float cursor_x = draw_text_x + textfield_get_width_to_pos(
                                           ctx, buffer, state->cursor, false);
        if (cursor_x >= text_clip.x &&
            cursor_x <= text_clip.x + text_clip.width) {
            ctx->renderer.draw_box(
                (iui_rect_t) {cursor_x, text_y, IUI_TEXTFIELD_CURSOR_WIDTH,
                              ctx->font_height},
                0.f, ctx->colors.primary, ctx->renderer.user);
        }
    }

    iui_newline(ctx);

    /* Only clear input if this field consumed it */
    if (has_focus) {
        ctx->key_pressed = 0;
        ctx->char_input = 0;
    }

    return modified;
}

/* Advanced TextField with Selection Support */

iui_textfield_result iui_textfield_with_selection(
    iui_context *ctx,
    char *buffer,
    size_t size,
    iui_edit_state *state,
    const iui_textfield_options *options)
{
    iui_textfield_result result = {0};

    if (!ctx->current_window || !buffer || !state || size == 0)
        return result;

    /* Register this text field for per-frame tracking */
    iui_register_textfield(ctx, buffer);

    /* Default options if NULL */
    iui_textfield_options opts = {0};
    if (options)
        opts = *options;

    size_t len = strlen(buffer);

    /* Clamp state values */
    if (state->cursor > len)
        state->cursor = len;
    if (state->selection_start > len)
        state->selection_start = len;
    if (state->selection_end > len)
        state->selection_end = len;

    /* Track time since last click for multi-click detection */
    state->last_click_time += ctx->delta_time;
    if (state->last_click_time > IUI_DOUBLE_CLICK_TIME)
        state->last_click_count = 0; /* Reset if too much time elapsed */

    /* Calculate icon dimensions */
    float icon_size = ctx->font_height, icon_padding = ctx->padding,
          leading_icon_width = (opts.leading_icon != IUI_TEXTFIELD_ICON_NONE)
                                   ? (icon_size + icon_padding)
                                   : 0.f,
          trailing_icon_width = (opts.trailing_icon != IUI_TEXTFIELD_ICON_NONE)
                                    ? (icon_size + icon_padding)
                                    : 0.f;

    /* MD3 spec: TextField height = 56dp */
    iui_rect_t edit_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = IUI_TEXTFIELD_HEIGHT,
    };

    /* Icon hit areas */
    iui_rect_t leading_icon_rect = {
        .x = edit_rect.x + icon_padding * 0.5f,
        .y = edit_rect.y + (edit_rect.height - icon_size) * 0.5f,
        .width = icon_size,
        .height = icon_size,
    };

    iui_rect_t trailing_icon_rect = {
        .x = edit_rect.x + edit_rect.width - icon_size - icon_padding * 0.5f,
        .y = edit_rect.y + (edit_rect.height - icon_size) * 0.5f,
        .width = icon_size,
        .height = icon_size,
    };

    /* Text area start position (between icons) */
    float text_x_start = edit_rect.x + ctx->padding + leading_icon_width;
    float text_area_width = edit_rect.width - ctx->padding * 2.f -
                            leading_icon_width - trailing_icon_width;

    /* Calculate total text width for scroll */
    float total_text_width;
    if (opts.password_mode && buffer[0] != '\0') {
        char masked[IUI_STRING_BUFFER_SIZE];
        size_t mask_len = len < sizeof(masked) - 1 ? len : sizeof(masked) - 1;
        for (size_t i = 0; i < mask_len; i++)
            masked[i] = '*';
        masked[mask_len] = '\0';
        total_text_width = iui_get_text_width(ctx, masked);
    } else {
        total_text_width = iui_get_text_width(ctx, buffer);
    }
    float max_scroll = fmaxf(0.f, total_text_width - text_area_width);

    /* Clamp scroll offset */
    if (state->scroll_offset > max_scroll)
        state->scroll_offset = max_scroll;
    if (state->scroll_offset < 0.f)
        state->scroll_offset = 0.f;

    /* Check icon clicks first (only if not disabled) */
    if (!opts.disabled && (ctx->mouse_pressed & IUI_MOUSE_LEFT)) {
        if (opts.leading_icon != IUI_TEXTFIELD_ICON_NONE &&
            in_rect(&leading_icon_rect, ctx->mouse_pos)) {
            result.leading_icon_clicked = true;
        }
        if (opts.trailing_icon != IUI_TEXTFIELD_ICON_NONE &&
            in_rect(&trailing_icon_rect, ctx->mouse_pos)) {
            result.trailing_icon_clicked = true;
        }
    }

    /* Focus handling (skip if disabled or read-only for editing) */
    bool has_focus = (ctx->focused_edit == buffer);
    bool hovered = in_rect(&edit_rect, ctx->mouse_pos);

    if (!opts.disabled) {
        if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) && hovered &&
            !result.leading_icon_clicked && !result.trailing_icon_clicked) {
            ctx->focused_edit = buffer;
            has_focus = true;
            ctx->cursor_blink = 0.f;

            /* Click-to-position with selection support */
            float click_x = ctx->mouse_pos.x + state->scroll_offset;
            size_t click_pos =
                iui_find_cursor_from_x(ctx, buffer, text_x_start, click_x);

            /* Double/triple click detection */
            if (state->last_click_count >= 2 &&
                fabsf((float) click_pos - (float) state->last_click_pos) < 3) {
                /* Triple click: select all */
                state->selection_start = 0;
                state->selection_end = len;
                state->cursor = len;
                state->last_click_count = 0;
            } else if (state->last_click_count == 1 &&
                       fabsf((float) click_pos -
                             (float) state->last_click_pos) < 3) {
                /* Double click: select word */
                iui_find_word_boundaries(buffer, click_pos,
                                         &state->selection_start,
                                         &state->selection_end);
                state->cursor = state->selection_end;
                state->last_click_count = 2;
            } else {
                /* Single click */
                state->cursor = click_pos;
                state->selection_start = click_pos;
                state->selection_end = click_pos;
                state->last_click_count = 1;
                state->last_click_pos = click_pos;
                state->is_dragging =
                    true; /* Allow drag selection in read-only */
            }
            /* Reset timing for next click detection */
            state->last_click_time = 0.f;
        } else if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) && !hovered &&
                   has_focus) {
            ctx->focused_edit = NULL;
            has_focus = false;
            state->is_dragging = false;
        }
    } else if (has_focus) {
        /* Widget is disabled but still has focus - clear it */
        ctx->focused_edit = NULL;
        has_focus = false;
    }

    /* Handle drag selection (allow in read-only mode) */
    if (state->is_dragging && (ctx->mouse_held & IUI_MOUSE_LEFT)) {
        float drag_x = ctx->mouse_pos.x + state->scroll_offset;
        size_t drag_pos =
            iui_find_cursor_from_x(ctx, buffer, text_x_start, drag_x);
        state->cursor = drag_pos;

        if (drag_pos < state->last_click_pos) {
            state->selection_start = drag_pos;
            state->selection_end = state->last_click_pos;
        } else {
            state->selection_start = state->last_click_pos;
            state->selection_end = drag_pos;
        }
    }

    if (ctx->mouse_released & IUI_MOUSE_LEFT) {
        state->is_dragging = false;
    }

    /* Handle keyboard input when focused (skip if read_only or disabled) */
    if (has_focus && !opts.read_only && !opts.disabled) {
        /* Handle Enter key (submit) first */
        if (ctx->key_pressed == IUI_KEY_ENTER)
            result.submitted = true;

        result.value_changed =
            iui_process_text_input_selection(ctx, buffer, size, state);

        if (ctx->key_pressed != IUI_KEY_NONE || ctx->char_input != 0)
            ctx->cursor_blink = 0.f;
    }

    /* Auto-scroll to keep cursor visible */
    if (has_focus) {
        float cursor_x_in_text = textfield_get_width_to_pos(
            ctx, buffer, state->cursor, opts.password_mode);
        if (cursor_x_in_text < state->scroll_offset)
            state->scroll_offset = cursor_x_in_text;
        if (cursor_x_in_text > state->scroll_offset + text_area_width)
            state->scroll_offset = cursor_x_in_text - text_area_width;
    }

    /* Calculate colors and draw background */
    textfield_colors_t colors =
        textfield_calc_colors(ctx, has_focus, hovered, opts.disabled);
    textfield_draw_background(ctx, edit_rect, opts.style, &colors);
    textfield_draw_icons(ctx, &opts, leading_icon_rect, trailing_icon_rect,
                         icon_size, colors.icon);

    /* Text area positioning */
    float text_y = edit_rect.y + (edit_rect.height - ctx->font_height) * 0.5f;
    float draw_text_x = text_x_start - state->scroll_offset;

    /* Draw selection highlight (MD3: primary @ 40% focused, 20% unfocused) */
    iui_normalize_selection(state);
    if (state->selection_start != state->selection_end) {
        float sel_start_x =
            draw_text_x + textfield_get_width_to_pos(ctx, buffer,
                                                     state->selection_start,
                                                     opts.password_mode);
        float sel_end_x = draw_text_x + textfield_get_width_to_pos(
                                            ctx, buffer, state->selection_end,
                                            opts.password_mode);
        float visible_start = fmaxf(sel_start_x, text_x_start);
        float visible_end = fminf(sel_end_x, text_x_start + text_area_width);

        if (visible_end > visible_start) {
            uint8_t sel_alpha =
                has_focus ? IUI_SELECTION_ALPHA : (IUI_SELECTION_ALPHA / 2);
            uint32_t selection_color =
                iui_state_layer(ctx->colors.primary, sel_alpha);
            ctx->renderer.draw_box(
                (iui_rect_t) {visible_start, text_y,
                              visible_end - visible_start, ctx->font_height},
                0.f, selection_color, ctx->renderer.user);
        }
    }

    /* Draw text or placeholder */
    if (buffer[0] == '\0' && opts.placeholder) {
        iui_internal_draw_text(
            ctx, text_x_start, text_y, opts.placeholder,
            iui_state_layer(ctx->colors.on_surface, IUI_PLACEHOLDER_ALPHA));
    } else if (opts.password_mode && buffer[0] != '\0') {
        char masked[IUI_STRING_BUFFER_SIZE];
        size_t mask_len = len < sizeof(masked) - 1 ? len : sizeof(masked) - 1;
        for (size_t i = 0; i < mask_len; i++)
            masked[i] = '*';
        masked[mask_len] = '\0';
        iui_internal_draw_text(ctx, draw_text_x, text_y, masked, colors.text);
    } else {
        iui_internal_draw_text(ctx, draw_text_x, text_y, buffer, colors.text);
    }

    /* Draw cursor when focused */
    if (has_focus && !opts.disabled && ctx->cursor_blink < 0.5f) {
        float cursor_x =
            draw_text_x + textfield_get_width_to_pos(ctx, buffer, state->cursor,
                                                     opts.password_mode);
        if (cursor_x >= text_x_start &&
            cursor_x <= text_x_start + text_area_width) {
            ctx->renderer.draw_box(
                (iui_rect_t) {cursor_x, text_y, IUI_TEXTFIELD_CURSOR_WIDTH,
                              ctx->font_height},
                0.f, ctx->colors.primary, ctx->renderer.user);
        }
    }

    iui_newline(ctx);

    if (has_focus && !opts.read_only && !opts.disabled) {
        ctx->key_pressed = 0;
        ctx->char_input = 0;
    }

    /* MD3 runtime validation: track rendered textfield dimensions */
    IUI_MD3_TRACK_TEXTFIELD(edit_rect, ctx->corner);

    return result;
}

/* Switch Widget */

bool iui_switch(iui_context *ctx,
                const char *label,
                bool *value,
                const char *on_icon,  /* optional: "check" glyph */
                const char *off_icon) /* optional: "x" glyph */
{
    if (!ctx->current_window || !label)
        return false;

    bool toggled = false;

    /* Calculate switch dimensions using MD3 proportions scaled to row
     * MD3 spec: 52x32dp (ratio 1.625:1), thumb 24dp (ratio 0.75)
     * Scale to fit within row_height for responsive layouts
     */
    float switch_track_height = fminf(IUI_SWITCH_TRACK_HEIGHT, ctx->row_height);
    float switch_width = switch_track_height *
                         (IUI_SWITCH_TRACK_WIDTH / IUI_SWITCH_TRACK_HEIGHT);
    float switch_height = switch_track_height;

    /* Position the switch track (centered vertically within row) */
    iui_rect_t track_rect = {
        .x = ctx->layout.x + ctx->layout.width - switch_width - ctx->padding,
        .y = ctx->layout.y + (ctx->row_height - switch_height) * 0.5f,
        .width = switch_width,
        .height = switch_height,
    };

    /* Register as focusable widget for keyboard navigation.
     * Combine label hash with position to avoid ID collision. */
    float corner = switch_track_height * 0.5f; /* Full round corners */
    uint32_t widget_id = iui_widget_id(label, track_rect);
    iui_register_focusable(ctx, widget_id, track_rect, corner);
    bool is_focused = iui_widget_is_focused(ctx, widget_id);

    /* Expand touch target for accessibility (48dp minimum per MD3) */
    iui_rect_t touch_rect = track_rect;
    iui_expand_touch_target_h(&touch_rect, IUI_SWITCH_TOUCH_TARGET);

    iui_state_t state = iui_get_component_state(ctx, touch_rect, false);

    /* Calculate thumb position based on state (off = left, on = right)
     * MD3: thumb is 24dp within 32dp height track (ratio 0.75)
     */
    float thumb_size =
        switch_track_height * (IUI_SWITCH_THUMB_SIZE / IUI_SWITCH_TRACK_HEIGHT);
    float thumb_margin = (switch_height - thumb_size) * 0.5f;
    float thumb_x_off = track_rect.x + thumb_margin;
    float thumb_x_on =
        track_rect.x + track_rect.width - thumb_size - thumb_margin;

    /* Toggle on click or keyboard activation */
    bool should_toggle = (state == IUI_STATE_PRESSED);
    if (is_focused && (ctx->key_pressed == IUI_KEY_ENTER)) {
        should_toggle = true;
        ctx->key_pressed = IUI_KEY_NONE; /* Consume key */
    }

    if (should_toggle) {
        *value = !(*value);
        toggled = true;
        /* Animation setup for smooth transition */
        ctx->animation = (iui_animation) {
            .value_key0 = *value ? thumb_x_on : thumb_x_off,
            .value_key1 = *value ? thumb_x_off : thumb_x_on,
            .color_key0 =
                *value ? ctx->colors.primary : ctx->colors.surface_container,
            .color_key1 =
                *value ? ctx->colors.surface_container : ctx->colors.primary,
            .key0_to_key1 = false,
            .widget = value,
        };
    }

    /* Calculate current thumb position with animation */
    float thumb_x = *value ? thumb_x_on : thumb_x_off;
    uint32_t track_color =
        *value ? ctx->colors.primary : ctx->colors.surface_container;

    if (ctx->animation.widget == value) {
        float t = ctx->animation.t;
        if (t < 1.f) {
            /* Animate thumb position from off to on or on to off */
            thumb_x =
                lerp_float(thumb_x_off, thumb_x_on, *value ? t : (1.f - t));
            /* Animate track color */
            uint32_t start_color =
                !(*value) ? ctx->colors.primary : ctx->colors.surface_container;
            uint32_t end_color =
                *value ? ctx->colors.primary : ctx->colors.surface_container;
            track_color =
                lerp_color(start_color, end_color, *value ? t : (1.f - t));
        } else {
            /* Animation complete, reset */
            ctx->animation.widget = NULL;
        }
    }

    /* Apply focus state layer and draw focus ring when focused */
    if (is_focused) {
        uint32_t focus_layer =
            iui_state_layer(ctx->colors.primary, IUI_STATE_FOCUS_ALPHA);
        track_color = iui_blend_color(track_color, focus_layer);
        iui_draw_focus_ring(ctx, track_rect, corner);
    }

    /* Draw track (MD3: filled track when on, surface_variant when off) */
    ctx->renderer.draw_box((iui_rect_t) {track_rect.x, track_rect.y,
                                         track_rect.width, switch_track_height},
                           corner, track_color, ctx->renderer.user);

    /* Draw thumb (filled circle) */
    float thumb_y = track_rect.y + thumb_margin;
    uint32_t thumb_color =
        *value ? ctx->colors.on_primary : ctx->colors.outline;
    ctx->renderer.draw_box(
        (iui_rect_t) {thumb_x, thumb_y, thumb_size, thumb_size},
        thumb_size * 0.5f, thumb_color, ctx->renderer.user);

    /* Optionally draw icons inside thumb (scale to fit within thumb) */
    if ((*value && on_icon) || (!(*value) && off_icon)) {
        const char *icon_text = *value ? on_icon : off_icon;
        float icon_width = iui_get_text_width(ctx, icon_text);
        /* Center icon within thumb; use smaller thumb_size and font_height
         * for vertical centering
         */
        float icon_h = fminf(thumb_size, ctx->font_height);
        float icon_x = thumb_x + (thumb_size - icon_width) * 0.5f;
        float icon_y = thumb_y + (thumb_size - icon_h) * 0.5f;
        uint32_t icon_color =
            *value ? ctx->colors.primary : ctx->colors.on_surface;
        iui_internal_draw_text(ctx, icon_x, icon_y, icon_text, icon_color);
    }

    /* Draw label to the left (centered vertically within row) */
    float text_y = ctx->layout.y + (ctx->row_height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, ctx->layout.x, text_y, label,
                           ctx->colors.on_surface);

    /* MD3 runtime validation: track touch target (not visual bounds) */
    IUI_MD3_TRACK_SWITCH(touch_rect, corner);

    iui_newline(ctx);
    return toggled;
}


/* Checkbox and Radio Widgets */

bool iui_checkbox(iui_context *ctx, const char *label, bool *checked)
{
    if (!ctx->current_window || !label)
        return false;

    /* Modal blocking is handled centrally by iui_get_component_state() which
     * returns IUI_STATE_DEFAULT when modal is active and rendering=false
     */

    bool toggled = false;

    float box_size = ctx->font_height;
    float corner = box_size * 0.15f;
    iui_rect_t box_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y + (ctx->row_height - box_size) * 0.5f,
        .width = box_size,
        .height = box_size,
    };

    iui_rect_t hit_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = ctx->row_height,
    };

    /* Register as focusable widget for keyboard navigation.
     * Combine label hash with position to avoid ID collision.
     */
    uint32_t widget_id = iui_widget_id(label, box_rect);
    iui_register_focusable(ctx, widget_id, box_rect, corner);
    bool is_focused = iui_widget_is_focused(ctx, widget_id);

    /* Toggle on click or keyboard activation */
    iui_state_t state = iui_get_component_state(ctx, hit_rect, false);
    bool should_toggle = (state == IUI_STATE_PRESSED);
    if (is_focused && (ctx->key_pressed == IUI_KEY_ENTER)) {
        should_toggle = true;
        ctx->key_pressed = IUI_KEY_NONE;
    }
    if (should_toggle) {
        *checked = !(*checked);
        toggled = true;
    }

    /* Draw focus ring when focused */
    if (is_focused)
        iui_draw_focus_ring(ctx, box_rect, corner);

    if (*checked) {
        /* Checked: filled box with primary, inner mark with on_primary */
        uint32_t bg_color = ctx->colors.primary;
        if (is_focused) {
            uint32_t focus_layer =
                iui_state_layer(ctx->colors.on_primary, IUI_STATE_FOCUS_ALPHA);
            bg_color = iui_blend_color(bg_color, focus_layer);
        }
        ctx->renderer.draw_box(box_rect, corner, bg_color, ctx->renderer.user);
        /* Checkmark (simplified as smaller inner square) */
        float mark_margin = box_size * 0.25f;
        ctx->renderer.draw_box(
            (iui_rect_t) {box_rect.x + mark_margin, box_rect.y + mark_margin,
                          box_size - mark_margin * 2,
                          box_size - mark_margin * 2},
            corner * 0.5f, ctx->colors.on_primary, ctx->renderer.user);
    } else {
        /* Unchecked: surface_variant background */
        uint32_t bg_color = ctx->colors.surface_container;
        if (is_focused) {
            uint32_t focus_layer =
                iui_state_layer(ctx->colors.primary, IUI_STATE_FOCUS_ALPHA);
            bg_color = iui_blend_color(bg_color, focus_layer);
        }
        ctx->renderer.draw_box(box_rect, corner, bg_color, ctx->renderer.user);
    }

    /* Draw label */
    float text_x = box_rect.x + box_size + ctx->padding;
    float text_y = ctx->layout.y + (ctx->row_height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, label, ctx->colors.on_surface);

    iui_newline(ctx);
    return toggled;
}

bool iui_radio(iui_context *ctx,
               const char *label,
               int *group_value,
               int button_value)
{
    if (!ctx->current_window || !label)
        return false;
    bool selected = false;

    float circle_size = ctx->font_height;
    float corner = circle_size * 0.5f; /* Full circle */
    iui_rect_t circle_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y + (ctx->row_height - circle_size) * 0.5f,
        .width = circle_size,
        .height = circle_size,
    };

    iui_rect_t hit_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = ctx->row_height,
    };

    /* Register as focusable widget for keyboard navigation.
     * Combine label hash with position to avoid ID collision. */
    uint32_t widget_id = iui_widget_id(label, circle_rect);
    iui_register_focusable(ctx, widget_id, circle_rect, corner);
    bool is_focused = iui_widget_is_focused(ctx, widget_id);

    /* Select on click or keyboard activation */
    iui_state_t state = iui_get_component_state(ctx, hit_rect, false);
    bool should_select = (state == IUI_STATE_PRESSED);
    if (is_focused && (ctx->key_pressed == IUI_KEY_ENTER)) {
        should_select = true;
        ctx->key_pressed = IUI_KEY_NONE;
    }
    if (should_select) {
        *group_value = button_value;
        selected = true;
    }

    bool is_selected = (*group_value == button_value);

    /* Draw focus ring when focused */
    if (is_focused)
        iui_draw_focus_ring(ctx, circle_rect, corner);

    /* Outer circle background with focus state layer */
    uint32_t bg_color =
        is_selected ? ctx->colors.primary : ctx->colors.surface_container;
    if (is_focused) {
        uint32_t focus_layer = iui_state_layer(
            is_selected ? ctx->colors.on_primary : ctx->colors.primary,
            IUI_STATE_FOCUS_ALPHA);
        bg_color = iui_blend_color(bg_color, focus_layer);
    }
    ctx->renderer.draw_box(circle_rect, corner, bg_color, ctx->renderer.user);

    /* Draw inner dot if selected */
    if (is_selected) {
        float dot_size = circle_size * 0.5f;
        float dot_margin = (circle_size - dot_size) * 0.5f;
        ctx->renderer.draw_box(
            (iui_rect_t) {circle_rect.x + dot_margin,
                          circle_rect.y + dot_margin, dot_size, dot_size},
            dot_size * 0.5f, ctx->colors.on_primary, ctx->renderer.user);
    }

    /* Draw label */
    float text_x = circle_rect.x + circle_size + ctx->padding;
    float text_y = ctx->layout.y + (ctx->row_height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, label, ctx->colors.on_surface);

    iui_newline(ctx);
    return selected;
}

/* Exposed Dropdown Menu Implementation
 * Reference: https://m3.material.io/components/menus
 * TextField-style component that reveals a menu of options
 */



bool iui_dropdown(iui_context *ctx, const iui_dropdown_options *options)
{
    if (!ctx || !ctx->current_window || !options || !options->options ||
        options->option_count <= 0 || !options->selected_index)
        return false;

    bool selection_changed = false;
    int selected = *options->selected_index;

    /* Clamp selected index to valid range */
    if (selected < 0)
        selected = 0;
    if (selected >= options->option_count)
        selected = options->option_count - 1;

    /* Calculate dropdown field rect */
    float field_h = IUI_DROPDOWN_HEIGHT;
    float corner = IUI_DROPDOWN_CORNER_RADIUS;
    float padding = ctx->spacing.md;      /* 16dp standard padding */
    float icon_size = IUI_MENU_ICON_SIZE; /* 24dp standard icon size */

    iui_rect_t field_rect = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = field_h,
    };

    /* Check if this dropdown's menu is open */
    bool is_open = ctx->dropdown.open &&
                   (ctx->dropdown.selected == options->selected_index);

    /* Get field interaction state */
    iui_state_t state =
        iui_get_component_state(ctx, field_rect, options->disabled);

    /* Draw field background */
    uint32_t bg_color =
        options->disabled ? iui_state_layer(ctx->colors.surface_container_high,
                                            IUI_STATE_DISABLE_ALPHA)
                          : ctx->colors.surface_container_highest;
    ctx->renderer.draw_box(field_rect, corner, bg_color, ctx->renderer.user);

    /* Draw underline indicator for filled style */
    if (!is_open) {
        uint32_t line_color = options->disabled
                                  ? iui_state_layer(ctx->colors.on_surface,
                                                    IUI_STATE_DISABLE_ALPHA)
                                  : ctx->colors.on_surface_variant;
        iui_rect_t underline = {field_rect.x, field_rect.y + field_h - 1.f,
                                field_rect.width, 1.f};
        ctx->renderer.draw_box(underline, 0.f, line_color, ctx->renderer.user);
    } else {
        /* Draw active indicator (2px primary underline) */
        iui_rect_t underline = {field_rect.x, field_rect.y + field_h - 2.f,
                                field_rect.width, 2.f};
        ctx->renderer.draw_box(underline, 0.f, ctx->colors.primary,
                               ctx->renderer.user);
    }

    /* Draw state layer */
    if (!options->disabled && iui_state_is_interactive(state)) {
        uint32_t layer_color =
            iui_state_layer(ctx->colors.on_surface, iui_state_get_alpha(state));
        ctx->renderer.draw_box(field_rect, corner, layer_color,
                               ctx->renderer.user);
    }

    /* Draw floating label */
    if (options->label) {
        uint32_t label_color =
            options->disabled ? iui_state_layer(ctx->colors.on_surface_variant,
                                                IUI_STATE_DISABLE_ALPHA)
                              : (is_open ? ctx->colors.primary
                                         : ctx->colors.on_surface_variant);
        /* Label floats above when there's a selection */
        float label_y = field_rect.y + 8.f;
        iui_internal_draw_text(ctx, field_rect.x + padding, label_y,
                               options->label, label_color);
    }

    /* Draw selected value text */
    uint32_t text_color =
        options->disabled
            ? iui_state_layer(ctx->colors.on_surface, IUI_STATE_DISABLE_ALPHA)
            : ctx->colors.on_surface;
    float text_y = field_rect.y + field_h - padding - ctx->font_height;
    if (selected >= 0 && selected < options->option_count) {
        iui_internal_draw_text(ctx, field_rect.x + padding, text_y,
                               options->options[selected], text_color);
    }

    /* Draw dropdown arrow icon */
    float arrow_x =
        field_rect.x + field_rect.width - padding - icon_size * 0.5f;
    float arrow_cy = field_rect.y + field_h * 0.5f;
    const char *arrow_icon = is_open ? "arrow_up" : "arrow_down";
    uint32_t icon_color = options->disabled
                              ? iui_state_layer(ctx->colors.on_surface_variant,
                                                IUI_STATE_DISABLE_ALPHA)
                              : ctx->colors.on_surface_variant;
    iui_draw_fab_icon(ctx, arrow_x, arrow_cy, icon_size, arrow_icon,
                      icon_color);

    /* Handle field click to toggle menu */
    if (!options->disabled && state == IUI_STATE_PRESSED) {
        if (is_open) {
            /* Close menu */
            ctx->dropdown.open = false;
        } else {
            /* Open menu */
            ctx->dropdown.open = true;
            ctx->dropdown.x = field_rect.x;
            ctx->dropdown.y = field_rect.y + field_h;
            ctx->dropdown.width = field_rect.width;
            ctx->dropdown.hovered_index = selected;
            ctx->dropdown.frames_since_open = 0;
            ctx->dropdown.selected = options->selected_index;
        }
    }

    /* Draw menu if open */
    if (is_open) {
        /* Capture frame count BEFORE incrementing for click protection.
         * This ensures that first frame after opening (frames_active == 0)
         * blocks clicks, preventing click-through from the field that opened
         * menu.
         */
        int frames_active = ctx->dropdown.frames_since_open++;

        /* Calculate menu height (capped at max) */
        float item_h = IUI_DROPDOWN_ITEM_HEIGHT;
        float menu_h = options->option_count * item_h;
        if (menu_h > IUI_DROPDOWN_MENU_MAX_HEIGHT)
            menu_h = IUI_DROPDOWN_MENU_MAX_HEIGHT;

        iui_rect_t menu_rect = {ctx->dropdown.x, ctx->dropdown.y,
                                ctx->dropdown.width, menu_h};

        /* Begin modal for menu */
        iui_begin_modal(ctx, "dropdown_menu_modal");
        iui_register_blocking_region(ctx, menu_rect);

        /* Draw menu shadow and background */
        iui_draw_shadow(ctx, menu_rect, corner, IUI_ELEVATION_3);
        ctx->renderer.draw_box(menu_rect, corner, ctx->colors.surface_container,
                               ctx->renderer.user);

        /* Draw menu items */
        for (int i = 0; i < options->option_count; i++) {
            float item_y = menu_rect.y + i * item_h;
            iui_rect_t item_rect = {menu_rect.x, item_y, menu_rect.width,
                                    item_h};

            iui_state_t item_state =
                iui_get_component_state(ctx, item_rect, false);

            /* Update hovered index */
            if (item_state == IUI_STATE_HOVERED ||
                item_state == IUI_STATE_PRESSED) {
                ctx->dropdown.hovered_index = i;
            }

            /* Draw selection/hover background */
            if (i == selected) {
                /* Selected item has subtle background */
                ctx->renderer.draw_box(item_rect, 0.f,
                                       ctx->colors.secondary_container,
                                       ctx->renderer.user);
            } else if (iui_state_is_interactive(item_state)) {
                uint32_t layer_color = iui_state_layer(
                    ctx->colors.on_surface, iui_state_get_alpha(item_state));
                ctx->renderer.draw_box(item_rect, 0.f, layer_color,
                                       ctx->renderer.user);
            }

            /* Draw item text */
            float item_text_y = item_y + (item_h - ctx->font_height) * 0.5f;
            uint32_t item_text_color = (i == selected)
                                           ? ctx->colors.on_secondary_container
                                           : ctx->colors.on_surface;
            iui_internal_draw_text(ctx, menu_rect.x + padding, item_text_y,
                                   options->options[i], item_text_color);

            /* Handle item click */
            if (frames_active >= 1 && item_state == IUI_STATE_PRESSED) {
                *options->selected_index = i;
                selection_changed = (i != selected);
                ctx->dropdown.open = false;
                iui_close_modal(ctx);
            }
        }

        /* Check for click outside menu to close */
        if (frames_active >= 1) {
            bool mouse_in_menu = in_rect(&menu_rect, ctx->mouse_pos);
            bool mouse_in_field = in_rect(&field_rect, ctx->mouse_pos);
            bool mouse_pressed = (ctx->mouse_pressed & IUI_MOUSE_LEFT);

            if (!mouse_in_menu && !mouse_in_field && mouse_pressed) {
                ctx->dropdown.open = false;
                iui_close_modal(ctx);
            }
        }

        iui_end_modal(ctx);
    }

    /* Draw helper text if provided */
    if (options->helper_text) {
        uint32_t helper_color =
            options->disabled ? iui_state_layer(ctx->colors.on_surface_variant,
                                                IUI_STATE_DISABLE_ALPHA)
                              : ctx->colors.on_surface_variant;
        float helper_y = field_rect.y + field_h + 4.f;
        iui_internal_draw_text(ctx, field_rect.x + padding, helper_y,
                               options->helper_text, helper_color);
    }

    /* Advance layout */
    float total_h = field_h;
    if (options->helper_text)
        total_h += ctx->font_height + 4.f;
    ctx->layout.y += total_h + ctx->padding;

    return selection_changed;
}
