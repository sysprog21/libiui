/*
 * Per-Frame Field ID Tracking Tests
 *
 * Tests for stale state prevention when widgets are conditionally hidden.
 */

#include "common.h"

/* Test textfield registration */
static void test_textfield_registration(void)
{
    TEST(textfield_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    /* Render a frame with the textfield */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);

    /* Before end_frame, field should be registered */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, text_buf));
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test slider registration */
static void test_slider_registration(void)
{
    TEST(slider_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    float value = 0.5f;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_slider_ex(ctx, value, 0.0f, 1.0f, 0.1f, NULL);
    iui_end_window(ctx);

    /* At least one slider should be registered */
    ASSERT_TRUE(ctx->field_tracking.slider_count > 0);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test stale textfield state cleared when not rendered */
static void test_textfield_stale_state_cleared(void)
{
    TEST(textfield_stale_state_cleared);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 2;

    /* Frame 1: Render textfield and focus it */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Verify textfield is focused */
    ASSERT_EQ(ctx->focused_edit, text_buf);

    /* Frame 2: Do NOT render the textfield (conditionally hidden) */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    /* textfield intentionally not rendered */
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Focused edit should be cleared since textfield wasn't rendered */
    ASSERT_NULL(ctx->focused_edit);

    free(buffer);
    PASS();
}

/* Test slider stale state cleared when not rendered */
static void test_slider_stale_state_cleared(void)
{
    TEST(slider_stale_state_cleared);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    float value = 50.0f;

    /* Frame 1: Render slider and start dragging */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Keep dragging */
    iui_update_mouse_pos(ctx, 220.0f, 150.0f);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 3: Do NOT render the slider (conditionally hidden) */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    /* slider intentionally not rendered */
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Active slider state should be cleared */
    ASSERT_EQ(ctx->slider.active_id & IUI_SLIDER_ID_MASK, 0);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    free(buffer);
    PASS();
}

/* Test multiple textfields per frame */
static void test_multiple_textfields(void)
{
    TEST(multiple_textfields);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf1[32] = "One";
    char buf2[32] = "Two";
    char buf3[32] = "Three";
    size_t c1 = 0, c2 = 0, c3 = 0;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, buf1, sizeof(buf1), &c1, NULL);
    iui_textfield(ctx, buf2, sizeof(buf2), &c2, NULL);
    iui_textfield(ctx, buf3, sizeof(buf3), &c3, NULL);
    iui_end_window(ctx);

    /* All three should be registered */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf1));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf2));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf3));
    ASSERT_EQ(ctx->field_tracking.textfield_count, 3);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test multiple sliders per frame */
static void test_multiple_sliders(void)
{
    TEST(multiple_sliders);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_slider_ex(ctx, 10.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_slider_ex(ctx, 50.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_slider_ex(ctx, 90.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);

    /* All three should be registered */
    ASSERT_EQ(ctx->field_tracking.slider_count, 3);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test frame counter increments */
static void test_frame_counter(void)
{
    TEST(frame_counter);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    uint32_t initial_frame = ctx->field_tracking.frame_number;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(ctx->field_tracking.frame_number, initial_frame + 1);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(ctx->field_tracking.frame_number, initial_frame + 2);

    free(buffer);
    PASS();
}

/* Test tracking reset between frames */
static void test_tracking_reset_between_frames(void)
{
    TEST(tracking_reset_between_frames);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    /* Frame 1: Register several fields */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_slider_ex(ctx, 50.0f, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Empty frame - counts should reset */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    /* After begin_frame, counts should be reset to 0 */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 0);
    ASSERT_EQ(ctx->field_tracking.slider_count, 0);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test duplicate registration (same field rendered twice) */
static void test_duplicate_registration(void)
{
    TEST(duplicate_registration);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";
    size_t cursor = 0;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    /* Register same buffer twice */
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);

    /* Should only count once (deduplication) */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 1);

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test edit_with_selection also registers */
static void test_edit_with_selection_registers(void)
{
    TEST(edit_with_selection_registers);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};
    state.cursor = 5;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);

    /* edit_with_selection should register the field */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, text_buf));

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test reset_field_ids public API */
static void test_reset_field_ids_api(void)
{
    TEST(reset_field_ids_api);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";

    /* Manually register a field */
    iui_register_textfield(ctx, text_buf);
    ASSERT_EQ(ctx->field_tracking.textfield_count, 1);

    /* Reset via public API */
    iui_reset_field_ids(ctx);

    /* Should be reset */
    ASSERT_EQ(ctx->field_tracking.textfield_count, 0);

    free(buffer);
    PASS();
}

/* Test search_bar registers its buffer for tracking */
static void test_search_bar_registers(void)
{
    TEST(search_bar_registers);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char search_buf[64] = "";
    size_t cursor = 0;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_search_bar(ctx, search_buf, sizeof(search_buf), &cursor, "Search...");
    iui_end_window(ctx);

    /* search_bar should register its buffer */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, search_buf));

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test textfield_with_selection registers its buffer */
static void test_textfield_with_selection_registers(void)
{
    TEST(textfield_with_selection_registers);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Test text";
    iui_edit_state state = {0};
    state.cursor = 4;

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_textfield_with_selection(ctx, text_buf, sizeof(text_buf), &state, NULL);
    iui_end_window(ctx);

    /* textfield_with_selection should register its buffer */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, text_buf));

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test focus persists across frames when field is rendered each frame.
 * This test catches bugs where text input widgets fail to register,
 * causing focus to be cleared at frame end.
 */
static void test_search_bar_focus_persists(void)
{
    TEST(search_bar_focus_persists);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char search_buf[64] = "";
    size_t cursor = 0;

    /* Frame 1: Click on search bar to focus it */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_search_bar(ctx, search_buf, sizeof(search_buf), &cursor, "Search...");
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Verify search bar gained focus */
    ASSERT_EQ(ctx->focused_edit, search_buf);

    /* Frame 2: Render search bar again without clicking */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_search_bar(ctx, search_buf, sizeof(search_buf), &cursor, "Search...");
    iui_end_window(ctx);

    /* Focus should persist because search bar registered this frame */
    ASSERT_EQ(ctx->focused_edit, search_buf);
    iui_end_frame(ctx);

    /* Focus should still be maintained after frame end */
    ASSERT_EQ(ctx->focused_edit, search_buf);

    free(buffer);
    PASS();
}

/* Test that unregistered text field loses focus (negative test).
 * Verifies the tracking system correctly clears stale focus.
 */
static void test_unregistered_field_loses_focus(void)
{
    TEST(unregistered_field_loses_focus);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Test";

    /* Manually set focus without proper registration */
    ctx->focused_edit = text_buf;

    /* Run a frame without rendering the textfield */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    /* Intentionally do NOT render any textfield */
    iui_button(ctx, "Button", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Focus should be cleared because buffer wasn't registered */
    ASSERT_NULL(ctx->focused_edit);

    free(buffer);
    PASS();
}

/* Test search_view registers its internal buffer for tracking */
static void test_search_view_registers(void)
{
    TEST(search_view_registers);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_search_view_state search = {0};
    iui_search_view_open(&search);

    iui_begin_frame(ctx, 1.0f / 60.0f);

    /* Render search view (simulating full screen) */
    bool open = iui_search_view_begin(ctx, &search, 800, 600, "Search...");

    /* Verify the query buffer inside the state struct is registered */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, search.query));

    /* Verify it auto-focused */
    ASSERT_EQ(ctx->focused_edit, search.query);

    if (open)
        iui_search_view_end(ctx, &search);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test focus switching between multiple text fields.
 * Verifies that when focus transfers from one field to another via click,
 * both fields remain properly registered with the tracking system.
 * Uses real mouse input to test actual focus handling logic.
 */
static void test_focus_switch_between_fields(void)
{
    TEST(focus_switch_between_fields);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf1[32] = "Field 1";
    char buf2[32] = "Field 2";
    size_t c1 = 0, c2 = 0;

    /* Frame 1: Click on field 1 to focus it.
     * Window at (0,0), field 1 starts after title bar (~40dp).
     * Click at y=60 should hit field 1 (height=56dp, so field 1 is ~40-96).
     */
    iui_update_mouse_pos(ctx, 150.0f, 60.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 300, 300, 0);
    iui_textfield(ctx, buf1, sizeof(buf1), &c1, NULL);
    iui_textfield(ctx, buf2, sizeof(buf2), &c2, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Verify Field 1 has focus and both are registered */
    ASSERT_EQ(ctx->focused_edit, buf1);
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf1));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf2));

    /* Frame 2: Click on field 2 to transfer focus.
     * Field 2 starts at ~96dp (after field 1), click at y=120.
     */
    iui_update_mouse_pos(ctx, 150.0f, 120.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 300, 300, 0);
    iui_textfield(ctx, buf1, sizeof(buf1), &c1, NULL);
    iui_textfield(ctx, buf2, sizeof(buf2), &c2, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Focus should have transferred to buf2 via actual click handling */
    ASSERT_EQ(ctx->focused_edit, buf2);

    /* Both should still be registered as valid fields */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf1));
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf2));

    free(buffer);
    PASS();
}

/* Test that disabled field clears focus when rendered as disabled */
static void test_disabled_field_clears_focus(void)
{
    TEST(disabled_field_clears_focus);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf[32] = "Text";
    size_t cursor = 0;

    /* Frame 1: Click to focus the field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, buf, sizeof(buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    ASSERT_EQ(ctx->focused_edit, buf);

    /* Frame 2: Same field, but now disabled */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield_options opts = {.disabled = true};
    iui_textfield(ctx, buf, sizeof(buf), &cursor, &opts);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Focus should be NULL because the field became disabled */
    ASSERT_NULL(ctx->focused_edit);

    /* But it should still be registered (tracking works, logic is in component)
     */
    ASSERT_TRUE(iui_textfield_is_registered(ctx, buf));

    free(buffer);
    PASS();
}

/* Test that read-only field clears focus when rendered as read_only */
static void test_read_only_field_clears_focus(void)
{
    TEST(read_only_field_clears_focus);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf[32] = "Text";
    size_t cursor = 0;

    /* Frame 1: Click to focus the field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, buf, sizeof(buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    ASSERT_EQ(ctx->focused_edit, buf);

    /* Frame 2: Same field, but now read_only */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield_options opts = {.read_only = true};
    iui_textfield(ctx, buf, sizeof(buf), &cursor, &opts);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Focus should be NULL because the field became read_only */
    ASSERT_NULL(ctx->focused_edit);

    free(buffer);
    PASS();
}

/* Test slider active state is cleared when slider not rendered */
static void test_slider_unrendered_clears_active(void)
{
    TEST(slider_unrendered_clears_active);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    float value = 50.0f;

    /* Frame 1: Render slider and start interaction */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Keep mouse down, slider should be active */
    iui_update_mouse_pos(ctx, 220.0f, 150.0f);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    value = iui_slider_ex(ctx, value, 0.0f, 100.0f, 1.0f, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Frame 3: Do NOT render the slider */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    /* slider intentionally not rendered */
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Active slider should be cleared */
    ASSERT_EQ(ctx->slider.active_id & IUI_SLIDER_ID_MASK, 0);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    free(buffer);
    PASS();
}

/* Test field skipped for one frame then re-rendered retains ability to focus */
static void test_rerender_after_skip_frame(void)
{
    TEST(rerender_after_skip_frame);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char buf[32] = "Test";
    size_t cursor = 0;

    /* Frame 1: Render and focus textfield */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, buf, sizeof(buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    ASSERT_EQ(ctx->focused_edit, buf);

    /* Frame 2: Skip rendering textfield - focus should be cleared */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_button(ctx, "Other", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_NULL(ctx->focused_edit);

    /* Frame 3: Re-render textfield and click to refocus */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, buf, sizeof(buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Should be able to regain focus after being re-rendered */
    ASSERT_EQ(ctx->focused_edit, buf);

    free(buffer);
    PASS();
}

/* Stress test: many text fields to verify hash table handles collisions */
static void test_many_textfields_stress(void)
{
    TEST(many_textfields_stress);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

#define NUM_FIELDS 32
    char bufs[NUM_FIELDS][16];
    size_t cursors[NUM_FIELDS] = {0};

    /* Initialize buffers with unique content */
    for (int i = 0; i < NUM_FIELDS; i++) {
        snprintf(bufs[i], sizeof(bufs[i]), "Field %d", i);
    }

    /* Frame: Render all fields */
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 800, 600, 0);
    for (int i = 0; i < NUM_FIELDS; i++) {
        iui_textfield(ctx, bufs[i], sizeof(bufs[i]), &cursors[i], NULL);
    }
    iui_end_window(ctx);

    /* All fields should be registered */
    ASSERT_EQ(ctx->field_tracking.textfield_count, NUM_FIELDS);

    /* Verify each buffer is registered (tests hash collision handling) */
    for (int i = 0; i < NUM_FIELDS; i++) {
        ASSERT_TRUE(iui_textfield_is_registered(ctx, bufs[i]));
    }

    iui_end_frame(ctx);

#undef NUM_FIELDS
    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_field_tracking_tests(void)
{
    SECTION_BEGIN("Field Tracking");
    test_textfield_registration();
    test_slider_registration();
    test_textfield_stale_state_cleared();
    test_slider_stale_state_cleared();
    test_multiple_textfields();
    test_multiple_sliders();
    test_frame_counter();
    test_tracking_reset_between_frames();
    test_duplicate_registration();
    test_edit_with_selection_registers();
    test_reset_field_ids_api();
    test_search_bar_registers();
    test_textfield_with_selection_registers();
    test_search_bar_focus_persists();
    test_unregistered_field_loses_focus();
    test_search_view_registers();
    test_focus_switch_between_fields();
    test_disabled_field_clears_focus();
    test_read_only_field_clears_focus();
    test_slider_unrendered_clears_active();
    test_rerender_after_skip_frame();
    test_many_textfields_stress();
    SECTION_END();
}
