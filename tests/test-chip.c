/*
 * Chip Component Tests
 *
 * Tests for assist, filter, input, and suggestion chips.
 */

#include "common.h"

/* Chip Component Tests */

static void test_chip_assist_basic(void)
{
    TEST(chip_assist_basic);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    bool clicked = iui_chip_assist(ctx, "Add event", NULL);
    ASSERT_FALSE(clicked);

    clicked = iui_chip_assist(ctx, "Settings", "settings");
    ASSERT_FALSE(clicked);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_filter_states(void)
{
    TEST(chip_filter_states);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    bool selected_false = false;
    bool selected_true = true;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_chip_filter(ctx, "Unselected", &selected_false);
    ASSERT_FALSE(selected_false);

    iui_chip_filter(ctx, "Selected", &selected_true);
    ASSERT_TRUE(selected_true);

    bool clicked = iui_chip_filter(ctx, "NoPtr", NULL);
    ASSERT_FALSE(clicked);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_input_removal(void)
{
    TEST(chip_input_removal);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    bool removed = false;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_chip_input(ctx, "user@example.com", &removed);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_FALSE(removed);

    free(buffer);
    PASS();
}

static void test_chip_suggestion_basic(void)
{
    TEST(chip_suggestion_basic);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    bool clicked = iui_chip_suggestion(ctx, "Try this");
    ASSERT_FALSE(clicked);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_null_context(void)
{
    TEST(chip_null_context);

    bool clicked = iui_chip_assist(NULL, "Test", NULL);
    ASSERT_FALSE(clicked);

    clicked = iui_chip_filter(NULL, "Test", NULL);
    ASSERT_FALSE(clicked);

    bool removed = false;
    clicked = iui_chip_input(NULL, "Test", &removed);
    ASSERT_FALSE(clicked);

    clicked = iui_chip_suggestion(NULL, "Test");
    ASSERT_FALSE(clicked);

    PASS();
}

static void test_chip_null_label(void)
{
    TEST(chip_null_label);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    bool clicked = iui_chip_assist(ctx, NULL, NULL);
    ASSERT_FALSE(clicked);

    clicked = iui_chip_suggestion(ctx, NULL);
    ASSERT_FALSE(clicked);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_outside_window(void)
{
    TEST(chip_outside_window);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    bool clicked = iui_chip_assist(ctx, "Test", NULL);
    ASSERT_FALSE(clicked);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_in_box_layout(void)
{
    TEST(chip_in_box_layout);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_box_begin(
        ctx, &(iui_box_config_t) {.child_count = 3, .cross = 40, .gap = 8});
    iui_box_next(ctx);
    iui_chip_assist(ctx, "One", NULL);
    iui_box_next(ctx);
    iui_chip_assist(ctx, "Two", NULL);
    iui_box_next(ctx);
    iui_chip_assist(ctx, "Three", NULL);
    iui_box_end(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_chip_outline_fallback(void)
{
    TEST(chip_outline_fallback);

    /* Test chips render correctly in light theme without crashing */
    void *buffer1 = malloc(iui_min_memory_size());
    iui_context *ctx1 = create_test_context(buffer1, false);
    ASSERT_NOT_NULL(ctx1);

    iui_begin_frame(ctx1, 1.0f / 60.0f);
    iui_begin_window(ctx1, "Test", 0, 0, 400, 300, 0);
    bool selected = false;
    iui_chip_filter(ctx1, "Outline", &selected);
    bool removed = false;
    iui_chip_input(ctx1, "Input", &removed);
    iui_end_window(ctx1);
    iui_end_frame(ctx1);

    /* Verify state unchanged without user interaction */
    ASSERT_FALSE(selected);
    ASSERT_FALSE(removed);
    free(buffer1);

    /* Test chips render correctly in dark theme without crashing */
    void *buffer2 = malloc(iui_min_memory_size());
    iui_context *ctx2 = create_test_context(buffer2, true);
    ASSERT_NOT_NULL(ctx2);

    iui_begin_frame(ctx2, 1.0f / 60.0f);
    iui_begin_window(ctx2, "Test", 0, 0, 400, 300, 0);
    selected = false;
    iui_chip_filter(ctx2, "Outline", &selected);
    removed = false;
    iui_chip_input(ctx2, "Input", &removed);
    iui_end_window(ctx2);
    iui_end_frame(ctx2);

    /* Verify state unchanged in dark theme too */
    ASSERT_FALSE(selected);
    ASSERT_FALSE(removed);
    free(buffer2);

    PASS();
}

/* Chip Interaction Tests */

static void test_chip_assist_click(void)
{
    TEST(chip_assist_click);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Frame 1: Render chip to get its bounds */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    reset_counters();
    iui_chip_assist(ctx, "Click me", NULL);
    iui_rect_t chip_bounds = test_get_last_widget_bounds();
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Click on the chip */
    float click_x = chip_bounds.x + chip_bounds.width / 2;
    float click_y = chip_bounds.y + chip_bounds.height / 2;
    iui_update_mouse_pos(ctx, click_x, click_y);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    bool clicked = iui_chip_assist(ctx, "Click me", NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Chip should report click */
    ASSERT_TRUE(clicked);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    free(buffer);
    PASS();
}

static void test_chip_filter_toggle(void)
{
    TEST(chip_filter_toggle);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    bool selected = false;

    /* Frame 1: Render chip to get its bounds */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    reset_counters();
    iui_chip_filter(ctx, "Filter", &selected);
    iui_rect_t chip_bounds = test_get_last_widget_bounds();
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_FALSE(selected);

    /* Frame 2: Click to toggle on */
    float click_x = chip_bounds.x + chip_bounds.width / 2;
    float click_y = chip_bounds.y + chip_bounds.height / 3;
    iui_update_mouse_pos(ctx, click_x, click_y);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_chip_filter(ctx, "Filter", &selected);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Should now be selected */
    ASSERT_TRUE(selected);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    free(buffer);
    PASS();
}

static void test_chip_click_outside(void)
{
    TEST(chip_click_outside);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Frame 1: Render chip to get its bounds */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    reset_counters();
    iui_chip_assist(ctx, "Test", NULL);
    iui_rect_t chip_bounds = test_get_last_widget_bounds();
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Click outside the chip */
    float click_x = chip_bounds.x + chip_bounds.width + 100;
    float click_y = chip_bounds.y + chip_bounds.height + 100;
    iui_update_mouse_pos(ctx, click_x, click_y);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    bool clicked = iui_chip_assist(ctx, "Test", NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Should not be clicked */
    ASSERT_FALSE(clicked);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_chip_tests(void)
{
    SECTION_BEGIN("Chip Component");
    test_chip_assist_basic();
    test_chip_filter_states();
    test_chip_input_removal();
    test_chip_suggestion_basic();
    test_chip_null_context();
    test_chip_null_label();
    test_chip_outside_window();
    test_chip_in_box_layout();
    test_chip_outline_fallback();
    /* Interaction tests */
    test_chip_assist_click();
    test_chip_filter_toggle();
    test_chip_click_outside();
    SECTION_END();
}
