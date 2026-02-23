/* Immediate Mode UI Library
 * Reference: Material Design 3 (MD3)
 *            https://m3.material.io/
 */

#ifndef IUI_H_
#define IUI_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* MD3 component specifications (dimensions, durations, accessibility) */
#include "iui-spec.h"

/* System Configuration - Override before including this header */
#ifndef IUI_MAX_WINDOWS
#define IUI_MAX_WINDOWS 16
#endif
#ifndef IUI_STRING_BUFFER_SIZE
#define IUI_STRING_BUFFER_SIZE 2048
#endif
#ifndef IUI_MAX_BOX_DEPTH
#define IUI_MAX_BOX_DEPTH 8
#endif
#ifndef IUI_MAX_BOX_CHILDREN
#define IUI_MAX_BOX_CHILDREN 16
#endif
#ifndef IUI_MAX_TABLE_COLS
#define IUI_MAX_TABLE_COLS 16
#endif
#ifndef IUI_SEARCH_QUERY_SIZE
#define IUI_SEARCH_QUERY_SIZE 256
#endif
#ifndef IUI_ID_STACK_SIZE
#define IUI_ID_STACK_SIZE 8
#endif
#ifndef IUI_MAX_INPUT_LAYERS
#define IUI_MAX_INPUT_LAYERS 8
#endif
#ifndef IUI_MAX_BLOCKING_REGIONS
#define IUI_MAX_BLOCKING_REGIONS 16
#endif
#ifndef IUI_MAX_FOCUSABLE_WIDGETS
#define IUI_MAX_FOCUSABLE_WIDGETS 64
#endif
#ifndef IUI_CLIP_STACK_SIZE
#define IUI_CLIP_STACK_SIZE 8
#endif

/* Public Structures */
/* Mouse button flags for multi-button support */
typedef enum iui_mouse_button {
    IUI_MOUSE_LEFT = 1 << 0,
    IUI_MOUSE_RIGHT = 1 << 1,
    IUI_MOUSE_MIDDLE = 1 << 2,
} iui_mouse_button_t;

typedef enum iui_text_alignment {
    IUI_ALIGN_LEFT,
    IUI_ALIGN_CENTER,
    IUI_ALIGN_RIGHT
} iui_text_alignment_t;

typedef enum iui_window_option {
    IUI_WINDOW_PINNED = 1 << 0,
    IUI_WINDOW_RESIZABLE = 1 << 1,
    IUI_WINDOW_AUTO_WIDTH = 1 << 2,  /* Auto-expand width to fit content */
    IUI_WINDOW_AUTO_HEIGHT = 1 << 3, /* Auto-expand height to fit content */
    IUI_WINDOW_AUTO_SIZE = (1 << 2) | (1 << 3) /* Both width and height */
} iui_window_option_t;

/* MD3 Textfield styles */
typedef enum iui_textfield_style {
    IUI_TEXTFIELD_FILLED,
    IUI_TEXTFIELD_OUTLINED, /* border instead of underline */
} iui_textfield_style_t;

/* TextField icon types for leading/trailing icons
 * Icons are drawn using vector primitives (lines, circles, arcs)
 */
typedef enum iui_textfield_icon {
    IUI_TEXTFIELD_ICON_NONE = 0,       /* no icon */
    IUI_TEXTFIELD_ICON_SEARCH,         /* magnifying glass */
    IUI_TEXTFIELD_ICON_CLEAR,          /* X to clear field */
    IUI_TEXTFIELD_ICON_VISIBILITY,     /* eye (show password) */
    IUI_TEXTFIELD_ICON_VISIBILITY_OFF, /* eye with slash (hide password) */
    IUI_TEXTFIELD_ICON_CHECK,          /* checkmark (validation) */
    IUI_TEXTFIELD_ICON_ERROR,          /* exclamation mark (error state) */
} iui_textfield_icon_t;

/* TextField options for advanced text input configuration */
typedef struct {
    iui_textfield_style_t style;        /* filled or outlined */
    const char *placeholder;            /* placeholder text when empty */
    iui_textfield_icon_t leading_icon;  /* on left side (or 0 for none) */
    iui_textfield_icon_t trailing_icon; /* on right side (or 0 for none) */
    bool password_mode; /* show '*' characters instead of text */
    bool read_only;     /* allow selection but not editing */
    bool disabled;      /* gray out and block all interaction */
} iui_textfield_options;

/* TextField result for multiple simultaneous states */
typedef struct {
    bool value_changed;         /* text was modified this frame */
    bool submitted;             /* Enter key was pressed */
    bool leading_icon_clicked;  /* leading icon was clicked */
    bool trailing_icon_clicked; /* trailing icon was clicked */
} iui_textfield_result;

/* Text selection state for advanced text editing
 * Reference: https://m3.material.io/components/text-fields/guidelines#selection
 *
 * Usage: User provides this structure for persistent selection state across
 * frames. The widget updates cursor, selection_start, selection_end as needed.
 *
 * When selection_start != selection_end, text between them is selected.
 * Selection is always normalized: selection_start <= selection_end.
 */
typedef struct {
    size_t cursor; /* current cursor position (insertion point) */
    size_t selection_start, selection_end; /* start/end of selection range */
    float scroll_offset;   /* horizontal scroll for text wider than field */
    float last_click_time; /* timestamp of last click (for double/triple) */
    int last_click_count;  /* 1=single, 2=double, 3=triple click detection */
    size_t last_click_pos; /* cursor position of last click (word anchor) */
    bool is_dragging;      /* true when mouse drag selection is active */
} iui_edit_state;

/* MD3 Slider options for enhanced slider configuration
 * Reference: https://m3.material.io/components/sliders/specs
 */
typedef struct {
    const char *start_text;        /* text label at left side */
    const char *end_text;          /* text label at right side */
    bool show_value_indicator;     /* bubble above thumb during drag */
    const char *value_format;      /* printf format string (NULL = "%.0f") */
    uint32_t active_track_color;   /* 0 = use theme primary */
    uint32_t inactive_track_color; /* 0 = use theme surface_container_highest */
    uint32_t handle_color;         /* 0 = use theme primary */
    bool disabled;                 /* grayed out, no interaction */
} iui_slider_options;

/* MD3 Card styles */
typedef enum iui_card_style {
    IUI_CARD_ELEVATED, /* shadow effect */
    IUI_CARD_FILLED,   /* surface_container_high */
    IUI_CARD_OUTLINED, /* surface + outline border */
} iui_card_style_t;

/* MD3 Top App Bar sizes
 * Reference: https://m3.material.io/components/top-app-bar/specs
 */
typedef enum iui_appbar_size {
    IUI_APPBAR_SMALL,  /* 64dp fixed height, title_large typography */
    IUI_APPBAR_CENTER, /* 64dp fixed height, centered title */
    IUI_APPBAR_MEDIUM, /* 112dp→64dp, collapses on scroll */
    IUI_APPBAR_LARGE,  /* 152dp→64dp, collapses on scroll */
} iui_appbar_size_t;

/* MD3 Elevation levels for shadow system
 * Elevation creates depth perception through layered shadows
 */
typedef enum iui_elevation {
    IUI_ELEVATION_0 = 0, /* No shadow (flat) */
    IUI_ELEVATION_1 = 1, /* Subtle shadow (cards, chips) */
    IUI_ELEVATION_2 = 2, /* Medium shadow (buttons, FAB) */
    IUI_ELEVATION_3 = 3, /* Pronounced shadow (menus, dialogs) */
    IUI_ELEVATION_4 = 4, /* High shadow (navigation drawers) */
    IUI_ELEVATION_5 = 5, /* Maximum shadow (modals) */
} iui_elevation_t;

/* MD3 Motion System - Easing Types
 * Reference: https://m3.material.io/styles/motion/easing-and-duration
 * Based on cubic-bezier curves from Material Design 3 specification
 */
typedef enum iui_easing {
    IUI_EASING_LINEAR,   /* Linear (no easing) */
    IUI_EASING_STANDARD, /* Standard: balanced enter/exit (0.2, 0.0, 0, 1.0) */
    IUI_EASING_STANDARD_DECELERATE, /* Enter animations (0.0, 0.0, 0, 1.0) */
    IUI_EASING_STANDARD_ACCELERATE, /* Exit animations (0.3, 0.0, 1.0, 1.0) */
    IUI_EASING_EMPHASIZED, /* Emphasized: transitions (0.2, 0.0, 0, 1.0) */
    IUI_EASING_EMPHASIZED_DECELERATE, /* Attention-grab (0.05,0.7,0.1,1.0) */
    IUI_EASING_EMPHASIZED_ACCELERATE, /* Quick dismissals (0.3,0.0,0.8,0.15) */
} iui_easing_t;

/* Motion configuration for asymmetric timing
 * Allows different durations/easings for enter vs exit transitions
 */
typedef struct {
    float enter_duration; /* Duration for enter/show animations (seconds) */
    float exit_duration;  /* Duration for exit/hide animations (seconds) */
    iui_easing_t enter_easing; /* Easing curve for enter animations */
    iui_easing_t exit_easing;  /* Easing curve for exit animations */
} iui_motion_config;

/* Pre-defined motion presets (accessed via getter functions)
 * Usage:
 * iui_motion_apply(t, is_entering, iui_motion_get_standard())
 */
const iui_motion_config *iui_motion_get_standard(void);   /* General-purpose */
const iui_motion_config *iui_motion_get_emphasized(void); /* Attention-grab */
const iui_motion_config *iui_motion_get_quick(void);      /* Fast micro */
const iui_motion_config *iui_motion_get_dialog(void); /* Modal/dialog reveal */
const iui_motion_config *iui_motion_get_menu(void);   /* Menu/dropdown expand */

typedef enum iui_key_code {
    IUI_KEY_NONE = 0,
    IUI_KEY_BACKSPACE = 8,
    IUI_KEY_TAB = 9,
    IUI_KEY_ENTER = 13,
    IUI_KEY_ESCAPE = 27,
    IUI_KEY_DELETE = 127,
    IUI_KEY_LEFT = 256,
    IUI_KEY_RIGHT,
    IUI_KEY_UP,
    IUI_KEY_DOWN,
    IUI_KEY_HOME,
    IUI_KEY_END
} iui_key_code_t;

/* Keyboard modifier flags for Ctrl/Shift/Alt combinations */
typedef enum iui_modifier {
    IUI_MOD_NONE = 0,
    IUI_MOD_CTRL = 1 << 0,
    IUI_MOD_SHIFT = 1 << 1,
    IUI_MOD_ALT = 1 << 2,
} iui_modifier_t;

/* Widget result flags for multiple simultaneous states */
typedef uint8_t iui_result_t;

#define IUI_NONE 0
#define IUI_HOVERED (1 << 0) /* Mouse is hovering over widget */
#define IUI_ACTIVE (1 << 1)  /* Mouse is pressed down on widget */
#define IUI_SUBMIT (1 << 2)  /* Action completed (click/enter pressed) */
#define IUI_CHANGED (1 << 3) /* Value was modified */

/* Clipboard callbacks for copy/paste support
 * User provides platform-specific clipboard implementation.
 */
typedef struct {
    const char *(*get)(void *user); /* get clipboard text */
    void (*set)(const char *text,
                size_t len,
                void *user); /* set clipboard text */
    void *user;              /* user context */
} iui_clipboard_t;

/* Snackbar state structure for timed notification messages */
typedef struct {
    const char *message;      /* message to display */
    const char *action_label; /* optional action button label */
    float duration;           /* display duration in seconds */
    float timer;              /* internal countdown timer */
    bool active;              /* true when snackbar is visible */
} iui_snackbar_state;

/* Scrollable container state (user-provided)
 * Content size is measured automatically during iui_scroll_end()
 */
typedef struct {
    float scroll_x, scroll_y;   /* current scroll offset (pixels) */
    float content_w, content_h; /* measured size (set by iui_scroll_end) */
    float velocity_y; /* for momentum scrolling (optional, user-managed) */
} iui_scroll_state;

/* Menu item structure for vertical menus */
typedef struct {
    const char *text;          /* menu item label */
    const char *leading_icon;  /* optional icon on left */
    const char *trailing_text; /* optional shortcut text */
    const char *trailing_icon; /* optional icon on right */
    bool is_divider;           /* visual separator line */
    bool is_gap;               /* vertical spacing without line */
    bool disabled;             /* grayed out, no interaction */
} iui_menu_item;

/* Menu options for customization */
typedef struct {
    bool icon_only_mode; /* reserved for future: horizontal icon bar mode */
    float min_width;     /* minimum menu width (0 = use default 200) */
    float max_width;     /* maximum menu width (0 = use default 400) */
} iui_menu_options;

/* Menu state structure (user-provided for menu lifecycle) */
typedef struct {
    bool open;             /* true when menu is visible */
    float x, y;            /* menu position */
    float width, height;   /* computed menu dimensions */
    uint32_t id;           /* hash of menu identifier */
    int hovered_index;     /* currently hovered item index */
    int frames_since_open; /* prevents premature close */
} iui_menu_state;

/* Dialog state structure for alert/confirmation dialogs
 * Distinct from iui_begin_modal() which allows arbitrary content
 */
typedef struct {
    const char *title;     /* dialog title text */
    const char *message;   /* dialog body message */
    const char *buttons;   /* semicolon-separated button labels */
    int selected_button;   /* 0-N for clicked button, -1 if dismissed */
    bool is_open;          /* true when dialog is visible */
    int frames_since_open; /* frame counter for open protection */
    int button_count;      /* parsed button count (internal) */
} iui_dialog_state;

/* Date picker state structure for calendar date selection
 * Reference: https://m3.material.io/components/date-pickers
 * Modal calendar picker with month/year navigation
 */
typedef struct {
    int year, month, day;      /* selected date (year/month/day) */
    int view_year, view_month; /* currently viewing year/month (1-12) */
    bool is_open;              /* true when picker is visible */
    int frames_since_open;     /* frame counter for open protection */
    bool confirmed;            /* true when user confirmed selection */
} iui_date_picker_state;

/* Time picker state structure for clock time selection
 * Reference: https://m3.material.io/components/time-pickers
 * Modal clock dial picker with hour/minute selection
 */
typedef struct {
    int hour, minute;      /* selected time (hour/minute) */
    bool is_pm;            /* AM/PM indicator (for 12H format) */
    bool use_24h;          /* true for 24H format, false for 12H */
    bool is_open;          /* true when picker is visible */
    int frames_since_open; /* frame counter for open protection */
    bool confirmed;        /* true when user confirmed selection */
    bool selecting_minute; /* true when selecting minute */
} iui_time_picker_state;

/* Full-screen dialog state for immersive tasks
 * Reference: https://m3.material.io/components/dialogs
 * Full-screen modal with app bar header and content area
 */
typedef struct {
    const char *title;     /* dialog title in header */
    bool is_open;          /* true when dialog is visible */
    int frames_since_open; /* frame counter for open protection */
    int action_count;      /* number of actions added this frame */
    float action_x;        /* next action button x position (internal) */
    float content_y;       /* content area start y (internal) */
} iui_fullscreen_dialog_state;

/* Search view state for full-screen search experience
 * Reference: https://m3.material.io/components/search
 * Modal search with suggestions list
 */
typedef struct {
    char query[IUI_SEARCH_QUERY_SIZE]; /* current search query text */
    size_t cursor;                     /* cursor position in query */
    bool is_open;                      /* true when search view is visible */
    int frames_since_open; /* frame counter for open protection (internal) */
    int suggestion_count;  /* suggestions added this frame (internal) */
    float suggestion_y;    /* next suggestion y position (internal) */
} iui_search_view_state;

/* Dropdown menu options for configuration */
typedef struct {
    const char **options;    /* array of option strings */
    int option_count;        /* number of options */
    int *selected_index;     /* pointer to selected option index */
    const char *label;       /* floating label text */
    const char *helper_text; /* helper text below field */
    bool disabled;           /* grayed out, no interaction */
} iui_dropdown_options;

/* MD3 List types
 * Reference: https://m3.material.io/components/lists
 */
typedef enum iui_list_type {
    IUI_LIST_ONE_LINE,   /* 56dp: headline only */
    IUI_LIST_TWO_LINE,   /* 72dp: headline + supporting text */
    IUI_LIST_THREE_LINE, /* 88dp: headline + 2-line supporting text */
} iui_list_type_t;

/* MD3 List leading element types */
typedef enum iui_list_leading {
    IUI_LIST_LEADING_NONE,     /* no leading element */
    IUI_LIST_LEADING_ICON,     /* 24dp icon */
    IUI_LIST_LEADING_AVATAR,   /* 40dp circular avatar */
    IUI_LIST_LEADING_IMAGE,    /* 56dp square image */
    IUI_LIST_LEADING_CHECKBOX, /* checkbox control */
    IUI_LIST_LEADING_RADIO,    /* radio button control */
} iui_list_leading_t;

/* MD3 List trailing element types */
typedef enum iui_list_trailing {
    IUI_LIST_TRAILING_NONE,     /* no trailing element */
    IUI_LIST_TRAILING_ICON,     /* 24dp icon */
    IUI_LIST_TRAILING_TEXT,     /* supporting text (meta) */
    IUI_LIST_TRAILING_CHECKBOX, /* checkbox control */
    IUI_LIST_TRAILING_SWITCH,   /* switch toggle */
} iui_list_trailing_t;

/* List item configuration structure */
typedef struct {
    const char *headline;      /* primary text (required) */
    const char *supporting;    /* secondary text (optional, for 2/3 line) */
    const char *overline;      /* overline text above headline (optional) */
    const char *leading_icon;  /* FAB icon name for leading_icon type */
    const char *trailing_icon; /* FAB icon name for trailing_icon type */
    const char *trailing_text; /* meta text for trailing_text type */
    iui_list_leading_t leading_type;   /* leading element type */
    iui_list_trailing_t trailing_type; /* trailing element type */
    bool *checkbox_value;              /* pointer for checkbox/switch state */
    int *radio_value;                  /* pointer for radio group value */
    int radio_option;                  /* this item's radio option value */
    bool disabled;                     /* grayed out, no interaction */
    bool show_divider;                 /* draw divider below this item */
} iui_list_item;

/* MD3 Navigation Rail state
 * Reference: https://m3.material.io/components/navigation-rail
 */
typedef struct {
    float x, y;     /* rail position */
    float height;   /* rail height */
    bool expanded;  /* true = expanded (shows labels), false = collapsed */
    int item_count; /* number of items added */
    int selected;   /* currently selected item index */
} iui_nav_rail_state;

/* MD3 Navigation Bar state
 * Reference: https://m3.material.io/components/navigation-bar
 */
typedef struct {
    float x, y;      /* bar position */
    float width;     /* bar width */
    int item_count;  /* number of items added so far */
    int total_items; /* expected total items (for width calculation) */
    int selected;    /* currently selected item index */
} iui_nav_bar_state;

/* MD3 Navigation Drawer state
 * Reference: https://m3.material.io/components/navigation-drawer
 */
typedef struct {
    bool open;           /* drawer visibility */
    bool modal;          /* true = modal with scrim, false = standard */
    float x, y;          /* drawer position */
    float height;        /* drawer height */
    int item_count;      /* number of items added */
    int selected;        /* currently selected item index */
    float anim_progress; /* 0.0 = closed, 1.0 = fully open */
} iui_nav_drawer_state;

/* MD3 Button styles */
typedef enum iui_button_style {
    IUI_BUTTON_TONAL,    /* current default (surface_container bg) */
    IUI_BUTTON_FILLED,   /* primary bg, on_primary text */
    IUI_BUTTON_OUTLINED, /* transparent + outline border */
    IUI_BUTTON_TEXT,     /* no background, just text */
    IUI_BUTTON_ELEVATED, /* surface_container_high + shadow */
} iui_button_style_t;

/* MD3 Typography Scale */
typedef struct {
    float display_large;   /* 57px (not commonly used in immediate mode) */
    float display_medium;  /* 45px (not commonly used in immediate mode) */
    float display_small;   /* 36px (not commonly used in immediate mode) */
    float headline_large;  /* 32px (not commonly used in immediate mode) */
    float headline_medium; /* 28px (not commonly used in immediate mode) */
    float headline_small;  /* 24px */
    float title_large;     /* 22px */
    float title_medium;    /* 16px */
    float title_small;     /* 14px */
    float body_large;      /* 16px (was body1 in previous MD spec) */
    float body_medium;     /* 14px (was body2 in previous MD spec) */
    float body_small;      /* 12px (was caption in previous MD spec) */
    float label_large;     /* 14px (used for buttons, chips) */
    float label_medium;    /* 12px (used for secondary buttons, chips) */
    float label_small;     /* 11px (used for overlines, helper text) */
} iui_typography_scale;

/* MD3 Shape Tokens */
typedef struct {
    float none;        /* 0dp */
    float extra_small; /* 2dp */
    float small;       /* 4dp */
    float medium;      /* 8dp */
    float large;       /* 12dp */
    float extra_large; /* 16dp */
    float full;        /* height/2 (pill shape) */
} iui_shape_tokens;

/* MD3 Spacing Tokens
 * Reference: https://m3.material.io/foundations/layout/applying-layout
 * All spacing values follow the 4dp grid system
 */
typedef struct {
    float none; /* 0dp */
    float xxs;  /* 4dp  (micro gaps, icon padding) */
    float xs;   /* 8dp  (standard component gaps) */
    float sm;   /* 12dp (menu padding, content spacing) */
    float md;   /* 16dp (container padding, FAB gaps) */
    float lg;   /* 24dp (dialog padding, section spacing) */
    float xl;   /* 32dp (large section margins) */
    float xxl;  /* 48dp (touch targets, major sections) */
} iui_spacing_tokens;

/* Snap value to nearest 4dp grid position
 * Returns the nearest multiple of 4 to maintain MD3 spacing consistency
 */
float iui_spacing_snap(float value);

typedef struct {
    float x, y;
    float width, height;
} iui_rect_t;

/* MD3 Window Size Class for adaptive layouts
 * Reference: https://m3.material.io/foundations/layout/applying-layout
 */
typedef enum iui_size_class {
    IUI_SIZE_CLASS_COMPACT = 0, /* < 600dp: phone portrait */
    IUI_SIZE_CLASS_MEDIUM,      /* 600-839dp: tablet portrait, foldable */
    IUI_SIZE_CLASS_EXPANDED,    /* 840-1199dp: tablet landscape */
    IUI_SIZE_CLASS_LARGE,       /* 1200-1599dp: desktop */
    IUI_SIZE_CLASS_XLARGE,      /* >= 1600dp: large desktop */
} iui_size_class_t;

/* Sizing modes for box children */
typedef enum iui_sizing_type {
    IUI_SIZE_GROW = 0, /* expand to fill (default when zero-initialized) */
    IUI_SIZE_FIXED,    /* exact pixels */
    IUI_SIZE_PERCENT,  /* fraction of parent (0.0-1.0) */
} iui_sizing_type_t;

typedef struct {
    iui_sizing_type_t type;
    float value; /* pixels, weight, or fraction */
    float min;   /* 0 = unconstrained */
    float max;   /* 0 = unconstrained */
} iui_sizing_t;

#define IUI_FIXED(px) ((iui_sizing_t) {IUI_SIZE_FIXED, (px), 0, 0})
#define IUI_GROW(w) ((iui_sizing_t) {IUI_SIZE_GROW, (w), 0, 0})
#define IUI_PERCENT(f) ((iui_sizing_t) {IUI_SIZE_PERCENT, (f), 0, 0})

/* Box container direction and alignment */
typedef enum iui_direction { IUI_DIR_ROW = 0, IUI_DIR_COLUMN } iui_direction_t;
typedef enum iui_cross_align {
    IUI_CROSS_STRETCH = 0,
    IUI_CROSS_START,
    IUI_CROSS_CENTER,
    IUI_CROSS_END
} iui_cross_align_t;
typedef struct {
    float left, right, top, bottom;
} iui_padding_t;

#define IUI_PAD_ALL(v) ((iui_padding_t) {(v), (v), (v), (v)})
#define IUI_PAD_XY(x, y) ((iui_padding_t) {(x), (x), (y), (y)})

/* Box container configuration */
typedef struct {
    iui_direction_t direction; /* ROW (default) or COLUMN */
    int child_count;           /* number of children */
    const iui_sizing_t *sizes; /* main-axis sizes (NULL = all GROW(1)) */
    float cross;               /* cross-axis size (0 = fill parent) */
    float gap;                 /* spacing between children (snapped to 4dp) */
    iui_padding_t padding;     /* inner padding (snapped to 4dp) */
    iui_cross_align_t align;   /* cross-axis child alignment */
} iui_box_config_t;

typedef struct {
    void (*draw_box)(iui_rect_t rect,
                     float radius,
                     uint32_t srgb_color,
                     void *user);
    void (*draw_text)(float x,
                      float y,
                      const char *text,
                      uint32_t srgb_color,
                      void *user); /* draw a text top-left aligned with x, y */
    void (*set_clip_rect)(uint16_t min_x,
                          uint16_t min_y,
                          uint16_t max_x,
                          uint16_t max_y,
                          void *user);
    float (*text_width)(const char *text, void *user);
    /* Vector primitives (optional, NULL = not supported)
     * Enables gauges, charts, progress indicators, and custom rendering
     */
    void (*draw_line)(float x0,
                      float y0,
                      float x1,
                      float y1,
                      float width,
                      uint32_t srgb_color,
                      void *user);
    void (*draw_circle)(float cx,
                        float cy,
                        float radius,
                        uint32_t fill_color,   /* 0 = no fill */
                        uint32_t stroke_color, /* 0 = no stroke */
                        float stroke_width,
                        void *user);
    void (*draw_arc)(float cx,
                     float cy,
                     float radius,
                     float start_angle, /* radians, 0 = right, PI/2 = down */
                     float end_angle,   /* radians */
                     float width,
                     uint32_t srgb_color,
                     void *user);
    void *user;
} iui_renderer_t;

/* Vector font callbacks for path-based text rendering
 * Glyph bytecode format (1/64 unit coords):
 * Header [6 bytes]: left_bearing, right_bearing, ascent, descent, n_snap_x,
 * n_snap_y Snap arrays: snap_x[n_snap_x], snap_y[n_snap_y] Opcodes: 'm' x y
 * (move), 'l' x y (line), 'c' x1 y1 x2 y2 x3 y3 (cubic), 'e' (end)
 */
typedef struct {
    void (*path_move)(float x, float y, void *user);
    void (*path_line)(float x, float y, void *user);
    void (*path_curve)(float x1,
                       float y1,
                       float x2,
                       float y2,
                       float x3,
                       float y3,
                       void *user);
    void (*path_stroke)(float width, uint32_t color, void *user);
} iui_vector_t;

typedef struct {
    void *buffer;            /* must be aligned on 8 bytes */
    iui_renderer_t renderer; /* draw_box and set_clip_rect required */
    float font_height;       /* logical font height in pixels */
    /* optional; when NULL, draw_text/text_width are used */
    const iui_vector_t *vector;
} iui_config_t;

typedef struct iui_context iui_context;

/* Component state enum for standardized widget interaction
 * Provides consistent state handling across all widgets following MD3 patterns
 */
typedef enum iui_state {
    IUI_STATE_DEFAULT,
    IUI_STATE_HOVERED,
    IUI_STATE_PRESSED,
    IUI_STATE_FOCUSED,
    IUI_STATE_DRAGGED,
    IUI_STATE_DISABLED,
} iui_state_t;

/* Theme structure for customizing UI colors (Material Design 3)
 * All colors are in ARGB format (0xAARRGGBB)
 */
typedef struct {
    /* Primary color group (4) */
    uint32_t primary;    /* Key color for prominent components (FAB, buttons) */
    uint32_t on_primary; /* Content color on primary */
    uint32_t primary_container;    /* Standout fill for key components */
    uint32_t on_primary_container; /* Content color on primary_container */

    /* Secondary color group (4) */
    uint32_t secondary;    /* Accent color for less prominent components */
    uint32_t on_secondary; /* Content color on secondary */
    uint32_t secondary_container;    /* Container for secondary accent */
    uint32_t on_secondary_container; /* Content on secondary container */

    /* Tertiary color group (4) */
    uint32_t tertiary;              /* Third accent for contrast/balance */
    uint32_t on_tertiary;           /* Content color on tertiary */
    uint32_t tertiary_container;    /* Container for tertiary accent */
    uint32_t on_tertiary_container; /* Content on tertiary container */

    /* Surface color group (7) */
    uint32_t surface;         /* Default background (cards, sheets, dialogs) */
    uint32_t on_surface;      /* Text and icons on surface */
    uint32_t surface_variant; /* Alternative surface for visual distinction */
    uint32_t on_surface_variant;        /* Content on surface variant */
    uint32_t surface_container_lowest;  /* Lowest elevation surface container */
    uint32_t surface_container_low;     /* Low elevation surface container */
    uint32_t surface_container;         /* Default surface container */
    uint32_t surface_container_high;    /* High elevation surface container */
    uint32_t surface_container_highest; /* Highest */

    /* Outline group (2) */
    uint32_t outline;         /* Subtle borders and dividers */
    uint32_t outline_variant; /* Lower emphasis borders */

    /* Error color group (4) */
    uint32_t error;              /* Error states */
    uint32_t on_error;           /* Content color on error */
    uint32_t error_container;    /* Container for error states */
    uint32_t on_error_container; /* Content on error container */

    /* Utility colors (5) */
    uint32_t shadow;             /* Shadow color for elevation */
    uint32_t scrim;              /* Overlay for modals and dialogs */
    uint32_t inverse_surface;    /* Inverted surface for snackbars, tooltips */
    uint32_t inverse_on_surface; /* Content on inverse surface */
    uint32_t inverse_primary;    /* Primary color for inverse surfaces */
} iui_theme_t;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the number of bytes needed to allocate a iui_context and its buffer
 */
size_t iui_min_memory_size(void);

/* Initializes the library
 * @config: configuration containing all required parameters
 *
 * Returns pointer to the initialized context
 */
iui_context *iui_init(const iui_config_t *config);

/* Convenience helper to build iui_config_t initialization structure
 * @buffer:      memory buffer to use for context allocation
 * @renderer:    function pointers for drawing operations
 * @font_height: base font height in pixels
 * @vector:      optional callbacks for vector font rendering
 *
 * Returns initialized iui_config_t structure
 */
iui_config_t iui_make_config(void *buffer,
                             iui_renderer_t renderer,
                             float font_height,
                             const iui_vector_t *vector);
bool iui_config_is_valid(const iui_config_t *config);

/* Updates the position of mouse in screen coordinate in pixels */
void iui_update_mouse_pos(iui_context *ctx, float x, float y);

/* Updates keyboard input for text editing
 * @ctx: current UI context
 * @key: key code from iui_key_code_t, or ASCII character
 */
void iui_update_key(iui_context *ctx, int key);

/* Updates text input character (for typing)
 * @ctx:       current UI context
 * @codepoint: Unicode codepoint (ASCII subset supported)
 */
void iui_update_char(iui_context *ctx, int codepoint);

/* Updates keyboard modifier state (Ctrl, Shift, Alt)
 * @ctx:       current UI context
 * @modifiers: bitfield of iui_modifier_t flags
 */
void iui_update_modifiers(iui_context *ctx, uint8_t modifiers);

/* Updates mouse button state for multi-button support
 * @pressed:  buttons pressed this frame
 * @released: buttons released this frame
 */
void iui_update_mouse_buttons(iui_context *ctx,
                              uint8_t pressed,
                              uint8_t released);

/* Theme API
 * Returns pointer to the current theme (can be modified directly for custom
 * colors)
 */
const iui_theme_t *iui_get_theme(iui_context *ctx);

/* Returns the built-in light theme (warm beige tones) */
const iui_theme_t *iui_theme_light(void);

/* Returns the built-in dark theme */
const iui_theme_t *iui_theme_dark(void);

/* Sets the current theme. Pass NULL to keep current theme unchanged.
 * Changes take effect immediately for subsequent draw calls.
 */
void iui_set_theme(iui_context *ctx, const iui_theme_t *theme);

/* ID stack for handling duplicate labels
 * Push a unique identifier onto the stack to differentiate widgets with same
 * labels
 * @ctx:  current UI context
 * @data: pointer to unique data (e.g., loop index, object pointer)
 * @size: size of the data in bytes
 *
 * Returns true on success, false if ID stack overflow
 */
bool iui_push_id(iui_context *ctx, const void *data, size_t size);

/* Pop an identifier from the ID stack */
void iui_pop_id(iui_context *ctx);

/* Box container layout (nestable, flexbox-like)
 * Supports FIXED, GROW, and PERCENT sizing with min/max constraints.
 * Nests up to IUI_MAX_BOX_DEPTH levels. Gap and padding snap to 4dp grid.
 *
 * Usage:
 *   iui_sizing_t sizes[] = { IUI_FIXED(200), IUI_GROW(1) };
 *   iui_box_begin(ctx, &(iui_box_config_t){
 *       .direction = IUI_DIR_ROW, .child_count = 2,
 *       .sizes = sizes, .gap = 8,
 *   });
 *   iui_rect_t left = iui_box_next(ctx);
 *   // ... draw widgets in left ...
 *   iui_rect_t right = iui_box_next(ctx);
 *   // ... draw widgets in right ...
 *   iui_box_end(ctx);
 */
iui_rect_t iui_box_begin(iui_context *ctx, const iui_box_config_t *config);
iui_rect_t iui_box_next(iui_context *ctx);
void iui_box_end(iui_context *ctx);
int iui_box_depth(const iui_context *ctx);

/* MD3 adaptive layout queries */
iui_size_class_t iui_size_class(float width);
int iui_layout_columns(iui_size_class_t sc);
float iui_layout_margin(iui_size_class_t sc);
float iui_layout_gutter(iui_size_class_t sc);

/* @ctx:        current UI context
 * @delta_time: elapsed time in seconds since the previous frame
 */
void iui_begin_frame(iui_context *ctx, float delta_time);

/* Begins a new window
 * @ctx:            current UI context
 * @name:           unique name, hashed under the hood
 * @x, @y:          initial position can be changed by the user
 * @width, @height: initial size in pixels can be changed by the user
 * @options:        combination of options from iui_window_option_t
 *
 * Returns true on success, false if nested call or window limit reached
 */
bool iui_begin_window(iui_context *ctx,
                      const char *name,
                      float x,
                      float y,
                      float width,
                      float height,
                      uint32_t options);

/* Displays text according to the alignment
 * @ctx:       current UI context
 * @alignment: text alignment (left, center, right)
 * @string:    can contain format expression (i.e %f) and additional params
 */
void iui_text(iui_context *ctx,
              iui_text_alignment_t alignment,
              const char *string,
              ...);

/* Moves the cursor to the next line, acts like a carriage return (CR) */
void iui_newline(iui_context *ctx);

/* MD3 Divider - horizontal visual separator
 * Reference: https://m3.material.io/components/divider
 * - Height: 1dp
 * - Color: outline_variant token
 * - Vertical margins: 8dp above and below
 */
void iui_divider(iui_context *ctx);

/* MD3 Inset Divider - divider with left padding (16dp) */
void iui_divider_inset(iui_context *ctx);

/* Displays a segmented control with mutually exclusive options
 * @ctx:         current UI context
 * @entries:     array of strings, must have a size of num_entries
 * @num_entries: number of entries in the array
 * @selected:    pointer to the index of the active segment
 */
void iui_segmented(iui_context *ctx,
                   const char **entries,
                   uint32_t num_entries,
                   uint32_t *selected);

/* Displays a horizontal slider with a label
 * @ctx:   current UI context
 * @label: label text for the slider
 * @min:   minimum allowed value
 * @max:   maximum allowed value
 * @step:  increment step (e.g. 0.1)
 * @value: pointer to the controlled float
 * @fmt:   printf-style format for the displayed numeric value
 */
void iui_slider(iui_context *ctx,
                const char *label,
                float min,
                float max,
                float step,
                float *value,
                const char *fmt);

/* Extended slider with MD3 enhancements
 * Supports custom colors, value indicator bubble, and start/end labels
 * Reference: https://m3.material.io/components/sliders/specs
 * @ctx:     current UI context
 * @value:   current slider value (modified by user interaction)
 * @min:     minimum allowed value
 * @max:     maximum allowed value
 * @step:    increment step (0 = continuous)
 * @options: configuration options (NULL = use defaults)
 *
 * Returns the updated value (same as *value after modification)
 */
float iui_slider_ex(iui_context *ctx,
                    float value,
                    float min,
                    float max,
                    float step,
                    const iui_slider_options *options);

/* Displays a clickable button
 * @ctx:       current UI context
 * @label:     button label text
 * @alignment: horizontal alignment of the the button in the window
 *
 * Returns true if the button was pressed this frame
 */
bool iui_button(iui_context *ctx,
                const char *label,
                iui_text_alignment_t alignment);

/* Displays a checkbox with a label
 * @ctx:       current UI context
 * @label:     text displayed next to the checkbox
 * @checked:   pointer to the boolean state
 *
 * Returns true if the checkbox was toggled this frame
 */
bool iui_checkbox(iui_context *ctx, const char *label, bool *checked);

/* Displays a radio button with a label
 * @ctx:          current UI context
 * @label:        text displayed next to the radio button
 * @group_value:  pointer to the group's selected value
 * @button_value: value this button represents
 *
 * Returns true if this radio button was selected this frame
 */
bool iui_radio(iui_context *ctx,
               const char *label,
               int *group_value,
               int button_value);

/* Begins a grid layout for placing widgets in a grid pattern
 * @ctx:    current UI context
 * @cols:   number of columns
 * @cell_w: width of each cell
 * @cell_h: height of each cell
 * @pad:    padding between cells
 */
iui_rect_t iui_grid_begin(iui_context *ctx,
                          int cols,
                          float cell_w,
                          float cell_h,
                          float pad);

/* Advances to the next cell in the grid */
iui_rect_t iui_grid_next(iui_context *ctx);

/* Ends the grid layout and returns to normal row layout */
void iui_grid_end(iui_context *ctx);


/* Returns the current layout rect (value copy), useful for custom rendering */
iui_rect_t iui_get_layout_rect(const iui_context *ctx);

/* Returns the current window's stored position
 * Used by scroll containers and custom layout widgets.
 * Returns (0,0,0,0) if no window is active.
 */
iui_rect_t iui_get_window_rect(const iui_context *ctx);

/* Returns the height remaining from the current layout cursor to the bottom
 * of the WINDOW clip rect (clip.stack[0]).  Unlike iui_get_window_rect(),
 * this reads the live clip stack so it correctly accounts for title bars and
 * padding without callers needing to know the internal geometry.
 * Returns 0 if no window is active.
 *
 * Note: always reads the window-level clip, not any nested scroll clip.
 * Use this when sizing widgets that must fill the rest of the window
 * (e.g. nav rail, list views that are NOT inside a scroll region).
 */
float iui_get_remaining_height(const iui_context *ctx);

/* Reports minimum content width requirement for auto-sizing windows.
 * Widgets call this to indicate their required width.
 * The window will expand to fit if window_auto_width flag is set.
 */
void iui_require_content_width(iui_context *ctx, float width);

/* Ends the current window. Must match iui_begin_window() */
void iui_end_window(iui_context *ctx);

/* Ends the current frame. Must match iui_begin_frame(). Until next frame no
 * more calls to lean_ui.
 */
void iui_end_frame(iui_context *ctx);

/* Vector font text width measurement (uses built-in glyph table)
 * Returns width in pixels at the given font height
 */
float iui_text_width_vec(const char *text, float font_height);

/* Draw a line from (x0, y0) to (x1, y1) with specified width and color
 * @ctx:     current UI context
 * @x0, @y0: start point coordinates
 * @x1, @y1: end point coordinates
 * @width:   line width in pixels
 * @color:   ARGB color value
 *
 * Returns true if the primitive was drawn
 */
bool iui_draw_line(iui_context *ctx,
                   float x0,
                   float y0,
                   float x1,
                   float y1,
                   float width,
                   uint32_t color);

/* Draw a circle at (cx, cy) with radius
 * @ctx:          current UI context
 * @cx, @cy:      circle center coordinates
 * @radius:       circle radius in pixels
 * @fill_color:   interior color (0 = no fill)
 * @stroke_color: outline color (0 = no stroke)
 * @stroke_width: outline thickness in pixels
 *
 * Returns true if the primitive was drawn
 */
bool iui_draw_circle(iui_context *ctx,
                     float cx,
                     float cy,
                     float radius,
                     uint32_t fill_color,
                     uint32_t stroke_color,
                     float stroke_width);

/* Draw an arc at (cx, cy) with radius from start_angle to end_angle
 * @ctx:         current UI context
 * @cx, @cy:     arc center coordinates
 * @radius:      arc radius in pixels
 * @start_angle: start angle in radians (0 = right, PI/2 = down)
 * @end_angle:   end angle in radians
 * @width:       arc line width in pixels
 * @color:       ARGB color value
 *
 * Returns true if the primitive was drawn
 */
bool iui_draw_arc(iui_context *ctx,
                  float cx,
                  float cy,
                  float radius,
                  float start_angle,
                  float end_angle,
                  float width,
                  uint32_t color);

/* Check if vector primitives are available */
bool iui_has_vector_primitives(const iui_context *ctx);

/* MD3 Motion System API
 * Reference: https://m3.material.io/styles/motion
 */

/* Apply an MD3 easing curve to a normalized time value (0.0 to 1.0)
 * Returns eased value (0.0 to 1.0)
 */
float iui_ease(float t, iui_easing_t easing);

/* Apply motion configuration for asymmetric enter/exit animations
 * @t:           normalized time (0.0 to 1.0, where 0=start, 1=complete)
 * @is_entering: true for enter/show, false for exit/hide
 * @config:      motion configuration (durations ignored, only easing used)
 *
 * Returns eased value (0.0 to 1.0)
 */
float iui_motion_apply(float t,
                       bool is_entering,
                       const iui_motion_config *config);

/* Calculate animation progress from elapsed time using motion config
 * @elapsed:      elapsed time in seconds
 * @is_entering:  true for enter/show, false for exit/hide
 * @config:       motion configuration with duration and easing
 *
 * Returns eased value (0.0 to 1.0), clamped to [0,1]
 */
float iui_motion_progress(float elapsed,
                          bool is_entering,
                          const iui_motion_config *config);

/* Get appropriate duration from motion config
 * @is_entering: true for enter duration, false for exit duration
 *
 * Returns duration in seconds
 */
float iui_motion_get_duration(bool is_entering,
                              const iui_motion_config *config);

/* Convenience: apply standard easing (most common case) */
static inline float iui_ease_standard(float t)
{
    return iui_ease(t, IUI_EASING_STANDARD);
}

/* Convenience: apply emphasized decelerate (attention-grabbing enter) */
static inline float iui_ease_emphasized_decel(float t)
{
    return iui_ease(t, IUI_EASING_EMPHASIZED_DECELERATE);
}

/* Component state helper functions
 * Get the current state of a component based on bounds and interaction
 */
iui_state_t iui_get_component_state(iui_context *ctx,
                                    iui_rect_t bounds,
                                    bool disabled);

/* Get appropriate color for a component based on its state */
uint32_t iui_get_state_color(iui_context *ctx,
                             iui_state_t state,
                             uint32_t base_color,
                             uint32_t hover_color,
                             uint32_t pressed_color);

/* MD3 Elevation/Shadow System
 * Creates depth perception through dual-shadow rendering (ambient + key)
 *
 * Material Design 3 uses two shadow types for realistic depth:
 * - Key shadow (umbra): Directional from light above, creates primary depth
 * - Ambient shadow (penumbra): Omnidirectional soft glow around edges
 *
 * Shadow uses theme.shadow color (typically black) with per-layer alpha
 * Configuration can be overridden via compile-time defines:
 * IUI_SHADOW_KEY_LAYERS (default 3), IUI_SHADOW_AMBIENT_LAYERS (default 2)
 * IUI_SHADOW_KEY_ALPHA (default 0.15), IUI_SHADOW_AMBIENT_ALPHA (default
 * 0.08)
 *
 * Elevation levels correspond to MD3 component types:
 * elevation_0: Flat surfaces (no shadow)
 * elevation_1: Cards, chips (subtle depth)
 * elevation_2: Buttons, FAB (interactive depth)
 * elevation_3: Menus, dialogs (overlay depth)
 * elevation_4: Navigation drawers (high prominence)
 * elevation_5: Modals, popovers (maximum depth)
 *
 * Draw shadow behind an element (call before drawing the element)
 */
void iui_draw_shadow(iui_context *ctx,
                     iui_rect_t bounds,
                     float corner_radius,
                     iui_elevation_t level);

/* Convenience: draw a rounded box with shadow behind it
 * Combines iui_draw_shadow + draw_box in one call
 */
void iui_draw_elevated_box(iui_context *ctx,
                           iui_rect_t bounds,
                           float corner_radius,
                           iui_elevation_t level,
                           uint32_t color);

/* Clip stack functions for nested clipping regions
 * Returns true on success, false if stack overflow
 */
bool iui_push_clip(iui_context *ctx, iui_rect_t rect);
void iui_pop_clip(iui_context *ctx);
bool iui_is_clipped(iui_context *ctx, iui_rect_t rect);

/* Modal blocking functions
 *
 * Usage pattern:
 * 1. Render background windows/widgets (will be blocked when modal is active)
 * 2. Call iui_begin_modal(ctx, "modal_id") to start modal
 * 3. Render modal content (will accept input)
 * 4. Call iui_end_modal(ctx)
 * 5. Check iui_modal_should_close(ctx) for click-outside-to-dismiss
 * 6. Call iui_close_modal(ctx) when user confirms/cancels to fully deactivate
 *
 * Overlay Naming Conventions:
 * The library uses two patterns for overlay lifecycle functions:
 *
 * show/close - Transient modals with confirmation semantics:
 *   iui_dialog_show/close, iui_snackbar_show, iui_date_picker_show/close,
 *   iui_time_picker_show/close
 *   These appear briefly, typically await user action, then disappear.
 *
 * open/close - Persistent overlays with toggle semantics:
 *   iui_menu_open/close, iui_nav_drawer_open/close,
 * iui_bottom_sheet_open/close, iui_fullscreen_dialog_open/close,
 * iui_search_view_open/close These remain visible until explicitly dismissed,
 * can be toggled.
 */

/* Begin a modal window
 * @ctx: current UI context
 * @id:  unique identifier for the modal
 */
void iui_begin_modal(iui_context *ctx, const char *id);

void iui_end_modal(iui_context *ctx);
void iui_close_modal(iui_context *ctx);
bool iui_is_modal_active(const iui_context *ctx);
bool iui_modal_should_close(const iui_context *ctx);

/* Input Layer System
 *
 * This system provides reliable input blocking for overlay components:
 * - Modal dialogs, bottom sheets, navigation drawers
 * - Nested overlays with correct Z-ordering
 * - Double-buffered registration prevents same-frame race conditions
 *
 * Usage:
 * // Push a layer before rendering overlay
 * int layer = iui_push_layer(ctx, 100);  // z_order: higher = on top
 *
 * // Register blocking region (auto-called by modal widgets)
 * iui_register_blocking_region(ctx, dialog_bounds);
 *
 * // Render overlay widgets...
 * iui_pop_layer(ctx);
 *
 * For widgets, replace manual modal checks with:
 * if (iui_should_process_input(ctx, widget_bounds)) { ... }
 */

/* Push a new input layer onto the stack
 * @z_order:    Higher values are "on top" and block lower layers
 *
 * Returns Layer ID (always > 0), or 0 on stack overflow
 */
int iui_push_layer(iui_context *ctx, int z_order);

/* Pop the current layer from the stack */
void iui_pop_layer(iui_context *ctx);

/* Check if input should be processed for the given bounds
 * Returns true if no blocking region covers the bounds, or if the bounds are
 *         within the current layer's regions.
 * This replaces scattered (modal.active && !modal.rendering) checks.
 */
bool iui_should_process_input(iui_context *ctx, iui_rect_t bounds);

/* Register a blocking region for the current layer
 * Regions registered in frame N block input in frame N+1 (double-buffered).
 * Called automatically by modal widgets (dialog, menu, sheet).
 * Returns true on success, false if region limit reached.
 */
bool iui_register_blocking_region(iui_context *ctx, iui_rect_t bounds);

/* Get current layer ID (0 = base layer, no overlay active) */
int iui_get_current_layer(const iui_context *ctx);

/* Check if any input layer is active (overlay is showing) */
bool iui_has_active_layer(const iui_context *ctx);

/* Get current layer stack depth (0 = no layers, 1 = one overlay, etc.) */
int iui_get_layer_depth(const iui_context *ctx);

/* MD3 Keyboard Focus System
 * Reference: https://m3.material.io/foundations/interaction/states#focus
 * Provides keyboard navigation and visual focus indicators for accessibility
 *
 * Features:
 * - Tab/Shift+Tab navigation between focusable widgets
 * - Arrow keys for navigation within component groups
 * - Escape clears focus or closes modals
 * - Focus trap: modals contain focus within bounds
 * - Visual focus indicator: 3dp primary color ring with 2dp offset
 * - Focus state layer: 12% opacity
 *
 * Usage:
 * In the SDL input handler:
 * if (key == SDLK_TAB) {
 *     if (shift_held)
 *         iui_focus_prev(ui);
 *     else
 *         iui_focus_next(ui);
 * }
 *
 * Widgets automatically register for focus and show focus ring when
 * focused. Use iui_has_focus() to check if a specific widget has focus.
 * if (iui_has_focus(ui, "my_button")) { ... }
 *
 * Note: Focus is automatically cleared when mouse clicks occur.
 * Set focus to a specific widget by its string identifier
 */
void iui_set_focus(iui_context *ctx, const char *id);

/* Check if a widget currently has keyboard focus */
bool iui_has_focus(const iui_context *ctx, const char *id);

/* Clear all keyboard focus */
void iui_clear_focus(iui_context *ctx);

/* Move focus to the next focusable widget (Tab key) */
void iui_focus_next(iui_context *ctx);

/* Move focus to the previous focusable widget (Shift+Tab) */
void iui_focus_prev(iui_context *ctx);

/* Get the currently focused widget ID (0 if none) */
uint32_t iui_get_focused_id(const iui_context *ctx);

/* Check if any widget currently has focus */
bool iui_has_any_focus(const iui_context *ctx);

/* Focus Trapping for Modal Layers
 * Reference: https://www.w3.org/WAI/ARIA/apg/patterns/dialog-modal/
 *
 * When a modal is active, focus navigation must be constrained to that layer.
 * This is required for accessibility compliance (ARIA modal pattern).
 *
 * Usage:
 * iui_begin_modal(ctx, "dialog");
 * iui_focus_trap_begin(ctx, layer_id);  // Trap focus to modal
 * // ... render modal content with focusable widgets ...
 * iui_focus_trap_end(ctx);
 * iui_end_modal(ctx);
 */

/* Begin focus trap - constrains Tab/Shift+Tab to current layer
 * @layer_id: layer ID from iui_push_layer() or modal.layer_id
 */
void iui_focus_trap_begin(iui_context *ctx, int layer_id);

/* End focus trap - restores normal focus navigation */
void iui_focus_trap_end(iui_context *ctx);

/* Check if focus trap is active for a given layer */
bool iui_layer_is_focused(const iui_context *ctx, int layer_id);

/* Input Capture for Drag Operations
 * Reference: raym3 InputLayer.cpp:BeginInputCapture()
 *
 * Prevents accidental activation when mouse enters component mid-drag.
 * When capture is active, only the capturing widget receives input.
 *
 * Usage:
 * if (iui_begin_input_capture(ctx, bounds, true)) {
 *     // This widget owns mouse input until release
 *     if (mouse_held) update_drag(ctx);
 *     if (mouse_released) iui_release_capture(ctx);
 * }
 */

/* Begin input capture for a widget
 * @bounds: widget bounds to capture within
 * @require_start_in_bounds: if true, only capture if mouse down started here
 *
 * Returns true if this widget owns capture (should process drag input)
 */
bool iui_begin_input_capture(iui_context *ctx,
                             iui_rect_t bounds,
                             bool require_start_in_bounds);

/* Check if input capture is currently active */
bool iui_is_input_captured(const iui_context *ctx);

/* Release input capture (call on mouse release) */
void iui_release_capture(iui_context *ctx);

/* IME (Input Method Editor) Support
 * For international text input (Chinese, Japanese, Korean, etc.)
 *
 * Usage:
 * // Platform callback for IME composition update
 * void on_ime_composition(const char *text, int cursor) {
 *     iui_update_composition(ctx, text, cursor);
 * }
 * void on_ime_commit(const char *text) {
 *     iui_commit_composition(ctx, text);
 * }
 */

/* Update IME composition text (called by platform during composition) */
void iui_update_composition(iui_context *ctx, const char *text, int cursor);

/* Commit IME composition (called when user confirms composition) */
void iui_commit_composition(iui_context *ctx, const char *text);

/* Check if IME composition is active */
bool iui_ime_is_composing(const iui_context *ctx);

/* Get current IME composition text (NULL if not composing) */
const char *iui_ime_get_text(const iui_context *ctx);

/* Clipboard Support
 *
 * Usage:
 * iui_clipboard_t clipboard = {
 *     .get = my_get_clipboard,
 *     .set = my_set_clipboard,
 *     .user = my_context
 * };
 * iui_set_clipboard_callbacks(ctx, &clipboard);
 *
 * // Then Ctrl+C/V work automatically in text fields
 */

/* Set clipboard callbacks for copy/paste support */
void iui_set_clipboard_callbacks(iui_context *ctx, const iui_clipboard_t *cb);

/* Copy selected text to clipboard (returns true if copied) */
bool iui_clipboard_copy(iui_context *ctx, const char *text, size_t len);

/* Paste text from clipboard into buffer (returns bytes pasted) */
size_t iui_clipboard_paste(iui_context *ctx, char *buffer, size_t buffer_size);

/* Per-frame field tracking - prevents stale state for conditionally hidden
 * widgets. Text fields and sliders register themselves automatically each
 * frame. State is cleared for fields not seen during the current frame.
 * This is called automatically by iui_begin_frame()/iui_end_frame().
 * Exposed for advanced use cases (e.g., manual stale state cleanup).
 */
void iui_reset_field_ids(iui_context *ctx);

/* MD3 Button variants */
bool iui_button_styled(iui_context *ctx,
                       const char *label,
                       iui_text_alignment_t alignment,
                       iui_button_style_t style);

/* Convenience macros for different button styles */
#define iui_filled_button(ctx, lbl, align) \
    iui_button_styled(ctx, lbl, align, IUI_BUTTON_FILLED)
#define iui_outlined_button(ctx, lbl, align) \
    iui_button_styled(ctx, lbl, align, IUI_BUTTON_OUTLINED)
#define iui_text_button(ctx, lbl, align) \
    iui_button_styled(ctx, lbl, align, IUI_BUTTON_TEXT)
#define iui_elevated_button(ctx, lbl, align) \
    iui_button_styled(ctx, lbl, align, IUI_BUTTON_ELEVATED)
#define iui_tonal_button(ctx, lbl, align) \
    iui_button_styled(ctx, lbl, align, IUI_BUTTON_TONAL)

/* MD3 FAB (Floating Action Button) Component
 * Reference: https://m3.material.io/components/floating-action-button
 * Primary action button with elevated surface and prominent icon
 *
 * Variants:
 * - Standard FAB: 56dp square with 24dp icon
 * - Large FAB: 96dp square with 36dp icon
 * - Extended FAB: variable width with icon and label
 *
 * Colors (MD3):
 * - Container: primary_container
 * - Icon/Label: on_primary_container
 * - Elevation: Level 3 (elevation_3)
 *
 * Usage:
 * Standard FAB with add icon
 * if (iui_fab(ctx, "add")) handle_add();
 *
 * Extended FAB with icon and label
 * if (iui_fab_extended(ctx, "compose", "New message")) handle_compose();
 *
 * Icon names: "add", "edit", "check", "close", "search", "favorite", "share"
 *
 * Note: FABs are positioned absolutely - they do not advance the layout cursor.
 * The caller should specify position using the x,y parameters.
 */

/* FAB size variants */
typedef enum iui_fab_size {
    IUI_FAB_STANDARD, /* 56dp */
    IUI_FAB_SMALL,    /* 40dp (for compact spaces) */
    IUI_FAB_LARGE     /* 96dp */
} iui_fab_size_t;

/* Standard FAB (56dp)
 * Returns true if clicked this frame
 */
bool iui_fab(iui_context *ctx, float x, float y, const char *icon);

/* Large FAB (96dp) */
bool iui_fab_large(iui_context *ctx, float x, float y, const char *icon);

/* Extended FAB with label (56dp height, variable width)
 * @icon: optional icon name (NULL for label-only)
 * @label: required text label
 */
bool iui_fab_extended(iui_context *ctx,
                      float x,
                      float y,
                      const char *icon,
                      const char *label);

/* MD3 Icon Button Component
 * Reference: https://m3.material.io/components/icon-buttons
 * Compact buttons for actions with icons only
 *
 * Variants:
 * - Standard: No container, icon only with state layer on hover/press
 * - Filled: Solid primary container background
 * - Filled Tonal: Secondary container background (lower emphasis)
 * - Outlined: Transparent with outline border
 *
 * All variants:
 * - Size: 40dp container, 48dp touch target
 * - Icon size: 24dp
 * - State layers: hover (8%), focus (12%), pressed (12%)
 * - Toggle support for all variants
 *
 * Colors (MD3):
 * - Standard: on_surface_variant icon, no container
 * - Filled: on_primary icon, primary container
 * - Filled Tonal: on_secondary_container icon, secondary_container
 * - Outlined: on_surface_variant icon, outline border
 *
 * Usage:
 * Standard icon button
 * if (iui_icon_button(ctx, "settings")) handle_settings();
 *
 * Toggle icon button (selected state)
 * static bool bookmarked = false;
 * if (iui_icon_button_toggle(ctx, "favorite", &bookmarked)) { }
 *
 */
/* Icon names: "add", "edit", "check", "close", "search", "favorite", "share",
 * "settings", "menu", "more_vert", "arrow_back", "arrow_forward"
 *
 * Note: Icon buttons participate in layout (unlike FAB which is positioned
 * absolutely).
 */

/* Standard Icon Button (no container)
 * Returns true if clicked this frame
 */
bool iui_icon_button(iui_context *ctx, const char *icon);

/* Filled Icon Button (primary container) */
bool iui_icon_button_filled(iui_context *ctx, const char *icon);

/* Filled Tonal Icon Button (secondary_container) */
bool iui_icon_button_tonal(iui_context *ctx, const char *icon);

/* Outlined Icon Button (outline border) */
bool iui_icon_button_outlined(iui_context *ctx, const char *icon);

/* Toggle Icon Button (standard style with selected state)
 * @selected: pointer to toggle state
 * Returns true if clicked this frame (toggle happens automatically)
 */
bool iui_icon_button_toggle(iui_context *ctx, const char *icon, bool *selected);

/* Toggle Icon Button with Filled style */
bool iui_icon_button_toggle_filled(iui_context *ctx,
                                   const char *icon,
                                   bool *selected);

/* MD3 Top App Bar Component
 * Reference: https://m3.material.io/components/top-app-bar/specs
 * Primary navigation and contextual actions at the top of screens
 *
 * Variants:
 * - Small: 64dp fixed, title_large, leading nav icon + trailing actions
 * - Center-aligned: 64dp fixed, centered title
 * - Medium: 112dp→64dp on scroll, headline_small, prominent title area
 * - Large: 152dp→64dp on scroll, headline_small, larger title area
 *
 * Features:
 * - Leading navigation icon (menu or back arrow)
 * - Up to 3 trailing action icons
 * - Scroll-driven collapse animation for Medium/Large variants
 * - Color transition: surface → surface_container on scroll
 * - Elevation transition: Level 0 → Level 2 on scroll
 *
 * Colors (MD3):
 * - Container: surface (default), surface_container (scrolled)
 * - Title: on_surface
 * - Icons: on_surface_variant
 *
 * Usage:
 * bool nav_clicked = iui_top_app_bar(ctx, "Page Title", appbar_small, 0.f);
 * if (iui_top_app_bar_action(ctx, "search")) handle_search();
 * if (iui_top_app_bar_action(ctx, "more_vert")) handle_menu();
 * if (nav_clicked) toggle_drawer();
 *
 * For collapsible variants, pass scroll_offset from scroll container:
 * bool nav = iui_top_app_bar(ctx, "Title", appbar_medium,
 * scroll_state.scroll_y);
 * Note: Top App Bar occupies full layout width and advances the layout cursor.
 */

/* Top App Bar
 * @title: displayed title text
 * @size: appbar_small, appbar_center, appbar_medium, or appbar_large
 * @scroll_offset: scroll position for collapse animation (0.f for fixed bars)
 *
 * Returns true if navigation icon was clicked
 */
bool iui_top_app_bar(iui_context *ctx,
                     const char *title,
                     iui_appbar_size_t size,
                     float scroll_offset);

/* Top App Bar Action Icon
 * Call after iui_top_app_bar() for each trailing action (max 3)
 * Icons are positioned right-to-left in call order
 * Returns true if this action icon was clicked
 */
bool iui_top_app_bar_action(iui_context *ctx, const char *icon);

/* MD3 Tabs Component
 * Reference: https://m3.material.io/components/tabs
 * Horizontal navigation component for switching between content views
 *
 * Variants:
 * - Primary tabs: Main navigation with label only or icon + label
 * - Secondary tabs: Sub-navigation within content (text only)
 *
 * Features:
 * - Animated active indicator (3dp height, primary color)
 * - Indicator slides between tabs on selection change
 * - Equal-width distribution across container
 * - State layers for hover/press feedback
 *
 * Dimensions (MD3):
 * - Height: 48dp
 * - Minimum tab width: 90dp
 * - Indicator height: 3dp
 * - Icon size: 24dp
 *
 * Colors (MD3):
 * - Active: primary (indicator and text/icon)
 * - Inactive: on_surface_variant
 * - Container: surface
 *
 * Usage:
 * static int selected_tab = 0;
 * const char *labels[] = {"Home", "Search", "Settings"};
 * int new_selection = iui_tabs(ctx, selected_tab, 3, labels);
 * if (new_selection != selected_tab) {
 *     selected_tab = new_selection;
 *     // Handle tab change
 * }
 *
 * Note: Tabs occupy full layout width and advance the layout cursor.
 */

/* Primary Tabs (label only)
 * Returns the newly selected tab index (may be same as input if no change)
 */
int iui_tabs(iui_context *ctx, int selected, int count, const char **labels);

/* Primary Tabs with Icons (icon + label)
 * @icons: array of icon names (same icons as FAB/icon buttons)
 *
 * Returns the newly selected tab index
 */
int iui_tabs_with_icons(iui_context *ctx,
                        int selected,
                        int count,
                        const char **labels,
                        const char **icons);

/* Secondary Tabs (text only, simpler styling) */
int iui_tabs_secondary(iui_context *ctx,
                       int selected,
                       int count,
                       const char **labels);

/* MD3 Typography functions - headline, title, body, label variants.
 * All share the same signature: (ctx, alignment, format_string, ...)
 * Explicit declarations for IDE go-to-definition and grep support.
 */
void iui_text_headline_small(iui_context *ctx,
                             iui_text_alignment_t alignment,
                             const char *string,
                             ...);
void iui_text_title_large(iui_context *ctx,
                          iui_text_alignment_t alignment,
                          const char *string,
                          ...);
void iui_text_title_medium(iui_context *ctx,
                           iui_text_alignment_t alignment,
                           const char *string,
                           ...);
void iui_text_title_small(iui_context *ctx,
                          iui_text_alignment_t alignment,
                          const char *string,
                          ...);
void iui_text_body_large(iui_context *ctx,
                         iui_text_alignment_t alignment,
                         const char *string,
                         ...);
void iui_text_body_medium(iui_context *ctx,
                          iui_text_alignment_t alignment,
                          const char *string,
                          ...);
void iui_text_body_small(iui_context *ctx,
                         iui_text_alignment_t alignment,
                         const char *string,
                         ...);
void iui_text_label_large(iui_context *ctx,
                          iui_text_alignment_t alignment,
                          const char *string,
                          ...);
void iui_text_label_medium(iui_context *ctx,
                           iui_text_alignment_t alignment,
                           const char *string,
                           ...);
void iui_text_label_small(iui_context *ctx,
                          iui_text_alignment_t alignment,
                          const char *string,
                          ...);

/* Advanced TextField with icons and extended options
 * Supports leading/trailing icons, password mode, read-only, and disabled
 * states
 *
 * Usage:
 * iui_textfield_options opts = {
 *     .style = textfield_outlined,
 *     .placeholder = "Search...",
 *     .leading_icon = textfield_icon_search,
 *     .trailing_icon = textfield_icon_clear,
 * };
 * iui_textfield_result res = iui_textfield(ctx, buffer, sizeof(buffer),
 *                                          &cursor, &opts);
 * if (res.trailing_icon_clicked) buffer[0] = '\0';  // clear on X click
 *
 * Returns result struct with value_changed, submitted, and icon click states
 */
iui_textfield_result iui_textfield(iui_context *ctx,
                                   char *buffer,
                                   size_t size,
                                   size_t *cursor,
                                   const iui_textfield_options *options);

/* Text Edit with Selection Support
 *
 * Enhanced text editing with full selection capabilities:
 * - Selection highlight (primary @ 40% opacity)
 * - Shift+arrow extends selection
 * - Double-click selects word
 * - Triple-click selects all (line)
 * - Click-to-position cursor with nearest-character snapping
 * - Mouse drag selection
 * - Horizontal scroll for text wider than field
 *
 * The iui_edit_state structure maintains selection state across frames.
 * Initialize state to zero on first use: iui_edit_state state = {0};
 *
 * Returns true if text was modified (inserted, deleted, or pasted).
 */
bool iui_edit_with_selection(iui_context *ctx,
                             char *buffer,
                             size_t buffer_size,
                             iui_edit_state *state);

/* Advanced TextField with Selection and Icons
 *
 * Combines full selection support with textfield styling, icons, and options.
 * Use this for feature-rich text input with leading/trailing icons.
 */
iui_textfield_result iui_textfield_with_selection(
    iui_context *ctx,
    char *buffer,
    size_t size,
    iui_edit_state *state,
    const iui_textfield_options *options);

/* Switch (Toggle) with Icons */
bool iui_switch(iui_context *ctx,
                const char *label,
                bool *value,
                const char *on_icon,   /* optional: "check" glyph */
                const char *off_icon); /* optional: "x" glyph */

/* Card Container functions */
void iui_card_begin(iui_context *ctx,
                    float x,
                    float y,
                    float w,
                    float h,
                    iui_card_style_t style);
void iui_card_end(iui_context *ctx);

/* Progress Indicators */
void iui_progress_linear(iui_context *ctx,
                         float value,
                         float max,
                         bool indeterminate);
void iui_progress_circular(iui_context *ctx,
                           float value,
                           float max,
                           float size,
                           bool indeterminate);

/* Snackbar Component
 * Timed notification at bottom of screen with optional action button
 * Uses MD3 inverse colors for contrast against both light and dark themes
 *
 * Usage:
 * static iui_snackbar_state snackbar = {0};
 * if (some_event)
 *     iui_snackbar_show(&snackbar, "Item deleted", 4.f, "Undo");
 * if (iui_snackbar(ctx, &snackbar, screen_width, screen_height))
 *     handle_undo_action();
 */

void iui_snackbar_show(iui_snackbar_state *snackbar,
                       const char *message,
                       float duration,
                       const char *action_label);
void iui_snackbar_hide(iui_snackbar_state *snackbar);
bool iui_snackbar(iui_context *ctx,
                  iui_snackbar_state *snackbar,
                  float screen_width,
                  float screen_height);

/* Tooltip
 * Reference: https://m3.material.io/components/tooltips
 * Plain tooltips display brief labels for UI elements on hover/focus.
 *
 * Usage:
 * if (button_hovered)
 *     iui_tooltip(ctx, "Save document");
 *
 * Rich tooltips support title, message, and optional action button.
 */

/* Display a plain tooltip near the current layout position
 * @ctx:  current UI context
 * @text: tooltip text to display
 */
void iui_tooltip(iui_context *ctx, const char *text);

/* Display a rich tooltip with title, body text, and optional action
 * @ctx:    current UI context
 * @title:  tooltip title (can be NULL)
 * @text:   tooltip body text
 * @action: action button label (can be NULL)
 *
 * Returns true if action button was clicked
 */
bool iui_tooltip_rich(iui_context *ctx,
                      const char *title,
                      const char *text,
                      const char *action);

/* Badge
 * Reference: https://m3.material.io/components/badges
 * Badges show notifications, counts, or status information on icons/buttons.
 *
 * Usage:
 * // After drawing an icon at (x, y):
 * iui_badge_dot(ctx, icon_x + icon_w, icon_y);           // notification dot
 * iui_badge_number(ctx, icon_x + icon_w, icon_y, 5, 99); // count badge "5"
 */

/* Display a small notification dot badge at the anchor position
 * @ctx:      current UI context
 * @anchor_x: x position of anchor (typically top-right of icon)
 * @anchor_y: y position of anchor
 */
void iui_badge_dot(iui_context *ctx, float anchor_x, float anchor_y);

/* Display a numbered badge at the anchor position
 * @ctx:       current UI context
 * @anchor_x:  x position of anchor
 * @anchor_y:  y position of anchor
 * @count:     number to display
 * @max_count: maximum before showing "99+" (0 = no limit)
 */
void iui_badge_number(iui_context *ctx,
                      float anchor_x,
                      float anchor_y,
                      int count,
                      int max_count);

/* Banner
 * Reference: https://m3.material.io/components/banners
 * Banners display important messages at the top of a screen with actions.
 *
 * Usage:
 * static bool show_banner = true;
 * iui_banner_options opts = {
 *     .message = "Network connection lost",
 *     .action1 = "Retry",
 *     .action2 = "Dismiss",
 *     .icon = "wifi_off",
 * };
 * int result = iui_banner(ctx, &opts);
 * if (result == 1) retry_connection();
 * if (result == 2) show_banner = false;
 */

/* Banner configuration options */
typedef struct {
    const char *message; /* required: banner message text */
    const char *action1; /* primary action button label (can be NULL) */
    const char *action2; /* secondary action button label (can be NULL) */
    const char *icon;    /* leading icon name (can be NULL) */
} iui_banner_options;

/* Display a banner at the top of the current layout
 * @ctx:     current UI context
 * @options: banner configuration
 *
 * Returns 0 if no action, 1 if action1 clicked, 2 if action2 clicked
 */
int iui_banner(iui_context *ctx, const iui_banner_options *options);

/* Data Table
 * Reference: https://m3.material.io/components/data-tables
 * Data tables display sets of data in a tabular format.
 *
 * Usage:
 * static iui_table_state table = {0};  // user-provided state
 * iui_table_begin(ctx, &table, 3, (float[]){-1, 100, -1});
 * iui_table_header(ctx, &table, "Name");
 * iui_table_header(ctx, &table, "Age");
 * iui_table_header(ctx, &table, "Email");
 * for (int i = 0; i < row_count; i++) {
 *     iui_table_row_begin(ctx, &table);
 *     iui_table_cell(ctx, &table, data[i].name);
 *     iui_table_cell(ctx, &table, "%d", data[i].age);
 *     iui_table_cell(ctx, &table, data[i].email);
 *     iui_table_row_end(ctx, &table);
 * }
 * iui_table_end(ctx, &table);
 */

/* Table state for layout and optional selection/sorting
 * User must provide this struct to table functions for proper state tracking.
 * This enables multiple tables, nested tables, and thread-safe rendering.
 *
 * PUBLIC FIELDS (user-managed, read/write):
 *   sort_column, sort_ascending, selected_row, multi_select, selected_mask
 *   Initialize these before first use; library reads but never modifies them.
 *
 * INTERNAL FIELDS (library-managed, read-only for user):
 *   cols, widths, start_x, current_x, row_y, current_col, in_header, row_index
 *   Do not modify these; they are managed by iui_table_* functions.
 */
typedef struct {
    /* PUBLIC: Selection/sorting state (user-managed) */
    int sort_column;        /* column index for sorting (-1 = none) */
    bool sort_ascending;    /* true = ascending, false = descending */
    int selected_row;       /* selected row index (-1 = none) */
    bool multi_select;      /* enable multiple row selection */
    uint64_t selected_mask; /* bitmask for multi-selection (up to 64 rows) */

    /* INTERNAL: Layout state (managed by table functions - do not modify) */
    int cols;                         /* number of columns */
    float widths[IUI_MAX_TABLE_COLS]; /* computed column widths */
    float start_x;                    /* table start x position */
    float current_x;                  /* current cell x position */
    float row_y;                      /* current row y position */
    int current_col;                  /* current column index */
    bool in_header;                   /* rendering header row */
    int row_index;                    /* current data row index */
} iui_table_state;

/* Begin a data table with specified columns
 * @ctx:     current UI context
 * @state:   table state (user-provided, enables multiple/nested tables)
 * @cols:    number of columns (max IUI_MAX_TABLE_COLS)
 * @widths:  column widths (positive=fixed, negative=flex ratio)
 */
void iui_table_begin(iui_context *ctx,
                     iui_table_state *state,
                     int cols,
                     const float *widths);

/* Render a table header cell
 * @ctx:   current UI context
 * @state: table state from iui_table_begin
 * @text:  header text
 */
void iui_table_header(iui_context *ctx,
                      iui_table_state *state,
                      const char *text);

/* Begin a table data row
 * @ctx:   current UI context
 * @state: table state from iui_table_begin
 */
void iui_table_row_begin(iui_context *ctx, iui_table_state *state);

/* Render a table cell with formatted text
 * @ctx:   current UI context
 * @state: table state from iui_table_begin
 * @text:  cell content (supports printf-style formatting)
 */
void iui_table_cell(iui_context *ctx,
                    iui_table_state *state,
                    const char *text,
                    ...);

/* End the current table row
 * @ctx:   current UI context
 * @state: table state from iui_table_begin
 */
void iui_table_row_end(iui_context *ctx, iui_table_state *state);

/* End the data table
 * @ctx:   current UI context
 * @state: table state from iui_table_begin
 */
void iui_table_end(iui_context *ctx, iui_table_state *state);

/* Scrollable Container
 * Creates a scrollable viewport using the existing clip stack.
 * Content rendered between begin/end is clipped and offset by scroll position.
 *
 * Usage:
 * static iui_scroll_state scroll = {0};
 * iui_scroll_begin(ctx, &scroll, 200, 300);  // 200x300 viewport
 * for (int i = 0; i < 50; i++) {
 *     iui_text(ctx, align_left, "Item %d", i);
 *     iui_newline(ctx);
 * }
 * iui_scroll_end(ctx, &scroll);  // measures content, renders scrollbar
 *
 * Mouse wheel input via iui_update_scroll() adjusts scroll_y automatically.
 * For momentum scrolling, update velocity_y in the game loop and call
 * iui_scroll_by().
 */

/* Begin a scrollable region with the given viewport size
 * Returns the content bounds rect (for drawing scrollbar or other purposes)
 */
iui_rect_t iui_scroll_begin(iui_context *ctx,
                            iui_scroll_state *state,
                            float view_w,
                            float view_h);

/* End the scrollable region; measures content size and renders optional
 * scrollbar
 * Returns true if content is larger than viewport (scrolling is active)
 */
bool iui_scroll_end(iui_context *ctx, iui_scroll_state *state);

/* Scroll by a delta amount (for mouse wheel, touch drag, momentum) */
void iui_scroll_by(iui_scroll_state *state, float dx, float dy);

/* Scroll to an absolute position */
void iui_scroll_to(iui_scroll_state *state, float x, float y);

/* Update scroll input from mouse wheel or trackpad
 * Call this once per frame with the scroll delta (typically from
 * SDL_MOUSEWHEEL)
 */
void iui_update_scroll(iui_context *ctx, float dx, float dy);

/* List Component
 * Continuous, vertical index of text and images for content display
 * Reference: https://m3.material.io/components/lists
 *
 * Usage:
 * for (int i = 0; i < item_count; i++) {
 *     iui_list_item item = {
 *         .headline = items[i].name,
 *         .supporting = items[i].description,
 *         .leading_type = list_leading_icon,
 *         .leading_icon = items[i].icon,
 *         .trailing_type = list_trailing_text,
 *         .trailing_text = items[i].date,
 *         .show_divider = (i < item_count - 1),
 *     };
 *     if (iui_list_item_ex(ctx, list_two_line, &item))
 *         handle_item_click(i);
 * }
 *
 * Simple one-line list:
 * if (iui_list_item_simple(ctx, "Settings", "settings"))
 *     open_settings();
 */

/* Render a list item with full configuration
 * @ctx:  current UI context
 * @type: list item type (one_line, two_line, three_line)
 * @item: list item configuration
 *
 * Returns true if item was clicked (for navigation/selection)
 */
bool iui_list_item_ex(iui_context *ctx,
                      iui_list_type_t type,
                      const iui_list_item *item);

/* Simple one-line list item with optional leading icon
 * @ctx:      current UI context
 * @headline: primary text
 * @icon:     optional leading icon name (NULL = no icon)
 *
 * Returns true if item was clicked
 */
bool iui_list_item_simple(iui_context *ctx,
                          const char *headline,
                          const char *icon);

/* Two-line list item with headline and supporting text
 * @ctx:        current UI context
 * @headline:   primary text
 * @supporting: secondary text
 * @icon:       optional leading icon name (NULL = no icon)
 *
 * Returns true if item was clicked
 */
bool iui_list_item_two_line(iui_context *ctx,
                            const char *headline,
                            const char *supporting,
                            const char *icon);

/* List divider (inset by 16dp from left) */
void iui_list_divider(iui_context *ctx);

/* Navigation Rail Component
 * Vertical navigation for medium-to-large screens (tablet/desktop)
 * Reference: https://m3.material.io/components/navigation-rail
 *
 * Usage:
 * static iui_nav_rail_state rail = {0};
 * iui_nav_rail_begin(ctx, &rail, 0, 0, screen_height);
 * if (iui_nav_rail_fab(ctx, &rail, "add"))
 *     handle_fab_click();
 * if (iui_nav_rail_item(ctx, &rail, "home", "Home", 0))
 *     current_view = VIEW_HOME;
 * if (iui_nav_rail_item(ctx, &rail, "search", "Search", 1))
 *     current_view = VIEW_SEARCH;
 * if (iui_nav_rail_item(ctx, &rail, "settings", "Settings", 2))
 *     current_view = VIEW_SETTINGS;
 * iui_nav_rail_end(ctx, &rail);
 */

/* Begin navigation rail at specified position
 * @state:  rail state (persists across frames)
 * @x, @y:  position of the rail
 * @height: rail height (typically screen height)
 */
void iui_nav_rail_begin(iui_context *ctx,
                        iui_nav_rail_state *state,
                        float x,
                        float y,
                        float height);

/* Optional FAB at top of rail
 * Returns true if FAB was clicked
 */
bool iui_nav_rail_fab(iui_context *ctx,
                      iui_nav_rail_state *state,
                      const char *icon);

/* Navigation rail item
 * @icon:  icon name (from icon set)
 * @label: text label (shown when expanded or on hover)
 * @index: item index for selection tracking
 *
 * Returns true if item was clicked
 */
bool iui_nav_rail_item(iui_context *ctx,
                       iui_nav_rail_state *state,
                       const char *icon,
                       const char *label,
                       int index);

/* End navigation rail rendering */
void iui_nav_rail_end(iui_context *ctx, iui_nav_rail_state *state);

/* Toggle rail expanded/collapsed state */
void iui_nav_rail_toggle(iui_nav_rail_state *state);

/* Navigation Bar Component
 * Horizontal navigation for mobile screens (bottom of screen)
 * Reference: https://m3.material.io/components/navigation-bar
 *
 * Usage:
 * static iui_nav_bar_state bar = {0};
 * iui_nav_bar_begin(ctx, &bar, 0, screen_height - 80, screen_width, 3);
 * if (iui_nav_bar_item(ctx, &bar, "home", "Home", 0))
 *     current_view = VIEW_HOME;
 * if (iui_nav_bar_item(ctx, &bar, "search", "Search", 1))
 *     current_view = VIEW_SEARCH;
 * if (iui_nav_bar_item(ctx, &bar, "profile", "Profile", 2))
 *     current_view = VIEW_PROFILE;
 * iui_nav_bar_end(ctx, &bar);
 */

/* Begin navigation bar at specified position
 * total_items: expected number of items for width calculation (3-5 typical) */
void iui_nav_bar_begin(iui_context *ctx,
                       iui_nav_bar_state *state,
                       float x,
                       float y,
                       float width,
                       int total_items);

/* Navigation bar item */
bool iui_nav_bar_item(iui_context *ctx,
                      iui_nav_bar_state *state,
                      const char *icon,
                      const char *label,
                      int index);

/* End navigation bar rendering */
void iui_nav_bar_end(iui_context *ctx, iui_nav_bar_state *state);

/* Navigation Drawer Component
 * Side panel navigation for larger screens
 * Reference: https://m3.material.io/components/navigation-drawer
 *
 * Usage:
 * static iui_nav_drawer_state drawer = {.modal = true};
 * if (hamburger_button_clicked)
 *     drawer.open = !drawer.open;
 * if (iui_nav_drawer_begin(ctx, &drawer, 0, 0, screen_height)) {
 *     if (iui_nav_drawer_item(ctx, &drawer, "inbox", "Inbox", 0))
 *         current_view = VIEW_INBOX;
 *     iui_nav_drawer_divider(ctx);
 *     if (iui_nav_drawer_item(ctx, &drawer, "settings", "Settings", 1))
 *         current_view = VIEW_SETTINGS;
 *     iui_nav_drawer_end(ctx, &drawer);
 * }
 */

/* Begin navigation drawer (modal or standard)
 * Returns true if drawer should be rendered (open or animating)
 */
bool iui_nav_drawer_begin(iui_context *ctx,
                          iui_nav_drawer_state *state,
                          float x,
                          float y,
                          float height);

/* Navigation drawer item */
bool iui_nav_drawer_item(iui_context *ctx,
                         iui_nav_drawer_state *state,
                         const char *icon,
                         const char *label,
                         int index);

/* Drawer section divider */
void iui_nav_drawer_divider(iui_context *ctx);

/* End navigation drawer rendering */
void iui_nav_drawer_end(iui_context *ctx, iui_nav_drawer_state *state);

/* Open/close drawer */
void iui_nav_drawer_open(iui_nav_drawer_state *state);
void iui_nav_drawer_close(iui_nav_drawer_state *state);

/* Bottom Sheet
 * Surfaces containing supplementary content anchored to the bottom of screen
 * Reference: https://m3.material.io/components/bottom-sheets
 *
 * Types:
 * - Standard: coexists with screen content, non-blocking
 * - Modal: blocks interaction with rest of screen
 *
 * Usage:
 * static iui_bottom_sheet_state sheet = {.height = 300.f};
 * if (iui_button(ctx, "Show Sheet", align_left))
 *     iui_bottom_sheet_open(&sheet);
 * if (iui_bottom_sheet_begin(ctx, &sheet, screen_width, screen_height)) {
 *     // Sheet content
 *     iui_text(ctx, align_left, "Sheet content");
 *     iui_bottom_sheet_end(ctx, &sheet);
 * }
 */

/* Bottom sheet types */
typedef enum iui_bottom_sheet_type {
    IUI_SHEET_STANDARD, /* Non-blocking, coexists with content */
    IUI_SHEET_MODAL     /* Blocks underlying content */
} iui_bottom_sheet_type_t;

/* Bottom sheet state - persistent across frames */
typedef struct {
    bool open;               /* Whether sheet is visible */
    bool modal;              /* Modal or standard sheet */
    float height;            /* Current sheet height */
    float target_height;     /* Target height for animation */
    float min_height;        /* Minimum draggable height */
    float max_height;        /* Maximum draggable height */
    float anim_progress;     /* Animation progress 0-1 */
    bool dragging;           /* Currently being dragged */
    float drag_start_y;      /* Y position when drag started */
    float drag_start_height; /* Height when drag started */
} iui_bottom_sheet_state;

/* Begin bottom sheet rendering. Returns true if sheet is visible */
bool iui_bottom_sheet_begin(iui_context *ctx,
                            iui_bottom_sheet_state *state,
                            float screen_width,
                            float screen_height);

/* End bottom sheet rendering */
void iui_bottom_sheet_end(iui_context *ctx, iui_bottom_sheet_state *state);

/* Open/close bottom sheet */
void iui_bottom_sheet_open(iui_bottom_sheet_state *state);
void iui_bottom_sheet_close(iui_bottom_sheet_state *state);

/* Set sheet height (for programmatic control) */
void iui_bottom_sheet_set_height(iui_bottom_sheet_state *state, float height);

/* Bottom App Bar
 * Navigation and key actions at bottom of mobile screens
 * Reference: https://m3.material.io/components/bottom-app-bar
 *
 * Usage:
 * static int action_count = 0;
 * iui_bottom_app_bar_begin(ctx, 0, screen_height - IUI_BOTTOM_APP_BAR_HEIGHT,
 *                          screen_width);
 * if (iui_bottom_app_bar_action(ctx, "search"))
 *     handle_search();
 * if (iui_bottom_app_bar_action(ctx, "delete"))
 *     handle_delete();
 * if (iui_bottom_app_bar_action(ctx, "archive"))
 *     handle_archive();
 * if (iui_bottom_app_bar_fab(ctx, "add", fab_standard))
 *     handle_add();
 * iui_bottom_app_bar_end(ctx);
 */

/* Bottom app bar state */
typedef struct {
    float x, y, width;
    int action_count;
} iui_bottom_app_bar_state;

/* Begin bottom app bar rendering */
void iui_bottom_app_bar_begin(iui_context *ctx,
                              iui_bottom_app_bar_state *state,
                              float x,
                              float y,
                              float width);

/* Add action icon button (left side). Returns true if clicked */
bool iui_bottom_app_bar_action(iui_context *ctx,
                               iui_bottom_app_bar_state *state,
                               const char *icon);

/* Add FAB (right side). Returns true if clicked */
bool iui_bottom_app_bar_fab(iui_context *ctx,
                            iui_bottom_app_bar_state *state,
                            const char *icon,
                            iui_fab_size_t size);

/* End bottom app bar rendering */
void iui_bottom_app_bar_end(iui_context *ctx, iui_bottom_app_bar_state *state);

/* Menu Component
 * Vertical dropdown menu with support for icons, shortcuts, dividers, and
 * disabled items Uses modal blocking internally for outside-click-to-close
 * behavior
 *
 * Usage:
 * static iui_menu_state menu = {0};
 * if (iui_button(ctx, "File", align_left))
 *     iui_menu_open(&menu, "file_menu", button_x, button_y + button_h);
 * if (iui_menu_begin(ctx, &menu, NULL)) {  // NULL = default options
 *     if (iui_menu_add_item(ctx, &menu,
 *                           &(iui_menu_item){.text = "New",
 *                                            .trailing_text = "Ctrl+N"}))
 *         handle_new();
 *     if (iui_menu_add_item(ctx, &menu,
 *                           &(iui_menu_item){.text = "Open",
 *                                            .trailing_text = "Ctrl+O"}))
 *         handle_open();
 *     iui_menu_add_item(ctx, &menu, &(iui_menu_item){.is_divider = true});
 *     if (iui_menu_add_item(ctx, &menu, &(iui_menu_item){.text = "Exit"}))
 *         handle_exit();
 *     iui_menu_end(ctx, &menu);
 * }
 */

/* Open a menu at the specified position
 * @id:    unique identifier for the menu (hashed internally)
 * @x, @y: position where menu should appear (typically below trigger button)
 */
void iui_menu_open(iui_menu_state *menu, const char *id, float x, float y);

/* Close the menu */
void iui_menu_close(iui_menu_state *menu);

/* Begin menu rendering.
 * @options: optional customization (NULL = use defaults)
 *
 * Returns true if menu is open and should be rendered.
 * Must be paired with iui_menu_end() when returns true
 */
bool iui_menu_begin(iui_context *ctx,
                    iui_menu_state *menu,
                    const iui_menu_options *options);

/* Render a menu item.
 * @item: menu item definition (text, icons, divider, etc.)
 *
 * Returns true if item was clicked.
 * For dividers/gaps, returns false (non-interactive)
 */
bool iui_menu_add_item(iui_context *ctx,
                       iui_menu_state *menu,
                       const iui_menu_item *item);

/* End menu rendering. Handles outside-click-to-close. */
void iui_menu_end(iui_context *ctx, iui_menu_state *menu);

/* Check if a menu is currently open */
bool iui_menu_is_open(const iui_menu_state *menu);

/* Dialog Component
 * Pre-built dialog widget for alerts, confirmations, and simple user choices
 * Auto-sizes based on content (min 320px, max 560px width)
 * Uses modal blocking internally and elevation_3 shadow
 *
 * Usage:
 * static iui_dialog_state dialog = {0};
 * if (some_trigger)
 *     iui_dialog_show(&dialog,
 *                     "Confirm", "Delete this item?", "Cancel;Delete");
 * int result = iui_dialog(ctx, &dialog, screen_width, screen_height);
 * if (result >= 0) {
 *     if (result == 1) handle_delete();  // "Delete" clicked
 *     iui_dialog_close(&dialog);
 * }
 */

/* Show a dialog with the given title, message, and semicolon-separated buttons
 * Example buttons: "OK" (single), "Yes;No" (two), "Save;Discard;Cancel" (three)
 */
void iui_dialog_show(iui_dialog_state *dialog,
                     const char *title,
                     const char *message,
                     const char *buttons);

/* Close the dialog and reset state */
void iui_dialog_close(iui_dialog_state *dialog);

/* Render the dialog.
 * @ctx:           current UI context
 * @dialog:        dialog state structure
 * @screen_width:  used for centering the dialog
 * @screen_height: used for centering the dialog
 *
 * Returns clicked button index (0-N), or -1 if no button clicked yet.
 * Call iui_dialog_close() after handling the button click.
 */
int iui_dialog(iui_context *ctx,
               iui_dialog_state *dialog,
               float screen_width,
               float screen_height);

/* Check if dialog is currently open */
bool iui_dialog_is_open(const iui_dialog_state *dialog);

/* MD3 Full-Screen Dialog Component
 * Reference: https://m3.material.io/components/dialogs
 * Modal dialog covering the entire screen for immersive tasks
 *
 * Features:
 * - Full screen coverage with surface background
 * - Top app bar header with close (X) icon and title
 * - Optional action buttons in header (max 2)
 * - Content area for custom widgets
 *
 * Dimensions (MD3):
 * - Header height: 56dp
 * - Close icon: 24dp with 48dp touch target
 * - Action buttons in header area
 *
 * Colors (MD3):
 * - Container: surface
 * - Header: surface (no elevation)
 * - Title: on_surface
 * - Icons: on_surface_variant
 *
 * Usage:
 * static iui_fullscreen_dialog_state dialog = {0};
 * if (trigger)
 *     iui_fullscreen_dialog_open(&dialog, "Edit Profile");
 * if (iui_fullscreen_dialog_begin(ctx, &dialog, screen_w, screen_h)) {
 *     if (iui_fullscreen_dialog_action(ctx, &dialog, "Save"))
 *         handle_save();
 *     // ... render content widgets ...
 *     iui_fullscreen_dialog_end(ctx, &dialog);
 * }
 */

/* Open a full-screen dialog */
void iui_fullscreen_dialog_open(iui_fullscreen_dialog_state *dialog,
                                const char *title);

/* Close the full-screen dialog */
void iui_fullscreen_dialog_close(iui_fullscreen_dialog_state *dialog);

/* Begin rendering full-screen dialog
 * Returns true if dialog is open (render content), false if closed
 * Close icon click automatically closes the dialog
 */
bool iui_fullscreen_dialog_begin(iui_context *ctx,
                                 iui_fullscreen_dialog_state *dialog,
                                 float screen_width,
                                 float screen_height);

/* Add action button to header (max 2, rightmost is primary)
 * Returns true if action was clicked
 */
bool iui_fullscreen_dialog_action(iui_context *ctx,
                                  iui_fullscreen_dialog_state *dialog,
                                  const char *label);

/* End full-screen dialog rendering */
void iui_fullscreen_dialog_end(iui_context *ctx,
                               iui_fullscreen_dialog_state *dialog);

/* Check if full-screen dialog is open */
bool iui_fullscreen_dialog_is_open(const iui_fullscreen_dialog_state *dialog);

/* MD3 Search View Component
 * Reference: https://m3.material.io/components/search
 * Full-screen modal search experience with suggestions
 *
 * Features:
 * - Full screen modal overlay
 * - Search input field in header with back arrow
 * - Suggestion list below search field
 * - Clear button when query is non-empty
 *
 * Dimensions (MD3):
 * - Header height: 72dp
 * - Suggestion item: 56dp height
 * - Icon size: 24dp
 *
 * Colors (MD3):
 * - Container: surface
 * - Search field: surface_container_high
 * - Suggestion text: on_surface
 * - Icons: on_surface_variant
 *
 * Usage:
 * static iui_search_view_state search = {0};
 * if (search_trigger)
 *     iui_search_view_open(&search);
 * if (iui_search_view_begin(ctx, &search, screen_w, screen_h, "Search...")) {
 *     if (iui_search_view_suggestion(ctx, &search, "search", "Recent Item 1"))
 *         handle_suggestion(0);
 *     if (iui_search_view_suggestion(ctx, &search, "history", "Recent Item 2"))
 *         handle_suggestion(1);
 *     iui_search_view_end(ctx, &search);
 * }
 */

/* Open the search view */
void iui_search_view_open(iui_search_view_state *search);

/* Close the search view */
void iui_search_view_close(iui_search_view_state *search);

/* Begin search view rendering
 * Returns true if search view is open, false if closed
 * Back arrow click automatically closes the search view
 * Query text is stored in search->query
 */
bool iui_search_view_begin(iui_context *ctx,
                           iui_search_view_state *search,
                           float screen_width,
                           float screen_height,
                           const char *placeholder);

/* Add a suggestion item
 * Returns true if suggestion was clicked
 */
bool iui_search_view_suggestion(iui_context *ctx,
                                iui_search_view_state *search,
                                const char *icon,
                                const char *text);

/* End search view rendering */
void iui_search_view_end(iui_context *ctx, iui_search_view_state *search);

/* Check if search view is open */
bool iui_search_view_is_open(const iui_search_view_state *search);

/* Get current search query (convenience accessor) */
const char *iui_search_view_get_query(const iui_search_view_state *search);

/* MD3 Exposed Dropdown Menu Component
 * Reference: https://m3.material.io/components/menus
 * Text field that reveals a menu of selectable options
 *
 * Features:
 * - TextField-style container with dropdown arrow
 * - Floating label support
 * - Menu appears below field when clicked
 * - Selected option shown in field
 * - Helper text support
 *
 * Dimensions (MD3):
 * - Field height: 56dp (same as TextField)
 * - Menu item height: 48dp
 * - Corner radius: 4dp (top only when menu open)
 *
 * Colors (MD3):
 * - Container: surface_container_highest
 * - Label: on_surface_variant (floating when focused)
 * - Selected text: on_surface
 * - Menu: surface_container
 *
 * Usage:
 * static int selected = 0;
 * const char *options[] = {"Option 1", "Option 2", "Option 3"};
 * iui_dropdown_options opts = {
 *     .options = options,
 *     .option_count = 3,
 *     .selected_index = &selected,
 *     .label = "Choose",
 * };
 * if (iui_dropdown(ctx, &opts))
 *     printf("Selected: %s\n", options[selected]);
 */

/* Exposed dropdown menu
 * Returns true if selection changed this frame
 */
bool iui_dropdown(iui_context *ctx, const iui_dropdown_options *options);

/* MD3 Date Picker Component
 * Modal calendar picker for selecting dates
 * Reference: https://m3.material.io/components/date-pickers
 *
 * Features:
 * - Calendar grid with 7 columns (Sun-Sat) and up to 6 rows
 * - Month/year navigation with arrow buttons
 * - Today indicator and selected date highlight
 * - Cancel/OK confirmation buttons
 *
 * Dimensions (MD3):
 * - Day cell: 48dp touch target
 * - Dialog width: 336dp (7 * 48dp)
 * - Header height: 56dp with month/year display
 *
 * Colors (MD3):
 * - Container: surface_container_high
 * - Selected day: primary circle
 * - Today: primary outline
 * - Weekday labels: on_surface_variant
 *
 * Usage:
 * static iui_date_picker_state picker = {0};
 * if (button_clicked)
 *     iui_date_picker_show(&picker, 2025, 1, 15);  // Jan 15, 2025
 * if (iui_date_picker(ctx, &picker, screen_w, screen_h)) {
 *     printf("Selected: %d-%02d-%02d\n", picker.year, picker.month,
 *            picker.day);
 * }
 */

/* Show the date picker with initial date
 * @year: 4-digit year (e.g., 2025)
 * @month: 1-12
 * @day: 1-31
 */
void iui_date_picker_show(iui_date_picker_state *picker,
                          int year,
                          int month,
                          int day);

/* Close the date picker and reset state */
void iui_date_picker_close(iui_date_picker_state *picker);

/* Render the date picker modal
 * Returns true if user confirmed selection (picker.confirmed is set)
 * Call iui_date_picker_close() to dismiss, or read picker.year/month/day for
 * result
 */
bool iui_date_picker(iui_context *ctx,
                     iui_date_picker_state *picker,
                     float screen_width,
                     float screen_height);

/* Check if date picker is currently open */
bool iui_date_picker_is_open(const iui_date_picker_state *picker);

/* MD3 Time Picker Component
 * Modal clock dial picker for selecting time
 * Reference: https://m3.material.io/components/time-pickers
 *
 * Features:
 * - Clock dial with hour/minute selection
 * - Visual selector hand from center to selected value
 * - 12H (with AM/PM toggle) or 24H format
 * - Two-step selection: hour first, then minute
 * - Cancel/OK confirmation buttons
 *
 * Dimensions (MD3):
 * - Clock dial: 256dp diameter
 * - Center dot: 8dp
 * - Selector: 40dp target area
 * - Header height: 80dp with time display
 *
 * Colors (MD3):
 * - Container: surface_container_high
 * - Dial: surface_container_highest
 * - Selected number: on_primary with primary circle
 * - Selector hand: primary
 * - AM/PM toggle: secondary_container
 *
 * Usage:
 * static iui_time_picker_state picker = {0};
 * if (button_clicked)
 *     iui_time_picker_show(&picker, 14, 30, false);  // 2:30 PM, 12H format
 * if (iui_time_picker(ctx, &picker, screen_w, screen_h)) {
 *     printf("Selected: %02d:%02d %s\n", picker.hour, picker.minute,
 *            picker.use_24h ? "" : (picker.is_pm ? "PM" : "AM"));
 * }
 */

/*
 * Show the time picker with initial time
 * @hour: 0-23 (internally converted for 12H display if needed)
 * @minute: 0-59
 * @use_24h: true for 24H format, false for 12H with AM/PM
 */
void iui_time_picker_show(iui_time_picker_state *picker,
                          int hour,
                          int minute,
                          bool use_24h);

/* Close the time picker and reset state */
void iui_time_picker_close(iui_time_picker_state *picker);

/* Render the time picker modal
 * Returns true if user confirmed selection (picker.confirmed is set)
 * Call iui_time_picker_close() to dismiss, or read picker.hour/minute for
 * result
 */
bool iui_time_picker(iui_context *ctx,
                     iui_time_picker_state *picker,
                     float screen_width,
                     float screen_height);

/* Check if time picker is currently open */
bool iui_time_picker_is_open(const iui_time_picker_state *picker);

/* MD3 Search Bar Component
 * Reference: https://m3.material.io/components/search
 * Persistent search input field with MD3 styling
 *
 * Features:
 * - Pill-shaped container with full-round corners (28dp radius)
 * - Leading search icon (24dp)
 * - Trailing clear icon (appears when text is non-empty)
 * - Surface container high background color
 * - Placeholder text support
 *
 * Dimensions (MD3):
 * - Height: 56dp
 * - Corner radius: 28dp (full round)
 * - Icon size: 24dp
 * - Horizontal padding: 16dp
 *
 * Colors (MD3):
 * - Container: surface_container_high
 * - Icon: on_surface_variant
 * - Text: on_surface
 * - Placeholder: on_surface_variant
 *
 * Usage:
 * static char search_text[256] = "";
 * static size_t search_cursor = 0;
 * if (iui_search_bar(ui, search_text, sizeof(search_text), &search_cursor,
 *                    "Search...")) {
 *     // User pressed Enter or clicked search
 *     handle_search(search_text);
 * }
 *
 * Note: Search bars participate in layout (occupy full layout width).
 */

/* Search Bar result structure */
typedef struct {
    bool value_changed; /* text was modified this frame */
    bool submitted;     /* Enter key was pressed */
    bool cleared;       /* clear button was clicked */
} iui_search_bar_result;

/* Simple Search Bar with default search icon and clear icon
 * @buffer:      user-provided buffer for text storage
 * @size:        size of the buffer in bytes
 * @cursor:      pointer to cursor position (updated by the widget)
 * @placeholder: placeholder text shown when buffer is empty (can be NULL)
 *
 * Returns true if user submitted (pressed Enter)
 */
bool iui_search_bar(iui_context *ctx,
                    char *buffer,
                    size_t size,
                    size_t *cursor,
                    const char *placeholder);

/* Extended Search Bar with custom icons
 * @leading_icon:  icon name for leading icon (NULL = "search")
 * @trailing_icon: icon name for trailing icon (NULL = "clear")
 *
 * Returns result struct with value_changed, submitted, and cleared states
 */
iui_search_bar_result iui_search_bar_ex(iui_context *ctx,
                                        char *buffer,
                                        size_t size,
                                        size_t *cursor,
                                        const char *placeholder,
                                        const char *leading_icon,
                                        const char *trailing_icon);

/* MD3 Chip Component
 * Reference: https://m3.material.io/components/chips
 * Compact interactive elements for filters, selections, and actions
 *
 * Variants:
 * - Assist chip: Contextual actions (no state, icon optional)
 * - Filter chip: Selection from options (toggleable, checkmark when selected)
 * - Input chip: User-entered information (removable with trailing X)
 * - Suggestion chip: Dynamic suggestions (no state, no icon)
 *
 * Dimensions (MD3):
 * - Height: 32dp
 * - Corner radius: 8dp (medium shape)
 * - Icon size: 18dp
 * - Horizontal padding: 16dp (8dp with leading icon)
 *
 * Colors (MD3):
 * - Assist: surface_container_low background, on_surface text
 * - Filter unselected: outline border, no fill
 * - Filter selected: secondary_container fill, on_secondary_container text
 * - Input: surface_container_low + outline, on_surface_variant text
 * - Suggestion: surface_container_low, on_surface_variant text
 *
 * State layers follow MD3: hover 8%, focus 12%, pressed 12%
 *
 * Usage:
 * Assist chip with icon
 * if (iui_chip_assist(ctx, "Add event", "add")) handle_add_event();
 *
 * Filter chip (toggleable)
 * static bool filter_active = false;
 * if (iui_chip_filter(ctx, "Active", &filter_active)) refresh_list();
 *
 * Input chip (removable tag)
 * static bool tag_visible = true;
 * if (iui_chip_input(ctx, "user@example.com", &tag_visible)) { }
 * if (!tag_visible) remove_tag();
 *
 * Suggestion chip
 * if (iui_chip_suggestion(ctx, "Try this")) apply_suggestion();
 *
 * Note: Chips participate in layout and advance the cursor.
 * For horizontal chip rows, use iui_box_begin() with IUI_DIR_ROW.
 */

/* Assist Chip - Contextual action
 * @icon: optional icon name (NULL for text-only)
 *
 * Returns true if clicked this frame
 */
bool iui_chip_assist(iui_context *ctx, const char *label, const char *icon);

/* Filter Chip - Toggleable selection
 * @selected: pointer to toggle state (shows checkmark when true)
 *
 * Returns true if clicked this frame (toggle happens automatically)
 */
bool iui_chip_filter(iui_context *ctx, const char *label, bool *selected);

/* Input Chip - User-entered information with remove action
 * @removed: set to true when user clicks remove (X) icon
 *
 * Returns true if chip body was clicked (not the remove icon)
 */
bool iui_chip_input(iui_context *ctx, const char *label, bool *removed);

/* Suggestion Chip - Dynamic suggestion
 * Returns true if clicked this frame
 */
bool iui_chip_suggestion(iui_context *ctx, const char *label);

/* WCAG Accessibility - Color Contrast Utilities
 * Reference: https://www.w3.org/WAI/WCAG21/Understanding/contrast-minimum.html
 * Contrast thresholds defined in iui-spec.h
 */

/* Calculate relative luminance of a color per WCAG 2.1
 * Color format: 0xAARRGGBB (alpha ignored)
 * Returns luminance value in range [0.0, 1.0]
 * Formula: L = 0.2126*R + 0.7152*G + 0.0722*B (after gamma correction)
 */
float iui_relative_luminance(uint32_t color);

/* Calculate contrast ratio between two colors per WCAG 2.1
 * Returns ratio in range [1.0, 21.0]
 * Formula: (L1 + 0.05) / (L2 + 0.05) where L1 >= L2
 */
float iui_contrast_ratio(uint32_t color1, uint32_t color2);

/* Check if contrast meets WCAG AA for normal text (4.5:1)
 * Returns true if contrast ratio >= 4.5
 */
bool iui_wcag_aa_normal(uint32_t foreground, uint32_t background);

/* Check if contrast meets WCAG AA for large text (3:1)
 * Returns true if contrast ratio >= 3.0
 */
bool iui_wcag_aa_large(uint32_t foreground, uint32_t background);

/* Check if contrast meets WCAG AAA for normal text (7:1)
 * Returns true if contrast ratio >= 7.0
 */
bool iui_wcag_aaa_normal(uint32_t foreground, uint32_t background);

/* Check if contrast meets WCAG AAA for large text (4.5:1)
 * Returns true if contrast ratio >= 4.5
 */
bool iui_wcag_aaa_large(uint32_t foreground, uint32_t background);

/* Validate a theme's color accessibility
 * Checks key text/background combinations against WCAG AA
 * Returns number of failing combinations (0 = all pass)
 */
int iui_theme_validate_contrast(const iui_theme_t *theme);

/* Accessibility - Screen Reader Support
 * Reference: https://m3.material.io/foundations/accessible-design
 * Reference: https://www.w3.org/WAI/ARIA/apg/patterns/
 */

/* Accessibility roles (based on ARIA/MD3 patterns)
 * These map to platform accessibility roles for screen readers
 */
typedef enum iui_a11y_role {
    IUI_A11Y_ROLE_NONE = 0, /* No semantic role (decorative) */
    IUI_A11Y_ROLE_BUTTON,
    IUI_A11Y_ROLE_CHECKBOX,
    IUI_A11Y_ROLE_RADIO,
    IUI_A11Y_ROLE_SWITCH,
    IUI_A11Y_ROLE_SLIDER,
    IUI_A11Y_ROLE_TEXTFIELD,
    IUI_A11Y_ROLE_COMBOBOX,
    IUI_A11Y_ROLE_MENU,
    IUI_A11Y_ROLE_MENUITEM,
    IUI_A11Y_ROLE_TAB,
    IUI_A11Y_ROLE_TABPANEL,
    IUI_A11Y_ROLE_DIALOG,
    IUI_A11Y_ROLE_ALERTDIALOG,
    IUI_A11Y_ROLE_ALERT,
    IUI_A11Y_ROLE_STATUS,
    IUI_A11Y_ROLE_PROGRESSBAR,
    IUI_A11Y_ROLE_LINK,
    IUI_A11Y_ROLE_HEADING,
    IUI_A11Y_ROLE_LIST,
    IUI_A11Y_ROLE_LISTITEM,
    IUI_A11Y_ROLE_IMG,
    IUI_A11Y_ROLE_SEARCH,
    IUI_A11Y_ROLE_SCROLLBAR,
} iui_a11y_role_t;

/* Accessibility state flags (can be combined with bitwise OR)
 * These communicate widget state to screen readers
 */
typedef enum iui_a11y_state {
    IUI_A11Y_STATE_NONE = 0,
    IUI_A11Y_STATE_CHECKED = 1 << 0,   /* Checkbox/switch is on */
    IUI_A11Y_STATE_SELECTED = 1 << 1,  /* Tab/radio/listitem is selected */
    IUI_A11Y_STATE_DISABLED = 1 << 2,  /* Widget is disabled */
    IUI_A11Y_STATE_EXPANDED = 1 << 3,  /* Menu/accordion is expanded */
    IUI_A11Y_STATE_COLLAPSED = 1 << 4, /* Menu/accordion is collapsed */
    IUI_A11Y_STATE_PRESSED = 1 << 5,   /* Toggle button is pressed */
    IUI_A11Y_STATE_FOCUSED = 1 << 6,   /* Has keyboard focus */
    IUI_A11Y_STATE_BUSY = 1 << 7,      /* In progress/loading */
    IUI_A11Y_STATE_INVALID = 1 << 8,   /* Validation error */
    IUI_A11Y_STATE_REQUIRED = 1 << 9,  /* Required field */
    IUI_A11Y_STATE_READONLY = 1 << 10, /* Read-only (not editable) */
    IUI_A11Y_STATE_MULTISELECTABLE = 1 << 11, /* Multiple selection allowed */
    IUI_A11Y_STATE_HASPOPUP = 1 << 12,        /* Has popup menu/dialog */
} iui_a11y_state_t;

/* Live region priority levels for announcements */
/* Reference: https://www.w3.org/WAI/ARIA/apg/practices/names-and-descriptions/
 */
typedef enum iui_a11y_live {
    IUI_A11Y_LIVE_OFF = 0,   /* No automatic announcements */
    IUI_A11Y_LIVE_POLITE,    /* Announce when screen reader is idle */
    IUI_A11Y_LIVE_ASSERTIVE, /* Interrupt and announce immediately */
} iui_a11y_live_t;

/* Heading level for semantic structure (h1-h6) */
typedef enum iui_a11y_heading_level {
    IUI_A11Y_HEADING_NONE = 0,
    IUI_A11Y_HEADING_1 = 1, /* Main title */
    IUI_A11Y_HEADING_2 = 2, /* Major section */
    IUI_A11Y_HEADING_3 = 3, /* Subsection */
    IUI_A11Y_HEADING_4 = 4,
    IUI_A11Y_HEADING_5 = 5,
    IUI_A11Y_HEADING_6 = 6,
} iui_a11y_heading_level_t;

/* Accessibility hint structure
 * Provides semantic information for screen readers
 * Pass to widget functions or use iui_a11y_push/pop for regions
 */
typedef struct {
    const char *label;       /* Human-readable name (required for a11y) */
    const char *description; /* Additional help text (optional) */
    const char *hint;        /* Usage hint (e.g., "Double-tap to activate") */
    iui_a11y_role_t role;    /* Semantic role */
    uint32_t state;          /* State flags (iui_a11y_state bitfield) */
    float value_now;         /* Current value (for sliders/progress) */
    float value_min, value_max; /* Min/Max value (for sliders/progress) */
    const char *value_text;     /* Text representation of value (optional) */
    int heading_level;          /* Heading level 1-6 (0 = not a heading) */
    int set_size;               /* Total items in set (for list/radio/tabs) */
    int position_in_set;        /* 1-based position in set */
} iui_a11y_hint;

/* Announcement callback type
 * Called when content should be announced to screen reader
 */
typedef void (*iui_a11y_announce_func_t)(const char *text,
                                         iui_a11y_live_t priority,
                                         void *user);

/* Focus change callback type
 * Called when keyboard focus moves to a new widget
 */
typedef void (*iui_a11y_focus_func_t)(const iui_a11y_hint *widget,
                                      iui_rect_t bounds,
                                      void *user);

/* State change callback type
 * Called when widget state changes (checked, expanded, etc.)
 */
typedef void (*iui_a11y_state_func_t)(const iui_a11y_hint *widget,
                                      uint32_t old_state,
                                      uint32_t new_state,
                                      void *user);

/* Value change callback type
 * Called when slider/progress value changes
 */
typedef void (*iui_a11y_value_func_t)(const iui_a11y_hint *widget,
                                      float old_value,
                                      float new_value,
                                      void *user);

/* Accessibility callbacks structure
 * Platform-specific screen reader integration
 */
typedef struct {
    iui_a11y_announce_func_t announce; /* Text announcement callback */
    iui_a11y_focus_func_t on_focus;    /* Focus change callback */
    iui_a11y_state_func_t on_state;    /* State change callback */
    iui_a11y_value_func_t on_value;    /* Value change callback */
    void *user;                        /* User data for all callbacks */
} iui_a11y_callbacks;

/* Set accessibility callbacks for screen reader integration
 * Pass NULL to disable accessibility callbacks
 */
void iui_set_a11y_callbacks(iui_context *ctx, const iui_a11y_callbacks *cbs);

/* Get current accessibility callbacks (returns NULL if not set) */
const iui_a11y_callbacks *iui_get_a11y_callbacks(const iui_context *ctx);

/* Check if accessibility is enabled (callbacks are set) */
bool iui_a11y_enabled(const iui_context *ctx);

/* Announce text to screen reader
 * Priority: a11y_live_polite (wait for idle) or a11y_live_assertive (interrupt)
 */
void iui_announce(iui_context *ctx, const char *text, iui_a11y_live_t priority);

/* Announce formatted text to screen reader */
void iui_announcef(iui_context *ctx,
                   iui_a11y_live_t priority,
                   const char *fmt,
                   ...);

/* Create a default accessibility hint with label and role
 * Returns hint struct that can be further customized
 */
iui_a11y_hint iui_a11y_make_hint(const char *label, iui_a11y_role_t role);

/* Create hint for slider/progress with value range */
iui_a11y_hint iui_a11y_make_slider_hint(const char *label,
                                        float value,
                                        float min,
                                        float max);

/* Create hint for item in a set (radio, tab, listitem) */
iui_a11y_hint iui_a11y_make_set_hint(const char *label,
                                     iui_a11y_role_t role,
                                     int position,
                                     int total);

/* Push accessibility context for a region
 * All widgets inside will inherit this context
 * Must be paired with iui_a11y_pop()
 */
void iui_a11y_push(iui_context *ctx, const iui_a11y_hint *hint);

/* Pop accessibility context */
void iui_a11y_pop(iui_context *ctx);

/* Notify accessibility system of state change
 * Triggers on_state callback if registered
 */
void iui_a11y_notify_state(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           uint32_t old_state,
                           uint32_t new_state);

/* Notify accessibility system of value change
 * Triggers on_value callback if registered
 */
void iui_a11y_notify_value(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           float old_value,
                           float new_value);

/* Notify accessibility system of focus change
 * Triggers on_focus callback if registered
 */
void iui_a11y_notify_focus(iui_context *ctx,
                           const iui_a11y_hint *widget,
                           iui_rect_t bounds);

/* Get role name as string for screen reader announcements
 * Used by iui_a11y_describe() to generate accessible text.
 */
const char *iui_a11y_role_name(iui_a11y_role_t role);

/* Get state description as string for screen reader announcements
 * Used by iui_a11y_describe() to generate accessible text.
 * Returns static buffer, not thread-safe.
 */
const char *iui_a11y_state_description(uint32_t state);

/* Build accessible description from hint
 * Combines label, role, state, and value into natural language
 * @buf:      output buffer
 * @buf_size: buffer size
 *
 * Returns number of characters written (excluding null terminator)
 */
int iui_a11y_describe(const iui_a11y_hint *hint, char *buf, size_t buf_size);

/* Performance Optimization API
 *
 * Three core performance systems (always available, disabled by default):
 * 1. Draw Call Batching - buffers draw commands to reduce state changes
 * 2. Dirty Rectangle Tracking - skips redrawing unchanged regions
 * 3. Text Width Caching - caches text measurement results
 *
 * Usage:
 * iui_batch_enable(ctx, true);      // Enable draw batching
 * iui_dirty_enable(ctx, true);      // Enable dirty rect tracking
 * iui_text_cache_enable(ctx, true); // Enable text cache
 */

/* Draw Call Batching
 * Enable/disable draw command batching. When enabled, draw calls are buffered
 * and flushed at frame end or when batch is full.
 */
void iui_batch_enable(iui_context *ctx, bool enable);

/* Query batching statistics
 * Returns number of batched commands in current frame
 */
int iui_batch_count(const iui_context *ctx);

/* Dirty Rectangle Tracking
 * Enable/disable dirty region tracking. When enabled, only regions marked
 * dirty are redrawn.
 */
void iui_dirty_enable(iui_context *ctx, bool enable);

/* Mark a region as needing redraw
 * @region: rectangular area that changed
 */
void iui_dirty_mark(iui_context *ctx, iui_rect_t region);

/* Check if a region needs redraw
 * Returns true if region overlaps any dirty region
 */
bool iui_dirty_check(const iui_context *ctx, iui_rect_t region);

/* Force full screen redraw next frame */
void iui_dirty_invalidate_all(iui_context *ctx);

/* Query dirty region count
 * Returns number of dirty regions in current frame
 */
int iui_dirty_count(const iui_context *ctx);

/* Text Width Caching
 * Enable/disable text measurement caching. When enabled, text width
 * calculations are cached to avoid redundant measurements.
 */
void iui_text_cache_enable(iui_context *ctx, bool enable);

/* Clear the text cache (call after font size changes) */
void iui_text_cache_clear(iui_context *ctx);

/* Query text cache statistics */
void iui_text_cache_stats(const iui_context *ctx, int *hits, int *misses);

#ifdef __cplusplus
}
#endif

#endif /* IUI_H_ */
