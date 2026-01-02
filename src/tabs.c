/* Tabs component implementation */

#include "internal.h"

/* Tab style variants */
typedef enum {
    TAB_STYLE_PRIMARY,   /* Primary tabs with full indicator */
    TAB_STYLE_SECONDARY, /* Secondary tabs with simpler styling */
} tab_style;

static int iui_tabs_internal(iui_context *ctx,
                             int selected,
                             int count,
                             const char **labels,
                             const char **icons,
                             tab_style style)
{
    if (!ctx || !labels || count <= 0)
        return selected;
    if (!ctx->current_window)
        return selected;

    /* Clamp selected to valid range */
    if (selected < 0)
        selected = 0;
    if (selected >= count)
        selected = count - 1;

    /* Calculate tab dimensions */
    float tab_height = IUI_TAB_HEIGHT;
    float container_width = ctx->layout.width;
    float tab_width = container_width / (float) count;

    /* Enforce minimum tab width */
    if (tab_width < IUI_TAB_MIN_WIDTH)
        tab_width = IUI_TAB_MIN_WIDTH;

    /* Report required width for auto-sizing windows */
    float required_width = (float) count * IUI_TAB_MIN_WIDTH;
    iui_require_content_width(ctx, required_width);

    float tabs_x = ctx->layout.x, tabs_y = ctx->layout.y;

    /* Draw container background (surface color) */
    iui_rect_t container_rect = {tabs_x, tabs_y, container_width, tab_height};
    ctx->renderer.draw_box(container_rect, 0.f, ctx->colors.surface,
                           ctx->renderer.user);

    /* Animation state for indicator sliding
     * Use a unique ID based on the labels pointer address for animation
     * tracking
     */
    void *anim_id = (void *) ((uintptr_t) labels ^ 0x7AB5);

    /* Calculate indicator position with animation */
    float indicator_x;
    if (ctx->animation.widget == anim_id && ctx->animation.t < 1.f) {
        indicator_x =
            lerp_float(ctx->animation.value_key0, ctx->animation.value_key1,
                       ease_out_back(ctx->animation.t));
    } else {
        indicator_x = tabs_x + tab_width * selected;
    }

    /* Draw each tab */
    int new_selected = selected;
    float tab_x = tabs_x;

    for (int i = 0; i < count; i++) {
        if (!labels[i]) {
            tab_x += tab_width;
            continue;
        }

        bool is_selected = (i == selected);
        iui_rect_t tab_rect = {
            .x = tab_x,
            .y = tabs_y,
            .width = tab_width,
            .height = tab_height,
        };

        /* Get component state for hover/press */
        iui_state_t state = iui_get_component_state(ctx, tab_rect, false);

        /* Draw state layer for hover/press (only for non-selected tabs in
         * primary style)
         */
        if (!is_selected && iui_state_is_interactive(state)) {
            uint32_t layer_color = iui_state_layer(ctx->colors.on_surface,
                                                   iui_state_get_alpha(state));
            ctx->renderer.draw_box(tab_rect, 0.f, layer_color,
                                   ctx->renderer.user);
        }

        /* Determine text/icon colors based on selection state */
        uint32_t content_color =
            is_selected ? ctx->colors.primary : ctx->colors.on_surface_variant;

        /* Calculate content position (centered) */
        float text_width = iui_get_text_width(ctx, labels[i]);

        if (icons && icons[i]) {
            /* Tab with icon + label (stacked vertically) */
            float icon_size = IUI_TAB_ICON_SIZE;
            float total_height =
                icon_size + IUI_TAB_ICON_LABEL_GAP + ctx->font_height;
            float content_start_y = tabs_y + (tab_height - total_height) * 0.5f;

            /* Draw icon */
            float icon_cx = tab_x + tab_width * 0.5f,
                  icon_cy = content_start_y + icon_size * 0.5f;
            iui_draw_fab_icon(ctx, icon_cx, icon_cy, icon_size, icons[i],
                              content_color);

            /* Draw label below icon */
            float label_x = tab_x + (tab_width - text_width) * 0.5f,
                  label_y =
                      content_start_y + icon_size + IUI_TAB_ICON_LABEL_GAP;
            iui_internal_draw_text(ctx, label_x, label_y, labels[i],
                                   content_color);
        } else {
            /* Label only (centered vertically) */
            float content_y = tabs_y + tab_height * 0.5f;
            float label_x = tab_x + (tab_width - text_width) * 0.5f,
                  label_y = content_y - ctx->font_height * 0.5f;
            iui_internal_draw_text(ctx, label_x, label_y, labels[i],
                                   content_color);
        }

        /* Handle click to change selection */
        if (state == IUI_STATE_PRESSED && !is_selected) {
            /* Start animation for indicator sliding */
            ctx->animation = (iui_animation) {
                .value_key0 = tabs_x + tab_width * selected,
                .value_key1 = tabs_x + tab_width * i,
                .widget = anim_id,
                .t = 0.f,
            };
            new_selected = i;
        }

        tab_x += tab_width;
    }

    /* Draw active indicator (3dp height, primary color) */
    float indicator_width = tab_width;
    float indicator_height = IUI_TAB_INDICATOR_HEIGHT;
    float indicator_y = tabs_y + tab_height - indicator_height;

    if (style == TAB_STYLE_PRIMARY) {
        /* Primary tabs: full-width indicator with rounded corners */
        iui_rect_t indicator_rect = {indicator_x, indicator_y, indicator_width,
                                     indicator_height};
        ctx->renderer.draw_box(indicator_rect, indicator_height * 0.5f,
                               ctx->colors.primary, ctx->renderer.user);
    } else {
        /* Secondary tabs: shorter indicator centered under tab */
        float short_indicator_width = indicator_width * 0.6f;
        float short_indicator_x =
            indicator_x + (indicator_width - short_indicator_width) * 0.5f;
        iui_rect_t short_indicator_rect = {short_indicator_x, indicator_y,
                                           short_indicator_width,
                                           indicator_height};
        ctx->renderer.draw_box(short_indicator_rect, indicator_height * 0.5f,
                               ctx->colors.primary, ctx->renderer.user);
    }

    /* Draw bottom divider line (outline_variant color) */
    iui_rect_t divider_rect = {tabs_x, tabs_y + tab_height - 1.f,
                               container_width, 1.f};
    ctx->renderer.draw_box(divider_rect, 0.f, ctx->colors.outline_variant,
                           ctx->renderer.user);

    /* Advance layout cursor */
    ctx->layout.y += tab_height + ctx->padding;

    return new_selected;
}

/* Primary Tabs (label only) */
int iui_tabs(iui_context *ctx, int selected, int count, const char **labels)
{
    return iui_tabs_internal(ctx, selected, count, labels, NULL,
                             TAB_STYLE_PRIMARY);
}

/* Primary Tabs with Icons */
int iui_tabs_with_icons(iui_context *ctx,
                        int selected,
                        int count,
                        const char **labels,
                        const char **icons)
{
    return iui_tabs_internal(ctx, selected, count, labels, icons,
                             TAB_STYLE_PRIMARY);
}

/* Secondary Tabs */
int iui_tabs_secondary(iui_context *ctx,
                       int selected,
                       int count,
                       const char **labels)
{
    return iui_tabs_internal(ctx, selected, count, labels, NULL,
                             TAB_STYLE_SECONDARY);
}
