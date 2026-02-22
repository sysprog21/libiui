/*
 * Demonstration Tests
 *
 * Widget showcase and layout examples.
 */

#include "common.h"

/* Demonstration Tests */

static void demo_basic_widgets(iui_context *ctx)
{
    TEST(demo_basic_widgets);
    reset_counters();

    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "Basic Widgets", 100, 100, 400, 600,
                     IUI_WINDOW_RESIZABLE);

    iui_text(ctx, IUI_ALIGN_LEFT, "Left aligned");
    iui_text(ctx, IUI_ALIGN_CENTER, "Centered");
    iui_text(ctx, IUI_ALIGN_RIGHT, "Right aligned");
    iui_newline(ctx);

    iui_divider(ctx);

    static bool toggle1 = false, toggle2 = true;
    iui_switch(ctx, "Option A", &toggle1, NULL, NULL);
    iui_switch(ctx, "Option B", &toggle2, NULL, NULL);

    const char *options[] = {"One", "Two", "Three"};
    static uint32_t selected = 1;
    iui_segmented(ctx, options, 3, &selected);

    static float slider_val = 50.f;
    iui_slider(ctx, "Volume", 0.f, 100.f, 1.f, &slider_val, "%.0f%%");

    iui_button(ctx, "Click Me", IUI_ALIGN_CENTER);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls > 0);
    ASSERT_TRUE(g_draw_text_calls > 0);
    PASS();
}

static void demo_new_widgets(iui_context *ctx)
{
    TEST(demo_new_widgets);
    reset_counters();

    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "New Widgets", 100, 100, 400, 400, 0);

    static bool check1 = false, check2 = true, check3 = false;
    iui_checkbox(ctx, "Enable feature", &check1);
    iui_checkbox(ctx, "Dark mode", &check2);
    iui_checkbox(ctx, "Notifications", &check3);

    iui_divider(ctx);

    static int radio_group = 0;
    iui_radio(ctx, "Small", &radio_group, 0);
    iui_radio(ctx, "Medium", &radio_group, 1);
    iui_radio(ctx, "Large", &radio_group, 2);

    iui_divider(ctx);

    static char edit_buffer[64] = "Hello World";
    static size_t cursor = 11;
    iui_textfield(ctx, edit_buffer, sizeof(edit_buffer), &cursor, NULL);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls > 0);
    PASS();
}

static void demo_grid_layout(iui_context *ctx)
{
    TEST(demo_grid_layout);
    reset_counters();

    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "Grid Test", 100, 100, 300, 300, 0);

    iui_grid_begin(ctx, 3, 80.f, 40.f, 5.f);

    const char *labels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
    for (int i = 0; i < 9; i++) {
        iui_button(ctx, labels[i], IUI_ALIGN_CENTER);
        iui_grid_next(ctx);
    }

    iui_grid_end(ctx);
    iui_text(ctx, IUI_ALIGN_CENTER, "After grid");

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls >= 9);
    PASS();
}

static void demo_row_layout(iui_context *ctx)
{
    TEST(demo_box_row_layout);
    reset_counters();

    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "Box Layout", 100, 100, 400, 300, 0);

    iui_sizing_t row1[] = {IUI_FIXED(100), IUI_GROW(1), IUI_GROW(1)};
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .sizes = row1});
    iui_box_next(ctx);
    iui_text(ctx, IUI_ALIGN_LEFT, "Label:");
    iui_box_next(ctx);
    iui_button(ctx, "OK", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "Cancel", IUI_ALIGN_CENTER);
    iui_box_end(ctx);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 4, .cross = 30.f});
    iui_box_next(ctx);
    iui_button(ctx, "A", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "B", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "C", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "D", IUI_ALIGN_CENTER);
    iui_box_end(ctx);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2});
    iui_box_next(ctx);
    iui_text(ctx, IUI_ALIGN_LEFT, "Left half");
    iui_box_next(ctx);
    iui_text(ctx, IUI_ALIGN_RIGHT, "Right half");
    iui_box_end(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls > 0);
    PASS();
}

static void demo_box_layout(iui_context *ctx)
{
    TEST(demo_box_layout);
    reset_counters();

    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "Box Layout 2", 100, 100, 400, 400, 0);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .gap = 4.f});
    iui_box_next(ctx);
    iui_button(ctx, "A", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "B", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "C", IUI_ALIGN_CENTER);
    iui_box_end(ctx);

    iui_sizing_t mixed[] = {IUI_GROW(1), IUI_FIXED(100), IUI_GROW(2)};
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 3, .sizes = mixed, .gap = 4.f});
    iui_box_next(ctx);
    iui_button(ctx, "Flex 1", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "Fixed", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "Flex 2", IUI_ALIGN_CENTER);
    iui_box_end(ctx);

    iui_sizing_t col[] = {IUI_FIXED(30), IUI_GROW(1), IUI_FIXED(30)};
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .direction = IUI_DIR_COLUMN,
                           .child_count = 3,
                           .sizes = col,
                           .cross = 120.f,
                           .gap = 4.f,
                       });
    iui_box_next(ctx);
    iui_button(ctx, "Top", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "Middle", IUI_ALIGN_CENTER);
    iui_box_next(ctx);
    iui_button(ctx, "Bottom", IUI_ALIGN_CENTER);
    iui_box_end(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls > 0);
    PASS();
}

static void demo_id_stack(iui_context *ctx)
{
    TEST(demo_id_stack);
    reset_counters();

    iui_update_mouse_pos(ctx, 150.f, 150.f);
    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "ID Stack", 100, 100, 300, 400, 0);

    for (int i = 0; i < 5; i++) {
        iui_push_id(ctx, &i, sizeof(i));
        iui_button(ctx, "Item", IUI_ALIGN_LEFT);
        iui_newline(ctx);
        iui_pop_id(ctx);
    }

    iui_divider(ctx);

    for (int row = 0; row < 2; row++) {
        iui_push_id(ctx, &row, sizeof(row));
        for (int col = 0; col < 3; col++) {
            iui_push_id(ctx, &col, sizeof(col));
            iui_button(ctx, "X", IUI_ALIGN_LEFT);
            iui_pop_id(ctx);
        }
        iui_newline(ctx);
        iui_pop_id(ctx);
    }

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(g_draw_box_calls > 0);
    PASS();
}

static void demo_theme_switching(iui_context *ctx)
{
    TEST(demo_theme_switching);

    const iui_theme_t *light = iui_theme_light();
    const iui_theme_t *dark = iui_theme_dark();

    ASSERT_NOT_NULL(light);
    ASSERT_NOT_NULL(dark);

    iui_set_theme(ctx, dark);
    const iui_theme_t *current = iui_get_theme(ctx);
    ASSERT_EQ(current->surface, dark->surface);

    reset_counters();
    iui_begin_frame(ctx, 1.f / 60.f);
    iui_begin_window(ctx, "Dark Theme", 100, 100, 300, 200, 0);
    iui_text(ctx, IUI_ALIGN_CENTER, "Dark mode");
    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_set_theme(ctx, light);
    current = iui_get_theme(ctx);
    ASSERT_EQ(current->surface, light->surface);

    iui_set_theme(ctx, NULL);
    current = iui_get_theme(ctx);
    ASSERT_EQ(current->surface, light->surface);

    PASS();
}

/* Test Suite Runner */


void run_demo_tests(iui_context *ctx)
{
    SECTION_BEGIN("Demonstration");
    demo_basic_widgets(ctx);
    demo_new_widgets(ctx);
    demo_grid_layout(ctx);
    demo_row_layout(ctx);
    demo_box_layout(ctx);
    demo_id_stack(ctx);
    demo_theme_switching(ctx);
    SECTION_END();
}
