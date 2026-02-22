/* MD3 Component Specifications
 * Reference: Material Design 3 (MD3)
 *            https://m3.material.io/
 *
 * This header contains all MD3 dimension constants (in dp).
 * Include before iui.h to override defaults with custom values.
 *
 * Usage:
 * #define IUI_FAB_SIZE 64.f   // Override before including
 * #include "iui.h"
 */

#ifndef IUI_SPEC_H_
#define IUI_SPEC_H_

/* MD3 Motion System - Duration Tokens
 * Reference: https://m3.material.io/styles/motion
 * All durations in seconds (multiply by 1000 for milliseconds)
 */

/* Short durations (50-200ms) - Micro-interactions, state changes */
#ifndef IUI_DURATION_SHORT_1
#define IUI_DURATION_SHORT_1 0.050f /* 50ms - Ripple start */
#endif
#ifndef IUI_DURATION_SHORT_2
#define IUI_DURATION_SHORT_2 0.100f /* 100ms - Toggle states */
#endif
#ifndef IUI_DURATION_SHORT_3
#define IUI_DURATION_SHORT_3 0.150f /* 150ms - Hover effects */
#endif
#ifndef IUI_DURATION_SHORT_4
#define IUI_DURATION_SHORT_4 0.200f /* 200ms - Button press */
#endif

/* Medium durations (250-400ms) - Component enter/exit */
#ifndef IUI_DURATION_MEDIUM_1
#define IUI_DURATION_MEDIUM_1 0.250f /* 250ms - Component enter/exit */
#endif
#ifndef IUI_DURATION_MEDIUM_2
#define IUI_DURATION_MEDIUM_2 0.300f /* 300ms - Menu open */
#endif
#ifndef IUI_DURATION_MEDIUM_3
#define IUI_DURATION_MEDIUM_3 0.350f /* 350ms - Dialog reveal */
#endif
#ifndef IUI_DURATION_MEDIUM_4
#define IUI_DURATION_MEDIUM_4 0.400f /* 400ms - Complex reveals */
#endif

/* Long durations (450ms+) - Full-screen transitions */
#ifndef IUI_DURATION_LONG_1
#define IUI_DURATION_LONG_1 0.450f /* 450ms - Complex animations */
#endif
#ifndef IUI_DURATION_LONG_2
#define IUI_DURATION_LONG_2 0.500f /* 500ms - Full-screen */
#endif

/* MD3 Component Dimensions (dp) */

/* Slider - https://m3.material.io/components/sliders/specs */
#ifndef IUI_SLIDER_TRACK_HEIGHT
#define IUI_SLIDER_TRACK_HEIGHT 4.f
#endif
#ifndef IUI_SLIDER_THUMB_IDLE
#define IUI_SLIDER_THUMB_IDLE 20.f
#endif
#ifndef IUI_SLIDER_THUMB_PRESSED
#define IUI_SLIDER_THUMB_PRESSED 28.f
#endif
#ifndef IUI_SLIDER_TOUCH_TARGET
#define IUI_SLIDER_TOUCH_TARGET 48.f
#endif
#ifndef IUI_SLIDER_VALUE_INDICATOR
#define IUI_SLIDER_VALUE_INDICATOR 28.f /* minimum width */
#endif

/* Segmented Button - https://m3.material.io/components/segmented-buttons/specs
 */
#ifndef IUI_SEGMENTED_HEIGHT
#define IUI_SEGMENTED_HEIGHT 40.f
#endif
#ifndef IUI_SEGMENTED_MIN_SEGMENTS
#define IUI_SEGMENTED_MIN_SEGMENTS 2
#endif
#ifndef IUI_SEGMENTED_MAX_SEGMENTS
#define IUI_SEGMENTED_MAX_SEGMENTS 5
#endif
#ifndef IUI_SEGMENTED_ICON_SIZE
#define IUI_SEGMENTED_ICON_SIZE 18.f
#endif

/* FAB (Floating Action Button) -
 * https://m3.material.io/components/floating-action-button/specs
 */
#ifndef IUI_FAB_SIZE
#define IUI_FAB_SIZE 56.f
#endif
#ifndef IUI_FAB_LARGE_SIZE
#define IUI_FAB_LARGE_SIZE 96.f
#endif
#ifndef IUI_FAB_ICON_SIZE
#define IUI_FAB_ICON_SIZE 24.f
#endif
#ifndef IUI_FAB_LARGE_ICON_SIZE
#define IUI_FAB_LARGE_ICON_SIZE 36.f
#endif
#ifndef IUI_FAB_CORNER_RADIUS
#define IUI_FAB_CORNER_RADIUS 16.f
#endif
#ifndef IUI_FAB_LARGE_CORNER_RADIUS
#define IUI_FAB_LARGE_CORNER_RADIUS 28.f
#endif
#ifndef IUI_FAB_EXTENDED_HEIGHT
#define IUI_FAB_EXTENDED_HEIGHT 56.f
#endif
#ifndef IUI_FAB_EXTENDED_PADDING
#define IUI_FAB_EXTENDED_PADDING 16.f
#endif
#ifndef IUI_FAB_EXTENDED_GAP
#define IUI_FAB_EXTENDED_GAP 8.f
#endif

/* Icon Button - https://m3.material.io/components/icon-buttons/specs */
#ifndef IUI_ICON_BUTTON_SIZE
#define IUI_ICON_BUTTON_SIZE 40.f
#endif
#ifndef IUI_ICON_BUTTON_TOUCH_TARGET
#define IUI_ICON_BUTTON_TOUCH_TARGET 48.f
#endif
#ifndef IUI_ICON_BUTTON_ICON_SIZE
#define IUI_ICON_BUTTON_ICON_SIZE 24.f
#endif
#ifndef IUI_ICON_BUTTON_CORNER_RADIUS
#define IUI_ICON_BUTTON_CORNER_RADIUS 20.f
#endif

/* Tabs - https://m3.material.io/components/tabs/specs */
#ifndef IUI_TAB_HEIGHT
#define IUI_TAB_HEIGHT 48.f
#endif
#ifndef IUI_TAB_MIN_WIDTH
#define IUI_TAB_MIN_WIDTH 90.f
#endif
#ifndef IUI_TAB_INDICATOR_HEIGHT
#define IUI_TAB_INDICATOR_HEIGHT 3.f
#endif
#ifndef IUI_TAB_ICON_SIZE
#define IUI_TAB_ICON_SIZE 24.f
#endif
#ifndef IUI_TAB_ICON_LABEL_GAP
#define IUI_TAB_ICON_LABEL_GAP 4.f
#endif
#ifndef IUI_TAB_HORIZONTAL_PADDING
#define IUI_TAB_HORIZONTAL_PADDING 16.f
#endif

/* Date Picker - material-components-android/datepicker/res/values/dimens.xml */
#ifndef IUI_DATE_PICKER_DAY_WIDTH
#define IUI_DATE_PICKER_DAY_WIDTH 36.f
#endif
#ifndef IUI_DATE_PICKER_DAY_HEIGHT
#define IUI_DATE_PICKER_DAY_HEIGHT 32.f
#endif
#ifndef IUI_DATE_PICKER_TOUCH_TARGET
#define IUI_DATE_PICKER_TOUCH_TARGET 48.f /* MD3 minimum touch target */
#endif
#ifndef IUI_DATE_PICKER_DAY_CORNER
#define IUI_DATE_PICKER_DAY_CORNER 15.f
#endif
#ifndef IUI_DATE_PICKER_HEADER_HEIGHT
#define IUI_DATE_PICKER_HEADER_HEIGHT 64.f
#endif
#ifndef IUI_DATE_PICKER_WEEKDAY_HEIGHT
#define IUI_DATE_PICKER_WEEKDAY_HEIGHT 24.f
#endif
#ifndef IUI_DATE_PICKER_PADDING
#define IUI_DATE_PICKER_PADDING 12.f
#endif
#ifndef IUI_DATE_PICKER_NAV_HEIGHT
#define IUI_DATE_PICKER_NAV_HEIGHT 48.f
#endif
#ifndef IUI_DATE_PICKER_NAV_BUTTON_SIZE
#define IUI_DATE_PICKER_NAV_BUTTON_SIZE 40.f
#endif

/* Time Picker - material-components-android/timepicker/res/values/dimens.xml */
#ifndef IUI_TIME_PICKER_DIAL_SIZE
#define IUI_TIME_PICKER_DIAL_SIZE 256.f
#endif
#ifndef IUI_TIME_PICKER_CENTER_DOT
#define IUI_TIME_PICKER_CENTER_DOT 8.f
#endif
#ifndef IUI_TIME_PICKER_SELECTOR_SIZE
#define IUI_TIME_PICKER_SELECTOR_SIZE 40.f
#endif
#ifndef IUI_TIME_PICKER_HEADER_HEIGHT
#define IUI_TIME_PICKER_HEADER_HEIGHT 80.f
#endif
#ifndef IUI_TIME_PICKER_DISPLAY_WIDTH
#define IUI_TIME_PICKER_DISPLAY_WIDTH 96.f
#endif
#ifndef IUI_TIME_PICKER_PADDING
#define IUI_TIME_PICKER_PADDING 24.f
#endif
#ifndef IUI_TIME_PICKER_AMPM_WIDTH
#define IUI_TIME_PICKER_AMPM_WIDTH 52.f
#endif
#ifndef IUI_TIME_PICKER_AMPM_HEIGHT
#define IUI_TIME_PICKER_AMPM_HEIGHT 96.f
#endif

/* Search Bar - https://m3.material.io/components/search */
#ifndef IUI_SEARCH_BAR_HEIGHT
#define IUI_SEARCH_BAR_HEIGHT 56.f
#endif
#ifndef IUI_SEARCH_BAR_CORNER_RADIUS
#define IUI_SEARCH_BAR_CORNER_RADIUS 28.f
#endif
#ifndef IUI_SEARCH_BAR_ICON_SIZE
#define IUI_SEARCH_BAR_ICON_SIZE 24.f
#endif
#ifndef IUI_SEARCH_BAR_PADDING_H
#define IUI_SEARCH_BAR_PADDING_H 16.f
#endif
#ifndef IUI_SEARCH_BAR_ICON_GAP
#define IUI_SEARCH_BAR_ICON_GAP 16.f
#endif

/* Chip - https://m3.material.io/components/chips */
#ifndef IUI_CHIP_HEIGHT
#define IUI_CHIP_HEIGHT 32.f
#endif
#ifndef IUI_CHIP_CORNER_RADIUS
#define IUI_CHIP_CORNER_RADIUS 8.f
#endif
#ifndef IUI_CHIP_ICON_SIZE
#define IUI_CHIP_ICON_SIZE 18.f
#endif
#ifndef IUI_CHIP_PADDING_H
#define IUI_CHIP_PADDING_H 16.f
#endif
#ifndef IUI_CHIP_PADDING_H_ICON
#define IUI_CHIP_PADDING_H_ICON 8.f
#endif
#ifndef IUI_CHIP_ICON_LABEL_GAP
#define IUI_CHIP_ICON_LABEL_GAP 8.f
#endif
#ifndef IUI_CHIP_TOUCH_TARGET
#define IUI_CHIP_TOUCH_TARGET 48.f
#endif

/* Switch - https://m3.material.io/components/switch/specs */
#ifndef IUI_SWITCH_TRACK_WIDTH
#define IUI_SWITCH_TRACK_WIDTH 52.f
#endif
#ifndef IUI_SWITCH_TRACK_HEIGHT
#define IUI_SWITCH_TRACK_HEIGHT 32.f
#endif
#ifndef IUI_SWITCH_THUMB_SIZE
#define IUI_SWITCH_THUMB_SIZE 24.f
#endif
#ifndef IUI_SWITCH_TOUCH_TARGET
#define IUI_SWITCH_TOUCH_TARGET 48.f
#endif
#ifndef IUI_SWITCH_CORNER_RADIUS
#define IUI_SWITCH_CORNER_RADIUS 16.f
#endif

/* TextField - https://m3.material.io/components/text-fields/specs */
#ifndef IUI_TEXTFIELD_HEIGHT
#define IUI_TEXTFIELD_HEIGHT 56.f
#endif
#ifndef IUI_TEXTFIELD_CORNER_RADIUS
#define IUI_TEXTFIELD_CORNER_RADIUS 4.f
#endif
#ifndef IUI_TEXTFIELD_CURSOR_WIDTH
#define IUI_TEXTFIELD_CURSOR_WIDTH 2.f
#endif

/* Button - https://m3.material.io/components/buttons/specs */
#ifndef IUI_BUTTON_HEIGHT
#define IUI_BUTTON_HEIGHT 40.f /* Same as IUI_SEGMENTED_HEIGHT */
#endif
#ifndef IUI_BUTTON_MIN_TOUCH_TARGET
#define IUI_BUTTON_MIN_TOUCH_TARGET 48.f /* MD3 minimum touch target */
#endif

/* Menu - https://m3.material.io/components/menus/specs */
#ifndef IUI_MENU_MIN_WIDTH
#define IUI_MENU_MIN_WIDTH 200.f
#endif
#ifndef IUI_MENU_MAX_WIDTH
#define IUI_MENU_MAX_WIDTH 400.f
#endif
#ifndef IUI_MENU_ITEM_HEIGHT
#define IUI_MENU_ITEM_HEIGHT 40.f
#endif
#ifndef IUI_MENU_DIVIDER_HEIGHT
#define IUI_MENU_DIVIDER_HEIGHT 9.f
#endif
#ifndef IUI_MENU_ICON_SIZE
#define IUI_MENU_ICON_SIZE 24.f
#endif
#ifndef IUI_MENU_GAP_HEIGHT
#define IUI_MENU_GAP_HEIGHT 8.f
#endif
#ifndef IUI_MENU_PADDING_H
#define IUI_MENU_PADDING_H 12.f
#endif
#ifndef IUI_MENU_PADDING_V
#define IUI_MENU_PADDING_V 8.f
#endif

/* Dialog - https://m3.material.io/components/dialogs/specs */
#ifndef IUI_DIALOG_PADDING
#define IUI_DIALOG_PADDING 24.f
#endif
#ifndef IUI_DIALOG_CORNER_RADIUS
#define IUI_DIALOG_CORNER_RADIUS 28.f
#endif
#ifndef IUI_DIALOG_MIN_WIDTH
#define IUI_DIALOG_MIN_WIDTH 280.f
#endif
#ifndef IUI_DIALOG_MAX_WIDTH
#define IUI_DIALOG_MAX_WIDTH 560.f /* MD3: up to 560dp on large screens */
#endif
#ifndef IUI_DIALOG_BUTTON_HEIGHT
#define IUI_DIALOG_BUTTON_HEIGHT 36.f
#endif
#ifndef IUI_DIALOG_BUTTON_SPACING
#define IUI_DIALOG_BUTTON_SPACING 8.f
#endif
#ifndef IUI_DIALOG_CONTENT_SPACING
#define IUI_DIALOG_CONTENT_SPACING 16.f
#endif

/* Full-Screen Dialog - https://m3.material.io/components/dialogs/specs */
#ifndef IUI_FULLSCREEN_DIALOG_HEADER_HEIGHT
#define IUI_FULLSCREEN_DIALOG_HEADER_HEIGHT 56.f
#endif
#ifndef IUI_FULLSCREEN_DIALOG_PADDING
#define IUI_FULLSCREEN_DIALOG_PADDING 16.f
#endif
#ifndef IUI_FULLSCREEN_DIALOG_MAX_ACTIONS
#define IUI_FULLSCREEN_DIALOG_MAX_ACTIONS 2
#endif

/* Search View - https://m3.material.io/components/search */
#ifndef IUI_SEARCH_VIEW_HEADER_HEIGHT
#define IUI_SEARCH_VIEW_HEADER_HEIGHT 72.f
#endif
#ifndef IUI_SEARCH_VIEW_SUGGESTION_HEIGHT
#define IUI_SEARCH_VIEW_SUGGESTION_HEIGHT 56.f
#endif
#ifndef IUI_SEARCH_VIEW_PADDING
#define IUI_SEARCH_VIEW_PADDING 16.f
#endif
#ifndef IUI_SEARCH_VIEW_ICON_SIZE
#define IUI_SEARCH_VIEW_ICON_SIZE 24.f
#endif

/* Navigation Rail - https://m3.material.io/components/navigation-rail */
#ifndef IUI_NAV_RAIL_WIDTH
#define IUI_NAV_RAIL_WIDTH 80.f /* collapsed width */
#endif
#ifndef IUI_NAV_RAIL_EXPANDED_WIDTH
#define IUI_NAV_RAIL_EXPANDED_WIDTH 256.f /* replaces drawer */
#endif
#ifndef IUI_NAV_RAIL_ITEM_HEIGHT
#define IUI_NAV_RAIL_ITEM_HEIGHT 56.f
#endif
#ifndef IUI_NAV_RAIL_ICON_SIZE
#define IUI_NAV_RAIL_ICON_SIZE 24.f
#endif
#ifndef IUI_NAV_RAIL_INDICATOR_WIDTH
#define IUI_NAV_RAIL_INDICATOR_WIDTH 56.f
#endif
#ifndef IUI_NAV_RAIL_INDICATOR_HEIGHT
#define IUI_NAV_RAIL_INDICATOR_HEIGHT 32.f
#endif
#ifndef IUI_NAV_RAIL_CORNER_RADIUS
#define IUI_NAV_RAIL_CORNER_RADIUS 16.f
#endif

/* Navigation Bar - https://m3.material.io/components/navigation-bar */
#ifndef IUI_NAV_BAR_HEIGHT
#define IUI_NAV_BAR_HEIGHT 80.f
#endif
#ifndef IUI_NAV_BAR_ICON_SIZE
#define IUI_NAV_BAR_ICON_SIZE 24.f
#endif
#ifndef IUI_NAV_BAR_INDICATOR_WIDTH
#define IUI_NAV_BAR_INDICATOR_WIDTH 64.f
#endif
#ifndef IUI_NAV_BAR_INDICATOR_HEIGHT
#define IUI_NAV_BAR_INDICATOR_HEIGHT 32.f
#endif
#ifndef IUI_NAV_BAR_LABEL_GAP
#define IUI_NAV_BAR_LABEL_GAP 4.f
#endif

/* Navigation Drawer - https://m3.material.io/components/navigation-drawer */
#ifndef IUI_NAV_DRAWER_WIDTH
#define IUI_NAV_DRAWER_WIDTH 280.f /* standard width on mobile */
#endif
#ifndef IUI_NAV_DRAWER_WIDTH_TABLET
#define IUI_NAV_DRAWER_WIDTH_TABLET 320.f /* width on tablet */
#endif
#ifndef IUI_NAV_DRAWER_ITEM_HEIGHT
#define IUI_NAV_DRAWER_ITEM_HEIGHT 56.f
#endif
#ifndef IUI_NAV_DRAWER_PADDING_H
#define IUI_NAV_DRAWER_PADDING_H 12.f
#endif
#ifndef IUI_NAV_DRAWER_ICON_GAP
#define IUI_NAV_DRAWER_ICON_GAP 12.f
#endif

/* List - https://m3.material.io/components/lists/specs */
#ifndef IUI_LIST_ONE_LINE_HEIGHT
#define IUI_LIST_ONE_LINE_HEIGHT 56.f
#endif
#ifndef IUI_LIST_TWO_LINE_HEIGHT
#define IUI_LIST_TWO_LINE_HEIGHT 72.f
#endif
#ifndef IUI_LIST_THREE_LINE_HEIGHT
#define IUI_LIST_THREE_LINE_HEIGHT 88.f
#endif
#ifndef IUI_LIST_PADDING_H
#define IUI_LIST_PADDING_H 16.f
#endif
#ifndef IUI_LIST_LEADING_PADDING
#define IUI_LIST_LEADING_PADDING 16.f
#endif
#ifndef IUI_LIST_TRAILING_PADDING
#define IUI_LIST_TRAILING_PADDING 24.f
#endif
#ifndef IUI_LIST_TEXT_INDENT
#define IUI_LIST_TEXT_INDENT \
    56.f /* Left padding for text when leading icon present */
#endif
#ifndef IUI_LIST_ICON_SIZE
#define IUI_LIST_ICON_SIZE 24.f
#endif
#ifndef IUI_LIST_AVATAR_SIZE
#define IUI_LIST_AVATAR_SIZE 40.f
#endif
#ifndef IUI_LIST_DIVIDER_INSET
#define IUI_LIST_DIVIDER_INSET 16.f
#endif

/* Dropdown Menu - https://m3.material.io/components/menus/specs */
#ifndef IUI_DROPDOWN_HEIGHT
#define IUI_DROPDOWN_HEIGHT 56.f
#endif
#ifndef IUI_DROPDOWN_CORNER_RADIUS
#define IUI_DROPDOWN_CORNER_RADIUS 4.f
#endif
#ifndef IUI_DROPDOWN_MENU_MAX_HEIGHT
#define IUI_DROPDOWN_MENU_MAX_HEIGHT 280.f
#endif
#ifndef IUI_DROPDOWN_ITEM_HEIGHT
#define IUI_DROPDOWN_ITEM_HEIGHT 48.f
#endif

/* Top App Bar - https://m3.material.io/components/top-app-bar/specs */
#ifndef IUI_APPBAR_SMALL_HEIGHT
#define IUI_APPBAR_SMALL_HEIGHT 64.f
#endif
#ifndef IUI_APPBAR_MEDIUM_HEIGHT
#define IUI_APPBAR_MEDIUM_HEIGHT 112.f
#endif
#ifndef IUI_APPBAR_LARGE_HEIGHT
#define IUI_APPBAR_LARGE_HEIGHT 152.f
#endif
#ifndef IUI_APPBAR_COLLAPSED_HEIGHT
#define IUI_APPBAR_COLLAPSED_HEIGHT 64.f
#endif
#ifndef IUI_APPBAR_PADDING_H
#define IUI_APPBAR_PADDING_H 16.f
#endif
#ifndef IUI_APPBAR_ICON_SIZE
#define IUI_APPBAR_ICON_SIZE 24.f
#endif
#ifndef IUI_APPBAR_ICON_BUTTON_SIZE
#define IUI_APPBAR_ICON_BUTTON_SIZE 48.f
#endif
#ifndef IUI_APPBAR_TITLE_MARGIN
#define IUI_APPBAR_TITLE_MARGIN 16.f
#endif
#ifndef IUI_APPBAR_ACTION_GAP
#define IUI_APPBAR_ACTION_GAP 8.f
#endif
#ifndef IUI_APPBAR_MAX_ACTIONS
#define IUI_APPBAR_MAX_ACTIONS 3
#endif
#ifndef IUI_APPBAR_TITLE_MARGIN_BOTTOM
#define IUI_APPBAR_TITLE_MARGIN_BOTTOM 16.f
#endif
#ifndef IUI_APPBAR_SHADOW_THRESHOLD
#define IUI_APPBAR_SHADOW_THRESHOLD 0.3f /* scroll progress before shadow */
#endif

/* Scrim - modal overlay opacity (50%) */
#ifndef IUI_SCRIM_ALPHA
#define IUI_SCRIM_ALPHA 0x80 /* 128/255 = 50% */
#endif

/* MD3 Shape Tokens - Named corner radii
 * Reference: https://m3.material.io/styles/shape
 */
#ifndef IUI_SHAPE_FULL
#define IUI_SHAPE_FULL 20.f /* fully rounded (half of 40dp button height) */
#endif
#ifndef IUI_SHAPE_EXTRA_LARGE
#define IUI_SHAPE_EXTRA_LARGE 28.f /* containers (pickers, dialogs) */
#endif

/* MD3 State Layer Opacity
 * Reference: https://m3.material.io/foundations/interaction/states
 */
#ifndef IUI_STATE_HOVER_ALPHA
#define IUI_STATE_HOVER_ALPHA 0x14 /* 8% */
#endif
#ifndef IUI_STATE_FOCUS_ALPHA
#define IUI_STATE_FOCUS_ALPHA 0x1F /* 12% */
#endif
#ifndef IUI_STATE_PRESS_ALPHA
#define IUI_STATE_PRESS_ALPHA 0x1F /* 12% */
#endif
#ifndef IUI_STATE_DRAG_ALPHA
#define IUI_STATE_DRAG_ALPHA 0x29 /* 16% */
#endif
#ifndef IUI_STATE_DISABLE_ALPHA
#define IUI_STATE_DISABLE_ALPHA 0x61 /* 38% */
#endif
#ifndef IUI_PLACEHOLDER_ALPHA
#define IUI_PLACEHOLDER_ALPHA 0x99 /* 60% */
#endif

/* MD3 Elevation/Shadow System
 * Reference: https://m3.material.io/styles/elevation
 */
#ifndef IUI_SHADOW_KEY_LAYERS
#define IUI_SHADOW_KEY_LAYERS 3
#endif
#ifndef IUI_SHADOW_AMBIENT_LAYERS
#define IUI_SHADOW_AMBIENT_LAYERS 2
#endif
#ifndef IUI_SHADOW_KEY_ALPHA
#define IUI_SHADOW_KEY_ALPHA 0.15f
#endif
#ifndef IUI_SHADOW_AMBIENT_ALPHA
#define IUI_SHADOW_AMBIENT_ALPHA 0.08f
#endif

/* MD3 Focus & Accessibility
 * Reference: https://m3.material.io/foundations/interaction/states
 */
#ifndef IUI_FOCUS_RING_WIDTH
#define IUI_FOCUS_RING_WIDTH 3.0f
#endif
#ifndef IUI_FOCUS_RING_OFFSET
#define IUI_FOCUS_RING_OFFSET 2.0f
#endif

/* Selection highlight opacity (MD3: primary at 40%) */
#ifndef IUI_SELECTION_ALPHA
#define IUI_SELECTION_ALPHA 0x66
#endif

/* Double/Triple-click timing thresholds (seconds) */
#ifndef IUI_DOUBLE_CLICK_TIME
#define IUI_DOUBLE_CLICK_TIME 0.3f
#endif
#ifndef IUI_TRIPLE_CLICK_TIME
#define IUI_TRIPLE_CLICK_TIME 0.3f
#endif

/* Bottom Sheet - https://m3.material.io/components/bottom-sheets/specs */
#ifndef IUI_BOTTOM_SHEET_DRAG_HANDLE_WIDTH
#define IUI_BOTTOM_SHEET_DRAG_HANDLE_WIDTH 32.f
#endif
#ifndef IUI_BOTTOM_SHEET_DRAG_HANDLE_HEIGHT
#define IUI_BOTTOM_SHEET_DRAG_HANDLE_HEIGHT 4.f
#endif
#ifndef IUI_BOTTOM_SHEET_DRAG_HANDLE_MARGIN
#define IUI_BOTTOM_SHEET_DRAG_HANDLE_MARGIN 22.f
#endif
#ifndef IUI_BOTTOM_SHEET_CORNER_RADIUS
#define IUI_BOTTOM_SHEET_CORNER_RADIUS 28.f
#endif
#ifndef IUI_BOTTOM_SHEET_MIN_HEIGHT
#define IUI_BOTTOM_SHEET_MIN_HEIGHT 64.f
#endif

/* Bottom App Bar - https://m3.material.io/components/bottom-app-bar/specs */
#ifndef IUI_BOTTOM_APP_BAR_HEIGHT
#define IUI_BOTTOM_APP_BAR_HEIGHT 80.f
#endif
#ifndef IUI_BOTTOM_APP_BAR_ICON_SIZE
#define IUI_BOTTOM_APP_BAR_ICON_SIZE 24.f
#endif
#ifndef IUI_BOTTOM_APP_BAR_ICON_CONTAINER_SIZE
#define IUI_BOTTOM_APP_BAR_ICON_CONTAINER_SIZE 48.f
#endif
#ifndef IUI_BOTTOM_APP_BAR_FAB_OFFSET
#define IUI_BOTTOM_APP_BAR_FAB_OFFSET 16.f
#endif
#ifndef IUI_BOTTOM_APP_BAR_ICON_PADDING
#define IUI_BOTTOM_APP_BAR_ICON_PADDING 4.f
#endif

/* Tooltip - https://m3.material.io/components/tooltips/specs */
#ifndef IUI_TOOLTIP_MIN_WIDTH
#define IUI_TOOLTIP_MIN_WIDTH 32.f
#endif
#ifndef IUI_TOOLTIP_MIN_HEIGHT
#define IUI_TOOLTIP_MIN_HEIGHT 32.f
#endif
#ifndef IUI_TOOLTIP_MAX_WIDTH
#define IUI_TOOLTIP_MAX_WIDTH 200.f
#endif
#ifndef IUI_TOOLTIP_PADDING
#define IUI_TOOLTIP_PADDING 12.f
#endif
#ifndef IUI_TOOLTIP_CORNER_RADIUS
#define IUI_TOOLTIP_CORNER_RADIUS 4.f
#endif
#ifndef IUI_TOOLTIP_OFFSET
#define IUI_TOOLTIP_OFFSET 8.f /* gap from anchor */
#endif
#ifndef IUI_TOOLTIP_DELAY
#define IUI_TOOLTIP_DELAY 0.5f /* seconds before showing */
#endif

/* Badge - https://m3.material.io/components/badges/specs */
#ifndef IUI_BADGE_DOT_SIZE
#define IUI_BADGE_DOT_SIZE 8.f
#endif
#ifndef IUI_BADGE_LABEL_SIZE
#define IUI_BADGE_LABEL_SIZE 16.f
#endif
#ifndef IUI_BADGE_LABEL_PADDING
#define IUI_BADGE_LABEL_PADDING 4.f
#endif
#ifndef IUI_BADGE_OFFSET_X
#define IUI_BADGE_OFFSET_X 6.f
#endif
#ifndef IUI_BADGE_OFFSET_Y
#define IUI_BADGE_OFFSET_Y 4.f
#endif

/* Banner - https://m3.material.io/components/banners/specs */
#ifndef IUI_BANNER_MIN_HEIGHT
#define IUI_BANNER_MIN_HEIGHT 52.f
#endif
#ifndef IUI_BANNER_PADDING
#define IUI_BANNER_PADDING 16.f
#endif
#ifndef IUI_BANNER_ICON_SIZE
#define IUI_BANNER_ICON_SIZE 24.f
#endif
#ifndef IUI_BANNER_ACTION_GAP
#define IUI_BANNER_ACTION_GAP 8.f
#endif

/* Data Table - https://m3.material.io/components/data-tables/specs */
#ifndef IUI_TABLE_ROW_HEIGHT
#define IUI_TABLE_ROW_HEIGHT 52.f
#endif
#ifndef IUI_TABLE_HEADER_HEIGHT
#define IUI_TABLE_HEADER_HEIGHT 56.f
#endif
#ifndef IUI_TABLE_CELL_PADDING
#define IUI_TABLE_CELL_PADDING 16.f
#endif
#ifndef IUI_TABLE_CHECKBOX_COLUMN_WIDTH
#define IUI_TABLE_CHECKBOX_COLUMN_WIDTH 66.f
#endif
#ifndef IUI_TABLE_DIVIDER_HEIGHT
#define IUI_TABLE_DIVIDER_HEIGHT 1.f
#endif

/* WCAG 2.1 Contrast Ratios
 * Reference: https://www.w3.org/WAI/WCAG21/Understanding/contrast-minimum
 */
#ifndef IUI_WCAG_AA_NORMAL
#define IUI_WCAG_AA_NORMAL 4.5f
#endif
#ifndef IUI_WCAG_AA_LARGE
#define IUI_WCAG_AA_LARGE 3.0f
#endif
#ifndef IUI_WCAG_AAA_NORMAL
#define IUI_WCAG_AAA_NORMAL 7.0f
#endif
#ifndef IUI_WCAG_AAA_LARGE
#define IUI_WCAG_AAA_LARGE 4.5f
#endif

/* MD3 Window Size Class Breakpoints (dp)
 * Reference: https://m3.material.io/foundations/layout/applying-layout
 */
#ifndef IUI_BREAKPOINT_COMPACT
#define IUI_BREAKPOINT_COMPACT 0.f
#endif
#ifndef IUI_BREAKPOINT_MEDIUM
#define IUI_BREAKPOINT_MEDIUM 600.f
#endif
#ifndef IUI_BREAKPOINT_EXPANDED
#define IUI_BREAKPOINT_EXPANDED 840.f
#endif
#ifndef IUI_BREAKPOINT_LARGE
#define IUI_BREAKPOINT_LARGE 1200.f
#endif
#ifndef IUI_BREAKPOINT_XLARGE
#define IUI_BREAKPOINT_XLARGE 1600.f
#endif

/* MD3 Layout Grid - Columns per window size class */
#ifndef IUI_LAYOUT_COLUMNS_COMPACT
#define IUI_LAYOUT_COLUMNS_COMPACT 4
#endif
#ifndef IUI_LAYOUT_COLUMNS_MEDIUM
#define IUI_LAYOUT_COLUMNS_MEDIUM 8
#endif
#ifndef IUI_LAYOUT_COLUMNS_EXPANDED
#define IUI_LAYOUT_COLUMNS_EXPANDED 12
#endif

/* MD3 Layout Grid - Margins per window size class */
#ifndef IUI_LAYOUT_MARGIN_COMPACT
#define IUI_LAYOUT_MARGIN_COMPACT 16.f
#endif
#ifndef IUI_LAYOUT_MARGIN_MEDIUM
#define IUI_LAYOUT_MARGIN_MEDIUM 24.f
#endif
#ifndef IUI_LAYOUT_MARGIN_EXPANDED
#define IUI_LAYOUT_MARGIN_EXPANDED 24.f
#endif

/* MD3 Layout Grid - Gutters (child gaps) per window size class */
#ifndef IUI_LAYOUT_GUTTER_COMPACT
#define IUI_LAYOUT_GUTTER_COMPACT 8.f
#endif
#ifndef IUI_LAYOUT_GUTTER_MEDIUM
#define IUI_LAYOUT_GUTTER_MEDIUM 8.f
#endif
#ifndef IUI_LAYOUT_GUTTER_EXPANDED
#define IUI_LAYOUT_GUTTER_EXPANDED 16.f
#endif

#endif /* IUI_SPEC_H_ */
