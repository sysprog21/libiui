/* List component implementation
 * Reference: https://m3.material.io/components/lists
 */

#include "internal.h"

/* Get item height based on list type */
static float get_list_height(iui_list_type_t type)
{
    switch (type) {
    case IUI_LIST_ONE_LINE:
        return IUI_LIST_ONE_LINE_HEIGHT;
    case IUI_LIST_TWO_LINE:
        return IUI_LIST_TWO_LINE_HEIGHT;
    case IUI_LIST_THREE_LINE:
        return IUI_LIST_THREE_LINE_HEIGHT;
    default:
        return IUI_LIST_ONE_LINE_HEIGHT;
    }
}

/* Draw leading element (icon, avatar, checkbox, radio) */
static void draw_leading_element(iui_context *ctx,
                                 const iui_list_item *item,
                                 float x,
                                 float cy,
                                 uint32_t color,
                                 bool *control_clicked)
{
    *control_clicked = false;

    switch (item->leading_type) {
    case IUI_LIST_LEADING_ICON:
        if (item->leading_icon)
            iui_draw_fab_icon(ctx, x + IUI_LIST_ICON_SIZE * 0.5f, cy,
                              IUI_LIST_ICON_SIZE, item->leading_icon, color);
        break;

    case IUI_LIST_LEADING_AVATAR:
        /* Draw circular avatar placeholder */
        iui_draw_circle(ctx, x + IUI_LIST_AVATAR_SIZE * 0.5f, cy,
                        IUI_LIST_AVATAR_SIZE * 0.5f,
                        ctx->colors.secondary_container, 0, 0);
        /* Draw avatar icon if provided */
        if (item->leading_icon)
            iui_draw_fab_icon(ctx, x + IUI_LIST_AVATAR_SIZE * 0.5f, cy,
                              IUI_LIST_ICON_SIZE, item->leading_icon,
                              ctx->colors.on_secondary_container);
        break;

    case IUI_LIST_LEADING_CHECKBOX:
        if (item->checkbox_value) {
            iui_rect_t cb_rect = {
                .x = x,
                .y = cy - IUI_LIST_ICON_SIZE * 0.5f,
                .width = IUI_LIST_ICON_SIZE,
                .height = IUI_LIST_ICON_SIZE,
            };
            iui_state_t state =
                iui_get_component_state(ctx, cb_rect, item->disabled);

            /* Draw checkbox box */
            uint32_t box_color = *item->checkbox_value ? ctx->colors.primary
                                                       : ctx->colors.outline;
            iui_draw_rect_outline(ctx, cb_rect, 2.f, box_color);

            /* Draw check mark if checked */
            if (*item->checkbox_value)
                iui_draw_icon_check(ctx, cb_rect.x + cb_rect.width * 0.5f,
                                    cb_rect.y + cb_rect.height * 0.5f,
                                    cb_rect.width * 0.6f, ctx->colors.primary);

            /* Handle click */
            if (state == IUI_STATE_PRESSED && !item->disabled) {
                *item->checkbox_value = !*item->checkbox_value;
                *control_clicked = true;
            }
        }
        break;

    case IUI_LIST_LEADING_RADIO:
        if (item->radio_value) {
            float radio_radius = IUI_LIST_ICON_SIZE * 0.5f;
            float cx = x + radio_radius;
            bool selected = (*item->radio_value == item->radio_option);

            iui_rect_t radio_rect = {
                .x = x,
                .y = cy - radio_radius,
                .width = IUI_LIST_ICON_SIZE,
                .height = IUI_LIST_ICON_SIZE,
            };
            iui_state_t state =
                iui_get_component_state(ctx, radio_rect, item->disabled);

            /* Draw outer circle */
            uint32_t circle_color =
                selected ? ctx->colors.primary : ctx->colors.outline;
            iui_draw_circle(ctx, cx, cy, radio_radius, 0, circle_color, 2.f);

            /* Draw inner dot if selected */
            if (selected)
                iui_draw_circle(ctx, cx, cy, radio_radius * 0.5f,
                                ctx->colors.primary, 0, 0);

            /* Handle click */
            if (state == IUI_STATE_PRESSED && !item->disabled) {
                *item->radio_value = item->radio_option;
                *control_clicked = true;
            }
        }
        break;

    case IUI_LIST_LEADING_IMAGE:
        /* Draw square image placeholder */
        ctx->renderer.draw_box(
            (iui_rect_t) {x, cy - IUI_LIST_ONE_LINE_HEIGHT * 0.5f,
                          IUI_LIST_ONE_LINE_HEIGHT, IUI_LIST_ONE_LINE_HEIGHT},
            4.f, ctx->colors.surface_container_high, ctx->renderer.user);
        break;

    default:
        break;
    }
}

/* Draw trailing element (icon, text, checkbox, switch) */
static void draw_trailing_element(iui_context *ctx,
                                  const iui_list_item *item,
                                  float x,
                                  float cy,
                                  uint32_t color,
                                  bool *control_clicked)
{
    *control_clicked = false;

    switch (item->trailing_type) {
    case IUI_LIST_TRAILING_ICON:
        if (item->trailing_icon)
            iui_draw_fab_icon(ctx, x, cy, IUI_LIST_ICON_SIZE,
                              item->trailing_icon, color);
        break;

    case IUI_LIST_TRAILING_TEXT:
        if (item->trailing_text) {
            float text_width = iui_get_text_width(ctx, item->trailing_text);
            iui_internal_draw_text(
                ctx, x - text_width, cy - ctx->font_height * 0.5f,
                item->trailing_text, ctx->colors.on_surface_variant);
        }
        break;

    case IUI_LIST_TRAILING_CHECKBOX:
        if (item->checkbox_value) {
            float cb_size = IUI_LIST_ICON_SIZE;
            iui_rect_t cb_rect = {
                .x = x - cb_size,
                .y = cy - cb_size * 0.5f,
                .width = cb_size,
                .height = cb_size,
            };
            iui_state_t state =
                iui_get_component_state(ctx, cb_rect, item->disabled);

            uint32_t box_color = *item->checkbox_value ? ctx->colors.primary
                                                       : ctx->colors.outline;
            iui_draw_rect_outline(ctx, cb_rect, 2.f, box_color);

            if (*item->checkbox_value)
                iui_draw_icon_check(ctx, cb_rect.x + cb_rect.width * 0.5f,
                                    cb_rect.y + cb_rect.height * 0.5f,
                                    cb_rect.width * 0.6f, ctx->colors.primary);

            if (state == IUI_STATE_PRESSED && !item->disabled) {
                *item->checkbox_value = !*item->checkbox_value;
                *control_clicked = true;
            }
        }
        break;

    case IUI_LIST_TRAILING_SWITCH:
        if (item->checkbox_value) {
            float switch_w = IUI_SWITCH_TRACK_WIDTH * 0.8f;
            float switch_h = IUI_SWITCH_TRACK_HEIGHT * 0.8f;
            float thumb_size = switch_h * 0.7f;

            iui_rect_t switch_rect = {
                .x = x - switch_w,
                .y = cy - switch_h * 0.5f,
                .width = switch_w,
                .height = switch_h,
            };
            iui_state_t state =
                iui_get_component_state(ctx, switch_rect, item->disabled);

            /* Draw track */
            uint32_t track_color = *item->checkbox_value
                                       ? ctx->colors.primary
                                       : ctx->colors.surface_container_highest;
            ctx->renderer.draw_box(switch_rect, switch_h * 0.5f, track_color,
                                   ctx->renderer.user);

            /* Draw thumb */
            float thumb_x = *item->checkbox_value
                                ? switch_rect.x + switch_w - switch_h * 0.5f
                                : switch_rect.x + switch_h * 0.5f;
            uint32_t thumb_color = *item->checkbox_value
                                       ? ctx->colors.on_primary
                                       : ctx->colors.outline;
            iui_draw_circle(ctx, thumb_x, cy, thumb_size * 0.5f, thumb_color, 0,
                            0);

            if (state == IUI_STATE_PRESSED && !item->disabled) {
                *item->checkbox_value = !*item->checkbox_value;
                *control_clicked = true;
            }
        }
        break;

    default:
        break;
    }
}

bool iui_list_item_ex(iui_context *ctx,
                      iui_list_type_t type,
                      const iui_list_item *item)
{
    if (!ctx || !item || !item->headline)
        return false;
    if (!ctx->current_window)
        return false;

    float item_height = get_list_height(type);
    float item_width = ctx->layout.width;
    float item_x = ctx->layout.x;
    float item_y = ctx->layout.y;

    iui_rect_t item_rect = {
        .x = item_x,
        .y = item_y,
        .width = item_width,
        .height = item_height,
    };

    /* Get component state for hover/press */
    iui_state_t state = iui_get_component_state(ctx, item_rect, item->disabled);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, item_rect, 0.f, ctx->colors.on_surface, state);

    /* Track component for MD3 validation */
    if (type == IUI_LIST_ONE_LINE)
        IUI_MD3_TRACK_LIST_ITEM_ONE_LINE(item_rect, 0.f);
    else if (type == IUI_LIST_TWO_LINE)
        IUI_MD3_TRACK_LIST_ITEM_TWO_LINE(item_rect, 0.f);
    else if (type == IUI_LIST_THREE_LINE)
        IUI_MD3_TRACK_LIST_ITEM_THREE_LINE(item_rect, 0.f);

    /* Calculate content positions */
    float cy = item_y + item_height * 0.5f;
    float content_x = item_x + IUI_LIST_PADDING_H;
    float text_x = content_x;
    float trailing_x = item_x + item_width - IUI_LIST_PADDING_H;

    /* Determine text colors */
    uint32_t headline_color =
        item->disabled
            ? iui_state_layer(ctx->colors.on_surface, IUI_STATE_DISABLE_ALPHA)
            : ctx->colors.on_surface;
    uint32_t supporting_color =
        item->disabled ? iui_state_layer(ctx->colors.on_surface_variant,
                                         IUI_STATE_DISABLE_ALPHA)
                       : ctx->colors.on_surface_variant;

    /* Draw leading element */
    bool leading_clicked = false;
    if (item->leading_type != IUI_LIST_LEADING_NONE) {
        draw_leading_element(ctx, item, content_x, cy, headline_color,
                             &leading_clicked);
        /* Adjust text position based on leading element */
        if (item->leading_type == IUI_LIST_LEADING_AVATAR)
            text_x = content_x + IUI_LIST_AVATAR_SIZE + IUI_LIST_PADDING_H;
        else if (item->leading_type == IUI_LIST_LEADING_IMAGE)
            text_x = content_x + IUI_LIST_ONE_LINE_HEIGHT + IUI_LIST_PADDING_H;
        else
            text_x = content_x + IUI_LIST_TEXT_INDENT;
    }

    /* Draw trailing element */
    bool trailing_clicked = false;
    if (item->trailing_type != IUI_LIST_TRAILING_NONE) {
        draw_trailing_element(ctx, item, trailing_x, cy, supporting_color,
                              &trailing_clicked);
        /* Reserve space for trailing element */
        trailing_x -= IUI_LIST_TRAILING_PADDING;
    }

    /* Calculate text area width (reserved for future text truncation) */
    (void) (trailing_x - text_x);

    /* Draw text content based on type */
    switch (type) {
    case IUI_LIST_ONE_LINE: {
        /* Single line: headline centered vertically */
        float text_y = cy - ctx->font_height * 0.5f;
        iui_internal_draw_text(ctx, text_x, text_y, item->headline,
                               headline_color);
        break;
    }

    case IUI_LIST_TWO_LINE: {
        /* Two lines: headline and supporting */
        float line_gap = 4.f;
        float total_text_h = ctx->font_height * 2 + line_gap;
        float text_start_y = cy - total_text_h * 0.5f;

        iui_internal_draw_text(ctx, text_x, text_start_y, item->headline,
                               headline_color);

        if (item->supporting)
            iui_internal_draw_text(ctx, text_x,
                                   text_start_y + ctx->font_height + line_gap,
                                   item->supporting, supporting_color);
        break;
    }

    case IUI_LIST_THREE_LINE: {
        /* Three lines: overline (optional), headline, and supporting */
        float line_gap = 4.f;
        float text_start_y;

        if (item->overline) {
            float total_text_h = ctx->font_height * 3 + line_gap * 2;
            text_start_y = cy - total_text_h * 0.5f;

            /* Draw overline (smaller, label style) */
            iui_internal_draw_text(ctx, text_x, text_start_y, item->overline,
                                   supporting_color);
            text_start_y += ctx->font_height + line_gap;
        } else {
            float total_text_h = ctx->font_height * 2 + line_gap;
            text_start_y = cy - total_text_h * 0.5f;
        }

        iui_internal_draw_text(ctx, text_x, text_start_y, item->headline,
                               headline_color);

        if (item->supporting) {
            text_start_y += ctx->font_height + line_gap;
            iui_internal_draw_text(ctx, text_x, text_start_y, item->supporting,
                                   supporting_color);
        }
        break;
    }
    }

    /* Draw divider if requested */
    if (item->show_divider) {
        float divider_x = text_x;
        float divider_y = item_y + item_height - 1.f;
        float divider_w = item_width - (text_x - item_x) - IUI_LIST_PADDING_H;
        ctx->renderer.draw_box(
            (iui_rect_t) {divider_x, divider_y, divider_w, 1.f}, 0.f,
            ctx->colors.outline_variant, ctx->renderer.user);
    }

    /* Advance layout */
    ctx->layout.y += item_height;

    /* Return true if clicked (but not on controls) */
    if (leading_clicked || trailing_clicked)
        return false;
    return state == IUI_STATE_PRESSED && !item->disabled;
}

bool iui_list_item_simple(iui_context *ctx,
                          const char *headline,
                          const char *icon)
{
    iui_list_item item = {
        .headline = headline,
        .leading_type = icon ? IUI_LIST_LEADING_ICON : IUI_LIST_LEADING_NONE,
        .leading_icon = icon,
    };
    return iui_list_item_ex(ctx, IUI_LIST_ONE_LINE, &item);
}

bool iui_list_item_two_line(iui_context *ctx,
                            const char *headline,
                            const char *supporting,
                            const char *icon)
{
    iui_list_item item = {
        .headline = headline,
        .supporting = supporting,
        .leading_type = icon ? IUI_LIST_LEADING_ICON : IUI_LIST_LEADING_NONE,
        .leading_icon = icon,
    };
    return iui_list_item_ex(ctx, IUI_LIST_TWO_LINE, &item);
}

bool iui_list_item_three_line(iui_context *ctx,
                              const char *overline,
                              const char *headline,
                              const char *supporting,
                              const char *icon)
{
    iui_list_item item = {
        .overline = overline,
        .headline = headline,
        .supporting = supporting,
        .leading_type = icon ? IUI_LIST_LEADING_ICON : IUI_LIST_LEADING_NONE,
        .leading_icon = icon,
    };
    return iui_list_item_ex(ctx, IUI_LIST_THREE_LINE, &item);
}

void iui_list_divider(iui_context *ctx)
{
    if (!ctx || !ctx->current_window)
        return;

    float divider_x = ctx->layout.x + IUI_LIST_DIVIDER_INSET;
    float divider_y = ctx->layout.y;
    float divider_w = ctx->layout.width - IUI_LIST_DIVIDER_INSET;

    ctx->renderer.draw_box((iui_rect_t) {divider_x, divider_y, divider_w, 1.f},
                           0.f, ctx->colors.outline_variant,
                           ctx->renderer.user);

    /* Add small vertical spacing */
    ctx->layout.y += 1.f;
}
