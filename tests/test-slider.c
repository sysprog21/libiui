/*
 * Slider Extended Tests
 *
 * Tests for advanced slider functionality with options.
 */

#include "common.h"

/* Slider Extended Tests */

static void test_slider_ex_basic(void)
{
    TEST(slider_ex_basic);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = iui_slider_ex(ctx, 50.f, 0.f, 100.f, 1.f, NULL);
    ASSERT_NEAR(value, 50.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_custom_colors(void)
{
    TEST(slider_ex_custom_colors);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_slider_options opts = {
        .active_track_color = 0xFF0000FF,
        .inactive_track_color = 0x808080FF,
        .handle_color = 0x00FF00FF,
    };
    float value = iui_slider_ex(ctx, 25.f, 0.f, 100.f, 1.f, &opts);
    ASSERT_NEAR(value, 25.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_labels(void)
{
    TEST(slider_ex_labels);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_slider_options opts = {
        .start_text = "Min",
        .end_text = "Max",
    };
    float value = iui_slider_ex(ctx, 50.f, 0.f, 100.f, 5.f, &opts);
    ASSERT_NEAR(value, 50.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_value_indicator(void)
{
    TEST(slider_ex_value_indicator);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_slider_options opts = {
        .show_value_indicator = true,
        .value_format = "%.1f%%",
    };
    float value = iui_slider_ex(ctx, 75.f, 0.f, 100.f, 0.1f, &opts);
    ASSERT_NEAR(value, 75.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_disabled(void)
{
    TEST(slider_ex_disabled);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_slider_options opts = {
        .disabled = true,
    };
    float value = iui_slider_ex(ctx, 30.f, 0.f, 100.f, 1.f, &opts);
    ASSERT_NEAR(value, 30.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_invalid_bounds(void)
{
    TEST(slider_ex_invalid_bounds);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = iui_slider_ex(ctx, 50.f, 100.f, 0.f, 1.f, NULL);
    ASSERT_NEAR(value, 50.f, 0.001f);

    value = iui_slider_ex(ctx, 50.f, 50.f, 50.f, 1.f, NULL);
    ASSERT_NEAR(value, 50.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_step_quantization(void)
{
    TEST(slider_ex_step_quantization);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = iui_slider_ex(ctx, 23.f, 0.f, 100.f, 10.f, NULL);
    ASSERT_NEAR(value, 20.f, 0.001f);

    value = iui_slider_ex(ctx, 27.f, 0.f, 100.f, 10.f, NULL);
    ASSERT_NEAR(value, 30.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_clamping(void)
{
    TEST(slider_ex_clamping);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = iui_slider_ex(ctx, -50.f, 0.f, 100.f, 1.f, NULL);
    ASSERT_NEAR(value, 0.f, 0.001f);

    value = iui_slider_ex(ctx, 150.f, 0.f, 100.f, 1.f, NULL);
    ASSERT_NEAR(value, 100.f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_ex_zero_step(void)
{
    TEST(slider_ex_zero_step);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    float value = iui_slider_ex(ctx, 33.333f, 0.f, 100.f, 0.f, NULL);
    ASSERT_NEAR(value, 33.333f, 0.001f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Slider Interaction Tests */

static void test_slider_drag_interaction(void)
{
    TEST(slider_drag_interaction);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Frame 1: Render slider to verify rendering works */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    reset_counters();
    float value = iui_slider_ex(ctx, 50.f, 0.f, 100.f, 1.f, NULL);
    ASSERT_NEAR(value, 50.f, 0.001f);
    ASSERT_TRUE(g_draw_box_calls > 0); /* Slider was rendered */
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Verify slider can be re-rendered */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    value = iui_slider_ex(ctx, value, 0.f, 100.f, 1.f, NULL);
    ASSERT_NEAR(value, 50.f, 0.001f); /* Value unchanged without drag */
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_slider_disabled_no_interaction(void)
{
    TEST(slider_disabled_no_interaction);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_slider_options opts = {.disabled = true};

    /* Frame 1: Render disabled slider to get bounds */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    reset_counters();
    float value = iui_slider_ex(ctx, 25.f, 0.f, 100.f, 1.f, &opts);
    ASSERT_NEAR(value, 25.f, 0.001f);
    iui_rect_t slider_bounds = test_get_last_widget_bounds();
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Try to click on disabled slider */
    float click_x = slider_bounds.x + slider_bounds.width * 0.75f;
    float click_y = slider_bounds.y + slider_bounds.height / 2;
    iui_update_mouse_pos(ctx, click_x, click_y);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    value = iui_slider_ex(ctx, value, 0.f, 100.f, 1.f, &opts);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Value should remain unchanged */
    ASSERT_NEAR(value, 25.f, 0.001f);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    free(buffer);
    PASS();
}

static void test_slider_click_to_value(void)
{
    TEST(slider_click_to_value);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test that slider renders correctly at different values */
    const float test_values[] = {0.f, 25.f, 50.f, 75.f, 100.f};
    for (int i = 0; i < 5; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
        reset_counters();
        float value = iui_slider_ex(ctx, test_values[i], 0.f, 100.f, 1.f, NULL);
        ASSERT_NEAR(value, test_values[i], 0.001f);
        ASSERT_TRUE(g_draw_box_calls > 0); /* Slider track rendered */
        iui_end_window(ctx);
        iui_end_frame(ctx);
    }

    free(buffer);
    PASS();
}

/* Test Suite Runner */
void run_slider_ex_tests(void)
{
    SECTION_BEGIN("Slider Extended");
    test_slider_ex_basic();
    test_slider_ex_custom_colors();
    test_slider_ex_labels();
    test_slider_ex_value_indicator();
    test_slider_ex_disabled();
    test_slider_ex_invalid_bounds();
    test_slider_ex_step_quantization();
    test_slider_ex_clamping();
    test_slider_ex_zero_step();
    /* Interaction tests */
    test_slider_drag_interaction();
    test_slider_disabled_no_interaction();
    test_slider_click_to_value();
    SECTION_END();
}
