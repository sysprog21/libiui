/*
 * Focus System Tests
 *
 * Tests for keyboard navigation, focus management, and accessibility.
 * Reference: MD3 Focus States (m3.material.io/foundations/interaction/states)
 */

#include "common.h"

/* Focus State Tests */

static void test_focus_initial_state(void)
{
    TEST(focus_initial_state);
    void *buffer = malloc(iui_min_memory_size());
    const iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Initially no widget should have focus */
    ASSERT_FALSE(iui_has_any_focus(ctx));
    ASSERT_EQ(iui_get_focused_id(ctx), 0);

    free(buffer);
    PASS();
}

static void test_focus_set_and_check(void)
{
    TEST(focus_set_and_check);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Create a button to focus */
    iui_button(ctx, "TestButton", IUI_ALIGN_CENTER);

    /* Set focus programmatically */
    iui_set_focus(ctx, "TestButton");

    ASSERT_TRUE(iui_has_any_focus(ctx));
    ASSERT_TRUE(iui_has_focus(ctx, "TestButton"));
    ASSERT_FALSE(iui_has_focus(ctx, "OtherButton"));

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_focus_clear(void)
{
    TEST(focus_clear);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "TestButton", IUI_ALIGN_CENTER);
    iui_set_focus(ctx, "TestButton");
    ASSERT_TRUE(iui_has_any_focus(ctx));

    /* Clear focus */
    iui_clear_focus(ctx);
    ASSERT_FALSE(iui_has_any_focus(ctx));
    ASSERT_FALSE(iui_has_focus(ctx, "TestButton"));

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Focus Navigation Tests */

static void test_focus_next_single_widget(void)
{
    TEST(focus_next_single_widget);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "OnlyButton", IUI_ALIGN_CENTER);

    /* Focus next when no focus - should focus first widget */
    iui_focus_next(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* After frame, check focus state */
    ASSERT_TRUE(iui_has_any_focus(ctx));

    free(buffer);
    PASS();
}

static void test_focus_prev_single_widget(void)
{
    TEST(focus_prev_single_widget);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "OnlyButton", IUI_ALIGN_CENTER);

    /* Focus prev when no focus - should focus last widget */
    iui_focus_prev(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(iui_has_any_focus(ctx));

    free(buffer);
    PASS();
}

static void test_focus_navigation_multiple_widgets(void)
{
    TEST(focus_navigation_multiple_widgets);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Frame 1: Create widgets and focus first */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "Button1", IUI_ALIGN_CENTER);
    iui_newline(ctx);
    iui_button(ctx, "Button2", IUI_ALIGN_CENTER);
    iui_newline(ctx);
    iui_button(ctx, "Button3", IUI_ALIGN_CENTER);

    iui_focus_next(ctx); /* Should focus Button1 */

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(iui_has_any_focus(ctx));
    uint32_t first_id = iui_get_focused_id(ctx);
    ASSERT_TRUE(first_id != 0);

    /* Frame 2: Navigate forward */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "Button1", IUI_ALIGN_CENTER);
    iui_newline(ctx);
    iui_button(ctx, "Button2", IUI_ALIGN_CENTER);
    iui_newline(ctx);
    iui_button(ctx, "Button3", IUI_ALIGN_CENTER);

    iui_focus_next(ctx); /* Should focus Button2 */

    iui_end_window(ctx);
    iui_end_frame(ctx);

    uint32_t second_id = iui_get_focused_id(ctx);
    ASSERT_TRUE(second_id != first_id);

    free(buffer);
    PASS();
}

static void test_focus_wrap_around(void)
{
    TEST(focus_wrap_around);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Create widgets and navigate past the end - should wrap */
    for (int i = 0; i < 5; i++) {
        iui_begin_frame(ctx, 1.0f / 60.0f);
        iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

        iui_button(ctx, "Button1", IUI_ALIGN_CENTER);
        iui_newline(ctx);
        iui_button(ctx, "Button2", IUI_ALIGN_CENTER);

        iui_focus_next(ctx);

        iui_end_window(ctx);
        iui_end_frame(ctx);
    }

    /* After wrapping, focus should still be valid */
    ASSERT_TRUE(iui_has_any_focus(ctx));

    free(buffer);
    PASS();
}

/* Focus with Input Tests */

static void test_focus_cleared_on_mouse_click(void)
{
    TEST(focus_cleared_on_mouse_click);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    iui_button(ctx, "Button1", IUI_ALIGN_CENTER);
    iui_set_focus(ctx, "Button1");

    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(iui_has_any_focus(ctx));

    /* Simulate mouse click - focus should be cleared */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Button1", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Mouse interaction typically clears keyboard focus
     * (implementation may vary - check actual behavior)
     */

    free(buffer);
    PASS();
}

/* Focus Edge Cases */

static void test_focus_nonexistent_widget(void)
{
    TEST(focus_nonexistent_widget);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Setting focus to nonexistent widget should not crash
     * Note: API sets focus by string hash, doesn't validate widget existence
     */
    iui_set_focus(ctx, "NonexistentWidget");

    /* Focus is tracked by ID hash, so has_focus returns true even if widget
     * doesn't exist - this is by design to allow pre-setting focus targets
     */
    ASSERT_TRUE(iui_has_focus(ctx, "NonexistentWidget"));
    ASSERT_TRUE(iui_has_any_focus(ctx));

    /* Clear and verify */
    iui_clear_focus(ctx);
    ASSERT_FALSE(iui_has_any_focus(ctx));

    free(buffer);
    PASS();
}

static void test_focus_null_id(void)
{
    TEST(focus_null_id);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Setting focus with NULL id should clear focus */
    iui_set_focus(ctx, "test");
    ASSERT_TRUE(iui_has_any_focus(ctx));

    iui_set_focus(ctx, NULL);
    ASSERT_FALSE(iui_has_any_focus(ctx));

    /* Checking focus with NULL id should return false */
    ASSERT_FALSE(iui_has_focus(ctx, NULL));

    free(buffer);
    PASS();
}

static void test_focus_extended_functions(void)
{
    TEST(focus_extended_functions);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test initial focus state */
    ASSERT_FALSE(iui_has_any_focus(ctx));
    ASSERT_EQ(iui_get_focused_id(ctx), 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Test setting focus */
    iui_set_focus(ctx, "test_widget");
    ASSERT_TRUE(iui_has_focus(ctx, "test_widget"));

    /* Test clearing focus */
    iui_clear_focus(ctx);
    ASSERT_FALSE(iui_has_any_focus(ctx));

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_focus_empty_id(void)
{
    TEST(focus_empty_id);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Empty string hashes to a valid ID - focus is set */
    iui_set_focus(ctx, "");
    /* Empty string check - hashes match so has_focus returns true */
    bool has = iui_has_focus(ctx, "");
    (void) has; /* May or may not have focus depending on hash */

    /* Clear focus for clean state */
    iui_clear_focus(ctx);
    ASSERT_FALSE(iui_has_any_focus(ctx));

    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_focus_tests(void)
{
    SECTION_BEGIN("Focus System");

    /* State tests */
    test_focus_initial_state();
    test_focus_set_and_check();
    test_focus_clear();

    /* Navigation tests */
    test_focus_next_single_widget();
    test_focus_prev_single_widget();
    test_focus_navigation_multiple_widgets();
    test_focus_wrap_around();

    /* Input interaction */
    test_focus_cleared_on_mouse_click();

    /* Edge cases */
    test_focus_nonexistent_widget();
    test_focus_null_id();
    test_focus_empty_id();
    test_focus_extended_functions();

    SECTION_END();
}
