#include "internal.h"

/* Modal system - Unified with Input Layer System
 *
 * The legacy modal API now wraps the input layer system internally.
 * This provides consistent behavior between modal dialogs and overlay widgets.
 */

/* Default z-order for modals (high priority) */
#define IUI_MODAL_Z_ORDER 1000

void iui_begin_modal(iui_context *ctx, const char *id)
{
    if (!id)
        return;

    uint32_t modal_id = iui_hash_str(id);

    if (ctx->modal.active) {
        /* Modal already active. Allow re-entering the same modal. */
        if (ctx->modal.id == modal_id) {
            ctx->modal.rendering = true; /* Re-enable for this frame */
            return;
        }
        /* Different modal trying to open - the previous modal's component is
         * no longer being rendered (orphaned state). Close it first to allow
         * the new modal to open properly.
         */
        iui_close_modal(ctx);
    }

    /* First time opening this modal */
    ctx->modal.active = true;
    ctx->modal.rendering = true; /* Widgets rendered after this can interact */
    ctx->modal.clicked_inside = false; /* Reset click tracking for new modal */
    /* Start frame counter for close protection */
    ctx->modal.frames_since_open = 0;
    ctx->modal.id = modal_id;

    /* Integrate with input layer system for unified blocking */
    ctx->modal.layer_id = iui_push_layer(ctx, IUI_MODAL_Z_ORDER);
}

void iui_end_modal(iui_context *ctx)
{
    ctx->modal.rendering = false; /* Stop allowing interaction */
    /* Note: modal.active stays true until explicitly closed via
     * iui_close_modal. This allows click-outside-to-close detection. Layer
     * stays active to maintain input blocking.
     */
}

/* Close the modal completely (call when user dismisses the dialog) */
void iui_close_modal(iui_context *ctx)
{
    /* Pop the input layer when modal closes */
    if (ctx->modal.layer_id > 0) {
        iui_pop_layer(ctx);
        ctx->modal.layer_id = 0;
    }

    ctx->modal.active = false;
    ctx->modal.rendering = false;
    ctx->modal.clicked_inside = false;
    ctx->modal.frames_since_open = 0;
    ctx->modal.id = 0;
}

bool iui_is_modal_active(const iui_context *ctx)
{
    return ctx->modal.active;
}

/* Check if a click occurred outside the modal bounds (useful for
 * click-to-dismiss)
 * Returns true only on mouse release when click started outside the modal
 */
bool iui_modal_should_close(const iui_context *ctx)
{
    if (!ctx->modal.active)
        return false;

    /* Don't close in the first frame (protects against opening click release).
     * Since mouse_pressed/released are cleared at end of each frame by
     * iui_input_frame_begin(), 1 frame protection is sufficient.
     */
    if (ctx->modal.frames_since_open < 1)
        return false;

    /* Close only if: modal finished rendering, click wasn't inside modal, and
     * mouse released
     */
    return !ctx->modal.rendering && !ctx->modal.clicked_inside &&
           (ctx->mouse_released & IUI_MOUSE_LEFT);
}
