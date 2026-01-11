/*
 * iui_internal.h - Internal shared declarations
 *
 * This header is NOT part of the public API. It provides shared types,
 * structures, and helper functions used across the modular source files.
 *
 * Module dependency graph:
 *   core -> layout -> draw -> icons -> basic/input/container/modal -> complex
 */

#ifndef IUI_INTERNAL_H
#define IUI_INTERNAL_H

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "font.h"
#include "iui.h"
#include "iui_config.h"

/* MD3 Validation - functions always available for tests */
#include "md3-validate.h"

/* Mathematical constants */
#ifndef IUI_PI
#define IUI_PI 3.14159265358979323846f
#endif

/* Performance optimization constants */
#ifndef IUI_DRAW_CMD_BUFFER_SIZE
#define IUI_DRAW_CMD_BUFFER_SIZE 256 /* max draw commands per batch */
#endif
#ifndef IUI_DIRTY_REGION_SIZE
#define IUI_DIRTY_REGION_SIZE 32 /* max dirty regions to track */
#endif
#ifndef IUI_TEXT_CACHE_SIZE
#define IUI_TEXT_CACHE_SIZE 64 /* text width cache entries */
#endif
#ifndef IUI_TEXT_CACHE_PROBE_LEN
#define IUI_TEXT_CACHE_PROBE_LEN 4 /* linear probing length */
#endif
#ifndef IUI_TEXT_CACHE_DECAY_COUNT
#define IUI_TEXT_CACHE_DECAY_COUNT 4 /* entries to age per frame */
#endif

/* Per-frame field tracking constants */
#ifndef IUI_MAX_TRACKED_TEXTFIELDS
#define IUI_MAX_TRACKED_TEXTFIELDS 32 /* max text fields per frame */
#endif
#ifndef IUI_MAX_TRACKED_SLIDERS
#define IUI_MAX_TRACKED_SLIDERS 32 /* max sliders per frame */
#endif

/* Slider state encoding constants (shared by basic.c and core.c).
 * Bit 31 distinguishes drag (0) from animation (1) states.
 * Bits 0-30 store the masked slider_id for identity comparison.
 *
 * These constants MUST be used consistently for:
 *   - iui_register_slider: store masked ID
 *   - iui_slider_is_registered: compare masked IDs
 *   - iui_field_tracking_frame_end: extract and compare masked IDs
 */
#define IUI_SLIDER_ANIM_FLAG 0x80000000u
#define IUI_SLIDER_ID_MASK 0x7FFFFFFFu

/* Mask slider ID to 31 bits, handling zero-ID edge case.
 * If hash is 0 or 0x80000000, masked result would be 0 (treated as "no
 * slider"). We return 1 in that case to ensure valid IDs are always non-zero.
 */
static inline uint32_t iui_slider_masked_id(uint32_t slider_id)
{
    uint32_t masked = slider_id & IUI_SLIDER_ID_MASK;
    return masked ? masked : 1u;
}

/* Font rendering constants
 * Vector font glyph coords are in 1/64 units relative to font_height.
 * IUI_FONT_UNITS_PER_EM defines this scale factor.
 * Runtime guards in core.c protect against invalid (zero/negative) values.
 */
#ifndef IUI_FONT_UNITS_PER_EM
#define IUI_FONT_UNITS_PER_EM 64.f
#endif
#ifndef IUI_FONT_PEN_WIDTH_DIVISOR
/* pen = font_height / 32 (finer strokes) */
#define IUI_FONT_PEN_WIDTH_DIVISOR 32.f
#endif
#ifndef IUI_FONT_PEN_WIDTH_MIN
/* minimum pen width for HiDPI legibility */
#define IUI_FONT_PEN_WIDTH_MIN 0.75f
#endif
#ifndef IUI_FONT_SIDE_BEARING_MIN
#define IUI_FONT_SIDE_BEARING_MIN 0.6f /* minimum margin as scale factor */
#endif
#ifndef IUI_FONT_SIDE_BEARING_EXTRA
#define IUI_FONT_SIDE_BEARING_EXTRA 0.4f /* additional margin */
#endif

/* Touch target expansion helpers
 * MD3 requires minimum 48dp touch targets for accessibility.
 * These helpers expand a rect to meet touch target requirements.
 */
static inline void iui_expand_touch_target_w(iui_rect_t *r, float min_width)
{
    if (r->width < min_width) {
        float expand = (min_width - r->width) * 0.5f;
        r->x -= expand;
        r->width = min_width;
    }
}

static inline void iui_expand_touch_target_h(iui_rect_t *r, float min_height)
{
    if (r->height < min_height) {
        float expand = (min_height - r->height) * 0.5f;
        r->y -= expand;
        r->height = min_height;
    }
}

static inline void iui_expand_touch_target(iui_rect_t *r, float min_size)
{
    iui_expand_touch_target_w(r, min_size);
    iui_expand_touch_target_h(r, min_size);
}

/* State layer helpers (opacity values defined in iui-spec.h) */

/* Check if component state is interactive (hovered or pressed) */
static inline bool iui_state_is_interactive(iui_state_t state)
{
    return state == IUI_STATE_HOVERED || state == IUI_STATE_PRESSED;
}

/* Get state layer alpha for hover/press states (common widget pattern) */
static inline uint8_t iui_state_get_alpha(iui_state_t state)
{
    switch (state) {
    case IUI_STATE_HOVERED:
        return IUI_STATE_HOVER_ALPHA;
    case IUI_STATE_PRESSED:
        return IUI_STATE_PRESS_ALPHA;
    case IUI_STATE_FOCUSED:
        return IUI_STATE_FOCUS_ALPHA;
    case IUI_STATE_DRAGGED:
        return IUI_STATE_DRAG_ALPHA;
    case IUI_STATE_DISABLED:
        return IUI_STATE_DISABLE_ALPHA;
    default:
        return 0;
    }
}

typedef struct {
    float x, y;
} iui_vec2;

typedef struct {
    const char *name;
    uint32_t id;
    iui_vec2 pos;
    float width, height;
    float min_width, min_height;
    uint32_t options;
    bool closed;
} iui_window;

typedef struct {
    float value_key0, value_key1;
    uint32_t color_key0, color_key1;
    bool key0_to_key1;
    float t;
    void *widget;
} iui_animation;

typedef struct {
    void *widget;
    float t;
} iui_hover;

typedef struct {
    iui_rect_t stack[IUI_CLIP_STACK_SIZE];
    int depth;
} iui_clip_state;

/* Blocking region for input layer system */
typedef struct {
    iui_rect_t bounds;      /* Bounds that blocks input */
    int layer_id, z_order;  /* Layer ID and z-order */
    int registration_order; /* Order within frame */
    bool blocks_input;      /* If true, input blocks below */
} iui_block_region_t;

/* Layer stack entry for nested overlays */
typedef struct {
    int layer_id; /* Unique layer ID */
    int z_order;  /* Z-order value (higher = on top) */
} iui_layer_entry_t;

/* Input layer state (internal, managed by context) */
typedef struct {
    /* Double-buffered regions: write to current, read from previous */
    iui_block_region_t regions[2][IUI_MAX_BLOCKING_REGIONS];
    int region_count[2]; /* Count per buffer */
    int current_buffer;  /* 0 or 1, swapped each frame */
    int next_reg_order;  /* Registration order counter */

    /* Layer stack for nested overlays (stores both ID and z_order) */
    iui_layer_entry_t layer_stack[IUI_MAX_INPUT_LAYERS];
    int layer_depth;      /* Current stack depth */
    int next_layer_id;    /* Unique layer ID counter */
    int current_layer_id; /* Active layer ID (0 = base) */
    int current_z_order;  /* Active layer z_order */
} iui_layer_state_t;

/* Modal state structure for blocking input outside modal */
typedef struct {
    bool active;         /* true when any modal is open */
    bool rendering;      /* true when currently drawing widgets inside modal */
    bool clicked_inside; /* true if a click occurred inside the modal */
    int frames_since_open; /* counts frames since modal opened */
    uint32_t id;           /* hash of the modal's identifier */
    int layer_id;          /* input layer ID for unified modal system */
} iui_modal_state;

/* Focus trap state for accessibility
 * Reference: https://www.w3.org/WAI/ARIA/apg/patterns/dialog-modal/
 * When active, focus navigation is constrained to the trapped layer.
 */
typedef struct {
    int layer_id;            /* layer ID to trap focus within (0 = none) */
    int first_focusable_idx; /* first focusable widget index in trap */
    int last_focusable_idx;  /* last focusable widget index in trap */
    bool active;             /* true when focus trap is active */
} iui_focus_trap_state;

/* Input capture state for drag operations
 * Prevents accidental activation when mouse enters component mid-drag.
 */
typedef struct {
    uint32_t owner_id;        /* hash of widget owning capture (0 = none) */
    float bounds_x, bounds_y; /* capture bounds position */
    float bounds_w, bounds_h; /* capture bounds size */
    bool active;              /* true when capture is active */
    bool require_start; /* if true, require mouse down started in bounds */
} iui_input_capture_state;

/* IME (Input Method Editor) state for international text input
 * Reference: raym3 NativeTextInput.h
 */
typedef struct {
    char composing_text[64]; /* text being composed (not yet committed) */
    int composing_cursor;    /* cursor position within composing text */
    bool is_composing;       /* true when IME composition is active */
} iui_ime_state;

typedef struct {
    uint16_t minx, miny, maxx, maxy;
} iui_clip_rect;

typedef struct {
    int cols, current_col;  /* number of columns, current column index */
    float cell_w, cell_h;   /* cell width and height */
    float pad;              /* padding between cells */
    float start_x, start_y; /* starting position */
} iui_grid_state;

typedef struct {
    float widths[IUI_MAX_ROW_ITEMS];
    int item_count, item_index; /* number of items, current item index */
    float height;               /* row height */
    float start_x;              /* starting x position */
    float next_row_y;           /* y position for next row */
} iui_row_state;

typedef struct {
    float sizes[IUI_MAX_FLEX_ITEMS]; /* computed sizes after layout */
    float start_x, start_y;          /* container origin */
    float container_main;            /* container size on main axis */
    float cross, gap, next_pos; /* size on cross axis, spacing between items,
                                   next item position on main axis */
    int count, index;           /* total items, current item index */
    bool is_column;             /* true=column, false=row */
    /* saved layout state for restoration */
    iui_rect_t saved_layout;
} iui_flex_state;

/* Performance optimization structures */

/* Draw command types for batching */
typedef enum iui_draw_cmd_type {
    IUI_CMD_RECT,   /* filled rectangle */
    IUI_CMD_TEXT,   /* text string */
    IUI_CMD_LINE,   /* line segment */
    IUI_CMD_CIRCLE, /* circle (fill and/or stroke) */
    IUI_CMD_ARC,    /* arc segment */
} iui_draw_cmd_type_t;

/* Draw command data */
typedef struct {
    iui_draw_cmd_type_t type;
    union {
        struct {
            float x, y, w, h, radius;
        } rect;
        struct {
            float x, y;
            char text[64]; /* inline text buffer */
        } text;
        struct {
            float x0, y0, x1, y1, width;
        } line;
        struct {
            float cx, cy, radius;
            uint32_t fill_color;
            float stroke_width;
        } circle;
        struct {
            float cx, cy, radius;
            float start_angle, end_angle, width;
        } arc;
    } data;
    uint32_t color;     /* primary color */
    uint32_t color2;    /* secondary color (stroke for circle) */
    iui_clip_rect clip; /* clip rect at time of command */
} iui_draw_cmd;

/* Draw call batch state */
typedef struct {
    iui_draw_cmd commands[IUI_DRAW_CMD_BUFFER_SIZE];
    int count;
    bool enabled;
} iui_draw_batch;

/* Dirty rectangle tracking state */
typedef struct {
    iui_rect_t regions[IUI_DIRTY_REGION_SIZE];
    int count;
    bool full_redraw; /* true = entire screen is dirty */
    bool enabled;
} iui_dirty_state;

/* Text width cache entry */
typedef struct {
    uint32_t hash;    /* 0 = empty slot */
    const char *text; /* pointer for collision detection */
    float width;      /* cached width */
    uint8_t hits;     /* usage counter for eviction */
} iui_text_cache_entry;

/* Text width cache state */
typedef struct {
    iui_text_cache_entry entries[IUI_TEXT_CACHE_SIZE];
    int hits, misses;       /* statistics */
    unsigned int decay_idx; /* next entry to decay (amortized) */
    bool enabled;
} iui_text_cache_state;

/* Per-frame field ID tracking - prevents stale state for conditionally hidden
 * widgets. Text fields and sliders register themselves each frame. State is
 * cleared for fields not seen during the current frame.
 */
typedef struct {
    void *textfield_ids[IUI_MAX_TRACKED_TEXTFIELDS]; /* buffer pointers */
    uint32_t slider_ids[IUI_MAX_TRACKED_SLIDERS];    /* slider hashes */
    int textfield_count;   /* fields seen this frame */
    int slider_count;      /* sliders seen this frame */
    uint32_t frame_number; /* current frame counter */
} iui_field_tracking;

/* Full iui_context definition
 *
 * Layout optimized for cache locality based on access frequency analysis.
 * Fields grouped: HOT (>100) -> WARM (20-100) -> COOL (<20) -> COLD (arrays).
 * Padding minimized by ordering: pointers -> size_t -> int -> float -> bool.
 */

struct iui_context {
    /* HOT PATH - Rendering (accessed every widget) */
    iui_renderer_t renderer;
    iui_theme_t colors;
    iui_rect_t layout;
    iui_window *current_window;
    float font_height, padding, row_height, corner, delta_time;

    /* HOT PATH - Input (accessed every frame) */
    iui_vec2 mouse_pos;
    int key_pressed, char_input;
    uint8_t mouse_pressed, mouse_held, mouse_released, modifiers;

    /* WARM PATH - Modal and Input Layer */
    iui_modal_state modal;
    iui_layer_state_t input_layer;
    iui_input_capture_state input_capture;
    iui_focus_trap_state focus_trap;

    /* WARM PATH - Animation and Interaction */
    iui_animation animation;
    iui_hover hover;
    void *dragging_object;
    iui_vec2 dragging_offset;

    /* WARM PATH - Text Editing (ptr -> size_t -> float to avoid padding) */
    void *focused_edit;
    size_t selection_start;
    float cursor_blink;

    /* WARM PATH - Clipping */
    iui_clip_state clip;
    iui_clip_rect current_clip;

    /* WARM PATH - Focus System (counters only, arrays moved to COLD) */
    uint32_t focused_widget_id;
    int focus_count, focus_index, focus_navigation_direction;

    /* COOL PATH - Layout Systems */
    iui_row_state row;
    iui_grid_state grid;
    iui_flex_state flex;
    float
        window_content_min_width; /* Max content width requirement this frame */

    /* COOL PATH - Layout Mode Flags (grouped to avoid per-struct padding) */
    bool in_row, in_grid, in_flex, focus_navigation_pending, appbar_active;

    /* COOL PATH - Widget-Specific State */
    struct {
        uint32_t active_id;
        float drag_offset, anim_start_x, anim_target_x, anim_t;
    } slider;

    struct {
        uint32_t icon_color;
        int action_count;
        float action_x, bar_y, bar_height;
    } appbar;

    int menu_item_index;
    float menu_prev_height;

    struct {
        bool open;
        float x, y, width;
        int hovered_index;
        int frames_since_open;
        const int *selected;
    } dropdown;

    /* COOL PATH - Scroll State */
    iui_scroll_state *active_scroll;
    iui_scroll_state *scroll_dragging;
    iui_rect_t scroll_viewport;
    float scroll_content_start_x, scroll_content_start_y;
    float scroll_wheel_dx, scroll_wheel_dy, scroll_drag_offset;

    /* COLD PATH - Typography and Token Systems */
    const iui_vector_t *vector;
    iui_typography_scale typography;
    iui_shape_tokens shapes;
    iui_spacing_tokens spacing;
    float pen_width, font_ascent_px, font_descent_px;

    /* COLD PATH - Accessibility */
    iui_a11y_callbacks a11y_callbacks;
    iui_a11y_hint a11y_stack[8];
    int a11y_stack_depth;
    bool a11y_enabled;

    /* COLD PATH - IME and Clipboard */
    iui_ime_state ime;
    iui_clipboard_t clipboard;

    /* COLD PATH - Window Management */
    iui_window windows[IUI_MAX_WINDOWS];
    iui_window *resizing_window;
    uint32_t num_windows;

    /* COLD PATH - Large Arrays (focus navigation, ~1.5KB) */
    uint32_t focus_order[IUI_MAX_FOCUSABLE_WIDGETS];
    iui_rect_t focus_rects[IUI_MAX_FOCUSABLE_WIDGETS];
    float focus_corners[IUI_MAX_FOCUSABLE_WIDGETS];

    /* COLD PATH - ID Stack and String Buffer */
    uint32_t id_stack[IUI_ID_STACK_SIZE];
    int id_stack_index;
    char string_buffer[IUI_STRING_BUFFER_SIZE];

    /* PERFORMANCE SYSTEMS - Optimization Caches */
    iui_dirty_state dirty;
    iui_text_cache_state text_cache;
    iui_draw_batch batch;
    iui_field_tracking field_tracking;
};

/* Inline helper functions */

static inline iui_vec2 iui_vec2_sub(iui_vec2 a, iui_vec2 b)
{
    return (iui_vec2) {a.x - b.x, a.y - b.y};
}

static inline iui_vec2 iui_vec2_add(iui_vec2 a, iui_vec2 b)
{
    return (iui_vec2) {a.x + b.x, a.y + b.y};
}

static inline bool in_rect(const iui_rect_t *rect, iui_vec2 pos)
{
    return ((pos.x >= rect->x) && (pos.x <= rect->x + rect->width) &&
            (pos.y >= rect->y) && (pos.y <= rect->y + rect->height));
}

static inline void expand_rect(iui_rect_t *rect, float amount)
{
    rect->x -= amount, rect->y -= amount;
    rect->width += amount * 2.f, rect->height += amount * 2.f;
}

static inline float lerp_float(float a, float b, float t)
{
    return fmaf(b - a, t, a);
}

static inline float clamp_float(float min_value, float max_value, float f)
{
    return fminf(max_value, fmaxf(min_value, f));
}

/* Easing functions */
static inline float ease_in_quad(float x)
{
    return x * x;
}

static inline float ease_in_cubic(float x)
{
    return x * x * x;
}

static inline float ease_impulse(float x)
{
    return ease_in_cubic(sinf(x * IUI_PI));
}

static inline float ease_in_expo(float x)
{
    return (x == 0.f) ? 0.f : powf(2.f, 10.f * x - 10.f);
}

static inline float ease_out_back(float x)
{
    const float c1 = 0.8f;
    const float c3 = c1 + 1.f;
    return 1.f + c3 * ease_in_cubic(x - 1.f) + c1 * ease_in_quad(x - 1.f);
}

/* Safe string utilities */

/* Safe string copy: copies up to dst_size-1 chars and null-terminates.
 * Returns number of chars copied (excluding null terminator).
 * Prevents buffer overflow in dialog buttons, menu items, etc. */
static inline size_t iui_safe_copy(char *dst,
                                   size_t dst_size,
                                   const char *src,
                                   size_t src_len)
{
    if (!dst || dst_size == 0)
        return 0;
    if (!src || src_len == 0) {
        dst[0] = '\0';
        return 0;
    }
    size_t copy_len = (src_len < dst_size - 1) ? src_len : dst_size - 1;
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
    return copy_len;
}

/* Safe float-to-uint16 conversion with clamping.
 * Prevents overflow when converting large coordinates to clip rect values. */
static inline uint16_t iui_float_to_u16(float v)
{
    if (v <= 0.f)
        return 0;
    if (v >= (float) UINT16_MAX)
        return UINT16_MAX;
    return (uint16_t) (v + 0.5f);
}

/* Core internal functions (iui_core.c) */

/* Hash function (FNV-1a) */
static inline uint32_t iui_hash(const void *data, size_t length)
{
    uint32_t hash = 0x811c9dc5;
    const uint8_t *p = (uint8_t *) data;
    for (size_t i = 0; i < length; ++i)
        hash = (hash ^ *p++) * 0x01000193;
    return hash;
}

/* Hash a null-terminated string for widget IDs */
static inline uint32_t iui_hash_str(const char *str)
{
    return iui_hash(str, strlen(str));
}

/* Generate position-based widget ID safely (avoids multiplication overflow).
 * Uses same *1000 scaling as original but with clamping for large values. */
static inline uint32_t iui_hash_pos(float x, float y)
{
    /* Clamp to prevent overflow: max safe value is UINT32_MAX/1000 ~ 4.29M */
    const float max_coord = 4000000.0f;
    float cx = (x > max_coord) ? max_coord : ((x < 0.f) ? 0.f : x);
    float cy = (y > max_coord) ? max_coord : ((y < 0.f) ? 0.f : y);
    return (uint32_t) (cx * 1000.f) ^ (uint32_t) (cy * 1000.f);
}

/* Generate unique widget ID from label string and bounding rect.
 * Combines label hash with position for stable per-instance identity.
 *
 * Design Rationale:
 *   Position is intentionally included to allow duplicate labels in different
 *   locations (e.g., multiple "OK" buttons in a dialog row). This is a common
 *   immediate-mode UI pattern where the same label appears multiple times.
 *
 * Trade-off:
 *   If widgets move (e.g., list reordering), their IDs change and active state
 *   (focus, press, drag) is lost. For dynamic lists, use iui_push_id()/pop_id()
 *   with a stable index instead of relying on position.
 *
 * Alternative for dynamic content:
 *   iui_push_id(ctx, item_index);
 *   iui_button(ctx, "Delete", ...);  // ID based on stack, not position
 *   iui_pop_id(ctx);
 */
static inline uint32_t iui_widget_id(const char *label, iui_rect_t rect)
{
    return iui_hash_str(label) ^ iui_hash_pos(rect.x, rect.y);
}

/* Color blending - clamp t to [0,1] to prevent integer overflow */
static inline uint32_t lerp_color(uint32_t a, uint32_t b, float t)
{
    /* Clamp t to valid range to prevent overflow in integer math */
    if (t <= 0.f)
        return a;
    if (t >= 1.f)
        return b;
    int tt = (int) (t * 256.f);
    int oneminust = 256 - tt;
    uint32_t A =
        (((a >> 24) & 0xFF) * oneminust + ((b >> 24) & 0xFF) * tt) >> 8;
    uint32_t B =
        (((a >> 16) & 0xFF) * oneminust + ((b >> 16) & 0xFF) * tt) >> 8;
    uint32_t G = (((a >> 8) & 0xFF) * oneminust + ((b >> 8) & 0xFF) * tt) >> 8;
    uint32_t R = (((a >> 0) & 0xFF) * oneminust + ((b >> 0) & 0xFF) * tt) >> 8;
    return (A << 24) | (B << 16) | (G << 8) | R;
}

/* MD3 state layer: overlay base_color with specified alpha (0x00-0xFF) */
static inline uint32_t iui_state_layer(uint32_t base_color, uint8_t alpha)
{
    return (base_color & 0x00FFFFFF) | ((uint32_t) alpha << 24);
}

/* Fixed-point division by 255: (x * 257 + 128) >> 16 gives exact x/255 */
#define IUI_DIV255(x) (((x) * 257 + 128) >> 16)

/* Blend two colors: dst over src with alpha (Porter-Duff "over") */
static inline uint32_t iui_blend_color(uint32_t dst, uint32_t src)
{
    uint32_t sa = (src >> 24) & 0xFF;
    if (sa == 0)
        return dst;
    if (sa == 255)
        return src;

    uint32_t da = (dst >> 24) & 0xFF;
    uint32_t dr = (dst >> 16) & 0xFF;
    uint32_t dg = (dst >> 8) & 0xFF;
    uint32_t db = dst & 0xFF;

    uint32_t sr = (src >> 16) & 0xFF;
    uint32_t sg = (src >> 8) & 0xFF;
    uint32_t sb = src & 0xFF;

    uint32_t inv_sa = 255 - sa;
    uint32_t oa = sa + IUI_DIV255(da * inv_sa);
    if (oa == 0)
        return 0;

    uint32_t da_inv = IUI_DIV255(da * inv_sa);
    uint32_t or = (sr * sa + dr * da_inv) / oa;
    uint32_t og = (sg * sa + dg * da_inv) / oa;
    uint32_t ob = (sb * sa + db * da_inv) / oa;

    return (oa << 24) | (or << 16) | (og << 8) | ob;
}

/* UTF-8 Handling Utilities
 * These functions provide proper multi-byte character handling for cursor
 * positioning, text selection, and word boundary detection.
 */

/* Check if byte is a UTF-8 continuation byte (10xxxxxx) */
static inline bool iui_utf8_is_continuation(unsigned char c)
{
    return (c & 0xC0) == 0x80;
}

/* Get the byte length of a UTF-8 code point from its leading byte.
 * Returns 1 for ASCII, 2-4 for multi-byte, 1 for invalid (treat as single). */
static inline size_t iui_utf8_codepoint_len(unsigned char c)
{
    if ((c & 0x80) == 0)
        return 1; /* ASCII: 0xxxxxxx */
    if ((c & 0xE0) == 0xC0)
        return 2; /* 2-byte: 110xxxxx */
    if ((c & 0xF0) == 0xE0)
        return 3; /* 3-byte: 1110xxxx */
    if ((c & 0xF8) == 0xF0)
        return 4; /* 4-byte: 11110xxx */
    return 1;     /* Invalid: treat as single byte */
}

/* Advance to the next UTF-8 code point boundary.
 * Returns new byte position, clamped to len. */
static inline size_t iui_utf8_next(const char *buffer, size_t pos, size_t len)
{
    if (pos >= len)
        return len;
    size_t cp_len = iui_utf8_codepoint_len((unsigned char) buffer[pos]);
    return (pos + cp_len > len) ? len : pos + cp_len;
}

/* Move to the previous UTF-8 code point boundary.
 * Returns new byte position, or 0 if at start. */
static inline size_t iui_utf8_prev(const char *buffer, size_t pos)
{
    if (pos == 0)
        return 0;
    /* Move back one byte */
    pos--;
    /* Skip continuation bytes to find leading byte */
    while (pos > 0 && iui_utf8_is_continuation((unsigned char) buffer[pos]))
        pos--;
    return pos;
}

/* Decode a UTF-8 code point at position with bounds checking.
 * Returns Unicode code point, or U+FFFD for invalid/truncated sequences. */
static inline uint32_t iui_utf8_decode(const char *buffer,
                                       size_t pos,
                                       size_t len)
{
    if (pos >= len)
        return 0xFFFD; /* Out of bounds */
    unsigned char c = (unsigned char) buffer[pos];
    if ((c & 0x80) == 0)
        return c; /* ASCII */
    if ((c & 0xE0) == 0xC0) {
        if (pos + 1 >= len)
            return 0xFFFD; /* Truncated */
        return ((c & 0x1F) << 6) | (buffer[pos + 1] & 0x3F);
    }
    if ((c & 0xF0) == 0xE0) {
        if (pos + 2 >= len)
            return 0xFFFD; /* Truncated */
        return ((c & 0x0F) << 12) | ((buffer[pos + 1] & 0x3F) << 6) |
               (buffer[pos + 2] & 0x3F);
    }
    if ((c & 0xF8) == 0xF0) {
        if (pos + 3 >= len)
            return 0xFFFD; /* Truncated */
        return ((c & 0x07) << 18) | ((buffer[pos + 1] & 0x3F) << 12) |
               ((buffer[pos + 2] & 0x3F) << 6) | (buffer[pos + 3] & 0x3F);
    }
    return 0xFFFD; /* Invalid lead byte */
}

/* Check if a Unicode code point is a word character (letters, digits, _).
 * Handles basic Latin, extended Latin, and common Unicode letter ranges. */
static inline bool iui_utf8_is_word_char(uint32_t cp)
{
    /* ASCII word characters */
    if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z') ||
        (cp >= '0' && cp <= '9') || cp == '_')
        return true;
    /* Latin Extended-A/B (accented letters) */
    if (cp >= 0x00C0 && cp <= 0x024F)
        return true;
    /* Greek and Coptic */
    if (cp >= 0x0370 && cp <= 0x03FF)
        return true;
    /* Cyrillic */
    if (cp >= 0x0400 && cp <= 0x04FF)
        return true;
    /* CJK Unified Ideographs (Chinese, Japanese, Korean) */
    if (cp >= 0x4E00 && cp <= 0x9FFF)
        return true;
    /* Hiragana */
    if (cp >= 0x3040 && cp <= 0x309F)
        return true;
    /* Katakana */
    if (cp >= 0x30A0 && cp <= 0x30FF)
        return true;
    /* Hangul Syllables */
    if (cp >= 0xAC00 && cp <= 0xD7AF)
        return true;
    return false;
}

/* Count UTF-8 code points in a string (not bytes) */
static inline size_t iui_utf8_strlen(const char *buffer)
{
    size_t count = 0;
    for (; *buffer; buffer++)
        if (!iui_utf8_is_continuation((unsigned char) *buffer))
            count++;
    return count;
}

/* Encode a Unicode code point to UTF-8. Returns bytes written (1-4).
 * Buffer must have at least 4 bytes available.
 * Invalid code points (surrogates, >U+10FFFF) encode as U+FFFD. */
static inline size_t iui_utf8_encode(uint32_t cp, char *out)
{
    /* Reject surrogates (U+D800..U+DFFF) and values > U+10FFFF */
    if ((cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF)
        cp = 0xFFFD; /* Replacement character */
    if (cp < 0x80) {
        out[0] = (char) cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (char) (0xC0 | (cp >> 6));
        out[1] = (char) (0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = (char) (0xE0 | (cp >> 12));
        out[1] = (char) (0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char) (0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = (char) (0xF0 | (cp >> 18));
    out[1] = (char) (0x80 | ((cp >> 12) & 0x3F));
    out[2] = (char) (0x80 | ((cp >> 6) & 0x3F));
    out[3] = (char) (0x80 | (cp & 0x3F));
    return 4;
}

/* Common text edit input processing (reduces code duplication)
 * Returns true if text was modified, also updates cursor position.
 * Handles: char input, backspace, delete, cursor movement, home/end.
 * Caller should clear ctx->key_pressed and ctx->char_input after if needed.
 * UTF-8 aware: cursor movement and deletion respect multi-byte sequences.
 */
static inline bool iui_process_text_input(iui_context *ctx,
                                          char *buffer,
                                          size_t buffer_size,
                                          size_t *cursor,
                                          bool handle_word_nav)
{
    bool modified = false;
    size_t len = strlen(buffer);
    size_t pos = *cursor;

    /* Clamp cursor to valid UTF-8 boundary */
    if (pos > len)
        pos = len;

    /* Ensure cursor is not on a continuation byte */
    while (pos > 0 && iui_utf8_is_continuation((unsigned char) buffer[pos]))
        pos--;
    if (pos != *cursor)
        *cursor = pos;

    /* Character input - support full Unicode via char_input */
    if (ctx->char_input >= 32) {
        char utf8_buf[4];
        size_t cp_len = iui_utf8_encode(ctx->char_input, utf8_buf);
        if (len + cp_len < buffer_size && pos + cp_len < buffer_size) {
            memmove(buffer + pos + cp_len, buffer + pos, len - pos + 1);
            memcpy(buffer + pos, utf8_buf, cp_len);
            *cursor = pos + cp_len;
            modified = true;
        }
    }

    /* Key handling with UTF-8 awareness */
    switch (ctx->key_pressed) {
    case IUI_KEY_BACKSPACE:
        if (pos > 0 && len > 0) {
            /* Find previous code point boundary and delete */
            size_t prev_pos = iui_utf8_prev(buffer, pos);
            memmove(buffer + prev_pos, buffer + pos, len - pos + 1);
            *cursor = prev_pos;
            modified = true;
        }
        break;
    case IUI_KEY_DELETE:
        if (pos < len) {
            /* Find next code point boundary and delete */
            size_t next_pos = iui_utf8_next(buffer, pos, len);
            memmove(buffer + pos, buffer + next_pos, len - next_pos + 1);
            modified = true;
        }
        break;
    case IUI_KEY_LEFT:
        if (pos > 0) {
            if (handle_word_nav && (ctx->modifiers & IUI_MOD_CTRL)) {
                /* Ctrl+Left: skip whitespace then word (UTF-8 aware) */
                while (*cursor > 0) {
                    size_t prev = iui_utf8_prev(buffer, *cursor);
                    uint32_t cp = iui_utf8_decode(buffer, prev, len);
                    if (cp != ' ' && cp != '\t')
                        break;
                    *cursor = prev;
                }
                while (*cursor > 0) {
                    size_t prev = iui_utf8_prev(buffer, *cursor);
                    uint32_t cp = iui_utf8_decode(buffer, prev, len);
                    if (!iui_utf8_is_word_char(cp))
                        break;
                    *cursor = prev;
                }
            } else {
                *cursor = iui_utf8_prev(buffer, pos);
            }
        }
        break;
    case IUI_KEY_RIGHT:
        if (pos < len) {
            if (handle_word_nav && (ctx->modifiers & IUI_MOD_CTRL)) {
                /* Ctrl+Right: skip word then whitespace (UTF-8 aware) */
                while (*cursor < len) {
                    uint32_t cp = iui_utf8_decode(buffer, *cursor, len);
                    if (!iui_utf8_is_word_char(cp))
                        break;
                    *cursor = iui_utf8_next(buffer, *cursor, len);
                }
                while (*cursor < len) {
                    uint32_t cp = iui_utf8_decode(buffer, *cursor, len);
                    if (cp != ' ' && cp != '\t')
                        break;
                    *cursor = iui_utf8_next(buffer, *cursor, len);
                }
            } else {
                *cursor = iui_utf8_next(buffer, pos, len);
            }
        }
        break;
    case IUI_KEY_HOME:
        *cursor = 0;
        break;
    case IUI_KEY_END:
        *cursor = len;
        break;
    default:
        break;
    }

    return modified;
}

/* Text rendering helpers (implemented in iui_draw.c) */
float iui_get_text_width(iui_context *ctx, const char *text);
float iui_get_codepoint_width(iui_context *ctx, uint32_t cp);
void iui_internal_draw_text(iui_context *ctx,
                            float x,
                            float y,
                            const char *text,
                            uint32_t color);
void draw_align_text(iui_context *ctx,
                     const iui_rect_t *rect,
                     const char *text,
                     uint32_t srgb_color,
                     iui_text_alignment_t alignment);

/* Icon rendering helpers (implemented in icons.c) */
void iui_draw_icon_check(iui_context *ctx,
                         float cx,
                         float cy,
                         float size,
                         uint32_t color);
void iui_draw_textfield_icon(iui_context *ctx,
                             iui_textfield_icon_t icon,
                             float cx,
                             float cy,
                             float size,
                             uint32_t color);
void iui_draw_fab_icon(iui_context *ctx,
                       float cx,
                       float cy,
                       float size,
                       const char *icon_name,
                       uint32_t color);

/* Vector font rendering (implemented in core.c) */
void iui_draw_text_vec(iui_context *ctx,
                       float x,
                       float y,
                       const char *text,
                       uint32_t color);
void iui_compute_vector_metrics(float font_height,
                                float *out_ascent_px,
                                float *out_descent_px);
float iui_vector_pen_for_height(float font_height);
float iui_codepoint_width_vec(uint32_t cp, float font_height);

/* Theme globals (defined in iui_core.c) */
extern const iui_theme_t g_theme_light;
extern const iui_theme_t g_theme_dark;

/* Typography, shape, and spacing token presets (defined in iui_core.c) */
extern const iui_typography_scale iui_typography_scale_default;
extern const iui_typography_scale iui_typography_scale_dense;
extern const iui_shape_tokens iui_shape_tokens_default;
extern const iui_shape_tokens iui_shape_tokens_compact;

/* Draw internal functions (iui_draw.c) */

/* Draw a rectangle outline */
void iui_draw_rect_outline(iui_context *ctx,
                           iui_rect_t rect,
                           float width,
                           uint32_t color);

/* Draw MD3 state layer overlay based on component state */
void iui_draw_state_layer(iui_context *ctx,
                          iui_rect_t bounds,
                          float corner_radius,
                          uint32_t content_color,
                          iui_state_t state);

/* Draw focus ring around a rect (called internally by widgets)
 * Uses primary color with 3dp width and 2dp offset per MD3 spec
 */
void iui_draw_focus_ring(iui_context *ctx,
                         iui_rect_t bounds,
                         float corner_radius);

/* Internal drawing helpers with software fallback */
void iui_draw_line_soft(iui_context *ctx,
                        float x0,
                        float y0,
                        float x1,
                        float y1,
                        float width,
                        uint32_t color);
void iui_draw_circle_soft(iui_context *ctx,
                          float cx,
                          float cy,
                          float radius,
                          uint32_t fill_color,
                          uint32_t stroke_color,
                          float stroke_width);
void iui_draw_arc_soft(iui_context *ctx,
                       float cx,
                       float cy,
                       float radius,
                       float start_angle,
                       float end_angle,
                       float width,
                       uint32_t color);

/* Icon drawing functions (iui_icons.c)
 * Icon functions are static within iui_icons.c and only used by textfield/FAB
 */

/* Event system functions (event.c) */

/* Frame start: clear per-frame input state */
void iui_input_frame_begin(iui_context *ctx);

/* Input layer system functions (core.c) */

/* Frame start: swap double buffers and reset registration counter */
void iui_input_layer_frame_begin(iui_context *ctx);

/* Focus system functions (iui_core.c) */

bool iui_register_focusable(iui_context *ctx,
                            uint32_t id,
                            iui_rect_t bounds,
                            float corner);
void iui_process_focus_navigation(iui_context *ctx);
bool iui_widget_is_focused(const iui_context *ctx, uint32_t id);

/* Layout internal functions (iui_layout.c) */

/* iui_flex_init is static within iui_layout.c */

/* Performance optimization functions (draw.c, layout.c) */

/* Draw call batching - internal functions (draw.c)
 * Note: iui_batch_enable(), iui_batch_count() are public, declared in iui.h
 */
void iui_batch_init(iui_context *ctx);
bool iui_batch_is_enabled(const iui_context *ctx);
void iui_batch_flush(iui_context *ctx);
bool iui_batch_add_rect(iui_context *ctx,
                        float x,
                        float y,
                        float w,
                        float h,
                        float radius,
                        uint32_t color);
bool iui_batch_add_text(iui_context *ctx,
                        float x,
                        float y,
                        const char *text,
                        uint32_t color);
bool iui_batch_add_line(iui_context *ctx,
                        float x0,
                        float y0,
                        float x1,
                        float y1,
                        float width,
                        uint32_t color);
bool iui_batch_add_circle(iui_context *ctx,
                          float cx,
                          float cy,
                          float radius,
                          uint32_t fill_color,
                          uint32_t stroke_color,
                          float stroke_width);
bool iui_batch_add_arc(iui_context *ctx,
                       float cx,
                       float cy,
                       float radius,
                       float start_angle,
                       float end_angle,
                       float width,
                       uint32_t color);

/* Dirty rectangle tracking - internal functions (layout.c)
 * Note: iui_dirty_enable/mark/invalidate_all/check/count are public in iui.h
 */
void iui_dirty_init(iui_context *ctx);
bool iui_dirty_is_enabled(const iui_context *ctx);
void iui_dirty_clear(iui_context *ctx);
bool iui_dirty_get_region(const iui_context *ctx, int index, iui_rect_t *out);

/* Text width caching - internal functions (draw.c)
 * Note: iui_text_cache_enable/clear/stats are public, declared in iui.h
 */
void iui_text_cache_init(iui_context *ctx);
bool iui_text_cache_is_enabled(const iui_context *ctx);
bool iui_text_cache_get(iui_context *ctx, const char *text, float *width);
void iui_text_cache_put(iui_context *ctx, const char *text, float width);
void iui_text_cache_frame_end(iui_context *ctx);

/* Date/time, Dialog, and Internal widget implementations */

/* Macro for MD3 typography functions */

#define IUI_DEFINE_TEXT_FUNC(name, size_field)                              \
    void iui_text_##name(iui_context *ctx, iui_text_alignment_t alignment,  \
                         const char *string, ...)                           \
    {                                                                       \
        if (!ctx->current_window || !string)                                \
            return;                                                         \
        va_list args;                                                       \
        va_start(args, string);                                             \
        int written = vsnprintf(ctx->string_buffer, IUI_STRING_BUFFER_SIZE, \
                                string, args);                              \
        if (written >= IUI_STRING_BUFFER_SIZE)                              \
            ctx->string_buffer[IUI_STRING_BUFFER_SIZE - 1] = '\0';          \
        va_end(args);                                                       \
        iui_text_with_size(ctx, alignment, ctx->string_buffer,              \
                           ctx->typography.size_field);                     \
    }

/* Stub macro for typography functions when CONFIG_FEATURE_TYPOGRAPHY disabled.
 * Properly formats variadic arguments and forwards to draw_align_text(). */
#define IUI_DEFINE_TEXT_STUB(name)                                          \
    void iui_text_##name(iui_context *ctx, iui_text_alignment_t a,          \
                         const char *s, ...)                                \
    {                                                                       \
        if (!ctx->current_window || !s)                                     \
            return;                                                         \
        va_list args;                                                       \
        va_start(args, s);                                                  \
        int written =                                                       \
            vsnprintf(ctx->string_buffer, IUI_STRING_BUFFER_SIZE, s, args); \
        if (written >= IUI_STRING_BUFFER_SIZE)                              \
            ctx->string_buffer[IUI_STRING_BUFFER_SIZE - 1] = '\0';          \
        va_end(args);                                                       \
        draw_align_text(ctx, &ctx->layout, ctx->string_buffer,              \
                        ctx->colors.on_surface, a);                         \
    }

/* Note: iui_release_capture() is public, declared in iui.h */

/* Per-frame field tracking functions (core.c) */

/* Reset field tracking for new frame - clears registration arrays */
void iui_field_tracking_frame_begin(iui_context *ctx);

/* Register a text field as active this frame */
void iui_register_textfield(iui_context *ctx, void *buffer);

/* Register a slider as active this frame */
void iui_register_slider(iui_context *ctx, uint32_t slider_id);

/* Clear stale state for fields not seen this frame */
void iui_field_tracking_frame_end(iui_context *ctx);

/* Check if a text field was registered this frame */
bool iui_textfield_is_registered(const iui_context *ctx, const void *buffer);

/* Check if a slider was registered this frame */
bool iui_slider_is_registered(const iui_context *ctx, uint32_t slider_id);

/* MD3 Runtime Validation (Debug Builds)
 * These macros validate that rendered components conform to MD3 specifications.
 * Active only when IUI_MD3_RUNTIME_VALIDATION is defined (debug builds).
 * Violations are reported via callback or stderr and tracked for testing.
 */

#ifdef IUI_MD3_RUNTIME_VALIDATION

/* Maximum tracked components per frame for validation */
#ifndef IUI_MD3_MAX_TRACKED
#define IUI_MD3_MAX_TRACKED 64
#endif

/* Component type identifiers for tracking */
typedef enum {
    IUI_MD3_COMP_BUTTON,
    IUI_MD3_COMP_FAB,
    IUI_MD3_COMP_FAB_LARGE,
    IUI_MD3_COMP_CHIP,
    IUI_MD3_COMP_TEXTFIELD,
    IUI_MD3_COMP_SWITCH,
    IUI_MD3_COMP_SLIDER,
    IUI_MD3_COMP_TAB,
    IUI_MD3_COMP_CHECKBOX,
    IUI_MD3_COMP_RADIO,
    IUI_MD3_COMP_SEGMENTED,
    IUI_MD3_COMP_COUNT
} iui_md3_comp_type_t;

/* Tracked component instance for validation */
typedef struct {
    iui_md3_comp_type_t type;
    iui_rect_t bounds;   /* Rendered bounds */
    float corner_radius; /* Applied corner radius */
    md3_violation_t violations;
} iui_md3_tracked_t;

/* Per-frame MD3 validation state */
typedef struct {
    iui_md3_tracked_t components[IUI_MD3_MAX_TRACKED];
    int count;
    int total_violations;
    float scale; /* DPI scale factor */
} iui_md3_validation_state_t;

/* Global validation state (thread-local in multi-threaded builds) */
extern iui_md3_validation_state_t g_md3_validation;

/* Begin frame validation - resets tracking */
static inline void iui_md3_frame_begin(float scale)
{
    g_md3_validation.count = 0;
    g_md3_validation.total_violations = 0;
    g_md3_validation.scale = (scale > 0.f) ? scale : 1.f;
}

/* Track a rendered component and validate against MD3 spec */
static inline void iui_md3_track(iui_md3_comp_type_t type,
                                 iui_rect_t bounds,
                                 float corner)
{
    if (g_md3_validation.count >= IUI_MD3_MAX_TRACKED)
        return;

    md3_violation_t v = MD3_OK;
    float scale = g_md3_validation.scale;
    int h = md3_round_px(bounds.height);
    int w = md3_round_px(bounds.width);

    /* Validate based on component type */
    switch (type) {
    case IUI_MD3_COMP_BUTTON:
        v |= md3_check_button(h, scale);
        v |= md3_check_touch_target(w, h, scale);
        break;
    case IUI_MD3_COMP_FAB:
        v |= md3_check_fab(h, scale);
        v |= md3_check_touch_target(w, h, scale);
        break;
    case IUI_MD3_COMP_FAB_LARGE:
        v |= md3_check_fab_large(h, scale);
        break;
    case IUI_MD3_COMP_CHIP:
        v |= md3_check_chip(h, scale);
        v |= md3_check_touch_target(w, h, scale);
        break;
    case IUI_MD3_COMP_TEXTFIELD:
        v |= md3_check_textfield(h, scale);
        break;
    case IUI_MD3_COMP_SWITCH:
    case IUI_MD3_COMP_SLIDER:
    case IUI_MD3_COMP_TAB:
    case IUI_MD3_COMP_CHECKBOX:
    case IUI_MD3_COMP_RADIO:
    case IUI_MD3_COMP_SEGMENTED:
        v |= md3_check_touch_target(w, h, scale);
        break;
    default:
        break;
    }

    iui_md3_tracked_t *t =
        &g_md3_validation.components[g_md3_validation.count++];
    t->type = type;
    t->bounds = bounds;
    t->corner_radius = corner;
    t->violations = v;

    if (v != MD3_OK)
        g_md3_validation.total_violations++;
}

/* Get validation results for current frame */
static inline int iui_md3_get_violations(void)
{
    return g_md3_validation.total_violations;
}

/* Get tracked component count */
static inline int iui_md3_get_tracked_count(void)
{
    return g_md3_validation.count;
}

/* Get tracked component by index */
static inline const iui_md3_tracked_t *iui_md3_get_tracked(int index)
{
    if (index < 0 || index >= g_md3_validation.count)
        return NULL;
    return &g_md3_validation.components[index];
}

/* Convenience macros for widget instrumentation */
#define IUI_MD3_TRACK_BUTTON(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_BUTTON, rect, corner)
#define IUI_MD3_TRACK_FAB(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_FAB, rect, corner)
#define IUI_MD3_TRACK_FAB_LARGE(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_FAB_LARGE, rect, corner)
#define IUI_MD3_TRACK_CHIP(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_CHIP, rect, corner)
#define IUI_MD3_TRACK_TEXTFIELD(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_TEXTFIELD, rect, corner)
#define IUI_MD3_TRACK_SWITCH(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_SWITCH, rect, corner)
#define IUI_MD3_TRACK_SLIDER(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_SLIDER, rect, corner)
#define IUI_MD3_TRACK_TAB(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_TAB, rect, corner)
#define IUI_MD3_TRACK_CHECKBOX(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_CHECKBOX, rect, corner)
#define IUI_MD3_TRACK_RADIO(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_RADIO, rect, corner)
#define IUI_MD3_TRACK_SEGMENTED(rect, corner) \
    iui_md3_track(IUI_MD3_COMP_SEGMENTED, rect, corner)

#else /* !IUI_MD3_RUNTIME_VALIDATION */

/* No-op stubs when validation disabled */
#define iui_md3_frame_begin(scale) ((void) 0)
#define iui_md3_track(type, bounds, corner) ((void) 0)
#define iui_md3_get_violations() (0)
#define iui_md3_get_tracked_count() (0)
#define iui_md3_get_tracked(index) ((void *) 0)

#define IUI_MD3_TRACK_BUTTON(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_FAB(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_FAB_LARGE(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_CHIP(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_TEXTFIELD(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_SWITCH(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_SLIDER(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_TAB(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_CHECKBOX(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_RADIO(rect, corner) ((void) 0)
#define IUI_MD3_TRACK_SEGMENTED(rect, corner) ((void) 0)

#endif /* IUI_MD3_RUNTIME_VALIDATION */

#endif /* IUI_INTERNAL_H */
