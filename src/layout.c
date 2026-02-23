#include <assert.h>
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

/* Box Container Layout */

/* Resolve main-axis sizes for box children */
static void box_resolve_sizes(float container_main,
                              int count,
                              const iui_sizing_t *sizes,
                              float gap,
                              float *out)
{
    float total_gaps = gap * (float) (count > 1 ? count - 1 : 0);
    float available = fmaxf(0.f, container_main - total_gaps);

    /* Pass 1: resolve FIXED and PERCENT, sum GROW weights */
    float used = 0.f, total_grow = 0.f;
    for (int i = 0; i < count; i++) {
        iui_sizing_t s =
            sizes ? sizes[i] : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
        switch (s.type) {
        case IUI_SIZE_FIXED:
            out[i] = s.value;
            used += out[i];
            break;
        case IUI_SIZE_PERCENT:
            out[i] = available * s.value;
            used += out[i];
            break;
        case IUI_SIZE_GROW:
        default:
            out[i] = -1.f; /* sentinel: resolve in pass 2 */
            total_grow += (s.value > 0.f) ? s.value : 1.f;
            break;
        }
    }

    /* Pass 2: distribute remaining to GROW children */
    float remaining = fmaxf(0.f, available - used);
    for (int i = 0; i < count; i++) {
        if (out[i] >= 0.f)
            continue;
        iui_sizing_t s =
            sizes ? sizes[i] : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
        float weight = (s.value > 0.f) ? s.value : 1.f;
        out[i] = (total_grow > 0.f) ? remaining * weight / total_grow : 0.f;
    }

    /* Pass 3: clamp to min/max, then redistribute surplus/deficit among
     * unclamped GROW siblings.  Once a child is frozen at its constraint it
     * stays frozen across iterations (like CSS flexbox freeze semantics).
     * Iterate until no new clamping occurs.
     */
    bool frozen[IUI_MAX_BOX_CHILDREN] = {false};
    for (int iter = 0; iter < count; iter++) {
        float surplus = 0.f; /* space freed by max-clamped children */
        float deficit = 0.f; /* extra space consumed by min-clamped children */
        float grow_pool = 0.f;

        for (int i = 0; i < count; i++) {
            if (frozen[i])
                continue;
            iui_sizing_t s =
                sizes ? sizes[i] : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
            if (s.min > 0.f && out[i] < s.min) {
                deficit += s.min - out[i];
                out[i] = s.min;
                frozen[i] = true;
            }
            if (s.max > 0.f && out[i] > s.max) {
                surplus += out[i] - s.max;
                out[i] = s.max;
                frozen[i] = true;
            }
            if (s.type == IUI_SIZE_GROW && !frozen[i])
                grow_pool += out[i];
        }

        float net = surplus - deficit;
        if (net > 0.001f || net < -0.001f) {
            if (grow_pool > 0.f) {
                /* Distribute proportionally to current sizes */
                for (int i = 0; i < count; i++) {
                    iui_sizing_t s =
                        sizes ? sizes[i]
                              : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
                    if (s.type != IUI_SIZE_GROW || frozen[i])
                        continue;
                    float share = out[i] / grow_pool * net;
                    out[i] = fmaxf(0.f, out[i] + share);
                }
            } else {
                /* All grow children are at zero; distribute by weight */
                float total_w = 0.f;
                for (int i = 0; i < count; i++) {
                    iui_sizing_t s =
                        sizes ? sizes[i]
                              : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
                    if (s.type == IUI_SIZE_GROW && !frozen[i])
                        total_w += (s.value > 0.f) ? s.value : 1.f;
                }
                if (total_w > 0.f) {
                    for (int i = 0; i < count; i++) {
                        iui_sizing_t s =
                            sizes ? sizes[i]
                                  : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
                        if (s.type != IUI_SIZE_GROW || frozen[i])
                            continue;
                        float w = (s.value > 0.f) ? s.value : 1.f;
                        out[i] = fmaxf(0.f, out[i] + w / total_w * net);
                    }
                } else {
                    break; /* no recipients at all */
                }
            }
        } else {
            break; /* stable */
        }
    }
}

/* MD3 spacing tokens */

float iui_spacing_snap(float value)
{
    if (value <= 0.f)
        return 0.f;
    return roundf(value / 4.f) * 4.f;
}

iui_rect_t iui_box_begin(iui_context *ctx, const iui_box_config_t *config)
{
    iui_rect_t empty = {0};
    if (!ctx->current_window || !config || config->child_count <= 0 ||
        config->child_count > IUI_MAX_BOX_CHILDREN)
        return empty;
    if (ctx->box_depth >= IUI_MAX_BOX_DEPTH)
        return empty;

    iui_box_entry_t *e = &ctx->box_stack[ctx->box_depth++];
    e->config = *config;
    e->child_index = 0;
    e->saved_layout = ctx->layout;

    /* Snap gap and padding to 4dp grid */
    float gap = iui_spacing_snap(config->gap);
    float pad_l = iui_spacing_snap(config->padding.left);
    float pad_r = iui_spacing_snap(config->padding.right);
    float pad_t = iui_spacing_snap(config->padding.top);
    float pad_b = iui_spacing_snap(config->padding.bottom);

    /* Container bounds from current layout.
     * cross is the cross-axis size: WIDTH for columns, HEIGHT for rows.
     * box_depth was already incremented, so >1 means nested.
     */
    float cx = ctx->layout.x;
    float cy = ctx->layout.y;
    float cw, ch;

    if (config->direction == IUI_DIR_COLUMN) {
        /* Column: main=vertical, cross=horizontal (width) */
        cw = (config->cross > 0) ? config->cross : ctx->layout.width;
        ch = (ctx->box_depth > 1 || ctx->in_grid)
                 ? ctx->layout.height
                 : (ctx->current_window->pos.y + ctx->current_window->height -
                    cy - ctx->padding * 2.f);
    } else {
        /* Row: main=horizontal, cross=vertical (height) */
        cw = ctx->layout.width;
        ch = (config->cross > 0) ? config->cross : ctx->layout.height;
    }

    /* Remember the resolved cross-axis size so box_end advances correctly */
    e->resolved_cross = (config->direction == IUI_DIR_COLUMN) ? cw : ch;

    /* Content area after padding */
    e->origin_x = cx + pad_l;
    e->origin_y = cy + pad_t;
    e->content_w = fmaxf(0.f, cw - pad_l - pad_r);
    e->content_h = fmaxf(0.f, ch - pad_t - pad_b);

    /* Determine container main-axis size */
    float container_main =
        (config->direction == IUI_DIR_COLUMN) ? e->content_h : e->content_w;

    /* Resolve child sizes */
    box_resolve_sizes(container_main, config->child_count, config->sizes, gap,
                      e->computed);

    /* Report required width for auto-sizing windows (row direction).
     * Sum guaranteed minimum widths: FIXED values plus min constraints
     * on GROW/PERCENT children that the solver cannot shrink below.
     */
    if (config->direction == IUI_DIR_ROW) {
        float total_min = 0.f;
        for (int i = 0; i < config->child_count; i++) {
            iui_sizing_t s = config->sizes
                                 ? config->sizes[i]
                                 : (iui_sizing_t) {IUI_SIZE_GROW, 1, 0, 0};
            if (s.type == IUI_SIZE_FIXED) {
                float w = s.value;
                if (s.min > 0.f && s.min > w)
                    w = s.min;
                total_min += w;
            } else if (s.min > 0.f) {
                total_min += s.min;
            }
        }
        if (total_min > 0.f) {
            float req =
                total_min + gap * (config->child_count - 1) + pad_l + pad_r;
            iui_require_content_width(ctx, req);
        }
    }

    /* Initialize cursor */
    e->next_pos =
        (config->direction == IUI_DIR_COLUMN) ? e->origin_y : e->origin_x;
    e->config.gap = gap; /* store snapped value */

    /* Return the overall container rect */
    return (iui_rect_t) {cx, cy, cw, ch};
}

iui_rect_t iui_box_next(iui_context *ctx)
{
    iui_rect_t rect = {0};
    if (ctx->box_depth <= 0)
        return rect;

    iui_box_entry_t *e = &ctx->box_stack[ctx->box_depth - 1];
    if (e->child_index >= e->config.child_count)
        return rect;

    float size = e->computed[e->child_index];
    float cross_total, cross_size, cross_pos;
    bool is_col = (e->config.direction == IUI_DIR_COLUMN);

    if (is_col) {
        cross_total = e->content_w;
        cross_size = cross_total;
        cross_pos = e->origin_x;
    } else {
        cross_total = e->content_h;
        cross_size = cross_total;
        cross_pos = e->origin_y;
    }

    /* Apply cross-axis alignment.
     *
     * In immediate-mode we do not know the child's intrinsic cross size,
     * so START/CENTER/END reduce the cross dimension to the main-axis size
     * (making the slot square-ish) and offset accordingly.  Widgets that
     * know their own height can further refine; STRETCH gives the full
     * cross extent.
     */
    switch (e->config.align) {
    case IUI_CROSS_START:
        cross_size = fminf(size, cross_total);
        break;
    case IUI_CROSS_CENTER:
        cross_size = fminf(size, cross_total);
        cross_pos += (cross_total - cross_size) * 0.5f;
        break;
    case IUI_CROSS_END:
        cross_size = fminf(size, cross_total);
        cross_pos += cross_total - cross_size;
        break;
    case IUI_CROSS_STRETCH:
    default:
        break;
    }

    if (is_col) {
        rect = (iui_rect_t) {cross_pos, e->next_pos, cross_size, size};
    } else {
        rect = (iui_rect_t) {e->next_pos, cross_pos, size, cross_size};
    }

    e->next_pos += size + e->config.gap;
    e->child_index++;

    /* Update ctx->layout so widgets can read it */
    ctx->layout = rect;
    return rect;
}

void iui_box_end(iui_context *ctx)
{
    if (ctx->box_depth <= 0 || !ctx->current_window)
        return;

    iui_box_entry_t *e = &ctx->box_stack[ctx->box_depth - 1];
    ctx->layout = e->saved_layout;

    bool is_col = (e->config.direction == IUI_DIR_COLUMN);
    float pad_t = iui_spacing_snap(e->config.padding.top);
    float pad_b = iui_spacing_snap(e->config.padding.bottom);
    float pad_r = iui_spacing_snap(e->config.padding.right);

    if (is_col) {
        /* origin_y already excludes top padding, so add it back.
         * next_pos includes a trailing gap after the last child; subtract it.
         * If no child was consumed, just advance by padding (no gap to remove).
         */
        float children_h = e->next_pos - e->origin_y;
        if (e->child_index > 0)
            children_h -= e->config.gap;
        float total_h = pad_t + fmaxf(0.f, children_h) + pad_b;
        ctx->layout.y += total_h;
    } else {
        ctx->layout.y += e->resolved_cross + ctx->padding;
        (void) pad_r;
    }

    ctx->box_depth--;
}

int iui_box_depth(const iui_context *ctx)
{
    return ctx->box_depth;
}

/* MD3 Adaptive Layout Queries */

iui_size_class_t iui_size_class(float width)
{
    if (width >= IUI_BREAKPOINT_XLARGE)
        return IUI_SIZE_CLASS_XLARGE;
    if (width >= IUI_BREAKPOINT_LARGE)
        return IUI_SIZE_CLASS_LARGE;
    if (width >= IUI_BREAKPOINT_EXPANDED)
        return IUI_SIZE_CLASS_EXPANDED;
    if (width >= IUI_BREAKPOINT_MEDIUM)
        return IUI_SIZE_CLASS_MEDIUM;
    return IUI_SIZE_CLASS_COMPACT;
}

int iui_layout_columns(iui_size_class_t sc)
{
    switch (sc) {
    case IUI_SIZE_CLASS_COMPACT:
        return IUI_LAYOUT_COLUMNS_COMPACT;
    case IUI_SIZE_CLASS_MEDIUM:
        return IUI_LAYOUT_COLUMNS_MEDIUM;
    default:
        return IUI_LAYOUT_COLUMNS_EXPANDED;
    }
}

float iui_layout_margin(iui_size_class_t sc)
{
    switch (sc) {
    case IUI_SIZE_CLASS_COMPACT:
        return IUI_LAYOUT_MARGIN_COMPACT;
    case IUI_SIZE_CLASS_MEDIUM:
        return IUI_LAYOUT_MARGIN_MEDIUM;
    default:
        return IUI_LAYOUT_MARGIN_EXPANDED;
    }
}

float iui_layout_gutter(iui_size_class_t sc)
{
    switch (sc) {
    case IUI_SIZE_CLASS_COMPACT:
        return IUI_LAYOUT_GUTTER_COMPACT;
    case IUI_SIZE_CLASS_MEDIUM:
        return IUI_LAYOUT_GUTTER_MEDIUM;
    default:
        return IUI_LAYOUT_GUTTER_EXPANDED;
    }
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

    /* Reset box layout state for new frame */
    ctx->box_depth = 0;

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

    /* Push window content clip so nested clips (scroll, banners) intersect
     * with window bounds instead of escaping when depth == 0. */
    iui_rect_t win_clip = {
        .x = ctx->layout.x,
        .y = ctx->layout.y,
        .width = ctx->layout.width,
        .height = w->pos.y + w->height - ctx->padding - ctx->layout.y,
    };
    if (!iui_push_clip(ctx, win_clip)) {
        ctx->current_window = NULL; /* clip overflow: abort window */
        return false;
    }
    return true;
}

void iui_newline(iui_context *ctx)
{
    if (!ctx->current_window)
        return;
    ctx->layout.y += ctx->row_height;
    if (ctx->active_scroll) {
        /* Preserve scroll-constrained width (view_w - scrollbar) and
         * reset x to the scroll-offset-adjusted left boundary. */
        ctx->layout.x =
            ctx->scroll_content_start_x - ctx->active_scroll->scroll_x;
    } else {
        ctx->layout.x = ctx->current_window->pos.x + ctx->padding * 2.f;
        ctx->layout.width = ctx->current_window->width - ctx->padding * 4.f;
    }
}

/* Grid Layout */

iui_rect_t iui_grid_begin(iui_context *ctx,
                          int cols,
                          float cell_w,
                          float cell_h,
                          float pad)
{
    if (!ctx->current_window || cols <= 0)
        return (iui_rect_t) {0};

    /* Report required width for auto-sizing windows */
    float required_width = (float) cols * cell_w + (float) (cols - 1) * pad;
    iui_require_content_width(ctx, required_width);

#ifndef NDEBUG
    /* Skip check for AUTO_SIZE windows: they legitimately start undersized and
     * grow to fit after the first frame via iui_require_content_width. */
    if (!(ctx->current_window->options & IUI_WINDOW_AUTO_SIZE)) {
        assert(required_width <= ctx->layout.width + 0.5f &&
               "grid exceeds layout width; reduce cols/cell_w or widen window");
    }
#endif

    ctx->in_grid = true;
    ctx->grid = (iui_grid_state) {
        .cols = cols,
        .cell_w = cell_w,
        .cell_h = cell_h,
        .pad = pad,
        .start_x = ctx->layout.x,
        .start_y = ctx->layout.y,
        .current_col = 0,
        .saved_layout = ctx->layout,
        .saved_row_height = ctx->row_height,
    };
    ctx->layout.width = cell_w;
    ctx->layout.height = cell_h;
    ctx->row_height = cell_h;
    return ctx->layout;
}

iui_rect_t iui_grid_next(iui_context *ctx)
{
    if (!ctx->in_grid)
        return (iui_rect_t) {0};

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
    return ctx->layout;
}

void iui_grid_end(iui_context *ctx)
{
    if (!ctx->in_grid || !ctx->current_window)
        return;

    ctx->in_grid = false;

    /* Restore saved layout and advance past the grid */
    ctx->layout = ctx->grid.saved_layout;
    ctx->layout.y = ctx->grid.start_y + ctx->grid.cell_h + ctx->grid.pad;
    ctx->row_height = ctx->grid.saved_row_height;
}


iui_rect_t iui_get_layout_rect(const iui_context *ctx)
{
    return ctx->layout;
}

iui_rect_t iui_get_window_rect(const iui_context *ctx)
{
    if (!ctx->current_window)
        return (iui_rect_t) {0, 0, 0, 0};
    return (iui_rect_t) {ctx->current_window->pos.x, ctx->current_window->pos.y,
                         ctx->current_window->width,
                         ctx->current_window->height};
}

float iui_get_remaining_height(const iui_context *ctx)
{
    if (!ctx || !ctx->current_window || ctx->clip.depth == 0)
        return 0.f;
    /* stack[0] is always the window clip pushed by iui_begin_window */
    const iui_rect_t *win_clip = &ctx->clip.stack[0];
    float clip_bottom = win_clip->y + win_clip->height;
    float remaining = clip_bottom - ctx->layout.y;
    return remaining > 0.f ? remaining : 0.f;
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

    ctx->current_window->min_height =
        ctx->layout.y - ctx->current_window->pos.y + ctx->row_height * 2.f;

    /* Update min_width for auto-sizing windows */
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

    /* Assert balanced clips and no abandoned scroll regions.
     * Clear current_window BEFORE iui_pop_clip so the floor guard
     * (depth >= 1 while inside a window) allows this final pop. */
    assert(!ctx->active_scroll &&
           "abandoned scroll region: iui_scroll_end not called inside window");
    assert(ctx->clip.depth == 1 &&
           "unbalanced iui_push_clip/iui_pop_clip inside window");
    ctx->clip.depth = 1;        /* safety: discard any leaked clips */
    ctx->current_window = NULL; /* must precede pop for floor guard bypass */
    iui_pop_clip(ctx);

    /* Reset layout modes to prevent state leaking between windows */
    ctx->in_grid = false;
    ctx->box_depth = 0;
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

    /* Reset scroll state for next frame */
    ctx->scroll_wheel_dx = 0;
    ctx->scroll_wheel_dy = 0;
    ctx->active_scroll = NULL;

    /* Each begin_window/end_window pair must balance its clip push/pop. */
    assert(ctx->clip.depth == 0 && "leaked clip region across frame boundary");
    if (ctx->clip.depth != 0) {
        /* Release recovery: reset both logical and renderer clip state so the
         * next frame starts clean rather than with stale clipping applied. */
        ctx->clip.depth = 0;
        ctx->current_clip = (iui_clip_rect) {0, 0, UINT16_MAX, UINT16_MAX};
        ctx->renderer.set_clip_rect(0, 0, UINT16_MAX, UINT16_MAX,
                                    ctx->renderer.user);
    }

    /* Flush batched draw commands */
    iui_batch_flush(ctx);

    /* Decay text cache hit counts (amortized) */
    iui_text_cache_frame_end(ctx);
}
