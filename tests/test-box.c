/*
 * Box Container Layout Tests
 *
 * Tests for the nestable flexbox-like box container system.
 */

#include "common.h"

/* Test NULL sizes = all equal GROW(1) */
static void test_box_null_sizes(void)
{
    BEGIN_TEST_WINDOW(box_null_sizes);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 4});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);
    iui_rect_t r3 = iui_box_next(ctx);
    iui_rect_t r4 = iui_box_next(ctx);

    ASSERT_NEAR(r1.width, r2.width, 1.0f);
    ASSERT_NEAR(r2.width, r3.width, 1.0f);
    ASSERT_NEAR(r3.width, r4.width, 1.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test FIXED + GROW distribution */
static void test_box_fixed_grow(void)
{
    BEGIN_TEST_WINDOW(box_fixed_grow);

    iui_sizing_t sizes[] = {IUI_GROW(1), IUI_FIXED(100), IUI_GROW(2)};
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .sizes = sizes});

    iui_box_next(ctx);
    iui_rect_t center = iui_box_next(ctx);
    iui_box_next(ctx);

    ASSERT_NEAR(center.width, 100.0f, 1.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test PERCENT sizing */
static void test_box_percent(void)
{
    BEGIN_TEST_WINDOW(box_percent);

    iui_sizing_t sizes[] = {IUI_PERCENT(0.25f), IUI_GROW(1)};
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = sizes});

    iui_rect_t left = iui_box_next(ctx);
    iui_rect_t right = iui_box_next(ctx);

    /* Left should be ~25% of available width */
    float total = left.width + right.width;
    ASSERT_NEAR(left.width / total, 0.25f, 0.05f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test column direction */
static void test_box_column(void)
{
    BEGIN_TEST_WINDOW(box_column);

    iui_sizing_t sizes[] = {IUI_FIXED(30), IUI_GROW(1), IUI_FIXED(30)};
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .direction = IUI_DIR_COLUMN,
                           .child_count = 3,
                           .sizes = sizes,
                           .cross = 200.0f,
                       });

    iui_rect_t top = iui_box_next(ctx);
    iui_rect_t middle = iui_box_next(ctx);
    iui_rect_t bottom = iui_box_next(ctx);

    ASSERT_NEAR(top.height, 30.0f, 1.0f);
    ASSERT_NEAR(bottom.height, 30.0f, 1.0f);
    ASSERT_TRUE(middle.height > 50.0f);
    ASSERT_TRUE(middle.y > top.y);
    ASSERT_TRUE(bottom.y > middle.y);
    /* cross=200 sets column WIDTH, not height */
    ASSERT_NEAR(top.width, 200.0f, 1.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test nested containers */
static void test_box_nested(void)
{
    BEGIN_TEST_WINDOW(box_nested);

    iui_sizing_t outer[] = {IUI_FIXED(200), IUI_GROW(1)};
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = outer});

    iui_rect_t left = iui_box_next(ctx);
    ASSERT_NEAR(left.width, 200.0f, 1.0f);
    ASSERT_EQ(iui_box_depth(ctx), 1);

    /* Nest a column inside the right slot */
    iui_rect_t right = iui_box_next(ctx);
    ASSERT_TRUE(right.width > 0);

    iui_sizing_t inner[] = {IUI_GROW(1), IUI_GROW(1)};
    /* cross=0: inherit width from parent slot, height from parent slot */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .direction = IUI_DIR_COLUMN,
                           .child_count = 2,
                           .sizes = inner,
                       });
    ASSERT_EQ(iui_box_depth(ctx), 2);

    iui_rect_t inner_top = iui_box_next(ctx);
    iui_rect_t inner_bottom = iui_box_next(ctx);
    ASSERT_NEAR(inner_top.height, inner_bottom.height, 1.0f);

    iui_box_end(ctx);
    iui_box_end(ctx);
    ASSERT_EQ(iui_box_depth(ctx), 0);

    END_TEST_WINDOW();
}

/* Test padding is snapped to 4dp grid and reduces content area */
static void test_box_padding(void)
{
    BEGIN_TEST_WINDOW(box_padding);

    /* Padding 10 -> snapped to 12 */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .padding = IUI_PAD_ALL(10),
                       });

    iui_rect_t child = iui_box_next(ctx);
    /* Child width should be reduced by 2 * snapped_padding */
    float snapped_pad = iui_spacing_snap(10.f); /* = 12 */
    float parent_w = ctx->box_stack[ctx->box_depth - 1].saved_layout.width;
    ASSERT_NEAR(child.width, parent_w - snapped_pad * 2, 1.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test gap is snapped to 4dp grid */
static void test_box_gap(void)
{
    BEGIN_TEST_WINDOW(box_gap);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .gap = 10});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);

    float snapped_gap = iui_spacing_snap(10.f); /* = 12 */
    float actual_gap = r2.x - (r1.x + r1.width);
    ASSERT_NEAR(actual_gap, snapped_gap, 1.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test cross-axis alignment */
static void test_box_cross_align(void)
{
    BEGIN_TEST_WINDOW(box_cross_align);

    iui_sizing_t one[] = {IUI_FIXED(50)};

    /* STRETCH (default): child gets full cross height */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .sizes = one,
                           .cross = 100,
                           .align = IUI_CROSS_STRETCH,
                       });
    iui_rect_t stretch = iui_box_next(ctx);
    ASSERT_NEAR(stretch.height, 100.0f, 1.0f);
    iui_box_end(ctx);

    /* CENTER: cross-size shrinks to main-axis size, centered in cross */
    float pre_y = ctx->layout.y; /* y before this box */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .sizes = one,
                           .cross = 100,
                           .align = IUI_CROSS_CENTER,
                       });
    iui_rect_t center = iui_box_next(ctx);
    ASSERT_NEAR(center.height, 50.0f, 1.0f);
    /* Should be offset 25px from the box origin */
    ASSERT_NEAR(center.y - pre_y, 25.0f, 2.0f);
    iui_box_end(ctx);

    /* END: child at bottom of cross space */
    pre_y = ctx->layout.y;
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .sizes = one,
                           .cross = 100,
                           .align = IUI_CROSS_END,
                       });
    iui_rect_t end = iui_box_next(ctx);
    ASSERT_NEAR(end.height, 50.0f, 1.0f);
    /* Should be offset 50px from the box origin */
    ASSERT_NEAR(end.y - pre_y, 50.0f, 2.0f);
    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test stack depth overflow protection */
static void test_box_overflow(void)
{
    BEGIN_TEST_WINDOW(box_overflow);

    /* Nest IUI_MAX_BOX_DEPTH times */
    int i;
    for (i = 0; i < IUI_MAX_BOX_DEPTH; i++) {
        iui_rect_t r =
            iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 1});
        if (r.width == 0 && r.height == 0)
            break;
        iui_box_next(ctx);
    }
    ASSERT_EQ(i, IUI_MAX_BOX_DEPTH);

    /* One more should fail */
    iui_rect_t over =
        iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 1});
    ASSERT_NEAR(over.width, 0.0f, 0.001f);

    /* Unwind */
    for (int j = 0; j < IUI_MAX_BOX_DEPTH; j++)
        iui_box_end(ctx);
    ASSERT_EQ(iui_box_depth(ctx), 0);

    END_TEST_WINDOW();
}

/* Test box inside scroll container */
static void test_box_with_scroll(void)
{
    BEGIN_TEST_WINDOW(box_with_scroll);

    iui_scroll_state scroll = {0};
    iui_scroll_begin(ctx, &scroll, 400, 200);

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2});
    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);
    ASSERT_TRUE(r1.width > 0);
    ASSERT_TRUE(r2.width > 0);
    iui_box_end(ctx);

    iui_scroll_end(ctx, &scroll);

    END_TEST_WINDOW();
}

/* Test min/max constraints on GROW */
static void test_box_min_max(void)
{
    BEGIN_TEST_WINDOW(box_min_max);

    iui_sizing_t sizes[] = {
        {IUI_SIZE_GROW, 1, 50, 80}, /* min=50, max=80 */
        {IUI_SIZE_GROW, 1, 0, 0},   /* unconstrained */
    };
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = sizes});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);

    ASSERT_TRUE(r1.width >= 50.0f);
    ASSERT_TRUE(r1.width <= 80.0f);
    ASSERT_TRUE(r2.width > 0);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test max-clamped GROW redistributes freed space to siblings */
static void test_box_grow_max_redistribute(void)
{
    BEGIN_TEST_WINDOW(box_grow_max_redistribute);

    /* Three equal-weight GROW children. First is capped at 80px.
     * Without redistribution the freed space would be lost, leaving a gap.
     * With redistribution, siblings split the surplus evenly.
     */
    iui_sizing_t sizes[] = {
        {IUI_SIZE_GROW, 1, 0, 80}, /* max=80 */
        {IUI_SIZE_GROW, 1, 0, 0},  /* unconstrained */
        {IUI_SIZE_GROW, 1, 0, 0},  /* unconstrained */
    };
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .sizes = sizes});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);
    iui_rect_t r3 = iui_box_next(ctx);

    /* r1 should be clamped at max */
    ASSERT_TRUE(r1.width <= 80.0f + 1.0f);

    /* r2 and r3 should absorb the freed space equally */
    ASSERT_NEAR(r2.width, r3.width, 1.0f);

    /* Total should account for all available width (no gap in this box) */
    float parent_w = ctx->box_stack[ctx->box_depth - 1].saved_layout.width;
    ASSERT_NEAR(r1.width + r2.width + r3.width, parent_w, 2.0f);

    /* Each unconstrained sibling should be larger than the clamped one */
    ASSERT_TRUE(r2.width > r1.width);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test cascading max-clamp: redistribution must not breach frozen siblings */
static void test_box_grow_cascading_max(void)
{
    BEGIN_TEST_WINDOW(box_grow_cascading_max);

    /* Three GROW(1) children with two different max values.
     * After initial distribution (300/3 = 100 each):
     *   child 0: 100 > max 80  -> clamped, frees 20
     *   child 1: 100 + 10 = 110 > max 105 -> clamped in next round, frees 5
     *   child 2: absorbs all freed space
     * Without persistent freeze, child 0 would re-enter the pool and exceed 80.
     */
    iui_sizing_t sizes[] = {
        {IUI_SIZE_GROW, 1, 0, 80},  /* max=80 */
        {IUI_SIZE_GROW, 1, 0, 105}, /* max=105 */
        {IUI_SIZE_GROW, 1, 0, 0},   /* unconstrained */
    };
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 3, .sizes = sizes});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);
    iui_rect_t r3 = iui_box_next(ctx);

    /* Both clamped children must respect their max */
    ASSERT_TRUE(r1.width <= 80.0f + 0.5f);
    ASSERT_TRUE(r2.width <= 105.0f + 0.5f);

    /* Unconstrained child absorbs all surplus */
    ASSERT_TRUE(r3.width > r2.width);

    /* Total must still fill the parent */
    float parent_w = ctx->box_stack[ctx->box_depth - 1].saved_layout.width;
    ASSERT_NEAR(r1.width + r2.width + r3.width, parent_w, 2.0f);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test surplus redistribution when grow children start at zero */
static void test_box_fixed_max_grow_redistribute(void)
{
    BEGIN_TEST_WINDOW(box_fixed_max_grow_redistribute);

    /* FIXED(100, max=50) + GROW(1) in a container.
     * Pass 1: FIXED gets 100, GROW gets remainder (maybe 0 if small container).
     * Pass 3: FIXED clamped to 50, freeing 50. GROW was at 0 -> grow_pool=0.
     * Without the weight fallback, freed 50px is lost. With it, GROW gets 50.
     */
    iui_sizing_t sizes[] = {
        {IUI_SIZE_FIXED, 100, 0, 50}, /* FIXED(100) with max=50 */
        {IUI_SIZE_GROW, 1, 0, 0},     /* unconstrained GROW */
    };
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = sizes});

    iui_rect_t r1 = iui_box_next(ctx);
    iui_rect_t r2 = iui_box_next(ctx);

    /* FIXED should be clamped to max=50 */
    ASSERT_NEAR(r1.width, 50.0f, 1.0f);

    /* GROW should absorb the freed 50px */
    float parent_w = ctx->box_stack[ctx->box_depth - 1].saved_layout.width;
    ASSERT_NEAR(r1.width + r2.width, parent_w, 2.0f);
    ASSERT_TRUE(r2.width > 0);

    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test cross-alignment clamp: size > cross_total must not overflow */
static void test_box_cross_align_clamp(void)
{
    BEGIN_TEST_WINDOW(box_cross_align_clamp);

    /* Row box with small cross (height=40). Child main-axis size=100.
     * Without clamping, CENTER would set cross_size=100 and offset negative.
     */
    iui_sizing_t sizes[] = {IUI_FIXED(100)};

    /* CENTER alignment */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .sizes = sizes,
                           .cross = 40,
                           .align = IUI_CROSS_CENTER,
                       });
    iui_rect_t center = iui_box_next(ctx);
    /* Cross dimension (height) must not exceed the container cross (40) */
    ASSERT_TRUE(center.height <= 40.0f + 0.5f);
    /* Must not shift outside the container */
    float box_y = ctx->box_stack[ctx->box_depth - 1].saved_layout.y;
    ASSERT_TRUE(center.y >= box_y);
    iui_box_end(ctx);

    /* END alignment */
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .child_count = 1,
                           .sizes = sizes,
                           .cross = 40,
                           .align = IUI_CROSS_END,
                       });
    iui_rect_t end = iui_box_next(ctx);
    ASSERT_TRUE(end.height <= 40.0f + 0.5f);
    box_y = ctx->box_stack[ctx->box_depth - 1].saved_layout.y;
    ASSERT_TRUE(end.y >= box_y);
    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test MD3 breakpoint classification */
static void test_size_class(void)
{
    TEST(size_class);

    ASSERT_EQ(iui_size_class(0.f), IUI_SIZE_CLASS_COMPACT);
    ASSERT_EQ(iui_size_class(599.f), IUI_SIZE_CLASS_COMPACT);
    ASSERT_EQ(iui_size_class(600.f), IUI_SIZE_CLASS_MEDIUM);
    ASSERT_EQ(iui_size_class(839.f), IUI_SIZE_CLASS_MEDIUM);
    ASSERT_EQ(iui_size_class(840.f), IUI_SIZE_CLASS_EXPANDED);
    ASSERT_EQ(iui_size_class(1199.f), IUI_SIZE_CLASS_EXPANDED);
    ASSERT_EQ(iui_size_class(1200.f), IUI_SIZE_CLASS_LARGE);
    ASSERT_EQ(iui_size_class(1599.f), IUI_SIZE_CLASS_LARGE);
    ASSERT_EQ(iui_size_class(1600.f), IUI_SIZE_CLASS_XLARGE);
    ASSERT_EQ(iui_size_class(2000.f), IUI_SIZE_CLASS_XLARGE);

    PASS();
}

/* Test MD3 grid queries */
static void test_layout_columns_margins_gutters(void)
{
    TEST(layout_columns_margins_gutters);

    ASSERT_EQ(iui_layout_columns(IUI_SIZE_CLASS_COMPACT), 4);
    ASSERT_EQ(iui_layout_columns(IUI_SIZE_CLASS_MEDIUM), 8);
    ASSERT_EQ(iui_layout_columns(IUI_SIZE_CLASS_EXPANDED), 12);
    ASSERT_EQ(iui_layout_columns(IUI_SIZE_CLASS_LARGE), 12);

    ASSERT_NEAR(iui_layout_margin(IUI_SIZE_CLASS_COMPACT), 16.f, 0.01f);
    ASSERT_NEAR(iui_layout_margin(IUI_SIZE_CLASS_MEDIUM), 24.f, 0.01f);

    ASSERT_NEAR(iui_layout_gutter(IUI_SIZE_CLASS_COMPACT), 8.f, 0.01f);
    ASSERT_NEAR(iui_layout_gutter(IUI_SIZE_CLASS_EXPANDED), 16.f, 0.01f);

    PASS();
}

/* Test nested column box inherits parent slot height (cross=0) */
static void test_box_nested_column_inherits_height(void)
{
    BEGIN_TEST_WINDOW(box_nested_column_inherits_height);

    /* Outer row box with explicit cross height */
    iui_sizing_t outer[] = {IUI_FIXED(200), IUI_GROW(1)};
    iui_rect_t container = iui_box_begin(
        ctx, &(iui_box_config_t) {
                 .child_count = 2, .sizes = outer, .cross = 120.0f});

    iui_box_next(ctx); /* left slot */
    iui_rect_t right = iui_box_next(ctx);

    /* Nested column with cross=0 should inherit parent slot height */
    iui_sizing_t inner[] = {IUI_GROW(1), IUI_GROW(1)};
    iui_rect_t col_container =
        iui_box_begin(ctx, &(iui_box_config_t) {.direction = IUI_DIR_COLUMN,
                                                .child_count = 2,
                                                .sizes = inner});

    /* Column container height should match the parent slot, not the window */
    ASSERT_NEAR(col_container.height, right.height, 1.0f);
    ASSERT_TRUE(col_container.height <= container.height);

    iui_rect_t top = iui_box_next(ctx);
    iui_rect_t bottom = iui_box_next(ctx);
    ASSERT_TRUE(top.height > 0);
    ASSERT_TRUE(bottom.y > top.y);

    iui_box_end(ctx);
    iui_box_end(ctx);

    END_TEST_WINDOW();
}

/* Test column box_end advances past full padded height */
static void test_box_column_padding_advance(void)
{
    BEGIN_TEST_WINDOW(box_column_padding_advance);

    float y_before = iui_get_layout_rect(ctx).y;

    iui_sizing_t sizes[] = {IUI_FIXED(40), IUI_FIXED(40)};
    iui_box_begin(ctx, &(iui_box_config_t) {.direction = IUI_DIR_COLUMN,
                                            .child_count = 2,
                                            .sizes = sizes,
                                            .cross = 120.0f,
                                            .gap = 4,
                                            .padding = IUI_PAD_ALL(8)});

    iui_box_next(ctx);
    iui_box_next(ctx);
    iui_box_end(ctx);

    float y_after = iui_get_layout_rect(ctx).y;
    /* Expected: pad_t(8) + child(40) + gap(4) + child(40) + pad_b(8) = 100 */
    float expected_advance = 8.0f + 40.0f + 4.0f + 40.0f + 8.0f;
    ASSERT_NEAR(y_after - y_before, expected_advance, 1.0f);

    END_TEST_WINDOW();
}

/* Test nested row box inherits parent slot height (cross=0) */
static void test_box_nested_row_inherits_height(void)
{
    BEGIN_TEST_WINDOW(box_nested_row_inherits_height);

    /* Outer column box with explicit cross (width) and known height */
    iui_sizing_t outer[] = {IUI_FIXED(60), IUI_GROW(1)};
    iui_rect_t container =
        iui_box_begin(ctx, &(iui_box_config_t) {.direction = IUI_DIR_COLUMN,
                                                .child_count = 2,
                                                .sizes = outer,
                                                .cross = 300.0f});

    iui_rect_t top = iui_box_next(ctx); /* 60px tall row */
    iui_rect_t bottom_slot = iui_box_next(ctx);

    /* Nested row box inside the bottom slot, cross=0 should inherit height */
    iui_sizing_t inner[] = {IUI_GROW(1), IUI_GROW(1)};
    iui_rect_t row_container = iui_box_begin(
        ctx, &(iui_box_config_t) {.child_count = 2, .sizes = inner});

    /* Row container height should match the parent slot, not row_height */
    ASSERT_NEAR(row_container.height, bottom_slot.height, 1.0f);
    ASSERT_TRUE(row_container.height > 0);

    iui_rect_t left = iui_box_next(ctx);
    iui_rect_t right = iui_box_next(ctx);

    /* Children should have the inherited height */
    ASSERT_NEAR(left.height, bottom_slot.height, 1.0f);
    ASSERT_NEAR(right.height, bottom_slot.height, 1.0f);

    iui_box_end(ctx);
    iui_box_end(ctx);

    (void) top;
    (void) container;

    END_TEST_WINDOW();
}

/* Test column box_end with zero children consumed */
static void test_box_column_no_children(void)
{
    BEGIN_TEST_WINDOW(box_column_no_children);

    float y_before = iui_get_layout_rect(ctx).y;

    /* Column box with padding but no iui_box_next calls */
    iui_box_begin(ctx, &(iui_box_config_t) {.direction = IUI_DIR_COLUMN,
                                            .child_count = 2,
                                            .cross = 100.0f,
                                            .gap = 8,
                                            .padding = IUI_PAD_ALL(12)});
    /* Skip all children -- conditional rendering scenario */
    iui_box_end(ctx);

    float y_after = iui_get_layout_rect(ctx).y;

    /* Should advance by at least the padding (never go backward) */
    ASSERT_TRUE(y_after >= y_before);
    /* Expected: pad_t(12) + 0 children + pad_b(12) = 24 */
    float snapped_pad = iui_spacing_snap(12.0f);
    ASSERT_NEAR(y_after - y_before, snapped_pad * 2, 1.0f);

    END_TEST_WINDOW();
}

/* Test GROW child never goes negative when deficit exceeds container */
static void test_box_grow_no_negative(void)
{
    BEGIN_TEST_WINDOW(box_grow_no_negative);

    /* GROW(1) + PERCENT(1.0, min=200) in ~384px container.
     * PERCENT(1.0) wants 384 but min=200 forces 200 => deficit pulls from
     * GROW.  Without the floor clamp the GROW child goes negative.
     */
    iui_sizing_t sizes[] = {
        {IUI_SIZE_GROW, 1, 0, 0},
        {IUI_SIZE_PERCENT, 1.0f, 200, 0},
    };

    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = sizes});
    iui_rect_t grow_r = iui_box_next(ctx);
    iui_rect_t pct_r = iui_box_next(ctx);
    iui_box_end(ctx);

    /* GROW child must never be negative */
    ASSERT_TRUE(grow_r.width >= 0.f);
    /* PERCENT child honoured its min */
    ASSERT_TRUE(pct_r.width >= 200.f - 1.f);

    END_TEST_WINDOW();
}

/* Test row box_end advances by resolved cross, not saved_layout.height */
static void test_box_row_end_advance(void)
{
    BEGIN_TEST_WINDOW(box_row_end_advance);

    /* Outer column gives two 100px-tall slots.  A row box (cross=0) placed
     * in the first slot should fill 100px and advance the cursor by 100px,
     * not by the full parent container height.
     */
    iui_sizing_t outer[] = {IUI_FIXED(100), IUI_FIXED(100)};
    iui_box_begin(ctx, &(iui_box_config_t) {
                           .direction = IUI_DIR_COLUMN,
                           .child_count = 2,
                           .sizes = outer,
                       });

    iui_rect_t slot1 = iui_box_next(ctx);
    float y_before = iui_get_layout_rect(ctx).y;

    /* Nested row box with cross=0 (fill parent slot) */
    iui_sizing_t inner[] = {IUI_GROW(1), IUI_GROW(1)};
    iui_box_begin(ctx, &(iui_box_config_t) {.child_count = 2, .sizes = inner});
    iui_rect_t left = iui_box_next(ctx);
    iui_rect_t right = iui_box_next(ctx);
    iui_box_end(ctx);

    float y_after = iui_get_layout_rect(ctx).y;
    float advance = y_after - y_before;

    /* Row children should fill the 100px slot height */
    ASSERT_NEAR(left.height, slot1.height, 1.0f);
    ASSERT_NEAR(right.height, slot1.height, 1.0f);

    /* box_end should advance by ~100 + padding, NOT by the full container */
    ASSERT_TRUE(advance < slot1.height * 2);
    ASSERT_TRUE(advance > 0);

    iui_rect_t slot2 = iui_box_next(ctx);
    iui_box_end(ctx);

    (void) slot2;

    END_TEST_WINDOW();
}

/* Test Suite Runner */
void run_box_tests(void)
{
    SECTION_BEGIN("Box Container");
    test_box_null_sizes();
    test_box_fixed_grow();
    test_box_percent();
    test_box_column();
    test_box_nested();
    test_box_padding();
    test_box_gap();
    test_box_cross_align();
    test_box_overflow();
    test_box_with_scroll();
    test_box_min_max();
    test_box_grow_max_redistribute();
    test_box_grow_cascading_max();
    test_box_fixed_max_grow_redistribute();
    test_box_cross_align_clamp();
    test_box_grow_no_negative();
    test_size_class();
    test_layout_columns_margins_gutters();
    test_box_nested_column_inherits_height();
    test_box_column_padding_advance();
    test_box_nested_row_inherits_height();
    test_box_column_no_children();
    test_box_row_end_advance();
    SECTION_END();
}
