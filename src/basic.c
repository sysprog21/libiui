#include "internal.h"

/* Segmented control */

void iui_segmented(iui_context *ctx,
                   const char **entries,
                   uint32_t num_entries,
                   uint32_t *selected)
{
    /* Safety: validate all pointers and MD3 segment count (2-5) */
    if (!ctx || !ctx->current_window || !entries || !selected ||
        num_entries < IUI_SEGMENTED_MIN_SEGMENTS ||
        num_entries > IUI_SEGMENTED_MAX_SEGMENTS)
        return;

    /* Use font-based height for better integration, pill shape */
    float seg_height = ctx->font_height + ctx->padding * 2,
          pill_radius = seg_height / 2.f,
          seg_width = ctx->layout.width / (float) num_entries,
          seg_y = ctx->layout.y + ctx->padding, seg_x_start = ctx->layout.x;

    /* Report required width for auto-sizing windows.
     * Segments have uniform width, so find max content width needed.
     * Selected state shows: icon (18px) + gap (8px) + text + padding.
     */
    float max_segment_width = 0.f;
    for (uint32_t i = 0; i < num_entries; ++i) {
        if (entries[i]) {
            float text_w = iui_get_text_width(ctx, entries[i]);
            /* Account for checkmark when selected: icon + gap + text */
            float content_w = IUI_SEGMENTED_ICON_SIZE + 8.f + text_w;
            float segment_w = content_w + ctx->padding * 2.f;
            if (segment_w > max_segment_width)
                max_segment_width = segment_w;
        }
    }
    iui_require_content_width(ctx, max_segment_width * (float) num_entries);

    /* Clamp selected to valid range */
    if (*selected >= num_entries)
        *selected = 0;

    /* MD3: Draw unified pill background (visible container for all segments) */
    ctx->renderer.draw_box(
        (iui_rect_t) {seg_x_start, seg_y, ctx->layout.width, seg_height},
        pill_radius, ctx->colors.surface_container_highest, ctx->renderer.user);

    /* Track component for MD3 validation */
    IUI_MD3_TRACK_SEGMENTED(
        ((iui_rect_t) {seg_x_start, seg_y, ctx->layout.width, seg_height}),
        pill_radius);

    /* Selected segment highlight with animation */
    if (*selected < num_entries) {
        float sel_x = (ctx->animation.widget == selected)
                          ? lerp_float(ctx->animation.value_key0,
                                       ctx->animation.value_key1,
                                       ease_out_back(ctx->animation.t))
                          : seg_x_start + seg_width * (*selected);

        /* Corner radius only for first/last segments */
        float corner = 0.f;
        if (*selected == 0 || *selected == num_entries - 1)
            corner = pill_radius;

        ctx->renderer.draw_box(
            (iui_rect_t) {sel_x, seg_y, seg_width, seg_height}, corner,
            ctx->colors.secondary_container, ctx->renderer.user);
    }

    /* Draw each segment */
    float seg_x = seg_x_start;
    for (uint32_t i = 0; i < num_entries; ++i) {
        /* Safety: skip null entries but still advance position */
        if (!entries[i]) {
            seg_x += seg_width;
            continue;
        }

        bool is_selected = (i == *selected);
        iui_rect_t button_rect = {
            .x = seg_x,
            .y = seg_y,
            .width = seg_width,
            .height = seg_height,
        };
        iui_state_t seg_state =
            iui_get_component_state(ctx, button_rect, false);

        /* Handle hover state layer for unselected segments */
        if (!is_selected && iui_state_is_interactive(seg_state)) {
            uint32_t hover_color = iui_state_layer(
                ctx->colors.on_surface, iui_state_get_alpha(seg_state));
            float corner = (i == 0 || i == num_entries - 1) ? pill_radius : 0.f;
            ctx->renderer.draw_box(
                (iui_rect_t) {seg_x, seg_y, seg_width, seg_height}, corner,
                hover_color, ctx->renderer.user);
        }

        /* Handle selection change */
        if (seg_state == IUI_STATE_PRESSED && !is_selected) {
            ctx->animation = (iui_animation) {
                .value_key0 = seg_x_start + seg_width * (*selected),
                .value_key1 = seg_x_start + seg_width * i,
                .widget = selected,
            };
            *selected = i;
            is_selected = true;
        }

        /* Text colors: selected uses on_secondary_container, unselected uses
         * on_surface */
        uint32_t text_color = is_selected ? ctx->colors.on_secondary_container
                                          : ctx->colors.on_surface;

        if (is_selected) {
            /* Calculate total content width: checkmark + gap + text */
            float icon_size = IUI_SEGMENTED_ICON_SIZE,
                  text_w = iui_get_text_width(ctx, entries[i]);
            float gap = 8.f, content_width = icon_size + gap + text_w,
                  content_x = seg_x + (seg_width - content_width) / 2.f,
                  icon_cx = content_x + icon_size / 2.f,
                  icon_cy = seg_y + seg_height / 2.f;

            /* Draw checkmark */
            iui_draw_icon_check(ctx, icon_cx, icon_cy, icon_size, text_color);

            /* Draw text after checkmark */
            float text_x = content_x + icon_size + gap,
                  text_y = seg_y + (seg_height - ctx->font_height) / 2.f;
            iui_internal_draw_text(ctx, text_x, text_y, entries[i], text_color);
        } else {
            /* Unselected: just center the text */
            draw_align_text(ctx, &button_rect, entries[i], text_color,
                            IUI_ALIGN_CENTER);
        }

        seg_x += seg_width;
    }

    /* Advance layout */
    ctx->layout.y += seg_height + ctx->padding;
}

/* Sliders */

void iui_slider(iui_context *ctx,
                const char *label,
                float min_value,
                float max_value,
                float step,
                float *value,
                const char *fmt)
{
    /* Use extended slider for consistent implementation */
    iui_slider_options options = {
        .start_text = label,
        .value_format = fmt,
    }; /* Use label as start text */

    /* Draw extended slider (returns float, but also updates pointer) */
    *value = iui_slider_ex(ctx, *value, min_value, max_value, step, &options);
}

/* Extended slider with customizable appearance and behavior.
 *
 * Animation State Encoding:
 *   ctx->slider.active_id uses a bitmask to distinguish drag vs animation:
 *   - Bits 0-30: slider_id (masked to 31 bits for consistent comparison)
 *   - Bit 31 (IUI_SLIDER_ANIM_FLAG): animation flag
 *     - Set (1): animating to clicked position on track
 *     - Clear (0): user is actively dragging the thumb
 *
 * Both drag and animation store masked IDs to prevent hash collisions
 * when slider_id naturally has bit 31 set (~50% of hashes).
 */
float iui_slider_ex(iui_context *ctx,
                    float value,
                    float min,
                    float max,
                    float step,
                    const iui_slider_options *options)
{
    if (!ctx->current_window || max <= min)
        return value;

    /* Generate unique ID for this slider based on layout position.
     * Use iui_slider_masked_id() to ensure consistent 31-bit IDs for tracking
     * and to handle the zero-ID edge case.
     */
    uint32_t slider_id = iui_slider_masked_id(
        iui_hash("slider_ex", 9) ^ iui_hash_pos(ctx->layout.x, ctx->layout.y));

    /* Register this slider for per-frame tracking */
    iui_register_slider(ctx, slider_id);

    /* Clamp input value */
    value = clamp_float(min, max, value);

    /* Get colors (use options if provided, otherwise theme defaults) */
    uint32_t active_color = (options && options->active_track_color)
                                ? options->active_track_color
                                : ctx->colors.primary;
    uint32_t inactive_color = (options && options->inactive_track_color)
                                  ? options->inactive_track_color
                                  : ctx->colors.surface_container_highest;
    uint32_t handle_color = (options && options->handle_color)
                                ? options->handle_color
                                : ctx->colors.primary;

    bool disabled = options && options->disabled;
    const char *fmt =
        (options && options->value_format) ? options->value_format : "%.0f";

    /* Disabled state: track at 12% (same as press/focus), handle at 38% (MD3
     * spec)
     */
    if (disabled) {
        active_color =
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_FOCUS_ALPHA);
        inactive_color =
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_FOCUS_ALPHA);
        handle_color =
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_DISABLE_ALPHA);
    }

    /* Draw start/end text labels if provided */
    if (options && options->start_text) {
        uint32_t label_color = disabled
                                   ? iui_state_layer(ctx->colors.on_surface,
                                                     IUI_STATE_DISABLE_ALPHA)
                                   : ctx->colors.on_surface;
        draw_align_text(ctx, &ctx->layout, options->start_text, label_color,
                        IUI_ALIGN_LEFT);
    }
    if (options && options->end_text) {
        uint32_t label_color = disabled
                                   ? iui_state_layer(ctx->colors.on_surface,
                                                     IUI_STATE_DISABLE_ALPHA)
                                   : ctx->colors.on_surface;
        draw_align_text(ctx, &ctx->layout, options->end_text, label_color,
                        IUI_ALIGN_RIGHT);
    }
    if (options && (options->start_text || options->end_text))
        iui_newline(ctx);

    float center_y = ctx->layout.y + .5f * ctx->layout.height;

    /* MD3 Slider: Use defined track height constant */
    float track_height = IUI_SLIDER_TRACK_HEIGHT;
    float track_margin = ctx->layout.width * 0.05f; /* 5% margin on each side */
    iui_rect_t track_rect = {
        .x = ctx->layout.x + track_margin,
        .y = center_y - track_height * .5f,
        .width = ctx->layout.width - track_margin * 2.f,
        .height = track_height,
    };

    /* Normalized value and thumb position */
    float norm_value = (value - min) / (max - min);
    float thumb_x = norm_value * track_rect.width + track_rect.x;

    /* MD3 thumb sizes: idle=20dp, pressed=28dp.
     * slider_id is already masked via iui_slider_masked_id().
     * Drag check: ID match in lower 31 bits AND animation flag clear. */
    bool is_dragging =
        ((ctx->slider.active_id & IUI_SLIDER_ID_MASK) == slider_id) &&
        !(ctx->slider.active_id & IUI_SLIDER_ANIM_FLAG);
    float thumb_size =
              is_dragging ? IUI_SLIDER_THUMB_PRESSED : IUI_SLIDER_THUMB_IDLE,
          half_size = thumb_size * .5f;

    /* Get component state for track and thumb */
    iui_rect_t thumb_rect = {
        .x = thumb_x - half_size,
        .y = center_y - half_size,
        .width = thumb_size,
        .height = thumb_size,
    };

    /* Expand hit area for touch target (48dp minimum) */
    iui_rect_t touch_rect = thumb_rect;
    iui_expand_touch_target(&touch_rect, IUI_SLIDER_TOUCH_TARGET);

    iui_state_t track_state =
        iui_get_component_state(ctx, track_rect, disabled);
    iui_state_t thumb_state =
        iui_get_component_state(ctx, touch_rect, disabled);

    /* Draw inactive track (full width, behind active track) */
    ctx->renderer.draw_box(track_rect, track_rect.height * .5f, inactive_color,
                           ctx->renderer.user);

    /* Draw active track (left side up to thumb) */
    float active_width = thumb_x - track_rect.x;
    if (active_width > 0) {
        ctx->renderer.draw_box((iui_rect_t) {track_rect.x, track_rect.y,
                                             active_width, track_rect.height},
                               track_rect.height * .5f, active_color,
                               ctx->renderer.user);
    }

    /* Handle thumb interaction */
    bool thumb_hovered = (thumb_state == IUI_STATE_HOVERED);
    bool thumb_pressed = (thumb_state == IUI_STATE_PRESSED);

    if (!disabled) {
        if (thumb_pressed && !is_dragging) {
            /* Start dragging: store ID without animation flag */
            ctx->slider.active_id = slider_id;
            ctx->slider.drag_offset = ctx->mouse_pos.x - thumb_x;
            is_dragging = true;
        } else if (track_state == IUI_STATE_PRESSED && !is_dragging) {
            /* Click on track: animate thumb to click position */
            ctx->slider.anim_start_x = thumb_x;
            ctx->slider.anim_target_x =
                clamp_float(track_rect.x, track_rect.x + track_rect.width,
                            ctx->mouse_pos.x);
            ctx->slider.anim_t = 0.f;
            /* Store ID with animation flag set */
            ctx->slider.active_id = slider_id | IUI_SLIDER_ANIM_FLAG;
        }

        /* Update animation: check ID match AND animation flag set */
        if ((ctx->slider.active_id & IUI_SLIDER_ID_MASK) == slider_id &&
            (ctx->slider.active_id & IUI_SLIDER_ANIM_FLAG)) {
            ctx->slider.anim_t += ctx->delta_time / IUI_DURATION_SHORT_4;
            if (ctx->slider.anim_t >= 1.f) {
                ctx->slider.anim_t = 1.f;
                ctx->slider.active_id = 0;
            }
            thumb_x =
                lerp_float(ctx->slider.anim_start_x, ctx->slider.anim_target_x,
                           ease_out_back(ctx->slider.anim_t));
        }

        /* Update drag position */
        if (is_dragging && (ctx->mouse_held & IUI_MOUSE_LEFT)) {
            thumb_x = ctx->mouse_pos.x - ctx->slider.drag_offset;
        } else if (is_dragging) {
            /* Release drag */
            ctx->slider.active_id = 0;
            is_dragging = false;
        }
    }

    /* Clamp thumb position to track bounds */
    thumb_x =
        clamp_float(track_rect.x, track_rect.x + track_rect.width, thumb_x);

    /* Calculate value from thumb position */
    norm_value = (thumb_x - track_rect.x) / track_rect.width;
    value = norm_value * (max - min) + min;
    if (step > 0.f)
        value = roundf(value / step) * step;
    value = clamp_float(min, max, value);

    /* Recalculate thumb_x after step quantization */
    norm_value = (value - min) / (max - min);
    thumb_x = norm_value * track_rect.width + track_rect.x;

    /* Update thumb rect with final position */
    thumb_rect.x = thumb_x - half_size;

    /* Track component for MD3 validation using final thumb/touch bounds */
    touch_rect = thumb_rect;
    iui_expand_touch_target(&touch_rect, IUI_SLIDER_TOUCH_TARGET);
    IUI_MD3_TRACK_SLIDER(touch_rect, touch_rect.height * .5f);

    /* MD3: State layer on hover/press/drag */
    if ((thumb_hovered || is_dragging) && !disabled) {
        float state_size = thumb_size * 1.5f;
        float state_x = thumb_x - state_size * 0.5f,
              state_y = center_y - state_size * 0.5f;
        uint8_t alpha =
            is_dragging ? IUI_STATE_DRAG_ALPHA : IUI_STATE_HOVER_ALPHA;
        uint32_t state_color = iui_state_layer(handle_color, alpha);
        ctx->renderer.draw_box(
            (iui_rect_t) {state_x, state_y, state_size, state_size},
            state_size * 0.5f, state_color, ctx->renderer.user);
    }

    /* Draw value indicator bubble during drag */
    if (options && options->show_value_indicator && is_dragging && !disabled) {
        /* Format the value */
        int written =
            snprintf(ctx->string_buffer, IUI_STRING_BUFFER_SIZE, fmt, value);
        if (written >= IUI_STRING_BUFFER_SIZE)
            ctx->string_buffer[IUI_STRING_BUFFER_SIZE - 1] = '\0';

        float text_width = iui_get_text_width(ctx, ctx->string_buffer),
              indicator_width =
                  fmaxf(IUI_SLIDER_VALUE_INDICATOR, text_width + ctx->padding),
              indicator_height = IUI_SLIDER_VALUE_INDICATOR,
              indicator_x = thumb_x - indicator_width * 0.5f,
              indicator_y = thumb_rect.y - indicator_height -
                            8.f; /* 8dp gap above thumb */

        /* Temporarily expand clip upward so the indicator isn't cut off near
         * window top
         */
        iui_clip_rect prev_clip = ctx->current_clip;
        bool expanded_clip = false;
        if (indicator_y < (float) prev_clip.miny) {
            uint16_t new_miny = (uint16_t) fmaxf(0.f, indicator_y);
            ctx->renderer.set_clip_rect(prev_clip.minx, new_miny,
                                        prev_clip.maxx, prev_clip.maxy,
                                        ctx->renderer.user);
            ctx->current_clip.miny = new_miny;
            expanded_clip = true;
        }

        /* Draw indicator background (pill shape with primary color) */
        ctx->renderer.draw_box((iui_rect_t) {indicator_x, indicator_y,
                                             indicator_width, indicator_height},
                               indicator_height * 0.5f, active_color,
                               ctx->renderer.user);

        /* Draw value text centered in indicator */
        iui_rect_t indicator_text_rect = {
            .x = indicator_x,
            .y = indicator_y + (indicator_height - ctx->font_height) * 0.5f,
            .width = indicator_width,
            .height = ctx->font_height,
        };
        draw_align_text(ctx, &indicator_text_rect, ctx->string_buffer,
                        ctx->colors.on_primary, IUI_ALIGN_CENTER);

        if (expanded_clip) {
            ctx->current_clip = prev_clip;
            ctx->renderer.set_clip_rect(prev_clip.minx, prev_clip.miny,
                                        prev_clip.maxx, prev_clip.maxy,
                                        ctx->renderer.user);
        }
    }

    /* Draw thumb (circle) */
    ctx->renderer.draw_box(thumb_rect, half_size, handle_color,
                           ctx->renderer.user);

    iui_newline(ctx);

    return value;
}

/* Buttons */

bool iui_button(iui_context *ctx,
                const char *label,
                iui_text_alignment_t alignment)
{
    /* Simple wrapper - tonal style is the default MD3 button */
    return iui_button_styled(ctx, label, alignment, IUI_BUTTON_TONAL);
}

/* MD3 styled button with configurable appearance.
 *
 * Button Layout:
 *   1. Compute button rect based on mode (grid vs flow layout)
 *   2. Apply MD3 dimensions: 40dp height, pill-shaped corners
 *   3. Register for focus navigation with unique widget ID
 *   4. Expand touch target to 48dp minimum for accessibility
 *   5. Apply style-specific colors (filled/tonal/outlined/text)
 *   6. Draw state layer overlay for hover/focus/press feedback
 *   7. Render centered label text
 *
 * Returns true if button was clicked this frame.
 */
bool iui_button_styled(iui_context *ctx,
                       const char *label,
                       iui_text_alignment_t alignment,
                       iui_button_style_t style)
{
    if (!ctx->current_window || !label)
        return false;

    /* Modal blocking is handled centrally by iui_get_component_state() which
     * returns IUI_STATE_DEFAULT when modal is active and rendering=false.
     */

    bool clicked = false;
    float text_width = iui_get_text_width(ctx, label);

    iui_rect_t button_rect;
    float corner;

    /* In grid mode, use full cell dimensions */
    if (ctx->in_grid) {
        button_rect = (iui_rect_t) {
            .x = ctx->layout.x,
            .y = ctx->layout.y,
            .width = ctx->layout.width,
            .height = ctx->layout.height,
        };
        corner =
            ctx->layout.height * 0.5f; /* Use layout height for grid mode */
    } else {
        /* MD3 spec: button height = 40dp, scale down for smaller rows */
        float btn_height = fminf(IUI_BUTTON_HEIGHT, ctx->row_height);
        /* MD3: Use pill-shaped corners (height/2) for buttons */
        corner = btn_height * 0.5f;
        button_rect = (iui_rect_t) {
            .y = ctx->layout.y + (ctx->row_height - btn_height) * 0.5f,
            .height = btn_height,
            .width = text_width + 2.f * ctx->padding,
        };

        switch (alignment) {
        case IUI_ALIGN_LEFT:
            button_rect.x = ctx->layout.x;
            break;
        case IUI_ALIGN_CENTER:
            button_rect.x = ctx->layout.x + ctx->layout.width * .5f -
                            button_rect.width * .5f;
            break;
        default:
            button_rect.x =
                ctx->layout.x + ctx->layout.width - button_rect.width;
            break;
        }
    }

    /* Register as focusable widget for keyboard navigation.
     * Combine label hash with layout position to avoid ID collision
     * when multiple buttons share the same label.
     */
    uint32_t widget_id = iui_widget_id(label, button_rect);
    iui_register_focusable(ctx, widget_id, button_rect, corner);
    bool is_focused = iui_widget_is_focused(ctx, widget_id);

    /* Center text in button */
    iui_vec2 text_pos = {
        button_rect.x + (button_rect.width - text_width) * .5f,
        button_rect.y + (button_rect.height - ctx->font_height) * .5f,
    };

    /* Expand touch target for accessibility (48dp minimum per MD3) */
    iui_rect_t touch_rect = button_rect;
    iui_expand_touch_target_h(&touch_rect, IUI_BUTTON_MIN_TOUCH_TARGET);

    iui_state_t state = iui_get_component_state(ctx, touch_rect, false);

    /* Handle Enter key when focused to activate button */
    if (is_focused && (ctx->key_pressed == IUI_KEY_ENTER)) {
        clicked = true;
        ctx->key_pressed = IUI_KEY_NONE; /* Consume key */
        ctx->animation = (iui_animation) {
            .widget = (void *) label,
        };
    }

    /* Determine colors based on button style */
    uint32_t bg_color = 0;
    uint32_t text_color = 0;
    uint32_t border_color = 0;
    float border_width = 0.f;
    uint32_t hover_layer = 0;

    switch (style) {
    case IUI_BUTTON_FILLED:
        bg_color = ctx->colors.primary;
        text_color = ctx->colors.on_primary;
        hover_layer =
            iui_state_layer(ctx->colors.on_primary, IUI_STATE_HOVER_ALPHA);
        break;
    case IUI_BUTTON_OUTLINED:
        bg_color = 0; /* transparent background */
        text_color = ctx->colors.primary;
        border_color = ctx->colors.outline;
        border_width = 1.f;
        hover_layer =
            iui_state_layer(ctx->colors.primary, IUI_STATE_HOVER_ALPHA / 2);
        break;
    case IUI_BUTTON_TEXT:
        bg_color = 0; /* transparent background */
        text_color = ctx->colors.primary;
        hover_layer =
            iui_state_layer(ctx->colors.primary, IUI_STATE_HOVER_ALPHA / 2);
        break;
    case IUI_BUTTON_ELEVATED:
        bg_color = ctx->colors.surface_container_high;
        text_color = ctx->colors.primary;
        hover_layer =
            iui_state_layer(ctx->colors.primary, IUI_STATE_HOVER_ALPHA / 3);
        break;
    case IUI_BUTTON_TONAL:
    default:
        bg_color = ctx->colors.surface_container;
        text_color = ctx->colors.on_surface;
        hover_layer =
            iui_state_layer(ctx->colors.on_surface, IUI_STATE_HOVER_ALPHA);
        break;
    }

    if (ctx->animation.widget == label) {
        /* Press animation: flash appropriate color then fade */
        bg_color = lerp_color(
            (style == IUI_BUTTON_FILLED)
                ? ctx->colors.primary
                : ((style == IUI_BUTTON_OUTLINED || style == IUI_BUTTON_TEXT)
                       ? 0
                       : (style == IUI_BUTTON_ELEVATED
                              ? ctx->colors.surface_container_high
                              : ctx->colors.surface_container)),
            bg_color, ease_in_expo(ctx->animation.t));
        expand_rect(&button_rect, -ease_impulse(ctx->animation.t) * 2.f);
    } else if (state == IUI_STATE_PRESSED) {
        clicked = true;
        ctx->animation = (iui_animation) {.widget = (void *) label};
    } else if (state == IUI_STATE_HOVERED) {
        /* Apply hover state layer for the button style */
        if (ctx->hover.widget == label) {
            /* Blend hover state layer */
            if (bg_color != 0)
                bg_color = iui_blend_color(bg_color, hover_layer);
        } else {
            ctx->hover = (iui_hover) {.widget = (void *) label};
        }
    } else if (ctx->hover.widget == label) {
        ctx->hover.widget = NULL;
    }

    /* Apply focus state layer (12% opacity per MD3) */
    uint32_t focus_layer = 0;
    if (is_focused && ctx->animation.widget != label) {
        focus_layer =
            iui_state_layer(ctx->colors.primary, IUI_STATE_FOCUS_ALPHA);
        if (bg_color != 0)
            bg_color = iui_blend_color(bg_color, focus_layer);
    }

    /* Draw focus ring when focused (before button background) */
    if (is_focused)
        iui_draw_focus_ring(ctx, button_rect, corner);

    if (bg_color != 0) {
        ctx->renderer.draw_box(button_rect, corner, bg_color,
                               ctx->renderer.user);
    } else if (is_focused && focus_layer != 0) {
        /* MD3: Show focus state layer for text/outlined buttons (no bg) */
        ctx->renderer.draw_box(button_rect, corner, focus_layer,
                               ctx->renderer.user);
    } else if (state == IUI_STATE_HOVERED && hover_layer != 0) {
        /* MD3: Text buttons show state layer on hover (no bg, but visible
         * hover) */
        ctx->renderer.draw_box(button_rect, corner, hover_layer,
                               ctx->renderer.user);
    }

    /* Draw border if specified (for outlined buttons) */
    if (border_width > 0.f)
        iui_draw_rect_outline(ctx, button_rect, border_width, border_color);

    iui_internal_draw_text(ctx, text_pos.x, text_pos.y, label, text_color);

    /* MD3 runtime validation: track touch target (not visual bounds) */
    IUI_MD3_TRACK_BUTTON(touch_rect, corner);

    return clicked;
}
