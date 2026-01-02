/*
 * Stack and Buffer Overflow Tests
 *
 * Tests for boundary conditions and overflow prevention in:
 * - Clip stack
 * - ID stack
 * - Input layers
 * - Large coordinate handling
 * - Widget ID generation
 */

#include "common.h"

/* Test clip stack overflow protection */
static void test_clip_stack_overflow(void)
{
    TEST(clip_stack_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Try to push more clips than the stack can hold */
    iui_rect_t clip = {10, 10, 100, 100};
    int successful_pushes = 0;

    for (int i = 0; i < IUI_CLIP_STACK_SIZE + 10; i++) {
        if (iui_push_clip(ctx, clip))
            successful_pushes++;
    }

    /* Should have stopped at max depth */
    ASSERT_TRUE(successful_pushes <= IUI_CLIP_STACK_SIZE);

    /* Pop all clips - should not crash */
    for (int i = 0; i < successful_pushes + 5; i++)
        iui_pop_clip(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test ID stack overflow protection */
static void test_id_stack_overflow(void)
{
    TEST(id_stack_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Try to push more IDs than the stack can hold */
    int successful_pushes = 0;
    for (int i = 0; i < IUI_ID_STACK_SIZE + 10; i++) {
        uint32_t id = (uint32_t) i;
        if (iui_push_id(ctx, &id, sizeof(id)))
            successful_pushes++;
    }

    /* Should have stopped at max depth */
    ASSERT_TRUE(successful_pushes <= IUI_ID_STACK_SIZE);

    /* Pop all IDs - should not crash */
    for (int i = 0; i < successful_pushes + 5; i++)
        iui_pop_id(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test layer stack overflow protection */
static void test_layer_overflow(void)
{
    TEST(layer_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Try to push more layers than allowed */
    int successful_pushes = 0;
    for (int i = 0; i < IUI_MAX_INPUT_LAYERS + 10; i++) {
        int layer_id = iui_push_layer(ctx, i);
        if (layer_id > 0) /* Returns 0 on failure, positive ID on success */
            successful_pushes++;
    }

    /* Should have stopped at max depth */
    ASSERT_TRUE(successful_pushes <= IUI_MAX_INPUT_LAYERS);

    /* Pop all layers - should not crash */
    for (int i = 0; i < successful_pushes + 5; i++)
        iui_pop_layer(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test large coordinate clip handling (uint16 clamping) */
static void test_large_coord_clip(void)
{
    TEST(large_coord_clip);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    reset_counters();

    /* Push clip with very large coordinates (should be clamped) */
    iui_rect_t large_clip = {100000.f, 100000.f, 50000.f, 50000.f};
    bool success = iui_push_clip(ctx, large_clip);
    ASSERT_TRUE(success);

    /* Verify coordinates were clamped to uint16 max (150000 -> 65535) */
    ASSERT_EQ(g_last_clip_max_x, UINT16_MAX);
    ASSERT_EQ(g_last_clip_max_y, UINT16_MAX);

    iui_pop_clip(ctx);

    /* Test negative coordinates (should clamp to 0) */
    iui_rect_t neg_clip = {-100.f, -100.f, 200.f, 200.f};
    success = iui_push_clip(ctx, neg_clip);
    ASSERT_TRUE(success);
    ASSERT_EQ(g_last_clip_min_x, 0);
    ASSERT_EQ(g_last_clip_min_y, 0);

    iui_pop_clip(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test widget ID generation with extreme coordinates */
static void test_widget_id_extreme_coords(void)
{
    TEST(widget_id_extreme_coords);

    /* Test iui_hash_pos with various coordinate values
     * Note: hash uses XOR so (x,x) always produces 0 - test with x != y */
    uint32_t id1 = iui_hash_pos(0.f, 0.f);
    uint32_t id2 = iui_hash_pos(100.f, 200.f); /* Different x,y for non-zero */
    uint32_t id3 =
        iui_hash_pos(10000000.f, 100.f); /* One large (clamped), one small */
    uint32_t id4 = iui_hash_pos(-100.f, -100.f); /* Negative */

    /* IDs should be different for different positions (when x != y) */
    ASSERT_TRUE(id1 != id2);

    /* Large x (clamped to ~4M) with small y produces non-zero ID */
    ASSERT_TRUE(id3 != 0);

    /* Negative coordinates should clamp to 0,0 */
    ASSERT_TRUE(id4 == id1); /* Both clamp to (0,0), XOR gives 0 */

    PASS();
}

/* Test iui_float_to_u16 helper directly */
static void test_float_to_u16_helper(void)
{
    TEST(float_to_u16_helper);

    /* Normal values */
    ASSERT_EQ(iui_float_to_u16(0.f), 0);
    ASSERT_EQ(iui_float_to_u16(100.f), 100);
    ASSERT_EQ(iui_float_to_u16(65535.f), 65535);

    /* Clamping at max */
    ASSERT_EQ(iui_float_to_u16(100000.f), UINT16_MAX);
    ASSERT_EQ(iui_float_to_u16(1000000.f), UINT16_MAX);

    /* Clamping at min (negative values) */
    ASSERT_EQ(iui_float_to_u16(-1.f), 0);
    ASSERT_EQ(iui_float_to_u16(-1000.f), 0);

    /* Rounding behavior */
    ASSERT_EQ(iui_float_to_u16(100.4f), 100);
    ASSERT_EQ(iui_float_to_u16(100.5f), 101);
    ASSERT_EQ(iui_float_to_u16(100.6f), 101);

    PASS();
}

/* Test textfield tracking overflow */
static void test_textfield_tracking_overflow(void)
{
    TEST(textfield_tracking_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Create more textfields than can be tracked */
    char text_bufs[IUI_MAX_TRACKED_TEXTFIELDS + 10][64];
    size_t cursors[IUI_MAX_TRACKED_TEXTFIELDS + 10];

    for (int i = 0; i < IUI_MAX_TRACKED_TEXTFIELDS + 10; i++) {
        snprintf(text_bufs[i], sizeof(text_bufs[i]), "Field %d", i);
        cursors[i] = 0;
        iui_textfield(ctx, text_bufs[i], sizeof(text_bufs[i]), &cursors[i],
                      NULL);
        iui_newline(ctx);
    }

    /* Should not crash - overflow handled gracefully */

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test slider tracking overflow */
static void test_slider_tracking_overflow(void)
{
    TEST(slider_tracking_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 600, 0);

    /* Create more sliders than can be tracked */
    float values[IUI_MAX_TRACKED_SLIDERS + 10];

    for (int i = 0; i < IUI_MAX_TRACKED_SLIDERS + 10; i++) {
        values[i] = iui_slider_ex(ctx, 50.f, 0.f, 100.f, 1.f, NULL);
        iui_newline(ctx);
    }
    (void) values; /* suppress unused warning - test only checks for crashes */

    /* Should not crash - overflow handled gracefully */

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test focusable widget registration overflow */
static void test_focusable_overflow(void)
{
    TEST(focusable_overflow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 2000, 0);

    /* Create many buttons (each registers as focusable) */
    for (int i = 0; i < 100; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Button %d", i);
        iui_button(ctx, label, IUI_ALIGN_LEFT);
        iui_newline(ctx);
    }

    /* Should not crash - overflow handled gracefully */

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Runner function */
void run_overflow_tests(void)
{
    SECTION_BEGIN("Stack/Buffer Overflow Protection");

    test_clip_stack_overflow();
    test_id_stack_overflow();
    test_layer_overflow();
    test_large_coord_clip();
    test_widget_id_extreme_coords();
    test_float_to_u16_helper();
    test_textfield_tracking_overflow();
    test_slider_tracking_overflow();
    test_focusable_overflow();

    SECTION_END();
}
