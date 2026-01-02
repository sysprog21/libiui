#include "internal.h"

/* Dirty rectangle tracking */

static bool dirty_rects_intersect(const iui_rect_t *a, const iui_rect_t *b)
{
    return !(a->x + a->width < b->x || b->x + b->width < a->x ||
             a->y + a->height < b->y || b->y + b->height < a->y);
}

static iui_rect_t dirty_rect_union(const iui_rect_t *a, const iui_rect_t *b)
{
    float min_x = fminf(a->x, b->x);
    float min_y = fminf(a->y, b->y);
    float max_x = fmaxf(a->x + a->width, b->x + b->width);
    float max_y = fmaxf(a->y + a->height, b->y + b->height);
    return (iui_rect_t) {min_x, min_y, max_x - min_x, max_y - min_y};
}

void iui_dirty_init(iui_context *ctx)
{
    if (!ctx)
        return;

    ctx->dirty.count = 0;
    ctx->dirty.full_redraw = true;
    ctx->dirty.enabled = false;
}

void iui_dirty_enable(iui_context *ctx, bool enable)
{
    if (!ctx)
        return;

    ctx->dirty.enabled = enable;
    if (enable)
        ctx->dirty.full_redraw = true;
}

void iui_dirty_mark(iui_context *ctx, iui_rect_t region)
{
    if (!ctx || !ctx->dirty.enabled || ctx->dirty.full_redraw)
        return;

    /* Aggressive merging: consolidate ALL intersecting regions */
    for (int i = 0; i < ctx->dirty.count; i++) {
        if (dirty_rects_intersect(&ctx->dirty.regions[i], &region)) {
            /* Union new region with intersecting one */
            region = dirty_rect_union(&ctx->dirty.regions[i], &region);
            /* Remove old region by swapping with last element */
            ctx->dirty.regions[i] = ctx->dirty.regions[--ctx->dirty.count];
            /* Re-check from start: the larger region may intersect others */
            i = -1;
        }
    }

    if (ctx->dirty.count < IUI_DIRTY_REGION_SIZE)
        ctx->dirty.regions[ctx->dirty.count++] = region;
    else
        ctx->dirty.full_redraw = true;
}

bool iui_dirty_check(const iui_context *ctx, iui_rect_t region)
{
    if (!ctx || !ctx->dirty.enabled || ctx->dirty.full_redraw)
        return true;
    if (ctx->dirty.count == 0)
        return false;

    for (int i = 0; i < ctx->dirty.count; i++) {
        if (dirty_rects_intersect(&ctx->dirty.regions[i], &region))
            return true;
    }
    return false;
}

void iui_dirty_invalidate_all(iui_context *ctx)
{
    if (ctx)
        ctx->dirty.full_redraw = true;
}

int iui_dirty_count(const iui_context *ctx)
{
    if (!ctx)
        return 0;
    return ctx->dirty.full_redraw ? -1 : ctx->dirty.count;
}

/* Row and Layout Management */

void iui_row(iui_context *ctx, int items, const float *widths, float height)
{
    if (!ctx->current_window || items <= 0 || items > IUI_MAX_ROW_ITEMS)
        return;

    /* End previous row if any */
    if (ctx->in_row)
        ctx->layout.y = ctx->row.next_row_y;

    ctx->in_row = true;
    ctx->row.item_count = items;
    ctx->row.item_index = 0;
    ctx->row.height = (height > 0) ? height : ctx->row_height;
    ctx->row.start_x = ctx->layout.x;
    ctx->row.next_row_y = ctx->layout.y + ctx->row.height + ctx->padding;

    /* Calculate widths: negative values mean relative, positive mean absolute
     */
    float available = ctx->layout.width;
    float total_relative = 0.f, total_absolute = 0.f;

    for (int i = 0; i < items; i++) {
        float w = widths ? widths[i] : -1.f;
        if (w < 0)
            total_relative += -w;
        else if (w > 0)
            total_absolute += w;
    }

    /* Report required width for auto-sizing windows (fixed-size items only) */
    if (total_absolute > 0.f)
        iui_require_content_width(ctx, total_absolute);

    float remaining = available - total_absolute;
    if (remaining < 0)
        remaining = 0;

    for (int i = 0; i < items; i++) {
        float w = widths ? widths[i] : -1.f;
        if (w < 0)
            ctx->row.widths[i] =
                (total_relative > 0) ? (remaining * (-w) / total_relative) : 0;
        else if (w > 0)
            ctx->row.widths[i] = w;
        else
            ctx->row.widths[i] = remaining; /* 0 = take all remaining */
    }
}

/* Get the next layout rect in a row, advances to next item */
iui_rect_t iui_layout_next(iui_context *ctx)
{
    iui_rect_t rect = {0};
    if (!ctx->current_window)
        return rect;

    if (ctx->in_row && ctx->row.item_index < ctx->row.item_count) {
        /* Calculate x position */
        float x = ctx->row.start_x;
        for (int i = 0; i < ctx->row.item_index; i++)
            x += ctx->row.widths[i];

        rect.x = x;
        rect.y = ctx->layout.y;
        rect.width = ctx->row.widths[ctx->row.item_index];
        rect.height = ctx->row.height;

        ctx->row.item_index++;

        /* Auto-advance to next row when all items consumed */
        if (ctx->row.item_index >= ctx->row.item_count) {
            ctx->layout.y = ctx->row.next_row_y;
            ctx->in_row = false;
        }
    } else {
        /* Fallback: use full width single-item row */
        rect.x = ctx->layout.x;
        rect.y = ctx->layout.y;
        rect.width = ctx->layout.width;
        rect.height = ctx->row_height;
        ctx->layout.y += ctx->row_height + ctx->padding;
    }

    /* Update ctx->layout for widgets that read it */
    ctx->layout.x = rect.x;
    ctx->layout.width = rect.width;
    ctx->layout.height = rect.height;

    return rect;
}

/* Frame and Window Management */

void iui_begin_frame(iui_context *ctx, float delta_time)
{
    ctx->current_window = NULL;
    /* Store for widget use (e.g., snackbar timer) */
    ctx->delta_time = delta_time;
    ctx->animation.t =
        fminf(1.f, ctx->animation.t + delta_time / IUI_DURATION_SHORT_4);
    ctx->hover.t = fminf(1.f, ctx->hover.t + delta_time / IUI_DURATION_SHORT_2);
    ctx->cursor_blink += delta_time;
    if (ctx->cursor_blink > 1.0f)
        ctx->cursor_blink -= 1.0f;

    /* Per-frame modal state updates */
    if (ctx->modal.active)
        ctx->modal.frames_since_open++;

    /* Input layer system: swap double buffers for new frame */
    iui_input_layer_frame_begin(ctx);

    /* Per-frame field tracking: reset registration arrays */
    iui_field_tracking_frame_begin(ctx);

    /* Only reset clicked_inside when no mouse buttons are held.
     * This preserves click origin tracking across frames during drag
     * operations.
     */
    if (ctx->mouse_held == 0)
        ctx->modal.clicked_inside = false;

    /* Reset focus tracking for new frame */
    ctx->focus_count = 0;
    ctx->focus_index = -1;

    /* Reset app bar state for new frame */
    ctx->appbar_active = false;

    /* Clear focus on mouse click (MD3: focus is keyboard-only) */
    if (ctx->mouse_pressed & IUI_MOUSE_LEFT)
        ctx->focused_widget_id = 0;

    /* NOTE: scroll_wheel_dx/dy are NOT reset here - they are accumulated from
     * events processed BEFORE iui_begin_frame() and consumed by
     * iui_scroll_begin() during the frame. Reset happens in iui_end_frame().
     */

    /* Reset batch command buffer for new frame */
    ctx->batch.count = 0;
}

bool iui_begin_window(iui_context *ctx,
                      const char *name,
                      float x,
                      float y,
                      float width,
                      float height,
                      uint32_t options)
{
    /* Guard: nested window calls not supported */
    if (ctx->current_window)
        return false;

    uint32_t id = iui_hash_str(name);

    /* already created? */
    for (uint32_t i = 0; i < ctx->num_windows; ++i)
        if (ctx->windows[i].id == id)
            ctx->current_window = &ctx->windows[i];

    /* not found, create one */
    if (!ctx->current_window) {
        /* Guard: window limit reached */
        if (ctx->num_windows >= IUI_MAX_WINDOWS)
            return false;

        ctx->current_window = &ctx->windows[ctx->num_windows++];
        *ctx->current_window = (iui_window) {
            .name = name,
            .id = id,
            .pos = {.x = x, .y = y},
            .min_width = iui_get_text_width(ctx, name) + ctx->padding * 2.f,
            .min_height = ctx->row_height * 2.f,
            .width = width,
            .height = height,
            .options = options,
            .closed = false,
        };
    }

    iui_window *w = ctx->current_window;

    /* Reset content width tracker for this frame */
    ctx->window_content_min_width = 0.f;

    /* Apply auto-sizing from previous frame's measurements */
    if (w->options & IUI_WINDOW_AUTO_WIDTH) {
        if (w->min_width > w->width)
            w->width = w->min_width;
    }
    if (w->options & IUI_WINDOW_AUTO_HEIGHT) {
        if (w->min_height > w->height)
            w->height = w->min_height;
    }

    /* resize */
    iui_rect_t handle_rect = {
        w->pos.x + w->width - ctx->corner,
        w->pos.y + w->height - ctx->corner,
        ctx->corner,
        ctx->corner,
    };
    if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
        in_rect(&handle_rect, ctx->mouse_pos) &&
        (w->options & IUI_WINDOW_RESIZABLE)) {
        ctx->resizing_window = w;
        ctx->dragging_offset =
            iui_vec2_sub(iui_vec2_add(w->pos, (iui_vec2) {w->width, w->height}),
                         ctx->mouse_pos);
    }

    if (ctx->mouse_held & IUI_MOUSE_LEFT && ctx->resizing_window == w) {
        w->width = fmaxf(ctx->mouse_pos.x + ctx->dragging_offset.x - w->pos.x,
                         w->min_width);
        w->height = fmaxf(ctx->mouse_pos.y + ctx->dragging_offset.y - w->pos.y,
                          w->min_height);
    }

    iui_rect_t title_rect = {
        w->pos.x + ctx->padding,
        w->pos.y + ctx->padding,
        w->width - ctx->padding * 2.f,
        ctx->row_height,
    };

    /* bring window to front and move if click on the title bar */
    if ((ctx->mouse_pressed & IUI_MOUSE_LEFT) &&
        in_rect(&title_rect, ctx->mouse_pos) &&
        !(w->options & IUI_WINDOW_PINNED) && ctx->resizing_window != w) {
        uint32_t idx = (uint32_t) (w - ctx->windows);
        if (idx + 1u != ctx->num_windows) {
            iui_window tmp = *w;
            memmove(&ctx->windows[idx], &ctx->windows[idx + 1],
                    (ctx->num_windows - idx - 1) * sizeof(iui_window));
            ctx->windows[ctx->num_windows - 1] = tmp;
            ctx->current_window = &ctx->windows[ctx->num_windows - 1];
        }
        ctx->dragging_object = ctx->current_window;
        ctx->dragging_offset =
            iui_vec2_sub(ctx->mouse_pos, ctx->current_window->pos);
    }

    if (ctx->mouse_held & IUI_MOUSE_LEFT && ctx->dragging_object == w)
        w->pos = iui_vec2_sub(ctx->mouse_pos, ctx->dragging_offset);

    /* MD3 Card/Dialog: unified surface_container_high background with outline
     */
    ctx->renderer.draw_box(
        (iui_rect_t) {w->pos.x, w->pos.y, w->width, w->height}, ctx->corner,
        ctx->colors.outline_variant, ctx->renderer.user);
    ctx->renderer.draw_box((iui_rect_t) {w->pos.x + 1.f, w->pos.y + 1.f,
                                         w->width - 2.f, w->height - 2.f},
                           ctx->corner, ctx->colors.surface_container_high,
                           ctx->renderer.user);

    ctx->layout = (iui_rect_t) {
        .x = w->pos.x + ctx->padding,
        .y = title_rect.y + title_rect.height + ctx->padding,
        .width = w->width - ctx->padding * 2.f,
        .height = ctx->row_height,
    };

    ctx->layout.x += ctx->padding;
    ctx->layout.width -= 2.f * ctx->padding;

    /* MD3: Title uses on_surface (headline style) */
    draw_align_text(ctx, &title_rect, w->name, ctx->colors.on_surface,
                    IUI_ALIGN_CENTER);

    /* draw resize handle */
    if (w->options & IUI_WINDOW_RESIZABLE)
        ctx->renderer.draw_box(handle_rect, 0.f, ctx->colors.outline_variant,
                               ctx->renderer.user);

    /* clip rect (use safe clamping to prevent uint16 overflow) */
    uint16_t clip_minx = iui_float_to_u16(ctx->layout.x);
    uint16_t clip_miny = iui_float_to_u16(ctx->layout.y);
    uint16_t clip_maxx = iui_float_to_u16(ctx->layout.x + ctx->layout.width);
    uint16_t clip_maxy = iui_float_to_u16(w->pos.y + w->height - ctx->padding);
    ctx->current_clip =
        (iui_clip_rect) {clip_minx, clip_miny, clip_maxx, clip_maxy};
    ctx->renderer.set_clip_rect(clip_minx, clip_miny, clip_maxx, clip_maxy,
                                ctx->renderer.user);
    return true;
}

void iui_newline(iui_context *ctx)
{
    if (!ctx->current_window)
        return;
    ctx->layout.x = ctx->current_window->pos.x + ctx->padding * 2.f;
    ctx->layout.y += ctx->row_height;
    ctx->layout.width = ctx->current_window->width - ctx->padding * 4.f;
}

/* Grid Layout */

void iui_grid_begin(iui_context *ctx,
                    int cols,
                    float cell_w,
                    float cell_h,
                    float pad)
{
    if (!ctx->current_window || cols <= 0)
        return;

    /* Report required width for auto-sizing windows */
    float required_width = (float) cols * cell_w + (float) (cols - 1) * pad;
    iui_require_content_width(ctx, required_width);

    ctx->in_grid = true;
    ctx->grid = (iui_grid_state) {
        .cols = cols,
        .cell_w = cell_w,
        .cell_h = cell_h,
        .pad = pad,
        .start_x = ctx->layout.x,
        .start_y = ctx->layout.y,
        .current_col = 0,
    };
    ctx->layout.width = cell_w;
    ctx->layout.height = cell_h;
    ctx->row_height = cell_h;
}

void iui_grid_next(iui_context *ctx)
{
    if (!ctx->in_grid)
        return;

    ctx->grid.current_col++;
    if (ctx->grid.current_col >= ctx->grid.cols) {
        ctx->grid.current_col = 0;
        ctx->grid.start_y += ctx->grid.cell_h + ctx->grid.pad;
    }
    ctx->layout.x = ctx->grid.start_x +
                    ctx->grid.current_col * (ctx->grid.cell_w + ctx->grid.pad);
    ctx->layout.y = ctx->grid.start_y;
    ctx->layout.width = ctx->grid.cell_w;
    ctx->layout.height = ctx->grid.cell_h;
}

void iui_grid_end(iui_context *ctx)
{
    if (!ctx->in_grid || !ctx->current_window)
        return;

    ctx->in_grid = false;
    /* Move layout to after the grid */
    ctx->layout.x = ctx->current_window->pos.x + ctx->padding * 2.f;
    ctx->layout.y = ctx->grid.start_y + ctx->grid.cell_h + ctx->grid.pad;
    ctx->layout.width = ctx->current_window->width - ctx->padding * 4.f;
    ctx->row_height = ctx->font_height * 1.5f;
}


/* Flex Layout */

static void iui_flex_init(iui_context *ctx,
                          int items,
                          const float *sizes,
                          float cross,
                          float gap,
                          bool is_column)
{
    if (!ctx->current_window || items <= 0 || items > IUI_MAX_FLEX_ITEMS)
        return;

    if (ctx->in_row) {
        ctx->layout.y = ctx->row.next_row_y;
        ctx->in_row = false;
    }

    ctx->in_flex = true;
    iui_flex_state *f = &ctx->flex;
    f->saved_layout = ctx->layout;
    f->count = items;
    f->index = 0;
    f->gap = gap;
    f->is_column = is_column;
    f->cross = (cross > 0) ? cross : ctx->row_height;
    f->start_x = ctx->layout.x;
    f->start_y = ctx->layout.y;
    f->next_pos = is_column ? f->start_y : f->start_x;
    f->container_main = is_column
                            ? (ctx->current_window->height - ctx->layout.y +
                               ctx->current_window->pos.y - ctx->padding * 2.f)
                            : ctx->layout.width;

    /* Pass 1: sum fixed and flex totals */
    float total_fixed = 0.f, total_flex = 0.f;
    for (int i = 0; i < items; i++) {
        float s = sizes ? sizes[i] : -1.f;
        if (s > 0)
            total_fixed += s;
        else if (s < 0)
            total_flex += -s;
    }

    /* Report required width for auto-sizing windows (horizontal flex only) */
    if (!is_column && total_fixed > 0.f) {
        float required_width = total_fixed + gap * (float) (items - 1);
        iui_require_content_width(ctx, required_width);
    }

    /* Pass 2: distribute remaining space */
    float remaining =
        fmaxf(0.f, f->container_main - total_fixed - gap * (items - 1));
    for (int i = 0; i < items; i++) {
        float s = sizes ? sizes[i] : -1.f;
        if (s > 0)
            f->sizes[i] = s;
        else if (s < 0)
            f->sizes[i] = total_flex > 0 ? remaining * (-s) / total_flex : 0;
        else
            f->sizes[i] = remaining;
    }
}

void iui_flex(iui_context *ctx,
              int items,
              const float *sizes,
              float cross,
              float gap)
{
    iui_flex_init(ctx, items, sizes, cross, gap, false);
}

void iui_flex_column(iui_context *ctx,
                     int items,
                     const float *sizes,
                     float cross,
                     float gap)
{
    iui_flex_init(ctx, items, sizes, cross, gap, true);
}

iui_rect_t iui_flex_next(iui_context *ctx)
{
    iui_rect_t rect = {0};
    if (!ctx->in_flex || !ctx->current_window)
        return rect;
    iui_flex_state *f = &ctx->flex;
    if (f->index >= f->count)
        return rect;

    float size = f->sizes[f->index++];
    if (f->is_column) {
        rect = (iui_rect_t) {f->start_x, f->next_pos, f->cross, size};
    } else {
        rect = (iui_rect_t) {f->next_pos, f->start_y, size, f->cross};
    }
    f->next_pos += size + f->gap;
    ctx->layout = rect;
    return rect;
}

void iui_flex_end(iui_context *ctx)
{
    if (!ctx->in_flex || !ctx->current_window)
        return;
    ctx->in_flex = false;
    iui_flex_state *f = &ctx->flex;
    ctx->layout = f->saved_layout;
    ctx->layout.y +=
        f->is_column ? (f->next_pos - f->start_y) : (f->cross + ctx->padding);
}

const iui_rect_t *iui_layout_get_current(const iui_context *ctx)
{
    return &ctx->layout;
}

iui_rect_t iui_get_window_rect(const iui_context *ctx)
{
    if (!ctx->current_window)
        return (iui_rect_t) {0, 0, 0, 0};
    return (iui_rect_t) {ctx->current_window->pos.x, ctx->current_window->pos.y,
                         ctx->current_window->width,
                         ctx->current_window->height};
}

void iui_require_content_width(iui_context *ctx, float width)
{
    if (!ctx->current_window)
        return;
    if (width > ctx->window_content_min_width)
        ctx->window_content_min_width = width;
}

/* Window and Frame Finalization */

void iui_end_window(iui_context *ctx)
{
    if (!ctx->current_window)
        return;

    /* Update min_height based on content (existing behavior) */
    ctx->current_window->min_height =
        ctx->layout.y - ctx->current_window->pos.y + ctx->row_height * 2.f;

    /* Update min_width based on content requirements (only for auto-sizing) */
    if (ctx->current_window->options &
        (IUI_WINDOW_AUTO_WIDTH | IUI_WINDOW_AUTO_HEIGHT)) {
        float content_min_width =
            ctx->window_content_min_width + ctx->padding * 4.f;
        float title_min_width =
            iui_get_text_width(ctx, ctx->current_window->name) +
            ctx->padding * 2.f;
        ctx->current_window->min_width =
            fmaxf(title_min_width, content_min_width);
    }
    ctx->window_content_min_width = 0.f;

    ctx->current_window = NULL;
    /* Reset layout modes to prevent state leaking between windows */
    ctx->in_row = false;
    ctx->in_grid = false;
    ctx->in_flex = false;
    ctx->current_clip = (iui_clip_rect) {0, 0, UINT16_MAX, UINT16_MAX};
    ctx->renderer.set_clip_rect(0, 0, UINT16_MAX, UINT16_MAX,
                                ctx->renderer.user);
}

void iui_end_frame(iui_context *ctx)
{
    /* Close any unclosed window to prevent state corruption */
    if (ctx->current_window)
        iui_end_window(ctx);

    if (ctx->animation.t >= 1.f) {
        ctx->animation.widget = NULL;
        ctx->animation.t = 1.f;
    }

    if (ctx->mouse_released & IUI_MOUSE_LEFT) {
        ctx->resizing_window = NULL;
        ctx->dragging_object = NULL;
        ctx->scroll_dragging = NULL;
    }

    /* Process keyboard focus navigation (Tab/Shift+Tab) */
    iui_process_focus_navigation(ctx);

    /* Clear stale field state for widgets not rendered this frame */
    iui_field_tracking_frame_end(ctx);

    /* Reset per-frame input edges (event.c) */
    iui_input_frame_begin(ctx);

    /* Reset scroll wheel delta for next frame (consumed by iui_scroll_begin) */
    ctx->scroll_wheel_dx = 0;
    ctx->scroll_wheel_dy = 0;
    ctx->active_scroll = NULL;

    /* Flush batched draw commands */
    iui_batch_flush(ctx);

    /* Decay text cache hit counts (amortized) */
    iui_text_cache_frame_end(ctx);
}
