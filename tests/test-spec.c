/*
 * Specification Validation Tests
 *
 * Validates that design token constants match the official MD3 specification:
 * - Component dimensions (heights, touch targets, corner radii)
 * - State layer opacities (hover, focus, pressed, dragged, disabled)
 * - Animation duration tokens (short, medium, long)
 *
 * These tests ensure compile-time constants are correct against:
 * - https://m3.material.io/
 * - material-components-android reference implementation
 */

#include "../src/internal.h" /* For state layer alpha constants */
#include "common.h"

/* Include DSL-generated MD3 extended validators and tests */
#include "../src/md3-validate-gen.inc"
#include "test-md3-gen.inc"

/* Component Dimension Tests */

static void test_button_height_40dp(void)
{
    TEST(button_height_40dp);
    /* MD3 spec: Button container height = 40dp */
    ASSERT_NEAR(IUI_SEGMENTED_HEIGHT, 40.f, 0.001f);
    PASS();
}

static void test_fab_standard_56dp(void)
{
    TEST(fab_standard_56dp);
    /* MD3 spec: Standard FAB = 56x56dp */
    ASSERT_NEAR(IUI_FAB_SIZE, 56.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_ICON_SIZE, 24.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_CORNER_RADIUS, 16.f, 0.001f);
    PASS();
}

static void test_fab_large_96dp(void)
{
    TEST(fab_large_96dp);
    /* MD3 spec: Large FAB = 96x96dp */
    ASSERT_NEAR(IUI_FAB_LARGE_SIZE, 96.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_LARGE_ICON_SIZE, 36.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_LARGE_CORNER_RADIUS, 28.f, 0.001f);
    PASS();
}

static void test_fab_extended_height_56dp(void)
{
    TEST(fab_extended_height_56dp);
    /* MD3 spec: Extended FAB height = 56dp */
    ASSERT_NEAR(IUI_FAB_EXTENDED_HEIGHT, 56.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_EXTENDED_PADDING, 16.f, 0.001f);
    ASSERT_NEAR(IUI_FAB_EXTENDED_GAP, 8.f, 0.001f);
    PASS();
}

static void test_icon_button_40dp(void)
{
    TEST(icon_button_40dp);
    /* MD3 spec: Icon button container = 40x40dp */
    ASSERT_NEAR(IUI_ICON_BUTTON_SIZE, 40.f, 0.001f);
    ASSERT_NEAR(IUI_ICON_BUTTON_ICON_SIZE, 24.f, 0.001f);
    /* Corner radius = size/2 for full round */
    ASSERT_NEAR(IUI_ICON_BUTTON_CORNER_RADIUS, 20.f, 0.001f);
    PASS();
}

static void test_chip_height_32dp(void)
{
    TEST(chip_height_32dp);
    /* MD3 spec: Chip height = 32dp */
    ASSERT_NEAR(IUI_CHIP_HEIGHT, 32.f, 0.001f);
    ASSERT_NEAR(IUI_CHIP_CORNER_RADIUS, 8.f, 0.001f);
    ASSERT_NEAR(IUI_CHIP_ICON_SIZE, 18.f, 0.001f);
    PASS();
}

static void test_slider_track_height_4dp(void)
{
    TEST(slider_track_height_4dp);
    /* MD3 spec: Slider track height = 4dp */
    ASSERT_NEAR(IUI_SLIDER_TRACK_HEIGHT, 4.f, 0.001f);
    ASSERT_NEAR(IUI_SLIDER_THUMB_IDLE, 20.f, 0.001f);
    ASSERT_NEAR(IUI_SLIDER_THUMB_PRESSED, 28.f, 0.001f);
    PASS();
}

static void test_tab_height_48dp(void)
{
    TEST(tab_height_48dp);
    /* MD3 spec: Tab height = 48dp */
    ASSERT_NEAR(IUI_TAB_HEIGHT, 48.f, 0.001f);
    ASSERT_NEAR(IUI_TAB_MIN_WIDTH, 90.f, 0.001f);
    ASSERT_NEAR(IUI_TAB_INDICATOR_HEIGHT, 3.f, 0.001f);
    PASS();
}

static void test_search_bar_height_56dp(void)
{
    TEST(search_bar_height_56dp);
    /* MD3 spec: Search bar height = 56dp */
    ASSERT_NEAR(IUI_SEARCH_BAR_HEIGHT, 56.f, 0.001f);
    /* Corner radius = height/2 for full round */
    ASSERT_NEAR(IUI_SEARCH_BAR_CORNER_RADIUS, 28.f, 0.001f);
    PASS();
}

static void test_side_sheet_dimensions(void)
{
    TEST(side_sheet_dimensions);
    /* MD3 spec: Side sheet width = 400dp */
    ASSERT_NEAR(IUI_SIDE_SHEET_WIDTH, 400.f, 0.001f);
    ASSERT_NEAR(IUI_SIDE_SHEET_PADDING, 24.f, 0.001f);
    PASS();
}

static void test_carousel_dimensions(void)
{
    TEST(carousel_dimensions);
    /* MD3 spec: Carousel item width = 240dp, corner = 28dp */
    ASSERT_NEAR(IUI_CAROUSEL_ITEM_WIDTH, 240.f, 0.001f);
    ASSERT_NEAR(IUI_CAROUSEL_ITEM_GAP, 8.f, 0.001f);
    ASSERT_NEAR(IUI_CAROUSEL_CORNER_RADIUS, 28.f, 0.001f);
    PASS();
}

static void test_date_picker_day_32dp(void)
{
    TEST(date_picker_day_32dp);
    /* MD3 spec: Date picker day cell height = 32dp */
    ASSERT_NEAR(IUI_DATE_PICKER_DAY_HEIGHT, 32.f, 0.001f);
    ASSERT_NEAR(IUI_DATE_PICKER_DAY_WIDTH, 36.f, 0.001f);
    ASSERT_NEAR(IUI_DATE_PICKER_DAY_CORNER, 15.f, 0.001f);
    PASS();
}

static void test_menu_item_height(void)
{
    TEST(menu_item_height);
    /* libiui uses 40dp for compact menus (MD3 spec allows 48dp or 56dp) */
    ASSERT_NEAR(IUI_MENU_ITEM_HEIGHT, 40.f, 0.001f);
    ASSERT_NEAR(IUI_MENU_ICON_SIZE, 24.f, 0.001f);
    PASS();
}

static void test_switch_dimensions_32dp(void)
{
    TEST(switch_dimensions_32dp);
    /* MD3 spec: Switch track = 52x32dp, thumb = 24dp */
    ASSERT_NEAR(IUI_SWITCH_TRACK_WIDTH, 52.f, 0.001f);
    ASSERT_NEAR(IUI_SWITCH_TRACK_HEIGHT, 32.f, 0.001f);
    ASSERT_NEAR(IUI_SWITCH_THUMB_SIZE, 24.f, 0.001f);
    /* Corner radius = height/2 for full round track */
    ASSERT_NEAR(IUI_SWITCH_CORNER_RADIUS, 16.f, 0.001f);
    PASS();
}

static void test_textfield_height_56dp(void)
{
    TEST(textfield_height_56dp);
    /* MD3 spec: TextField height = 56dp */
    ASSERT_NEAR(IUI_TEXTFIELD_HEIGHT, 56.f, 0.001f);
    ASSERT_NEAR(IUI_TEXTFIELD_CORNER_RADIUS, 4.f, 0.001f);
    PASS();
}

static void test_button_height_alias(void)
{
    TEST(button_height_alias);
    /* IUI_BUTTON_HEIGHT should equal IUI_SEGMENTED_HEIGHT (40dp) */
    ASSERT_NEAR(IUI_BUTTON_HEIGHT, 40.f, 0.001f);
    ASSERT_NEAR(IUI_BUTTON_HEIGHT, IUI_SEGMENTED_HEIGHT, 0.001f);
    PASS();
}

static void test_appbar_small_height_64dp(void)
{
    TEST(appbar_small_height_64dp);
    /* MD3 spec: Small/Center-aligned Top App Bar = 64dp */
    ASSERT_NEAR(IUI_APPBAR_SMALL_HEIGHT, 64.f, 0.001f);
    ASSERT_NEAR(IUI_APPBAR_COLLAPSED_HEIGHT, 64.f, 0.001f);
    PASS();
}

static void test_appbar_medium_height_112dp(void)
{
    TEST(appbar_medium_height_112dp);
    /* MD3 spec: Medium Top App Bar expanded = 112dp */
    ASSERT_NEAR(IUI_APPBAR_MEDIUM_HEIGHT, 112.f, 0.001f);
    PASS();
}

static void test_appbar_large_height_152dp(void)
{
    TEST(appbar_large_height_152dp);
    /* MD3 spec: Large Top App Bar expanded = 152dp */
    ASSERT_NEAR(IUI_APPBAR_LARGE_HEIGHT, 152.f, 0.001f);
    PASS();
}

static void test_appbar_icon_size_24dp(void)
{
    TEST(appbar_icon_size_24dp);
    /* MD3 spec: App bar icons = 24dp */
    ASSERT_NEAR(IUI_APPBAR_ICON_SIZE, 24.f, 0.001f);
    PASS();
}

static void test_appbar_padding_16dp(void)
{
    TEST(appbar_padding_16dp);
    /* MD3 spec: App bar horizontal padding = 16dp */
    ASSERT_NEAR(IUI_APPBAR_PADDING_H, 16.f, 0.001f);
    ASSERT_NEAR(IUI_APPBAR_TITLE_MARGIN, 16.f, 0.001f);
    /* MD3 spec: m3_appbar_expanded_title_margin_bottom = 16dp */
    ASSERT_NEAR(IUI_APPBAR_TITLE_MARGIN_BOTTOM, 16.f, 0.001f);
    PASS();
}

/* Touch Target Tests (Accessibility)
 * MD3 requires minimum 48dp touch target for interactive components
 */

static void test_touch_target_slider_48dp(void)
{
    TEST(touch_target_slider_48dp);
    ASSERT_TRUE(IUI_SLIDER_TOUCH_TARGET >= 48.f);
    PASS();
}

static void test_touch_target_icon_button_48dp(void)
{
    TEST(touch_target_icon_button_48dp);
    ASSERT_TRUE(IUI_ICON_BUTTON_TOUCH_TARGET >= 48.f);
    PASS();
}

static void test_touch_target_chip_48dp(void)
{
    TEST(touch_target_chip_48dp);
    ASSERT_TRUE(IUI_CHIP_TOUCH_TARGET >= 48.f);
    PASS();
}

static void test_touch_target_tab_48dp(void)
{
    TEST(touch_target_tab_48dp);
    /* Tab height itself is the touch target */
    ASSERT_TRUE(IUI_TAB_HEIGHT >= 48.f);
    PASS();
}

static void test_touch_target_date_nav_48dp(void)
{
    TEST(touch_target_date_nav_48dp);
    ASSERT_TRUE(IUI_DATE_PICKER_NAV_HEIGHT >= 48.f);
    PASS();
}

static void test_touch_target_switch_48dp(void)
{
    TEST(touch_target_switch_48dp);
    ASSERT_TRUE(IUI_SWITCH_TOUCH_TARGET >= 48.f);
    PASS();
}

static void test_touch_target_appbar_48dp(void)
{
    TEST(touch_target_appbar_48dp);
    /* MD3 spec: App bar icon buttons must have 48dp touch target */
    ASSERT_TRUE(IUI_APPBAR_ICON_BUTTON_SIZE >= 48.f);
    PASS();
}

/* State Layer Opacity Tests
 * MD3 spec: https://m3.material.io/foundations/interaction/states
 */

static void test_state_layer_hover_8_percent(void)
{
    TEST(state_layer_hover_8_percent);
    /* MD3 spec: Hover state = 8% opacity = 0x14 (20/255 = 7.8%) */
    ASSERT_EQ(IUI_STATE_HOVER_ALPHA, 0x14);
    PASS();
}

static void test_state_layer_focus_12_percent(void)
{
    TEST(state_layer_focus_12_percent);
    /* MD3 spec: Focus state = 12% opacity = 0x1F (31/255 = 12.2%) */
    ASSERT_EQ(IUI_STATE_FOCUS_ALPHA, 0x1F);
    PASS();
}

static void test_state_layer_pressed_12_percent(void)
{
    TEST(state_layer_pressed_12_percent);
    /* MD3 spec: Pressed state = 12% opacity = 0x1F */
    ASSERT_EQ(IUI_STATE_PRESS_ALPHA, 0x1F);
    PASS();
}

static void test_state_layer_dragged_16_percent(void)
{
    TEST(state_layer_dragged_16_percent);
    /* MD3 spec: Dragged state = 16% opacity = 0x29 (41/255 = 16.1%) */
    ASSERT_EQ(IUI_STATE_DRAG_ALPHA, 0x29);
    PASS();
}

static void test_state_layer_disabled_38_percent(void)
{
    TEST(state_layer_disabled_38_percent);
    /* MD3 spec: Disabled state = 38% opacity = 0x61 (97/255 = 38%) */
    ASSERT_EQ(IUI_STATE_DISABLE_ALPHA, 0x61);
    PASS();
}

/* Animation Duration Token Tests
 * MD3 spec: https://m3.material.io/styles/motion
 */

static void test_duration_short_1_50ms(void)
{
    TEST(duration_short_1_50ms);
    /* MD3 spec: Short 1 = 50ms */
    ASSERT_NEAR(IUI_DURATION_SHORT_1, 0.050f, 0.001f);
    PASS();
}

static void test_duration_short_2_100ms(void)
{
    TEST(duration_short_2_100ms);
    /* MD3 spec: Short 2 = 100ms */
    ASSERT_NEAR(IUI_DURATION_SHORT_2, 0.100f, 0.001f);
    PASS();
}

static void test_duration_short_3_150ms(void)
{
    TEST(duration_short_3_150ms);
    /* MD3 spec: Short 3 = 150ms */
    ASSERT_NEAR(IUI_DURATION_SHORT_3, 0.150f, 0.001f);
    PASS();
}

static void test_duration_short_4_200ms(void)
{
    TEST(duration_short_4_200ms);
    /* MD3 spec: Short 4 = 200ms */
    ASSERT_NEAR(IUI_DURATION_SHORT_4, 0.200f, 0.001f);
    PASS();
}

static void test_duration_medium_1_250ms(void)
{
    TEST(duration_medium_1_250ms);
    /* MD3 spec: Medium 1 = 250ms */
    ASSERT_NEAR(IUI_DURATION_MEDIUM_1, 0.250f, 0.001f);
    PASS();
}

static void test_duration_medium_2_300ms(void)
{
    TEST(duration_medium_2_300ms);
    /* MD3 spec: Medium 2 = 300ms */
    ASSERT_NEAR(IUI_DURATION_MEDIUM_2, 0.300f, 0.001f);
    PASS();
}

static void test_duration_medium_3_350ms(void)
{
    TEST(duration_medium_3_350ms);
    /* MD3 spec: Medium 3 = 350ms */
    ASSERT_NEAR(IUI_DURATION_MEDIUM_3, 0.350f, 0.001f);
    PASS();
}

static void test_duration_medium_4_400ms(void)
{
    TEST(duration_medium_4_400ms);
    /* MD3 spec: Medium 4 = 400ms */
    ASSERT_NEAR(IUI_DURATION_MEDIUM_4, 0.400f, 0.001f);
    PASS();
}

static void test_duration_long_1_450ms(void)
{
    TEST(duration_long_1_450ms);
    /* MD3 spec: Long 1 = 450ms */
    ASSERT_NEAR(IUI_DURATION_LONG_1, 0.450f, 0.001f);
    PASS();
}

static void test_duration_long_2_500ms(void)
{
    TEST(duration_long_2_500ms);
    /* MD3 spec: Long 2 = 500ms */
    ASSERT_NEAR(IUI_DURATION_LONG_2, 0.500f, 0.001f);
    PASS();
}

/* Easing Function Tests
 * MD3 spec: https://m3.material.io/styles/motion/easing-and-duration
 */

static void test_easing_standard_bounds(void)
{
    TEST(easing_standard_bounds);
    /* Easing functions should map [0,1] -> [0,1] for boundary inputs */
    ASSERT_NEAR(ease_in_quad(0.f), 0.f, 0.001f);
    ASSERT_NEAR(ease_in_quad(1.f), 1.f, 0.001f);
    ASSERT_NEAR(ease_in_cubic(0.f), 0.f, 0.001f);
    ASSERT_NEAR(ease_in_cubic(1.f), 1.f, 0.001f);
    PASS();
}

static void test_easing_emphasized_midpoint(void)
{
    TEST(easing_emphasized_midpoint);
    /* ease_in_quad at midpoint should be 0.25 (x^2 = 0.5^2) */
    ASSERT_NEAR(ease_in_quad(0.5f), 0.25f, 0.001f);
    /* ease_in_cubic at midpoint should be 0.125 (x^3 = 0.5^3) */
    ASSERT_NEAR(ease_in_cubic(0.5f), 0.125f, 0.001f);
    PASS();
}

/* WCAG Accessibility Contrast Tests
 * Reference: https://www.w3.org/WAI/WCAG21/Understanding/contrast-minimum.html
 */

static void test_relative_luminance_black(void)
{
    TEST(relative_luminance_black);
    /* Pure black should have luminance 0.0 */
    float lum = iui_relative_luminance(0xFF000000);
    ASSERT_NEAR(lum, 0.f, 0.001f);
    PASS();
}

static void test_relative_luminance_white(void)
{
    TEST(relative_luminance_white);
    /* Pure white should have luminance 1.0 */
    float lum = iui_relative_luminance(0xFFFFFFFF);
    ASSERT_NEAR(lum, 1.f, 0.001f);
    PASS();
}

static void test_relative_luminance_gray(void)
{
    TEST(relative_luminance_gray);
    /* Mid-gray (sRGB 0x808080) has luminance ~0.22 due to gamma */
    float lum = iui_relative_luminance(0xFF808080);
    /* Should be around 0.2158 (sRGB encoded) */
    ASSERT_TRUE(lum > 0.18f && lum < 0.26f);
    PASS();
}

static void test_contrast_ratio_black_white(void)
{
    TEST(contrast_ratio_black_white);
    /* Black to white should be maximum contrast: 21:1 */
    float ratio = iui_contrast_ratio(0xFF000000, 0xFFFFFFFF);
    ASSERT_NEAR(ratio, 21.f, 0.01f);
    PASS();
}

static void test_contrast_ratio_same_color(void)
{
    TEST(contrast_ratio_same_color);
    /* Same color should have ratio 1:1 */
    float ratio = iui_contrast_ratio(0xFF6750A4, 0xFF6750A4);
    ASSERT_NEAR(ratio, 1.f, 0.001f);
    PASS();
}

static void test_contrast_ratio_order_independent(void)
{
    TEST(contrast_ratio_order_independent);
    /* Contrast ratio should be same regardless of argument order */
    float ratio1 = iui_contrast_ratio(0xFF000000, 0xFFFFFFFF),
          ratio2 = iui_contrast_ratio(0xFFFFFFFF, 0xFF000000);
    ASSERT_NEAR(ratio1, ratio2, 0.001f);
    PASS();
}

static void test_wcag_aa_normal_threshold(void)
{
    TEST(wcag_aa_normal_threshold);
    /* WCAG AA normal text requires 4.5:1 contrast
     * Black on white passes (21:1)
     */
    ASSERT_TRUE(iui_wcag_aa_normal(0xFF000000, 0xFFFFFFFF));
    /* Light gray on white fails (insufficient contrast) */
    ASSERT_FALSE(iui_wcag_aa_normal(0xFFAAAAAA, 0xFFFFFFFF));
    PASS();
}

static void test_wcag_aa_large_threshold(void)
{
    TEST(wcag_aa_large_threshold);
    /* WCAG AA large text requires 3:1 contrast
     * Mid-gray on white should pass for large text but fail for normal
     * 0xFF777777 on white: ratio ~4.48:1 - passes AA large
     */
    ASSERT_TRUE(iui_wcag_aa_large(0xFF777777, 0xFFFFFFFF));
    PASS();
}

static void test_wcag_aaa_normal_threshold(void)
{
    TEST(wcag_aaa_normal_threshold);
    /* WCAG AAA normal text requires 7:1 contrast
     * Black on white passes (21:1)
     */
    ASSERT_TRUE(iui_wcag_aaa_normal(0xFF000000, 0xFFFFFFFF));
    /* Dark gray on white at exactly 7:1 boundary
     * 0xFF595959 is approximately 7:1 on white
     */
    float ratio = iui_contrast_ratio(0xFF595959, 0xFFFFFFFF);
    ASSERT_TRUE(ratio >= 6.5f); /* Close to threshold */
    PASS();
}

static void test_light_theme_surface_contrast(void)
{
    TEST(light_theme_surface_contrast);
    /* Light theme: on_surface (0xFF1D1B20) on surface (0xFFFEF7FF) */
    const iui_theme_t *theme = iui_theme_light();
    float ratio = iui_contrast_ratio(theme->on_surface, theme->surface);
    /* MD3 themes should pass WCAG AA (4.5:1) */
    ASSERT_TRUE(ratio >= IUI_WCAG_AA_NORMAL);
    PASS();
}

static void test_dark_theme_surface_contrast(void)
{
    TEST(dark_theme_surface_contrast);
    /* Dark theme: on_surface (0xFFE6E0E9) on surface (0xFF141218) */
    const iui_theme_t *theme = iui_theme_dark();
    float ratio = iui_contrast_ratio(theme->on_surface, theme->surface);
    /* MD3 themes should pass WCAG AA (4.5:1) */
    ASSERT_TRUE(ratio >= IUI_WCAG_AA_NORMAL);
    PASS();
}

static void test_light_theme_primary_contrast(void)
{
    TEST(light_theme_primary_contrast);
    /* Light theme: on_primary (white) on primary (0xFF6750A4) */
    const iui_theme_t *theme = iui_theme_light();
    float ratio = iui_contrast_ratio(theme->on_primary, theme->primary);
    /* Primary buttons should have readable text */
    ASSERT_TRUE(ratio >= IUI_WCAG_AA_LARGE);
    PASS();
}

static void test_dark_theme_primary_contrast(void)
{
    TEST(dark_theme_primary_contrast);
    /* Dark theme: on_primary (0xFF381E72) on primary (0xFFD0BCFF) */
    const iui_theme_t *theme = iui_theme_dark();
    float ratio = iui_contrast_ratio(theme->on_primary, theme->primary);
    /* Primary buttons should have readable text */
    ASSERT_TRUE(ratio >= IUI_WCAG_AA_LARGE);
    PASS();
}

static void test_light_theme_error_contrast(void)
{
    TEST(light_theme_error_contrast);
    /* Light theme: on_error (white) on error (0xFFB3261E) */
    const iui_theme_t *theme = iui_theme_light();
    float ratio = iui_contrast_ratio(theme->on_error, theme->error);
    /* Error states should be clearly readable */
    ASSERT_TRUE(ratio >= IUI_WCAG_AA_LARGE);
    PASS();
}

static void test_theme_validate_light(void)
{
    TEST(theme_validate_light);
    /* Light theme should pass all WCAG AA checks */
    const iui_theme_t *theme = iui_theme_light();
    int failures = iui_theme_validate_contrast(theme);
    ASSERT_EQ(failures, 0);
    PASS();
}

static void test_theme_validate_dark(void)
{
    TEST(theme_validate_dark);
    /* Dark theme should pass all WCAG AA checks */
    const iui_theme_t *theme = iui_theme_dark();
    int failures = iui_theme_validate_contrast(theme);
    ASSERT_EQ(failures, 0);
    PASS();
}

static void test_theme_validate_null(void)
{
    TEST(theme_validate_null);
    /* NULL theme should return -1 */
    int failures = iui_theme_validate_contrast(NULL);
    ASSERT_EQ(failures, -1);
    PASS();
}

/* Screen Reader Accessibility Tests
 * Reference: https://m3.material.io/foundations/accessible-design
 * Reference: https://www.w3.org/WAI/ARIA/apg/patterns/
 */

static void test_a11y_role_names(void)
{
    TEST(a11y_role_names);
    /* Verify all roles have proper string names */
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_NONE), "none");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_BUTTON), "button");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_CHECKBOX), "checkbox");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_RADIO), "radio");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_SWITCH), "switch");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_SLIDER), "slider");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_TEXTFIELD), "textfield");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_MENU), "menu");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_MENUITEM), "menuitem");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_TAB), "tab");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_DIALOG), "dialog");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_ALERT), "alert");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_PROGRESSBAR), "progressbar");
    ASSERT_STR_EQ(iui_a11y_role_name(IUI_A11Y_ROLE_SEARCH), "search");
    /* Invalid role should return "unknown" */
    ASSERT_STR_EQ(iui_a11y_role_name((enum iui_a11y_role) 999), "unknown");
    PASS();
}

static void test_a11y_state_descriptions(void)
{
    TEST(a11y_state_descriptions);
    /* No state should return empty string */
    const char *desc = iui_a11y_state_description(IUI_A11Y_STATE_NONE);
    ASSERT_STR_EQ(desc, "");

    /* Single states */
    desc = iui_a11y_state_description(IUI_A11Y_STATE_CHECKED);
    ASSERT_TRUE(strstr(desc, "checked") != NULL);

    desc = iui_a11y_state_description(IUI_A11Y_STATE_DISABLED);
    ASSERT_TRUE(strstr(desc, "disabled") != NULL);

    desc = iui_a11y_state_description(IUI_A11Y_STATE_EXPANDED);
    ASSERT_TRUE(strstr(desc, "expanded") != NULL);

    /* Combined states */
    desc = iui_a11y_state_description(IUI_A11Y_STATE_CHECKED |
                                      IUI_A11Y_STATE_FOCUSED);
    ASSERT_TRUE(strstr(desc, "checked") != NULL);
    ASSERT_TRUE(strstr(desc, "focused") != NULL);
    PASS();
}

static void test_a11y_make_hint(void)
{
    TEST(a11y_make_hint);
    iui_a11y_hint hint = iui_a11y_make_hint("Submit", IUI_A11Y_ROLE_BUTTON);
    ASSERT_STR_EQ(hint.label, "Submit");
    ASSERT_EQ(hint.role, IUI_A11Y_ROLE_BUTTON);
    ASSERT_EQ(hint.state, IUI_A11Y_STATE_NONE);
    ASSERT_TRUE(hint.description == NULL);
    PASS();
}

static void test_a11y_make_slider_hint(void)
{
    TEST(a11y_make_slider_hint);
    iui_a11y_hint hint = iui_a11y_make_slider_hint("Volume", 50.f, 0.f, 100.f);
    ASSERT_STR_EQ(hint.label, "Volume");
    ASSERT_EQ(hint.role, IUI_A11Y_ROLE_SLIDER);
    ASSERT_NEAR(hint.value_now, 50.f, 0.001f);
    ASSERT_NEAR(hint.value_min, 0.f, 0.001f);
    ASSERT_NEAR(hint.value_max, 100.f, 0.001f);
    PASS();
}

static void test_a11y_make_set_hint(void)
{
    TEST(a11y_make_set_hint);
    iui_a11y_hint hint =
        iui_a11y_make_set_hint("Home", IUI_A11Y_ROLE_TAB, 1, 4);
    ASSERT_STR_EQ(hint.label, "Home");
    ASSERT_EQ(hint.role, IUI_A11Y_ROLE_TAB);
    ASSERT_EQ(hint.position_in_set, 1);
    ASSERT_EQ(hint.set_size, 4);
    PASS();
}

static void test_a11y_describe_button(void)
{
    TEST(a11y_describe_button);
    iui_a11y_hint hint = iui_a11y_make_hint("Submit", IUI_A11Y_ROLE_BUTTON);
    char buf[256];
    int len = iui_a11y_describe(&hint, buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    ASSERT_TRUE(strstr(buf, "Submit") != NULL);
    ASSERT_TRUE(strstr(buf, "button") != NULL);
    PASS();
}

static void test_a11y_describe_checkbox(void)
{
    TEST(a11y_describe_checkbox);
    iui_a11y_hint hint =
        iui_a11y_make_hint("Enable notifications", IUI_A11Y_ROLE_CHECKBOX);
    hint.state = IUI_A11Y_STATE_CHECKED;
    char buf[256];
    int len = iui_a11y_describe(&hint, buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    ASSERT_TRUE(strstr(buf, "Enable notifications") != NULL);
    ASSERT_TRUE(strstr(buf, "checkbox") != NULL);
    ASSERT_TRUE(strstr(buf, "checked") != NULL);
    PASS();
}

static void test_a11y_describe_slider(void)
{
    TEST(a11y_describe_slider);
    iui_a11y_hint hint = iui_a11y_make_slider_hint("Volume", 75.f, 0.f, 100.f);
    char buf[256];
    int len = iui_a11y_describe(&hint, buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    ASSERT_TRUE(strstr(buf, "Volume") != NULL);
    ASSERT_TRUE(strstr(buf, "slider") != NULL);
    ASSERT_TRUE(strstr(buf, "75") != NULL); /* Value in description */
    PASS();
}

static void test_a11y_describe_radio(void)
{
    TEST(a11y_describe_radio);
    iui_a11y_hint hint =
        iui_a11y_make_set_hint("Option B", IUI_A11Y_ROLE_RADIO, 2, 3);
    hint.state = IUI_A11Y_STATE_SELECTED;
    char buf[256];
    int len = iui_a11y_describe(&hint, buf, sizeof(buf));
    ASSERT_TRUE(len > 0);
    ASSERT_TRUE(strstr(buf, "Option B") != NULL);
    ASSERT_TRUE(strstr(buf, "radio") != NULL);
    ASSERT_TRUE(strstr(buf, "2 of 3") != NULL); /* Position in set */
    ASSERT_TRUE(strstr(buf, "selected") != NULL);
    PASS();
}

static void test_accessibility_functions(void)
{
    TEST(accessibility_functions);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test accessibility callbacks setup */
    ASSERT_FALSE(iui_a11y_enabled(ctx));

    /* Create a simple hint */
    iui_a11y_hint hint =
        iui_a11y_make_hint("Test Button", IUI_A11Y_ROLE_BUTTON);
    ASSERT_TRUE(hint.role == IUI_A11Y_ROLE_BUTTON);

    /* Test slider hint */
    iui_a11y_hint slider_hint =
        iui_a11y_make_slider_hint("Volume", 50.0f, 0.0f, 100.0f);
    ASSERT_TRUE(slider_hint.role == IUI_A11Y_ROLE_SLIDER);

    /* Test set hint */
    iui_a11y_hint set_hint =
        iui_a11y_make_set_hint("Option 1", IUI_A11Y_ROLE_RADIO, 1, 3);
    ASSERT_TRUE(set_hint.role == IUI_A11Y_ROLE_RADIO);

    /* Test hint description */
    char desc[256];
    int len = iui_a11y_describe(&hint, desc, sizeof(desc));
    ASSERT_TRUE(len > 0);

    /* Test accessibility notification functions */
    iui_rect_t bounds = {0, 0, 100, 30};
    iui_a11y_notify_focus(ctx, &hint, bounds);
    iui_a11y_notify_state(ctx, &hint, IUI_A11Y_STATE_NONE,
                          IUI_A11Y_STATE_CHECKED);
    iui_a11y_notify_value(ctx, &slider_hint, 50.0f, 60.0f);

    free(buffer);
    PASS();
}

static void test_wcag_contrast_functions(void)
{
    TEST(wcag_contrast_functions);

    /* Test luminance calculation */
    float lum = iui_relative_luminance(0xFF0000FF); /* Red */
    ASSERT_TRUE(lum >= 0.0f && lum <= 1.0f);

    /* Test contrast ratio */
    float ratio = iui_contrast_ratio(0xFF000000, 0xFFFFFFFF); /* Black/White */
    ASSERT_NEAR(ratio, 21.0f, 0.1f);

    /* Test WCAG compliance checks */
    bool aa_normal = iui_wcag_aa_normal(0xFF000000, 0xFFFFFFFF);
    ASSERT_TRUE(aa_normal);

    bool aa_large = iui_wcag_aa_large(0xFF000000, 0xFFFFFFFF);
    ASSERT_TRUE(aa_large);

    bool aaa_normal = iui_wcag_aaa_normal(0xFF000000, 0xFFFFFFFF);
    ASSERT_TRUE(aaa_normal);

    bool aaa_large = iui_wcag_aaa_large(0xFF000000, 0xFFFFFFFF);
    ASSERT_TRUE(aaa_large);

    PASS();
}

static void test_a11y_callbacks_disabled_by_default(void)
{
    TEST(a11y_callbacks_disabled_by_default);
    const iui_context *ctx = test_init_context();
    ASSERT_NOT_NULL(ctx);
    /* Accessibility should be disabled by default */
    ASSERT_FALSE(iui_a11y_enabled(ctx));
    ASSERT_TRUE(iui_get_a11y_callbacks(ctx) == NULL);
    PASS();
}

/* Test callback tracking */
static int g_test_announce_count;
static char g_test_announce_text[256];
static enum iui_a11y_live g_test_announce_priority;

static void test_announce_callback(const char *text,
                                   enum iui_a11y_live priority,
                                   void *user)
{
    (void) user;
    g_test_announce_count++;
    if (text) {
        strncpy(g_test_announce_text, text, sizeof(g_test_announce_text) - 1);
        g_test_announce_text[sizeof(g_test_announce_text) - 1] = '\0';
    }
    g_test_announce_priority = priority;
}

static void test_a11y_callbacks_enable_disable(void)
{
    TEST(a11y_callbacks_enable_disable);
    iui_context *ctx = test_init_context();
    ASSERT_NOT_NULL(ctx);

    /* Enable callbacks */
    iui_a11y_callbacks cbs = {0};
    cbs.announce = test_announce_callback;
    cbs.user = NULL;
    iui_set_a11y_callbacks(ctx, &cbs);

    ASSERT_TRUE(iui_a11y_enabled(ctx));
    ASSERT_NOT_NULL(iui_get_a11y_callbacks(ctx));

    /* Test announcement */
    g_test_announce_count = 0;
    iui_announce(ctx, "Test message", IUI_A11Y_LIVE_POLITE);
    ASSERT_EQ(g_test_announce_count, 1);
    ASSERT_STR_EQ(g_test_announce_text, "Test message");
    ASSERT_EQ(g_test_announce_priority, IUI_A11Y_LIVE_POLITE);

    /* Disable callbacks */
    iui_set_a11y_callbacks(ctx, NULL);
    ASSERT_FALSE(iui_a11y_enabled(ctx));

    /* Announcement should not trigger callback */
    g_test_announce_count = 0;
    iui_announce(ctx, "Should not announce", IUI_A11Y_LIVE_ASSERTIVE);
    ASSERT_EQ(g_test_announce_count, 0);
    PASS();
}

static void test_a11y_push_pop(void)
{
    TEST(a11y_push_pop);
    iui_context *ctx = test_init_context();
    ASSERT_NOT_NULL(ctx);

    /* Initial stack should be empty */
    ASSERT_EQ(ctx->a11y_stack_depth, 0);

    /* Push context */
    iui_a11y_hint hint = iui_a11y_make_hint("Navigation", IUI_A11Y_ROLE_MENU);
    iui_a11y_push(ctx, &hint);
    ASSERT_EQ(ctx->a11y_stack_depth, 1);
    ASSERT_STR_EQ(ctx->a11y_stack[0].label, "Navigation");

    /* Push another */
    iui_a11y_hint hint2 = iui_a11y_make_hint("File", IUI_A11Y_ROLE_MENUITEM);
    iui_a11y_push(ctx, &hint2);
    ASSERT_EQ(ctx->a11y_stack_depth, 2);

    /* Pop */
    iui_a11y_pop(ctx);
    ASSERT_EQ(ctx->a11y_stack_depth, 1);

    iui_a11y_pop(ctx);
    ASSERT_EQ(ctx->a11y_stack_depth, 0);

    /* Pop on empty should be safe */
    iui_a11y_pop(ctx);
    ASSERT_EQ(ctx->a11y_stack_depth, 0);
    PASS();
}

/* MD3 Validation API Tests
 * Tests the validation functions in md3-validate.h
 */

static void test_md3_validate_rounding(void)
{
    TEST(md3_validate_rounding);
    /* dp to px should clamp to at least 1px for positive inputs */
    ASSERT_EQ(md3_dp_to_px(0.1f, 4.f), 1);
    /* Non-positive scale returns 0 to avoid bogus checks */
    ASSERT_EQ(md3_dp_to_px(0.f, 48.f), 0);
    ASSERT_EQ(md3_round_px(0.6f), 1);
    ASSERT_EQ(md3_round_px(0.f), 0);
    PASS();
}

static void test_md3_validate_button(void)
{
    TEST(md3_validate_button);
    /* Button at 40dp (1.0 scale) should pass */
    ASSERT_EQ(md3_check_button(40, 1.f), MD3_OK);
    /* Button below 40dp should fail */
    ASSERT_TRUE(md3_check_button(30, 1.f) & MD3_HEIGHT_LOW);
    /* Button at 2x scale: 80px = 40dp, should pass */
    ASSERT_EQ(md3_check_button(80, 2.f), MD3_OK);
    PASS();
}

static void test_md3_validate_fab(void)
{
    TEST(md3_validate_fab);
    /* Standard FAB at 56dp should pass */
    ASSERT_EQ(md3_check_fab(56, 1.f), MD3_OK);
    /* FAB at 55dp (1px tolerance) should pass */
    ASSERT_EQ(md3_check_fab(55, 1.f), MD3_OK);
    /* FAB too small should fail (exact-size uses MD3_SIZE_MISMATCH) */
    ASSERT_TRUE(md3_check_fab(50, 1.f) & MD3_SIZE_MISMATCH);
    /* Large FAB at 96dp should pass */
    ASSERT_EQ(md3_check_fab_large(96, 1.f), MD3_OK);
    /* Large FAB too small should fail */
    ASSERT_TRUE(md3_check_fab_large(80, 1.f) & MD3_SIZE_MISMATCH);
    PASS();
}

static void test_md3_validate_chip(void)
{
    TEST(md3_validate_chip);
    /* Chip at 32dp should pass */
    ASSERT_EQ(md3_check_chip(32, 1.f), MD3_OK);
    /* Chip below 32dp should fail */
    ASSERT_TRUE(md3_check_chip(28, 1.f) & MD3_HEIGHT_LOW);
    /* Chip at 2x scale: 64px = 32dp, should pass */
    ASSERT_EQ(md3_check_chip(64, 2.f), MD3_OK);
    PASS();
}

static void test_md3_validate_textfield(void)
{
    TEST(md3_validate_textfield);
    /* TextField at 56dp should pass */
    ASSERT_EQ(md3_check_textfield(56, 1.f), MD3_OK);
    /* TextField below 56dp should fail */
    ASSERT_TRUE(md3_check_textfield(48, 1.f) & MD3_HEIGHT_LOW);
    PASS();
}

static void test_md3_validate_touch_target(void)
{
    TEST(md3_validate_touch_target);
    /* 48x48 should pass */
    ASSERT_EQ(md3_check_touch_target(48, 48, 1.f), MD3_OK);
    /* 56x56 should pass */
    ASSERT_EQ(md3_check_touch_target(56, 56, 1.f), MD3_OK);
    /* 40x48 should fail (width too small) */
    ASSERT_TRUE(md3_check_touch_target(40, 48, 1.f) & MD3_TOUCH_TARGET);
    /* 48x40 should fail (height too small) */
    ASSERT_TRUE(md3_check_touch_target(48, 40, 1.f) & MD3_TOUCH_TARGET);
    /* At 2x scale: 96x96 = 48x48dp, should pass */
    ASSERT_EQ(md3_check_touch_target(96, 96, 2.f), MD3_OK);
    PASS();
}

static void test_md3_validate_grid_align(void)
{
    TEST(md3_validate_grid_align);
    /* Multiple of 4 should pass */
    ASSERT_EQ(md3_check_grid_align(0, 1.f), MD3_OK);
    ASSERT_EQ(md3_check_grid_align(4, 1.f), MD3_OK);
    ASSERT_EQ(md3_check_grid_align(8, 1.f), MD3_OK);
    ASSERT_EQ(md3_check_grid_align(100, 1.f), MD3_OK);
    /* Not multiple of 4 should fail */
    ASSERT_TRUE(md3_check_grid_align(5, 1.f) & MD3_GRID_ALIGN);
    ASSERT_TRUE(md3_check_grid_align(7, 1.f) & MD3_GRID_ALIGN);
    /* At 2x scale: grid is 8px */
    ASSERT_EQ(md3_check_grid_align(16, 2.f), MD3_OK);
    ASSERT_TRUE(md3_check_grid_align(18, 2.f) & MD3_GRID_ALIGN);
    PASS();
}

/* MD3 Boundary Condition Tests - test exact thresholds */
static void test_md3_validate_boundaries(void)
{
    TEST(md3_validate_boundaries);
    /* Button: exactly at 40dp passes, 39dp fails */
    ASSERT_EQ(md3_check_button(40, 1.f), MD3_OK);
    ASSERT_TRUE(md3_check_button(39, 1.f) & MD3_HEIGHT_LOW);

    /* FAB: tolerance is ±1px, so 54dp fails (56-54=2 > 1) */
    ASSERT_EQ(md3_check_fab(57, 1.f), MD3_OK);               /* +1 tolerance */
    ASSERT_EQ(md3_check_fab(55, 1.f), MD3_OK);               /* -1 tolerance */
    ASSERT_TRUE(md3_check_fab(54, 1.f) & MD3_SIZE_MISMATCH); /* -2 fails */
    ASSERT_TRUE(md3_check_fab(58, 1.f) & MD3_SIZE_MISMATCH); /* +2 fails */

    /* Large FAB: 96dp ±1px tolerance */
    ASSERT_EQ(md3_check_fab_large(97, 1.f), MD3_OK);
    ASSERT_EQ(md3_check_fab_large(95, 1.f), MD3_OK);
    ASSERT_TRUE(md3_check_fab_large(94, 1.f) & MD3_SIZE_MISMATCH);

    /* Chip: exactly at 32dp passes, 31dp fails */
    ASSERT_EQ(md3_check_chip(32, 1.f), MD3_OK);
    ASSERT_TRUE(md3_check_chip(31, 1.f) & MD3_HEIGHT_LOW);

    /* TextField: exactly at 56dp passes, 55dp fails */
    ASSERT_EQ(md3_check_textfield(56, 1.f), MD3_OK);
    ASSERT_TRUE(md3_check_textfield(55, 1.f) & MD3_HEIGHT_LOW);

    /* Touch target: exactly at 48dp passes, 47dp fails */
    ASSERT_EQ(md3_check_touch_target(48, 48, 1.f), MD3_OK);
    ASSERT_TRUE(md3_check_touch_target(47, 48, 1.f) & MD3_TOUCH_TARGET);
    ASSERT_TRUE(md3_check_touch_target(48, 47, 1.f) & MD3_TOUCH_TARGET);
    PASS();
}

/* MD3 Scale Factor Tests - various DPI scales */
static void test_md3_validate_scale_factors(void)
{
    TEST(md3_validate_scale_factors);
    /* 0.5x scale (low DPI): 40dp = 20px */
    ASSERT_EQ(md3_check_button(20, 0.5f), MD3_OK);
    ASSERT_TRUE(md3_check_button(19, 0.5f) & MD3_HEIGHT_LOW);

    /* 1.5x scale (hdpi): 40dp = 60px */
    ASSERT_EQ(md3_check_button(60, 1.5f), MD3_OK);
    ASSERT_TRUE(md3_check_button(59, 1.5f) & MD3_HEIGHT_LOW);

    /* 3x scale (xxhdpi): 40dp = 120px */
    ASSERT_EQ(md3_check_button(120, 3.f), MD3_OK);
    ASSERT_TRUE(md3_check_button(119, 3.f) & MD3_HEIGHT_LOW);

    /* Touch target at various scales */
    ASSERT_EQ(md3_check_touch_target(24, 24, 0.5f), MD3_OK);  /* 48dp at 0.5x */
    ASSERT_EQ(md3_check_touch_target(72, 72, 1.5f), MD3_OK);  /* 48dp at 1.5x */
    ASSERT_EQ(md3_check_touch_target(144, 144, 3.f), MD3_OK); /* 48dp at 3x */

    /* Grid at various scales */
    ASSERT_EQ(md3_check_grid_align(2, 0.5f), MD3_OK); /* 4dp at 0.5x = 2px */
    ASSERT_EQ(md3_check_grid_align(6, 1.5f), MD3_OK); /* 4dp at 1.5x = 6px */
    ASSERT_EQ(md3_check_grid_align(12, 3.f), MD3_OK); /* 4dp at 3x = 12px */
    PASS();
}

/* MD3 Edge Cases - invalid inputs */
static void test_md3_validate_edge_cases(void)
{
    TEST(md3_validate_edge_cases);
    /* Zero scale should not crash, returns 0 from md3_dp_to_px */
    ASSERT_EQ(md3_dp_to_px(0.f, 40.f), 0);
    ASSERT_EQ(md3_dp_to_px(-1.f, 40.f), 0);

    /* Zero dp should return 0 */
    ASSERT_EQ(md3_dp_to_px(1.f, 0.f), 0);
    ASSERT_EQ(md3_dp_to_px(1.f, -1.f), 0);

    /* Tiny scale should clamp to at least 1px for positive dp */
    ASSERT_EQ(md3_dp_to_px(0.01f, 40.f), 1); /* 0.4 rounds to 1 (clamped) */

    /* Zero/negative px rounding */
    ASSERT_EQ(md3_round_px(-1.f), 0);
    ASSERT_EQ(md3_round_px(0.f), 0);
    ASSERT_EQ(md3_round_px(0.4f), 0); /* rounds down */
    ASSERT_EQ(md3_round_px(0.5f), 1); /* rounds up */

    /* Grid align with zero scale should return OK (safe fallback) */
    ASSERT_EQ(md3_check_grid_align(5, 0.f), MD3_OK);
    PASS();
}

/* MD3 Bitmask Combination Tests */
static void test_md3_validate_bitmask(void)
{
    TEST(md3_validate_bitmask);
    /* Test that violations can be combined via OR */
    md3_violation_t v1 = MD3_HEIGHT_LOW;
    md3_violation_t v2 = MD3_TOUCH_TARGET;
    md3_violation_t combined = v1 | v2;

    ASSERT_TRUE(combined & MD3_HEIGHT_LOW);
    ASSERT_TRUE(combined & MD3_TOUCH_TARGET);
    ASSERT_FALSE(combined & MD3_GRID_ALIGN);

    /* Test all flags are distinct powers of 2 (matches DSL-generated enum) */
    ASSERT_EQ(MD3_OK, 0);
    ASSERT_EQ(MD3_HEIGHT_LOW, 1 << 0);
    ASSERT_EQ(MD3_SIZE_MISMATCH, 1 << 1);
    ASSERT_EQ(MD3_TOUCH_TARGET, 1 << 2);
    ASSERT_EQ(MD3_GRID_ALIGN, 1 << 3);
    ASSERT_EQ(MD3_STATE_OPACITY, 1 << 4);
    ASSERT_EQ(MD3_CORNER_RADIUS, 1 << 5);
    ASSERT_EQ(MD3_THUMB_SIZE, 1 << 6);
    ASSERT_EQ(MD3_ICON_SIZE, 1 << 7);
    ASSERT_EQ(MD3_PADDING, 1 << 8);
    ASSERT_EQ(MD3_GAP, 1 << 9);
    ASSERT_EQ(MD3_INDICATOR, 1 << 10);
    ASSERT_EQ(MD3_WIDTH_LOW, 1 << 11);
    PASS();
}

/* MD3 State Alpha Tests (uses iui_state_t from iui.h) */
static void test_md3_validate_state_alpha(void)
{
    TEST(md3_validate_state_alpha);
    /* Correct alpha values should pass */
    ASSERT_EQ(md3_check_state_alpha(IUI_STATE_HOVER_ALPHA, IUI_STATE_HOVERED),
              MD3_OK);
    ASSERT_EQ(md3_check_state_alpha(IUI_STATE_FOCUS_ALPHA, IUI_STATE_FOCUSED),
              MD3_OK);
    ASSERT_EQ(md3_check_state_alpha(IUI_STATE_PRESS_ALPHA, IUI_STATE_PRESSED),
              MD3_OK);
    ASSERT_EQ(md3_check_state_alpha(IUI_STATE_DRAG_ALPHA, IUI_STATE_DRAGGED),
              MD3_OK);
    ASSERT_EQ(
        md3_check_state_alpha(IUI_STATE_DISABLE_ALPHA, IUI_STATE_DISABLED),
        MD3_OK);

    /* Wrong alpha values should fail */
    ASSERT_TRUE(md3_check_state_alpha(0x00, IUI_STATE_HOVERED) &
                MD3_STATE_OPACITY);
    ASSERT_TRUE(md3_check_state_alpha(0xFF, IUI_STATE_PRESSED) &
                MD3_STATE_OPACITY);

    /* Default state (none) should always pass regardless of alpha */
    ASSERT_EQ(md3_check_state_alpha(0x00, IUI_STATE_DEFAULT), MD3_OK);
    ASSERT_EQ(md3_check_state_alpha(0xFF, IUI_STATE_DEFAULT), MD3_OK);
    PASS();
}

/* Test Runner */

void run_spec_tests(void)
{
    SECTION_BEGIN("Spec: Component Dimensions");
    test_button_height_40dp();
    test_fab_standard_56dp();
    test_fab_large_96dp();
    test_fab_extended_height_56dp();
    test_icon_button_40dp();
    test_chip_height_32dp();
    test_slider_track_height_4dp();
    test_tab_height_48dp();
    test_search_bar_height_56dp();
    test_side_sheet_dimensions();
    test_carousel_dimensions();
    test_date_picker_day_32dp();
    test_menu_item_height();
    test_switch_dimensions_32dp();
    test_textfield_height_56dp();
    test_button_height_alias();
    test_appbar_small_height_64dp();
    test_appbar_medium_height_112dp();
    test_appbar_large_height_152dp();
    test_appbar_icon_size_24dp();
    test_appbar_padding_16dp();
    SECTION_END();

    SECTION_BEGIN("Spec: Touch Targets (48dp min)");
    test_touch_target_slider_48dp();
    test_touch_target_icon_button_48dp();
    test_touch_target_chip_48dp();
    test_touch_target_tab_48dp();
    test_touch_target_date_nav_48dp();
    test_touch_target_switch_48dp();
    test_touch_target_appbar_48dp();
    SECTION_END();

    SECTION_BEGIN("Spec: State Layer Opacity");
    test_state_layer_hover_8_percent();
    test_state_layer_focus_12_percent();
    test_state_layer_pressed_12_percent();
    test_state_layer_dragged_16_percent();
    test_state_layer_disabled_38_percent();
    SECTION_END();

    SECTION_BEGIN("Spec: Duration Tokens (Short)");
    test_duration_short_1_50ms();
    test_duration_short_2_100ms();
    test_duration_short_3_150ms();
    test_duration_short_4_200ms();
    SECTION_END();

    SECTION_BEGIN("Spec: Duration Tokens (Medium)");
    test_duration_medium_1_250ms();
    test_duration_medium_2_300ms();
    test_duration_medium_3_350ms();
    test_duration_medium_4_400ms();
    SECTION_END();

    SECTION_BEGIN("Spec: Duration Tokens (Long)");
    test_duration_long_1_450ms();
    test_duration_long_2_500ms();
    SECTION_END();

    SECTION_BEGIN("Spec: Easing Functions");
    test_easing_standard_bounds();
    test_easing_emphasized_midpoint();
    SECTION_END();

    SECTION_BEGIN("Spec: WCAG Luminance");
    test_relative_luminance_black();
    test_relative_luminance_white();
    test_relative_luminance_gray();
    SECTION_END();

    SECTION_BEGIN("Spec: WCAG Contrast Ratio");
    test_contrast_ratio_black_white();
    test_contrast_ratio_same_color();
    test_contrast_ratio_order_independent();
    SECTION_END();

    SECTION_BEGIN("Spec: WCAG Thresholds");
    test_wcag_aa_normal_threshold();
    test_wcag_aa_large_threshold();
    test_wcag_aaa_normal_threshold();
    SECTION_END();

    SECTION_BEGIN("Spec: Theme Contrast (Light)");
    test_light_theme_surface_contrast();
    test_light_theme_primary_contrast();
    test_light_theme_error_contrast();
    test_theme_validate_light();
    SECTION_END();

    SECTION_BEGIN("Spec: Theme Contrast (Dark)");
    test_dark_theme_surface_contrast();
    test_dark_theme_primary_contrast();
    test_theme_validate_dark();
    test_theme_validate_null();
    SECTION_END();

    SECTION_BEGIN("Spec: Screen Reader Accessibility");
    test_a11y_role_names();
    test_a11y_state_descriptions();
    test_a11y_make_hint();
    test_a11y_make_slider_hint();
    test_a11y_make_set_hint();
    test_a11y_describe_button();
    test_a11y_describe_checkbox();
    test_a11y_describe_slider();
    test_a11y_describe_radio();
    test_a11y_callbacks_disabled_by_default();
    test_a11y_callbacks_enable_disable();
    test_a11y_push_pop();
    test_accessibility_functions();
    test_wcag_contrast_functions();
    SECTION_END();

    /* MD3 Validation Tests - always run (header provides stubs when disabled)
     */
    SECTION_BEGIN("Spec: MD3 Validation API");
    test_md3_validate_button();
    test_md3_validate_fab();
    test_md3_validate_chip();
    test_md3_validate_textfield();
    test_md3_validate_touch_target();
    test_md3_validate_grid_align();
    test_md3_validate_rounding();
    test_md3_validate_boundaries();
    test_md3_validate_scale_factors();
    test_md3_validate_edge_cases();
    test_md3_validate_bitmask();
    test_md3_validate_state_alpha();
    SECTION_END();

    /* DSL-generated MD3 component validation tests */
    SECTION_BEGIN("Spec: MD3 Generated Validators");
    run_md3_gen_tests();
    SECTION_END();
}
