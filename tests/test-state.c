/*
 * State Machine & Animation Tests
 *
 * Tests for state machine integrity and animation timing.
 */

#include "common.h"

/* Animation Tests */

static void test_animation_delta_time_zero(void)
{
    TEST(animation_delta_time_zero);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_animation_delta_time_negative(void)
{
    TEST(animation_delta_time_negative);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, -1.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_animation_delta_time_large(void)
{
    TEST(animation_delta_time_large);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1000.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_hover_animation_sequence(void)
{
    TEST(hover_animation_sequence);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_update_mouse_pos(ctx, 0.0f, 0.0f);

    for (int i = 0; i < 20; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
        iui_button(ctx, "Test", IUI_ALIGN_CENTER);
        iui_end_window(ctx);
        iui_end_frame(ctx);
    }

    iui_update_mouse_pos(ctx, 200.0f, 150.0f);

    for (int i = 0; i < 20; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
        iui_button(ctx, "Test", IUI_ALIGN_CENTER);
        iui_end_window(ctx);
        iui_end_frame(ctx);
    }

    free(buffer);
    PASS();
}

/* Motion System Tests */

static void test_motion_system(void)
{
    TEST(motion_system);

    /* Test basic easing */
    float result = iui_ease(0.5f, IUI_EASING_STANDARD);
    ASSERT_TRUE(result >= 0.0f && result <= 1.0f);

    /* Test motion apply */
    result = iui_motion_apply(0.5f, true, iui_motion_get_standard());
    ASSERT_TRUE(result >= 0.0f && result <= 1.0f);

    /* Test motion progress */
    result = iui_motion_progress(0.1f, true, iui_motion_get_standard());
    ASSERT_TRUE(result >= 0.0f && result <= 1.0f);

    /* Test motion duration */
    float duration = iui_motion_get_duration(true, iui_motion_get_standard());
    ASSERT_NEAR(duration, IUI_DURATION_SHORT_4, 0.001f);

    PASS();
}

static void test_motion_presets(void)
{
    TEST(motion_presets);

    float result1 = iui_ease_standard(0.5f);
    ASSERT_TRUE(result1 >= 0.0f && result1 <= 1.0f);

    float result2 = iui_ease_emphasized_decel(0.5f);
    ASSERT_TRUE(result2 >= 0.0f && result2 <= 1.0f);

    PASS();
}

/* State Machine Integrity Tests */


static void test_unclosed_layout_at_window_end(void)
{
    TEST(unclosed_layout_at_window_end);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_grid_begin(ctx, 2, 50.0f, 30.0f, 5.0f);
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 2, .cross = 30.0f, .gap = 4.0f});

    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test2", 0, 0, 400, 300, 0);
    iui_button(ctx, "Should Work", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_nested_window_prevention(void)
{
    TEST(nested_window_prevention);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Outer", 0, 0, 400, 300, 0);

    iui_begin_window(ctx, "Inner", 50, 50, 200, 200, 0);
    iui_end_window(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_frame_without_windows(void)
{
    TEST(frame_without_windows);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_end_frame(ctx);

    for (int i = 0; i < 100; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_end_frame(ctx);
    }

    free(buffer);
    PASS();
}

static void test_window_reuse_across_frames(void)
{
    TEST(window_reuse_across_frames);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    for (int i = 0; i < 10; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_begin_window(ctx, "Persistent", 10, 10, 200, 200, 0);
        iui_text(ctx, IUI_ALIGN_LEFT, "Frame %d", i);
        iui_end_window(ctx);
        iui_end_frame(ctx);
    }

    free(buffer);
    PASS();
}

static void test_layout_contamination_between_windows(void)
{
    TEST(layout_contamination_between_windows);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);

    iui_begin_window(ctx, "Window1", 0, 0, 200, 200, 0);
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .cross = 30.0f});
    iui_box_next(ctx);
    iui_box_end(ctx);
    iui_end_window(ctx);

    iui_begin_window(ctx, "Window2", 250, 0, 200, 200, 0);
    iui_rect_t layout = iui_get_layout_rect(ctx);
    ASSERT_TRUE(layout.width > 0);
    iui_button(ctx, "Clean State", IUI_ALIGN_CENTER);
    iui_end_window(ctx);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test Suite Runners */

void run_animation_tests(void)
{
    SECTION_BEGIN("Animation & State");
    test_animation_delta_time_zero();
    test_animation_delta_time_negative();
    test_animation_delta_time_large();
    test_hover_animation_sequence();
    test_motion_system();
    test_motion_presets();
    SECTION_END();
}

void run_state_machine_tests(void)
{
    SECTION_BEGIN("State Machine Integrity");
    test_unclosed_layout_at_window_end();
    test_nested_window_prevention();
    test_frame_without_windows();
    test_window_reuse_across_frames();
    test_layout_contamination_between_windows();
    SECTION_END();
}
