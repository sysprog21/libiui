/*
 * Widget Edge Case Tests
 *
 * Tests for widget boundary conditions and error handling.
 */

#include "common.h"

/* Slider Edge Cases */

static void test_slider_min_equals_max(void)
{
    TEST(slider_min_equals_max);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = 50.0f;
    iui_slider(ctx, "Test", 50.0f, 50.0f, 1.0f, &value, "%.0f");

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_component_state_functions(void)
{
    TEST(component_state_functions);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_rect_t bounds = {10.0f, 10.0f, 100.0f, 30.0f};
    iui_state_t state = iui_get_component_state(ctx, bounds, false);
    ASSERT_TRUE(state == IUI_STATE_DEFAULT || state == IUI_STATE_HOVERED);

    /* Test state color function */
    uint32_t color = iui_get_state_color(ctx, IUI_STATE_HOVERED, 0xFF0000FF,
                                         0x00FF00FF, 0x0000FFFF);
    ASSERT_TRUE(color != 0);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_min_greater_than_max(void)
{
    TEST(slider_min_greater_than_max);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = 50.0f;
    iui_slider(ctx, "Test", 100.0f, 0.0f, 1.0f, &value, "%.0f");

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_step_larger_than_range(void)
{
    TEST(slider_step_larger_than_range);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = 5.0f;
    iui_slider(ctx, "Test", 0.0f, 10.0f, 100.0f, &value, "%.0f");

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Segmented Control Edge Cases */

static void test_segmented_empty_entries(void)
{
    TEST(segmented_empty_entries);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    uint32_t selected = 0;
    iui_segmented(ctx, NULL, 0, &selected);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_segmented_selected_out_of_range(void)
{
    TEST(segmented_selected_out_of_range);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    const char *entries[] = {"A", "B", "C"};
    uint32_t selected = 100;
    iui_segmented(ctx, entries, 3, &selected);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Edit Field Edge Cases */

static void test_edit_cursor_beyond_length(void)
{
    TEST(edit_cursor_beyond_length);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 100;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_edit_empty_buffer(void)
{
    TEST(edit_empty_buffer);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    char text_buf[32] = "";
    size_t cursor = 0;
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_edit_buffer_size_one(void)
{
    TEST(edit_buffer_size_one);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);

    char text_buf[1] = "";
    size_t cursor = 0;
    iui_textfield(ctx, text_buf, 1, &cursor, NULL);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    iui_update_char(ctx, 'A');

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, 1, &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(text_buf[0], '\0');

    free(buffer);
    PASS();
}

/* Widgets Outside Window */

static void test_widgets_outside_window(void)
{
    TEST(widgets_outside_window);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    reset_counters();
    iui_begin_frame(ctx, 1.0f / 60.0f);

    iui_text(ctx, IUI_ALIGN_LEFT, "Test");
    iui_newline(ctx);
    iui_divider(ctx);

    static bool b = false;
    iui_switch(ctx, "Toggle", &b, NULL, NULL);
    iui_checkbox(ctx, "Check", &b);

    static int r = 0;
    iui_radio(ctx, "Radio", &r, 0);

    static float v = 0.0f;
    iui_slider(ctx, "Slider", 0.0f, 100.0f, 1.0f, &v, "%.0f");

    iui_button(ctx, "Button", IUI_ALIGN_CENTER);

    static char buf[32] = "";
    static size_t cursor = 0;
    iui_textfield(ctx, buf, sizeof(buf), &cursor, NULL);

    const char *entries[] = {"A", "B"};
    static uint32_t sel = 0;
    iui_segmented(ctx, entries, 2, &sel);

    iui_grid_begin(ctx, 2, 50.0f, 30.0f, 5.0f);
    iui_grid_next(ctx);
    iui_grid_end(ctx);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .cross = 30.0f});
    iui_box_next(ctx);
    iui_box_end(ctx);

    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 2, .cross = 30.0f, .gap = 4.0f});
    iui_box_next(ctx);
    iui_box_end(ctx);

    iui_end_frame(ctx);

    ASSERT_EQ(g_draw_box_calls, 0);
    ASSERT_EQ(g_draw_text_calls, 0);

    free(buffer);
    PASS();
}

/* Test Suite Runner */
void run_widget_tests(void)
{
    SECTION_BEGIN("Widget Edge Cases");
    test_component_state_functions();
    test_slider_min_equals_max();
    test_slider_min_greater_than_max();
    test_slider_step_larger_than_range();
    test_segmented_empty_entries();
    test_segmented_selected_out_of_range();
    test_edit_cursor_beyond_length();
    test_edit_empty_buffer();
    test_edit_buffer_size_one();
    test_widgets_outside_window();
    SECTION_END();
}
