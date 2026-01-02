/* Dialog component implementation
 * Dimension constants defined in iui-spec.h
 */

#include "internal.h"

/* Count number of buttons in semicolon-separated string.
 * @buttons: Semicolon-separated button labels string
 *
 * Returns number of buttons (minimum 0)
 */
static int iui_dialog_count_buttons(const char *buttons)
{
    if (!buttons || !buttons[0])
        return 0;

    int count = 1;
    for (const char *p = buttons; *p; p++) {
        if (*p == ';')
            count++;
    }
    return count;
}

/* Helper: get nth button label from semicolon-separated string.
 * @buttons: Semicolon-separated button labels string
 * @index:   Zero-based index of desired button
 * @out_len: Output parameter for label length
 *
 * Returns pointer within the original string to the requested button label
 */
static const char *iui_dialog_get_button(const char *buttons,
                                         int index,
                                         int *out_len)
{
    if (!buttons || index < 0) {
        *out_len = 0;
        return NULL;
    }

    const char *start = buttons;
    int current = 0;

    while (*start) {
        /* Find end of current label (semicolon or end of string) */
        const char *end = start;
        while (*end && *end != ';')
            end++;

        if (current == index) {
            *out_len = (int) (end - start);
            return start;
        }

        /* Move to next label */
        if (*end != ';')
            break;
        start = end + 1;

        current++;
    }

    *out_len = 0;
    return NULL;
}

void iui_dialog_show(iui_dialog_state *dialog,
                     const char *title,
                     const char *message,
                     const char *buttons)
{
    if (!dialog)
        return;

    dialog->title = title;
    dialog->message = message;
    dialog->buttons = buttons;
    dialog->selected_button = -1;
    dialog->is_open = true;
    dialog->frames_since_open = 0;
    dialog->button_count = iui_dialog_count_buttons(buttons);
}

void iui_dialog_close(iui_dialog_state *dialog)
{
    if (!dialog)
        return;

    dialog->is_open = false;
    dialog->title = NULL;
    dialog->message = NULL;
    dialog->buttons = NULL;
    dialog->selected_button = -1;
    dialog->frames_since_open = 0;
    dialog->button_count = 0;
}

bool iui_dialog_is_open(const iui_dialog_state *dialog)
{
    return dialog && dialog->is_open;
}

int iui_dialog(iui_context *ctx,
               iui_dialog_state *dialog,
               float screen_width,
               float screen_height)
{
    if (!ctx || !dialog || !dialog->is_open)
        return -1;

    /* Capture frame count BEFORE incrementing for click protection.
     * This ensures the first frame after opening (frames_active == 0) blocks
     * clicks, preventing click-through from the button that opened this dialog.
     */
    int frames_active = dialog->frames_since_open++;

    /* Calculate dialog dimensions based on content */
    float title_width = 0.f, message_width = 0.f, buttons_width = 0.f;

    if (dialog->title)
        title_width = iui_get_text_width(ctx, dialog->title);
    if (dialog->message)
        message_width = iui_get_text_width(ctx, dialog->message);

    /* Calculate total buttons width */
    int btn_count = dialog->button_count;
    if (btn_count > 0 && dialog->buttons) {
        for (int i = 0; i < btn_count; i++) {
            int len = 0;
            const char *label = iui_dialog_get_button(dialog->buttons, i, &len);
            if (label && len > 0) {
                /* Temporarily null-terminate for text width measurement */
                char btn_text[64];
                iui_safe_copy(btn_text, sizeof(btn_text), label, (size_t) len);
                float btn_w =
                    iui_get_text_width(ctx, btn_text) + IUI_DIALOG_PADDING;
                buttons_width += btn_w;
            }
        }
        buttons_width += (btn_count - 1) * IUI_DIALOG_BUTTON_SPACING;
    }

    /* Dialog width: max of content widths, clamped to min/max */
    float content_width = fmaxf(title_width, message_width);
    content_width = fmaxf(content_width, buttons_width);
    float dialog_w = content_width + IUI_DIALOG_PADDING * 2.f;
    dialog_w =
        clamp_float(IUI_DIALOG_MIN_WIDTH, IUI_DIALOG_MAX_WIDTH, dialog_w);

    /* Dialog height: title + message + buttons + spacing */
    float dialog_h = IUI_DIALOG_PADDING; /* top padding */
    if (dialog->title)
        dialog_h += ctx->font_height * 1.5f +
                    IUI_DIALOG_CONTENT_SPACING; /* title (larger) */
    if (dialog->message)
        dialog_h += ctx->font_height + IUI_DIALOG_CONTENT_SPACING; /* message */
    dialog_h += IUI_DIALOG_BUTTON_HEIGHT +
                IUI_DIALOG_PADDING; /* buttons + bottom padding */

    /* Center on screen */
    float dialog_x = (screen_width - dialog_w) * 0.5f;
    float dialog_y = (screen_height - dialog_h) * 0.5f;

    /* Begin modal blocking */
    iui_begin_modal(ctx, "dialog_modal");

    /* Register blocking region for input layer system */
    iui_rect_t dialog_bounds = {dialog_x, dialog_y, dialog_w, dialog_h};
    iui_register_blocking_region(ctx, dialog_bounds);

    /* Draw scrim (semi-transparent overlay) */
    uint32_t scrim_color = ctx->colors.scrim;
    ctx->renderer.draw_box((iui_rect_t) {0, 0, screen_width, screen_height}, 0,
                           scrim_color, ctx->renderer.user);

    /* Draw dialog shadow (elevation_3 for dialogs per MD3) */
    float corner = IUI_DIALOG_CORNER_RADIUS;
    iui_draw_shadow(ctx, dialog_bounds, corner, IUI_ELEVATION_3);

    /* Draw dialog background */
    ctx->renderer.draw_box(dialog_bounds, corner,
                           ctx->colors.surface_container_high,
                           ctx->renderer.user);

    /* Draw title */
    float content_x = dialog_x + IUI_DIALOG_PADDING;
    float content_y = dialog_y + IUI_DIALOG_PADDING;
    if (dialog->title) {
        iui_internal_draw_text(ctx, content_x, content_y, dialog->title,
                               ctx->colors.on_surface);
        content_y += ctx->font_height * 1.5f + IUI_DIALOG_CONTENT_SPACING;
    }

    /* Draw message */
    if (dialog->message) {
        iui_internal_draw_text(ctx, content_x, content_y, dialog->message,
                               ctx->colors.on_surface_variant);
        /* content_y not needed after this - buttons use btn_y */
    }

    /* Draw buttons (right-aligned, horizontal layout) */
    int result = -1;
    if (btn_count > 0 && dialog->buttons) {
        /* Calculate button positions (right-aligned) */
        float btn_y =
            dialog_y + dialog_h - IUI_DIALOG_PADDING - IUI_DIALOG_BUTTON_HEIGHT;
        float btn_x = dialog_x + dialog_w - IUI_DIALOG_PADDING;

        /* Draw buttons right-to-left so primary action is rightmost */
        for (int i = btn_count - 1; i >= 0; i--) {
            int len = 0;
            const char *label = iui_dialog_get_button(dialog->buttons, i, &len);
            if (!label || len <= 0)
                continue;

            /* Copy to null-terminated buffer */
            char btn_text[64];
            iui_safe_copy(btn_text, sizeof(btn_text), label, (size_t) len);

            float btn_text_w = iui_get_text_width(ctx, btn_text),
                  btn_w = btn_text_w + IUI_DIALOG_PADDING;
            btn_x -= btn_w;

            iui_rect_t btn_rect = {btn_x, btn_y, btn_w,
                                   IUI_DIALOG_BUTTON_HEIGHT};

            /* Get button state */
            iui_state_t state = iui_get_component_state(ctx, btn_rect, false);

            /* Draw button based on position (rightmost:filled, others:text) */
            uint32_t text_color = ctx->colors.primary;
            /* MD3 button corner radius (full rounded) */
            float btn_corner = IUI_SHAPE_FULL;

            if (i == btn_count - 1) {
                /* Primary button (rightmost) - filled style */
                uint32_t bg_color = ctx->colors.primary;
                text_color = ctx->colors.on_primary;
                ctx->renderer.draw_box(btn_rect, btn_corner, bg_color,
                                       ctx->renderer.user);
            }

            /* Draw state layer for hover/press */
            iui_draw_state_layer(ctx, btn_rect, btn_corner, text_color, state);

            /* Draw button text (centered) */
            float text_x = btn_rect.x + (btn_rect.width - btn_text_w) * 0.5f;
            float text_y =
                btn_rect.y + (btn_rect.height - ctx->font_height) * 0.5f;
            iui_internal_draw_text(ctx, text_x, text_y, btn_text, text_color);

            /* Handle click (only after frame protection) */
            if (frames_active >= 1 && state == IUI_STATE_PRESSED) {
                result = i;
                dialog->selected_button = i;
                /* Auto-close dialog when button is clicked */
                dialog->is_open = false;
                iui_close_modal(ctx);
                return result;
            }

            btn_x -= IUI_DIALOG_BUTTON_SPACING;
        }
    }

    /* End modal blocking (but keep modal active for next frame) */
    iui_end_modal(ctx);

    return result;
}

/* Full-Screen Dialog Implementation
 * Reference: https://m3.material.io/components/dialogs
 */

void iui_fullscreen_dialog_open(iui_fullscreen_dialog_state *dialog,
                                const char *title)
{
    if (!dialog)
        return;

    dialog->title = title;
    dialog->is_open = true;
    dialog->frames_since_open = 0;
    dialog->action_count = 0;
    dialog->action_x = 0.f;
    dialog->content_y = 0.f;
}

void iui_fullscreen_dialog_close(iui_fullscreen_dialog_state *dialog)
{
    if (!dialog)
        return;

    dialog->is_open = false;
    dialog->title = NULL;
    dialog->frames_since_open = 0;
    dialog->action_count = 0;
}

bool iui_fullscreen_dialog_is_open(const iui_fullscreen_dialog_state *dialog)
{
    return dialog && dialog->is_open;
}

bool iui_fullscreen_dialog_begin(iui_context *ctx,
                                 iui_fullscreen_dialog_state *dialog,
                                 float screen_width,
                                 float screen_height)
{
    if (!ctx || !dialog)
        return false;

    /* If dialog was closed externally, clean up the orphaned modal state to
     * prevent blocking other modals.
     */
    if (!dialog->is_open) {
        if (ctx->modal.active &&
            ctx->modal.id == iui_hash_str("fullscreen_dialog_modal")) {
            iui_close_modal(ctx);
        }
        return false;
    }

    /* Note: frame counter is incremented in iui_fullscreen_dialog_end() AFTER
     * all click protection checks, ensuring the first frame after opening
     * (frames_since_open == 0) blocks clicks.
     */

    /* Reset action count for this frame */
    dialog->action_count = 0;

    /* Begin modal blocking */
    iui_begin_modal(ctx, "fullscreen_dialog_modal");

    /* Full screen bounds */
    iui_rect_t screen_bounds = {0, 0, screen_width, screen_height};
    iui_register_blocking_region(ctx, screen_bounds);

    /* Draw full-screen surface background */
    ctx->renderer.draw_box(screen_bounds, 0.f, ctx->colors.surface,
                           ctx->renderer.user);

    /* Header bar dimensions */
    float header_h = IUI_FULLSCREEN_DIALOG_HEADER_HEIGHT;
    float padding = IUI_FULLSCREEN_DIALOG_PADDING;
    float icon_size = 24.f;
    float touch_target = 48.f;

    /* Draw close (X) icon button on left */
    iui_rect_t close_rect = {
        .x = padding,
        .y = (header_h - touch_target) * 0.5f,
        .width = touch_target,
        .height = touch_target,
    };

    iui_state_t close_state = iui_get_component_state(ctx, close_rect, false);

    /* Draw state layer for close button */
    iui_draw_state_layer(ctx, close_rect, touch_target * 0.5f,
                         ctx->colors.on_surface_variant, close_state);

    /* Draw X icon */
    float icon_cx = close_rect.x + touch_target * 0.5f;
    float icon_cy = close_rect.y + touch_target * 0.5f;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, "close",
                      ctx->colors.on_surface_variant);

    /* Handle close click */
    if (dialog->frames_since_open >= 1 && close_state == IUI_STATE_PRESSED) {
        iui_fullscreen_dialog_close(dialog);
        iui_close_modal(ctx);
        return false;
    }

    /* Draw title after close icon */
    if (dialog->title) {
        float title_x = padding + touch_target + padding * 0.5f;
        float title_y = (header_h - ctx->font_height) * 0.5f;
        iui_internal_draw_text(ctx, title_x, title_y, dialog->title,
                               ctx->colors.on_surface);
    }

    /* Store action x position for iui_fullscreen_dialog_action */
    dialog->action_x = screen_width - padding;

    /* Store content area start y */
    dialog->content_y = header_h;

    /* Set up layout for content area */
    ctx->layout.x = padding;
    ctx->layout.y = header_h + padding;
    ctx->layout.width = screen_width - padding * 2.f;
    ctx->layout.height = screen_height - header_h - padding * 2.f;

    return true;
}

bool iui_fullscreen_dialog_action(iui_context *ctx,
                                  iui_fullscreen_dialog_state *dialog,
                                  const char *label)
{
    if (!ctx || !dialog || !dialog->is_open || !label)
        return false;

    /* Max 2 actions */
    if (dialog->action_count >= IUI_FULLSCREEN_DIALOG_MAX_ACTIONS)
        return false;

    float header_h = IUI_FULLSCREEN_DIALOG_HEADER_HEIGHT;
    float btn_padding = 16.f;
    float btn_height = 36.f;

    /* Calculate button width */
    float text_w = iui_get_text_width(ctx, label);
    float btn_w = text_w + btn_padding * 2.f;

    /* Position button from right */
    dialog->action_x -= btn_w;
    float btn_x = dialog->action_x;
    float btn_y = (header_h - btn_height) * 0.5f;
    dialog->action_x -= 8.f; /* spacing between actions */

    iui_rect_t btn_rect = {btn_x, btn_y, btn_w, btn_height};

    /* Get button state */
    iui_state_t state = iui_get_component_state(ctx, btn_rect, false);

    /* Draw button (primary style for first action, text for others) */
    uint32_t text_color;
    float corner = IUI_SHAPE_FULL;

    if (dialog->action_count == 0) {
        /* Primary action - filled button */
        ctx->renderer.draw_box(btn_rect, corner, ctx->colors.primary,
                               ctx->renderer.user);
        text_color = ctx->colors.on_primary;
    } else {
        /* Secondary action - text button */
        text_color = ctx->colors.primary;
    }

    /* State layer */
    iui_draw_state_layer(ctx, btn_rect, corner, text_color, state);

    /* Draw button text centered */
    float text_x = btn_rect.x + (btn_rect.width - text_w) * 0.5f;
    float text_y = btn_rect.y + (btn_rect.height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, text_x, text_y, label, text_color);

    dialog->action_count++;

    /* Handle click */
    if (dialog->frames_since_open >= 1 && state == IUI_STATE_PRESSED)
        return true;

    return false;
}

void iui_fullscreen_dialog_end(iui_context *ctx,
                               iui_fullscreen_dialog_state *dialog)
{
    if (!ctx || !dialog || !dialog->is_open)
        return;

    /* End modal blocking */
    iui_end_modal(ctx);

    /* Increment frame counter AFTER all protection checks */
    dialog->frames_since_open++;
}
