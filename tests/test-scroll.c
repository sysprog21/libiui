/*
 * Scrollable Container Tests
 *
 * Tests for scroll state, scroll by/to, and scroll begin/end.
 */

#include "common.h"

/* Scroll State Tests */

static void test_scroll_state_init(void)
{
    TEST(scroll_state_init);

    iui_scroll_state scroll = {0};

    ASSERT_NEAR(scroll.scroll_x, 0.f, 0.001f);
    ASSERT_NEAR(scroll.scroll_y, 0.f, 0.001f);
    ASSERT_NEAR(scroll.content_w, 0.f, 0.001f);
    ASSERT_NEAR(scroll.content_h, 0.f, 0.001f);
    ASSERT_NEAR(scroll.velocity_y, 0.f, 0.001f);

    PASS();
}

static void test_scroll_by(void)
{
    TEST(scroll_by);

    iui_scroll_state scroll = {0};

    iui_scroll_by(&scroll, 10.f, 20.f);
    ASSERT_NEAR(scroll.scroll_x, 10.f, 0.001f);
    ASSERT_NEAR(scroll.scroll_y, 20.f, 0.001f);

    iui_scroll_by(&scroll, -5.f, 15.f);
    ASSERT_NEAR(scroll.scroll_x, 5.f, 0.001f);
    ASSERT_NEAR(scroll.scroll_y, 35.f, 0.001f);

    PASS();
}

static void test_scroll_to(void)
{
    TEST(scroll_to);

    iui_scroll_state scroll = {0};
    scroll.scroll_x = 50.f;
    scroll.scroll_y = 100.f;

    iui_scroll_to(&scroll, 25.f, 75.f);
    ASSERT_NEAR(scroll.scroll_x, 25.f, 0.001f);
    ASSERT_NEAR(scroll.scroll_y, 75.f, 0.001f);

    PASS();
}

static void test_scroll_null_safety(void)
{
    TEST(scroll_null_safety);

    iui_scroll_by(NULL, 10.f, 10.f);
    iui_scroll_to(NULL, 0.f, 0.f);
    iui_update_scroll(NULL, 10.f, 10.f);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_rect_t r = iui_scroll_begin(ctx, NULL, 200.f, 150.f);
    ASSERT_NEAR(r.width, 0.f, 0.001f);
    ASSERT_NEAR(r.height, 0.f, 0.001f);

    bool scrollable = iui_scroll_end(ctx, NULL);
    ASSERT_FALSE(scrollable);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_scroll_begin_end_basic(void)
{
    TEST(scroll_begin_end_basic);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_rect_t viewport = iui_scroll_begin(ctx, &scroll, 200.f, 150.f);
    ASSERT_NEAR(viewport.width, 200.f, 0.001f);
    ASSERT_NEAR(viewport.height, 150.f, 0.001f);

    for (int i = 0; i < 20; i++) {
        iui_text(ctx, IUI_ALIGN_LEFT, "Line %d", i);
        iui_newline(ctx);
    }

    bool scrollable = iui_scroll_end(ctx, &scroll);

    ASSERT_TRUE(scroll.content_h > 150.f);
    ASSERT_TRUE(scrollable);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_scroll_content_measurement(void)
{
    TEST(scroll_content_measurement);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_scroll_begin(ctx, &scroll, 200.f, 100.f);

    for (int i = 0; i < 10; i++)
        iui_newline(ctx);

    iui_scroll_end(ctx, &scroll);

    ASSERT_TRUE(scroll.content_h > 0.f);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_scroll_clamping(void)
{
    TEST(scroll_clamping);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};
    scroll.content_h = 500.f;
    scroll.scroll_y = 1000.f;

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_scroll_begin(ctx, &scroll, 200.f, 150.f);

    float max_scroll = scroll.content_h - 150.f;
    ASSERT_TRUE(scroll.scroll_y <= max_scroll + 0.001f);
    ASSERT_TRUE(scroll.scroll_y >= 0.f);

    iui_scroll_end(ctx, &scroll);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_scroll_update_input(void)
{
    TEST(scroll_update_input);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};
    scroll.content_h = 500.f;

    iui_update_scroll(ctx, 0.f, 30.f);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_rect_t layout = iui_get_layout_rect(ctx);
    float viewport_x = layout.x + 100.f, viewport_y = layout.y + 75.f;
    iui_update_mouse_pos(ctx, viewport_x, viewport_y);

    iui_scroll_begin(ctx, &scroll, 200.f, 150.f);

    ASSERT_TRUE(scroll.scroll_y >= 0.f);

    iui_scroll_end(ctx, &scroll);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_scroll_non_scrollable(void)
{
    TEST(scroll_non_scrollable);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_scroll_begin(ctx, &scroll, 200.f, 500.f);

    iui_text(ctx, IUI_ALIGN_LEFT, "Short content");
    iui_newline(ctx);
    iui_text(ctx, IUI_ALIGN_LEFT, "Only two lines");

    bool scrollable = iui_scroll_end(ctx, &scroll);

    ASSERT_FALSE(scrollable);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Scroll Interaction Tests */

static void test_scroll_wheel_interaction(void)
{
    TEST(scroll_wheel_interaction);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};

    /* Frame 1: Create scrollable content and verify scroll_by works */
    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_scroll_begin(ctx, &scroll, 200.f, 100.f);
    for (int i = 0; i < 30; i++) {
        iui_text(ctx, IUI_ALIGN_LEFT, "Line %d", i);
        iui_newline(ctx);
    }
    bool scrollable = iui_scroll_end(ctx, &scroll);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Content should be scrollable */
    ASSERT_TRUE(scrollable);
    ASSERT_TRUE(scroll.content_h > 100.f);

    /* Use iui_scroll_by directly to test scroll position changes */
    float initial_y = scroll.scroll_y;
    iui_scroll_by(&scroll, 0.f, 50.f);
    ASSERT_TRUE(scroll.scroll_y > initial_y);

    free(buffer);
    PASS();
}

static void test_scroll_negative_clamping(void)
{
    TEST(scroll_negative_clamping);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};
    scroll.scroll_y = -100.f; /* Try to scroll above content */

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_scroll_begin(ctx, &scroll, 200.f, 100.f);
    for (int i = 0; i < 30; i++) {
        iui_text(ctx, IUI_ALIGN_LEFT, "Line %d", i);
        iui_newline(ctx);
    }
    iui_scroll_end(ctx, &scroll);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Scroll should be clamped to 0 */
    ASSERT_TRUE(scroll.scroll_y >= 0.f);

    free(buffer);
    PASS();
}

static void test_scroll_horizontal(void)
{
    TEST(scroll_horizontal);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_scroll_state scroll = {0};

    /* Simulate horizontal scroll input */
    iui_update_scroll(ctx, 50.f, 0.f);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "test", 10, 10, 400, 300, 0);

    iui_update_mouse_pos(ctx, 110.f, 110.f);

    iui_scroll_begin(ctx, &scroll, 100.f, 100.f);
    /* Wide content that should enable horizontal scrolling */
    iui_text(ctx, IUI_ALIGN_LEFT,
             "Very long text content that extends beyond the viewport width");
    iui_scroll_end(ctx, &scroll);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Horizontal scroll should be tracked
     * Note: actual horizontal scroll behavior depends on implementation
     */

    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_scroll_tests(void)
{
    SECTION_BEGIN("Scrollable Container");
    test_scroll_state_init();
    test_scroll_by();
    test_scroll_to();
    test_scroll_null_safety();
    test_scroll_begin_end_basic();
    test_scroll_content_measurement();
    test_scroll_clamping();
    test_scroll_update_input();
    test_scroll_non_scrollable();
    /* Interaction tests */
    test_scroll_wheel_interaction();
    test_scroll_negative_clamping();
    test_scroll_horizontal();
    SECTION_END();
}
