#include "internal.h"

/* MD3 icon drawing functions */

static void iui_draw_icon_search(iui_context *ctx,
                                 float cx,
                                 float cy,
                                 float size,
                                 uint32_t color)
{
    /* Magnifying glass: circle + diagonal handle. */
    float radius = size * 0.35f;
    float handle_len = size * 0.3f;

    iui_draw_circle_soft(ctx, cx - size * 0.1f, cy - size * 0.1f, radius, 0,
                         color, 1.5f);

    float start_x = cx + radius * 0.5f;
    float start_y = cy + radius * 0.5f;
    iui_draw_line_soft(ctx, start_x, start_y, start_x + handle_len,
                       start_y + handle_len, 1.5f, color);
}

static void iui_draw_icon_clear(iui_context *ctx,
                                float cx,
                                float cy,
                                float size,
                                uint32_t color)
{
    /* X mark: two crossed lines */
    float half = size * 0.35f;

    iui_draw_line_soft(ctx, cx - half, cy - half, cx + half, cy + half, 1.5f,
                       color);
    iui_draw_line_soft(ctx, cx - half, cy + half, cx + half, cy - half, 1.5f,
                       color);
}

static void iui_draw_icon_visibility(iui_context *ctx,
                                     float cx,
                                     float cy,
                                     float size,
                                     uint32_t color)
{
    /* Eye: ellipse shape with pupil */
    float w = size * 0.45f, h = size * 0.25f;

    /* Draw eye outline using arcs or fallback */
    iui_draw_arc_soft(ctx, cx, cy + h * 0.5f, w, IUI_PI + 0.5f,
                      2.f * IUI_PI - 0.5f, 1.5f, color);
    iui_draw_arc_soft(ctx, cx, cy - h * 0.5f, w, 0.5f, IUI_PI - 0.5f, 1.5f,
                      color);

    /* Pupil (small filled circle) */
    float pupil_r = size * 0.12f;
    iui_draw_circle_soft(ctx, cx, cy, pupil_r, color, 0, 0);
}

static void iui_draw_icon_visibility_off(iui_context *ctx,
                                         float cx,
                                         float cy,
                                         float size,
                                         uint32_t color)
{
    /* Eye with diagonal slash */
    iui_draw_icon_visibility(ctx, cx, cy, size, color);

    /* Diagonal slash */
    float half = size * 0.4f;
    iui_draw_line_soft(ctx, cx - half, cy + half, cx + half, cy - half, 2.f,
                       color);
}

void iui_draw_icon_check(iui_context *ctx,
                         float cx,
                         float cy,
                         float size,
                         uint32_t color)
{
    /* Checkmark: two connected lines forming a check */
    float w = size * 0.4f, h = size * 0.3f;

    /* Short leg going down-left to bottom */
    iui_draw_line_soft(ctx, cx - w * 0.6f, cy - h * 0.3f, cx - w * 0.1f,
                       cy + h * 0.7f, 2.f, color);
    /* Long leg going up-right */
    iui_draw_line_soft(ctx, cx - w * 0.1f, cy + h * 0.7f, cx + w * 0.6f,
                       cy - h * 0.5f, 2.f, color);
}

static void iui_draw_icon_error(iui_context *ctx,
                                float cx,
                                float cy,
                                float size,
                                uint32_t color)
{
    /* Exclamation mark: vertical line + dot */
    float bar_h = size * 0.35f;
    float dot_r = size * 0.08f;

    /* Vertical bar */
    ctx->renderer.draw_box(
        (iui_rect_t) {cx - 1.5f, cy - bar_h, 3.f, bar_h * 1.4f}, 1.f, color,
        ctx->renderer.user);

    /* Dot below */
    iui_draw_circle_soft(ctx, cx, cy + bar_h * 0.6f, dot_r, color, 0, 0);
}

void iui_draw_textfield_icon(iui_context *ctx,
                             iui_textfield_icon_t icon,
                             float cx,
                             float cy,
                             float size,
                             uint32_t color)
{
    switch (icon) {
    case IUI_TEXTFIELD_ICON_SEARCH:
        iui_draw_icon_search(ctx, cx, cy, size, color);
        break;
    case IUI_TEXTFIELD_ICON_CLEAR:
        iui_draw_icon_clear(ctx, cx, cy, size, color);
        break;
    case IUI_TEXTFIELD_ICON_VISIBILITY:
        iui_draw_icon_visibility(ctx, cx, cy, size, color);
        break;
    case IUI_TEXTFIELD_ICON_VISIBILITY_OFF:
        iui_draw_icon_visibility_off(ctx, cx, cy, size, color);
        break;
    case IUI_TEXTFIELD_ICON_CHECK:
        iui_draw_icon_check(ctx, cx, cy, size, color);
        break;
    case IUI_TEXTFIELD_ICON_ERROR:
        iui_draw_icon_error(ctx, cx, cy, size, color);
        break;
    default:
        break;
    }
}


/* FAB icon drawing */

void iui_draw_fab_icon(iui_context *ctx,
                       float cx,
                       float cy,
                       float size,
                       const char *icon_name,
                       uint32_t color)
{
    if (!icon_name || !ctx)
        return;

    float half = size * 0.5f;
    float stroke = fmaxf(2.f, size / 12.f);

    if (!strcmp(icon_name, "add") || !strcmp(icon_name, "plus")) {
        /* Plus sign: vertical and horizontal lines */
        iui_draw_line_soft(ctx, cx, cy - half, cx, cy + half, stroke, color);
        iui_draw_line_soft(ctx, cx - half, cy, cx + half, cy, stroke, color);
    } else if (!strcmp(icon_name, "edit") || !strcmp(icon_name, "pencil")) {
        /* Pencil: diagonal line with tip */
        /* Main body diagonal */
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy + half * 0.7f,
                           cx + half * 0.5f, cy - half * 0.5f, stroke, color);
        /* Pencil tip */
        iui_draw_line_soft(ctx, cx + half * 0.5f, cy - half * 0.5f,
                           cx + half * 0.7f, cy - half * 0.7f, stroke * 0.8f,
                           color);
        /* Eraser end */
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy + half * 0.7f,
                           cx - half * 0.5f, cy + half * 0.9f, stroke, color);
    } else if (!strcmp(icon_name, "check") || !strcmp(icon_name, "checkmark")) {
        /* Checkmark */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy, cx - half * 0.1f,
                           cy + half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.1f, cy + half * 0.4f,
                           cx + half * 0.5f, cy - half * 0.3f, stroke, color);
    } else if (!strcmp(icon_name, "close") || !strcmp(icon_name, "x")) {
        /* X mark */
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy - half * 0.6f,
                           cx + half * 0.6f, cy + half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy + half * 0.6f,
                           cx + half * 0.6f, cy - half * 0.6f, stroke, color);
    } else if (!strcmp(icon_name, "search")) {
        /* Magnifying glass */
        float radius = size * 0.3f;
        iui_draw_circle_soft(ctx, cx - size * 0.1f, cy - size * 0.1f, radius, 0,
                             color, stroke);

        float handle_start = radius * 0.7f;
        iui_draw_line_soft(ctx, cx + handle_start * 0.5f,
                           cy + handle_start * 0.5f, cx + half * 0.7f,
                           cy + half * 0.7f, stroke, color);
    } else if (!strcmp(icon_name, "favorite") || !strcmp(icon_name, "heart")) {
        /* Heart shape (simplified) */
        float r = size * 0.2f;
        iui_draw_circle_soft(ctx, cx - r * 0.8f, cy - r * 0.3f, r, color, 0, 0);
        iui_draw_circle_soft(ctx, cx + r * 0.8f, cy - r * 0.3f, r, color, 0, 0);

        /* Bottom point */
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy, cx, cy + half * 0.6f,
                           stroke * 1.5f, color);
        iui_draw_line_soft(ctx, cx + half * 0.6f, cy, cx, cy + half * 0.6f,
                           stroke * 1.5f, color);
    } else if (!strcmp(icon_name, "share")) {
        /* Share: arrow pointing up with two branches */
        /* Vertical stem */
        iui_draw_line_soft(ctx, cx, cy - half * 0.6f, cx, cy + half * 0.3f,
                           stroke, color);
        /* Arrow head */
        iui_draw_line_soft(ctx, cx - half * 0.4f, cy - half * 0.2f, cx,
                           cy - half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.4f, cy - half * 0.2f, cx,
                           cy - half * 0.6f, stroke, color);
        /* Base curve (simplified as horizontal) */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy + half * 0.5f,
                           cx + half * 0.5f, cy + half * 0.5f, stroke, color);
    } else if (!strcmp(icon_name, "compose") || !strcmp(icon_name, "mail")) {
        /* Envelope/compose icon */
        /* Envelope outline */
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy - half * 0.4f,
                           cx + half * 0.7f, cy - half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy - half * 0.4f,
                           cx - half * 0.7f, cy + half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.7f, cy - half * 0.4f,
                           cx + half * 0.7f, cy + half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy + half * 0.4f,
                           cx + half * 0.7f, cy + half * 0.4f, stroke, color);
        /* Flap lines */
        iui_draw_line_soft(ctx, cx - half * 0.7f, cy - half * 0.4f, cx,
                           cy + half * 0.1f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.7f, cy - half * 0.4f, cx,
                           cy + half * 0.1f, stroke, color);
    } else if (!strcmp(icon_name, "settings") || !strcmp(icon_name, "gear")) {
        /* Settings gear icon */
        float r = size * 0.25f;
        /* Outer gear */
        iui_draw_circle_soft(ctx, cx, cy, r, 0, color, stroke);
        /* Inner circle */
        iui_draw_circle_soft(ctx, cx, cy, r * 0.4f, color, 0, 0);

        /* Add gear teeth as lines */
        float tr = r * 1.3f;
        for (int i = 0; i < 4; i++) {
            float angle = i * IUI_PI / 2.f;
            float dx = cosf(angle), dy = sinf(angle);
            iui_draw_line_soft(ctx, cx + dx * r, cy + dy * r, cx + dx * tr,
                               cy + dy * tr, stroke * 1.2f, color);
        }
    } else if (!strcmp(icon_name, "menu") || !strcmp(icon_name, "hamburger")) {
        /* Hamburger menu (3 horizontal lines) */
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy - half * 0.4f,
                           cx + half * 0.6f, cy - half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy, cx + half * 0.6f, cy,
                           stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.6f, cy + half * 0.4f,
                           cx + half * 0.6f, cy + half * 0.4f, stroke, color);
    } else if (!strcmp(icon_name, "more_vert")) {
        /* Vertical more dots (3 vertical dots) */
        float dot_r = size * 0.08f;
        iui_draw_circle_soft(ctx, cx, cy - half * 0.4f, dot_r, color, 0, 0);
        iui_draw_circle_soft(ctx, cx, cy, dot_r, color, 0, 0);
        iui_draw_circle_soft(ctx, cx, cy + half * 0.4f, dot_r, color, 0, 0);
    } else if (!strcmp(icon_name, "more_horiz")) {
        /* Horizontal more dots (3 horizontal dots) */
        float dot_r = size * 0.08f;
        iui_draw_circle_soft(ctx, cx - half * 0.4f, cy, dot_r, color, 0, 0);
        iui_draw_circle_soft(ctx, cx, cy, dot_r, color, 0, 0);
        iui_draw_circle_soft(ctx, cx + half * 0.4f, cy, dot_r, color, 0, 0);
    } else if (!strcmp(icon_name, "arrow_back") || !strcmp(icon_name, "back")) {
        /* Left arrow */
        /* Arrow shaft */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy, cx + half * 0.5f, cy,
                           stroke, color);
        /* Arrow head */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy, cx - half * 0.1f,
                           cy - half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy, cx - half * 0.1f,
                           cy + half * 0.4f, stroke, color);
    } else if (!strcmp(icon_name, "arrow_forward") ||
               !strcmp(icon_name, "forward")) {
        /* Right arrow */
        /* Arrow shaft */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy, cx + half * 0.5f, cy,
                           stroke, color);
        /* Arrow head */
        iui_draw_line_soft(ctx, cx + half * 0.5f, cy, cx + half * 0.1f,
                           cy - half * 0.4f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.5f, cy, cx + half * 0.1f,
                           cy + half * 0.4f, stroke, color);
    } else if (!strcmp(icon_name, "bookmark")) {
        /* Bookmark icon */
        /* Left edge */
        iui_draw_line_soft(ctx, cx - half * 0.4f, cy - half * 0.6f,
                           cx - half * 0.4f, cy + half * 0.5f, stroke, color);
        /* Right edge */
        iui_draw_line_soft(ctx, cx + half * 0.4f, cy - half * 0.6f,
                           cx + half * 0.4f, cy + half * 0.5f, stroke, color);
        /* Top edge */
        iui_draw_line_soft(ctx, cx - half * 0.4f, cy - half * 0.6f,
                           cx + half * 0.4f, cy - half * 0.6f, stroke, color);
        /* Bottom V */
        iui_draw_line_soft(ctx, cx - half * 0.4f, cy + half * 0.5f, cx,
                           cy + half * 0.1f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.4f, cy + half * 0.5f, cx,
                           cy + half * 0.1f, stroke, color);
    } else if (!strcmp(icon_name, "delete") || !strcmp(icon_name, "trash")) {
        /* Trash can icon */
        /* Lid */
        iui_draw_line_soft(ctx, cx - half * 0.5f, cy - half * 0.4f,
                           cx + half * 0.5f, cy - half * 0.4f, stroke, color);
        /* Handle */
        iui_draw_line_soft(ctx, cx - half * 0.15f, cy - half * 0.4f,
                           cx - half * 0.15f, cy - half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.15f, cy - half * 0.4f,
                           cx + half * 0.15f, cy - half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.15f, cy - half * 0.6f,
                           cx + half * 0.15f, cy - half * 0.6f, stroke, color);
        /* Can body */
        iui_draw_line_soft(ctx, cx - half * 0.4f, cy - half * 0.4f,
                           cx - half * 0.35f, cy + half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.4f, cy - half * 0.4f,
                           cx + half * 0.35f, cy + half * 0.6f, stroke, color);
        iui_draw_line_soft(ctx, cx - half * 0.35f, cy + half * 0.6f,
                           cx + half * 0.35f, cy + half * 0.6f, stroke, color);
    } else if (!strcmp(icon_name, "arrow_right") ||
               !strcmp(icon_name, "chevron")) {
        /* Right chevron (for submenus) */
        iui_draw_line_soft(ctx, cx - half * 0.2f, cy - half * 0.4f,
                           cx + half * 0.3f, cy, stroke, color);
        iui_draw_line_soft(ctx, cx + half * 0.3f, cy, cx - half * 0.2f,
                           cy + half * 0.4f, stroke, color);
    } else {
        /* Default / "dot": draw a simple filled circle */
        iui_draw_circle_soft(ctx, cx, cy, size * 0.15f, color, 0, 0);
    }
}
