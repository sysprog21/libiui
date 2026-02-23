/* Navigation component implementation
 * Reference: https://m3.material.io/components/navigation-rail
 *            https://m3.material.io/components/navigation-bar
 *            https://m3.material.io/components/navigation-drawer
 */

#include "internal.h"

/* Navigation Rail */

void iui_nav_rail_begin(iui_context *ctx,
                        iui_nav_rail_state *state,
                        float x,
                        float y,
                        float height)
{
    if (!ctx || !state)
        return;

    state->x = x;
    state->y = y;
    state->height = height;
    state->item_count = 0;

    float width =
        state->expanded ? IUI_NAV_RAIL_EXPANDED_WIDTH : IUI_NAV_RAIL_WIDTH;

    /* Draw rail background */
    iui_rect_t rail_rect = {x, y, width, height};
    ctx->renderer.draw_box(rail_rect, 0.f, ctx->colors.surface,
                           ctx->renderer.user);

    /* Push clip for rail content */
    iui_push_clip(ctx, rail_rect);
}

bool iui_nav_rail_fab(iui_context *ctx,
                      iui_nav_rail_state *state,
                      const char *icon)
{
    if (!ctx || !state || !icon)
        return false;

    float rail_width =
        state->expanded ? IUI_NAV_RAIL_EXPANDED_WIDTH : IUI_NAV_RAIL_WIDTH;
    float fab_size = IUI_FAB_SIZE;
    float fab_x = state->x + (rail_width - fab_size) * 0.5f;
    float fab_y = state->y + 16.f;

    iui_rect_t fab_rect = {fab_x, fab_y, fab_size, fab_size};
    iui_state_t comp_state = iui_get_component_state(ctx, fab_rect, false);

    /* Draw FAB background */
    uint32_t fab_bg = ctx->colors.primary_container;
    ctx->renderer.draw_box(fab_rect, IUI_FAB_CORNER_RADIUS, fab_bg,
                           ctx->renderer.user);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, fab_rect, IUI_FAB_CORNER_RADIUS,
                         ctx->colors.on_primary_container, comp_state);

    /* Draw FAB icon */
    float icon_cx = fab_x + fab_size * 0.5f;
    float icon_cy = fab_y + fab_size * 0.5f;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, IUI_FAB_ICON_SIZE, icon,
                      ctx->colors.on_primary_container);

    return comp_state == IUI_STATE_PRESSED;
}

bool iui_nav_rail_item(iui_context *ctx,
                       iui_nav_rail_state *state,
                       const char *icon,
                       const char *label,
                       int index)
{
    if (!ctx || !state || !icon)
        return false;

    float rail_width =
        state->expanded ? IUI_NAV_RAIL_EXPANDED_WIDTH : IUI_NAV_RAIL_WIDTH;

    /* FAB area: 16dp top margin + FAB height + 16dp gap below FAB */
    float fab_area_h = 16.f + IUI_FAB_SIZE + 16.f;
    float item_y =
        state->y + fab_area_h + state->item_count * IUI_NAV_RAIL_ITEM_HEIGHT;
    float item_height = IUI_NAV_RAIL_ITEM_HEIGHT;

    iui_rect_t item_rect = {state->x, item_y, rail_width, item_height};
    iui_state_t comp_state = iui_get_component_state(ctx, item_rect, false);

    bool selected = (state->selected == index);

    /* In collapsed mode the indicator sits at the top of the item (4dp
     * margin) so the label has room below.  In expanded mode the label
     * is inline (right of the icon), so the indicator is centered. */
    float indicator_y;
    if (state->expanded)
        indicator_y =
            item_y + (item_height - IUI_NAV_RAIL_INDICATOR_HEIGHT) * 0.5f;
    else
        indicator_y = item_y + 4.f;

    /* Draw selection indicator (pill shape) */
    if (selected) {
        float indicator_x =
            state->x + (rail_width - IUI_NAV_RAIL_INDICATOR_WIDTH) * 0.5f;

        if (state->expanded) {
            indicator_x = state->x + 12.f;
            float text_width = label ? iui_get_text_width(ctx, label) : 0;
            /* icon column (68dp from indicator_x) + 12dp gap + label + 12dp
             * trailing = IUI_NAV_RAIL_WIDTH + text_width */
            float indicator_w = IUI_NAV_RAIL_WIDTH + text_width;
            iui_rect_t indicator_rect = {indicator_x, indicator_y, indicator_w,
                                         IUI_NAV_RAIL_INDICATOR_HEIGHT};
            ctx->renderer.draw_box(indicator_rect, IUI_NAV_RAIL_CORNER_RADIUS,
                                   ctx->colors.secondary_container,
                                   ctx->renderer.user);
        } else {
            iui_rect_t indicator_rect = {indicator_x, indicator_y,
                                         IUI_NAV_RAIL_INDICATOR_WIDTH,
                                         IUI_NAV_RAIL_INDICATOR_HEIGHT};
            ctx->renderer.draw_box(indicator_rect, IUI_NAV_RAIL_CORNER_RADIUS,
                                   ctx->colors.secondary_container,
                                   ctx->renderer.user);
        }
    }

    /* Draw state layer on hover/press */
    if (iui_state_is_interactive(comp_state) && !selected) {
        uint32_t layer_color = iui_state_layer(ctx->colors.on_surface,
                                               iui_state_get_alpha(comp_state));
        float indicator_x =
            state->x + (rail_width - IUI_NAV_RAIL_INDICATOR_WIDTH) * 0.5f;
        iui_rect_t hover_rect = {indicator_x, indicator_y,
                                 IUI_NAV_RAIL_INDICATOR_WIDTH,
                                 IUI_NAV_RAIL_INDICATOR_HEIGHT};
        ctx->renderer.draw_box(hover_rect, IUI_NAV_RAIL_CORNER_RADIUS,
                               layer_color, ctx->renderer.user);
    }

    /* Draw icon — centered inside the indicator */
    float icon_cx, icon_cy;
    icon_cy = indicator_y + IUI_NAV_RAIL_INDICATOR_HEIGHT * 0.5f;
    if (state->expanded)
        icon_cx = state->x + IUI_NAV_RAIL_WIDTH * 0.5f;
    else
        icon_cx = state->x + rail_width * 0.5f;

    uint32_t icon_color = selected ? ctx->colors.on_secondary_container
                                   : ctx->colors.on_surface_variant;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, IUI_NAV_RAIL_ICON_SIZE, icon,
                      icon_color);

    /* Draw label */
    if (label) {
        uint32_t label_color = selected ? ctx->colors.on_secondary_container
                                        : ctx->colors.on_surface_variant;
        if (state->expanded) {
            /* Label starts after the icon column (IUI_NAV_RAIL_WIDTH wide) */
            float label_x = state->x + IUI_NAV_RAIL_WIDTH;
            float label_y = item_y + (item_height - ctx->font_height) * 0.5f;
            iui_internal_draw_text(ctx, label_x, label_y, label, label_color);
        } else {
            float label_width = iui_get_text_width(ctx, label);
            float label_x = state->x + (rail_width - label_width) * 0.5f;
            /* 4dp gap below the indicator bottom */
            float label_y = indicator_y + IUI_NAV_RAIL_INDICATOR_HEIGHT + 4.f;
            iui_internal_draw_text(ctx, label_x, label_y, label, label_color);
        }
    }

    state->item_count++;

    if (comp_state == IUI_STATE_PRESSED) {
        state->selected = index;
        return true;
    }
    return false;
}

void iui_nav_rail_end(iui_context *ctx, iui_nav_rail_state *state)
{
    if (!ctx || !state)
        return;

    iui_pop_clip(ctx);
}

void iui_nav_rail_toggle(iui_nav_rail_state *state)
{
    if (state)
        state->expanded = !state->expanded;
}

/* Navigation Bar */

void iui_nav_bar_begin(iui_context *ctx,
                       iui_nav_bar_state *state,
                       float x,
                       float y,
                       float width,
                       int total_items)
{
    if (!ctx || !state)
        return;

    state->x = x;
    state->y = y;
    state->width = width;
    state->item_count = 0;
    state->total_items = (total_items > 0) ? total_items : 5; /* default 5 */

    /* Draw bar background */
    iui_rect_t bar_rect = {x, y, width, IUI_NAV_BAR_HEIGHT};
    ctx->renderer.draw_box(bar_rect, 0.f, ctx->colors.surface_container,
                           ctx->renderer.user);

    /* Push clip for bar content */
    iui_push_clip(ctx, bar_rect);
}

bool iui_nav_bar_item(iui_context *ctx,
                      iui_nav_bar_state *state,
                      const char *icon,
                      const char *label,
                      int index)
{
    if (!ctx || !state || !icon)
        return false;

    /* Calculate item width (evenly distributed across total items) */
    int n = (state->total_items > 0) ? state->total_items : 5;
    float item_width = state->width / (float) n;
    if (item_width < 64.f)
        item_width = 64.f;

    float item_x = state->x + state->item_count * item_width;
    float item_y = state->y;

    iui_rect_t item_rect = {item_x, item_y, item_width, IUI_NAV_BAR_HEIGHT};
    iui_state_t comp_state = iui_get_component_state(ctx, item_rect, false);

    bool selected = (state->selected == index);

    /* Calculate vertical positions */
    float indicator_y = item_y + 12.f;
    float icon_cy = indicator_y + IUI_NAV_BAR_INDICATOR_HEIGHT * 0.5f;
    float label_y =
        item_y + 12.f + IUI_NAV_BAR_INDICATOR_HEIGHT + IUI_NAV_BAR_LABEL_GAP;

    /* Draw selection indicator */
    if (selected) {
        float indicator_x =
            item_x + (item_width - IUI_NAV_BAR_INDICATOR_WIDTH) * 0.5f;
        iui_rect_t indicator_rect = {indicator_x, indicator_y,
                                     IUI_NAV_BAR_INDICATOR_WIDTH,
                                     IUI_NAV_BAR_INDICATOR_HEIGHT};
        ctx->renderer.draw_box(
            indicator_rect, IUI_NAV_BAR_INDICATOR_HEIGHT * 0.5f,
            ctx->colors.secondary_container, ctx->renderer.user);
    }

    /* Draw state layer on hover/press */
    if (iui_state_is_interactive(comp_state) && !selected) {
        uint32_t layer_color = iui_state_layer(ctx->colors.on_surface,
                                               iui_state_get_alpha(comp_state));
        float indicator_x =
            item_x + (item_width - IUI_NAV_BAR_INDICATOR_WIDTH) * 0.5f;
        iui_rect_t hover_rect = {indicator_x, indicator_y,
                                 IUI_NAV_BAR_INDICATOR_WIDTH,
                                 IUI_NAV_BAR_INDICATOR_HEIGHT};
        ctx->renderer.draw_box(hover_rect, IUI_NAV_BAR_INDICATOR_HEIGHT * 0.5f,
                               layer_color, ctx->renderer.user);
    }

    /* Draw icon */
    float icon_cx = item_x + item_width * 0.5f;
    uint32_t icon_color = selected ? ctx->colors.on_secondary_container
                                   : ctx->colors.on_surface_variant;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, IUI_NAV_BAR_ICON_SIZE, icon,
                      icon_color);

    /* Draw label */
    if (label) {
        float label_width = iui_get_text_width(ctx, label);
        float label_x = item_x + (item_width - label_width) * 0.5f;
        uint32_t label_color =
            selected ? ctx->colors.on_surface : ctx->colors.on_surface_variant;
        iui_internal_draw_text(ctx, label_x, label_y, label, label_color);
    }

    state->item_count++;

    if (comp_state == IUI_STATE_PRESSED) {
        state->selected = index;
        return true;
    }
    return false;
}

void iui_nav_bar_end(iui_context *ctx, iui_nav_bar_state *state)
{
    if (!ctx || !state)
        return;

    iui_pop_clip(ctx);
}

/* Navigation Drawer */

void iui_nav_drawer_open(iui_nav_drawer_state *state)
{
    if (state)
        state->open = true;
}

void iui_nav_drawer_close(iui_nav_drawer_state *state)
{
    if (state)
        state->open = false;
}

bool iui_nav_drawer_begin(iui_context *ctx,
                          iui_nav_drawer_state *state,
                          float x,
                          float y,
                          float height)
{
    if (!ctx || !state)
        return false;

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

    state->x = x;
    state->y = y;
    state->height = height;
    state->item_count = 0;

    float drawer_width = IUI_NAV_DRAWER_WIDTH;
    float animated_x = x - drawer_width * (1.0f - state->anim_progress);

    /* Draw scrim for modal drawer */
    if (state->modal && state->anim_progress > 0.0f) {
        iui_rect_t screen_rect = {0, 0, 10000.f, height};
        uint8_t scrim_alpha =
            (uint8_t) (IUI_SCRIM_ALPHA * state->anim_progress);
        ctx->renderer.draw_box(
            screen_rect, 0.f,
            (scrim_alpha << 24) | (ctx->colors.scrim & 0x00FFFFFF),
            ctx->renderer.user);

        /* Push modal layer for input blocking */
        iui_push_layer(ctx, 100);

        /* Handle click outside to close */
        iui_rect_t drawer_rect = {animated_x, y, drawer_width, height};
        if (ctx->mouse_pressed && !in_rect(&drawer_rect, ctx->mouse_pos)) {
            state->open = false;
        }
    }

    /* Draw drawer background */
    iui_rect_t drawer_rect = {animated_x, y, drawer_width, height};
    ctx->renderer.draw_box(drawer_rect, 0.f, ctx->colors.surface,
                           ctx->renderer.user);

    /* Register blocking region */
    iui_register_blocking_region(ctx, drawer_rect);

    /* Push clip for drawer content */
    iui_push_clip(ctx, drawer_rect);

    /* Store animated x for items */
    state->x = animated_x;

    return true;
}

bool iui_nav_drawer_item(iui_context *ctx,
                         iui_nav_drawer_state *state,
                         const char *icon,
                         const char *label,
                         int index)
{
    if (!ctx || !state)
        return false;

    float item_y = state->y + IUI_NAV_DRAWER_PADDING_H +
                   state->item_count * IUI_NAV_DRAWER_ITEM_HEIGHT;
    float item_width = IUI_NAV_DRAWER_WIDTH - IUI_NAV_DRAWER_PADDING_H * 2;
    float item_x = state->x + IUI_NAV_DRAWER_PADDING_H;

    iui_rect_t item_rect = {item_x, item_y, item_width,
                            IUI_NAV_DRAWER_ITEM_HEIGHT};
    iui_state_t comp_state = iui_get_component_state(ctx, item_rect, false);

    bool selected = (state->selected == index);

    /* Draw selection or hover background */
    if (selected) {
        ctx->renderer.draw_box(item_rect, 28.f, ctx->colors.secondary_container,
                               ctx->renderer.user);
    } else if (iui_state_is_interactive(comp_state)) {
        uint32_t layer_color = iui_state_layer(ctx->colors.on_surface,
                                               iui_state_get_alpha(comp_state));
        ctx->renderer.draw_box(item_rect, 28.f, layer_color,
                               ctx->renderer.user);
    }

    /* Draw icon */
    if (icon) {
        float icon_x =
            item_x + IUI_NAV_DRAWER_PADDING_H + IUI_LIST_ICON_SIZE * 0.5f;
        float icon_cy = item_y + IUI_NAV_DRAWER_ITEM_HEIGHT * 0.5f;
        uint32_t icon_color = selected ? ctx->colors.on_secondary_container
                                       : ctx->colors.on_surface_variant;
        iui_draw_fab_icon(ctx, icon_x, icon_cy, IUI_LIST_ICON_SIZE, icon,
                          icon_color);
    }

    /* Draw label */
    if (label) {
        float label_x = item_x + IUI_NAV_DRAWER_PADDING_H + IUI_LIST_ICON_SIZE +
                        IUI_NAV_DRAWER_ICON_GAP;
        float label_y =
            item_y + (IUI_NAV_DRAWER_ITEM_HEIGHT - ctx->font_height) * 0.5f;
        uint32_t label_color = selected ? ctx->colors.on_secondary_container
                                        : ctx->colors.on_surface;
        iui_internal_draw_text(ctx, label_x, label_y, label, label_color);
    }

    state->item_count++;

    if (comp_state == IUI_STATE_PRESSED) {
        state->selected = index;
        if (state->modal)
            state->open = false;
        return true;
    }
    return false;
}

void iui_nav_drawer_divider(iui_context *ctx)
{
    if (!ctx || !ctx->current_window)
        return;

    /* MD3 drawer divider: 1dp line with 8dp vertical padding on each side */
    ctx->layout.y += 8.f;
    float x = ctx->layout.x + IUI_NAV_DRAWER_PADDING_H;
    float w = IUI_NAV_DRAWER_WIDTH - 2 * IUI_NAV_DRAWER_PADDING_H;
    if (w > 0.f) {
        ctx->renderer.draw_box((iui_rect_t) {x, ctx->layout.y, w, 1.f}, 0.f,
                               ctx->colors.outline_variant, ctx->renderer.user);
    }
    ctx->layout.y += 1.f + 8.f;
}

void iui_nav_drawer_end(iui_context *ctx, iui_nav_drawer_state *state)
{
    if (!ctx || !state)
        return;

    iui_pop_clip(ctx);

    /* Pop modal layer if used */
    if (state->modal && state->anim_progress > 0.0f) {
        iui_pop_layer(ctx);
    }
}

/* Bottom App Bar
 * Reference: https://m3.material.io/components/bottom-app-bar
 */

void iui_bottom_app_bar_begin(iui_context *ctx,
                              iui_bottom_app_bar_state *state,
                              float x,
                              float y,
                              float width)
{
    if (!ctx || !state)
        return;

    state->x = x;
    state->y = y;
    state->width = width;
    state->action_count = 0;

    /* Draw bar background */
    iui_rect_t bar_rect = {x, y, width, IUI_BOTTOM_APP_BAR_HEIGHT};
    ctx->renderer.draw_box(bar_rect, 0.f, ctx->colors.surface_container,
                           ctx->renderer.user);

    /* Push clip for bar content */
    iui_push_clip(ctx, bar_rect);
}

bool iui_bottom_app_bar_action(iui_context *ctx,
                               iui_bottom_app_bar_state *state,
                               const char *icon)
{
    if (!ctx || !state || !icon)
        return false;

    /* Calculate action position (left side, with padding) */
    float container_size = IUI_BOTTOM_APP_BAR_ICON_CONTAINER_SIZE;
    float action_x = state->x + IUI_BOTTOM_APP_BAR_ICON_PADDING +
                     state->action_count *
                         (container_size + IUI_BOTTOM_APP_BAR_ICON_PADDING);
    float action_y =
        state->y + (IUI_BOTTOM_APP_BAR_HEIGHT - container_size) * 0.5f;

    iui_rect_t action_rect = {action_x, action_y, container_size,
                              container_size};
    iui_state_t comp_state = iui_get_component_state(ctx, action_rect, false);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, action_rect, container_size * 0.5f,
                         ctx->colors.on_surface, comp_state);

    /* Draw icon */
    float icon_cx = action_x + container_size * 0.5f;
    float icon_cy = action_y + container_size * 0.5f;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, IUI_BOTTOM_APP_BAR_ICON_SIZE, icon,
                      ctx->colors.on_surface_variant);

    state->action_count++;

    return comp_state == IUI_STATE_PRESSED;
}

bool iui_bottom_app_bar_fab(iui_context *ctx,
                            iui_bottom_app_bar_state *state,
                            const char *icon,
                            iui_fab_size_t size)
{
    if (!ctx || !state || !icon)
        return false;

    /* FAB positioned on the right side */
    float fab_size, corner_radius, icon_size;

    switch (size) {
    case IUI_FAB_SMALL:
        fab_size = 40.f;
        corner_radius = 12.f;
        icon_size = 24.f;
        break;
    case IUI_FAB_LARGE:
        fab_size = IUI_FAB_LARGE_SIZE;
        corner_radius = IUI_FAB_LARGE_CORNER_RADIUS;
        icon_size = IUI_FAB_LARGE_ICON_SIZE;
        break;
    case IUI_FAB_STANDARD:
    default:
        fab_size = IUI_FAB_SIZE;
        corner_radius = IUI_FAB_CORNER_RADIUS;
        icon_size = IUI_FAB_ICON_SIZE;
        break;
    }

    float fab_x =
        state->x + state->width - fab_size - IUI_BOTTOM_APP_BAR_FAB_OFFSET;
    float fab_y = state->y + (IUI_BOTTOM_APP_BAR_HEIGHT - fab_size) * 0.5f;

    iui_rect_t fab_rect = {fab_x, fab_y, fab_size, fab_size};
    iui_state_t comp_state = iui_get_component_state(ctx, fab_rect, false);

    /* Draw FAB background */
    ctx->renderer.draw_box(fab_rect, corner_radius,
                           ctx->colors.primary_container, ctx->renderer.user);

    /* Draw state layer for hover/press */
    iui_draw_state_layer(ctx, fab_rect, corner_radius,
                         ctx->colors.on_primary_container, comp_state);

    /* Draw FAB icon */
    float icon_cx = fab_x + fab_size * 0.5f;
    float icon_cy = fab_y + fab_size * 0.5f;
    iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icon,
                      ctx->colors.on_primary_container);

    return comp_state == IUI_STATE_PRESSED;
}

void iui_bottom_app_bar_end(iui_context *ctx, iui_bottom_app_bar_state *state)
{
    if (!ctx || !state)
        return;

    iui_pop_clip(ctx);
}
