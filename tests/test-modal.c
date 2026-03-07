/*
 * Modal System Tests
 *
 * Tests for modal, dialog, menu, snackbar, elevation, and textfield icons.
 */

#include "common.h"

/* Modal Blocking Tests */

static void test_modal_blocking_state(void)
{
    TEST(modal_blocking_state);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    ASSERT_FALSE(iui_is_modal_active(ctx));

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    iui_begin_modal(ctx, "test_modal");
    ASSERT_TRUE(iui_is_modal_active(ctx));

    iui_end_modal(ctx);
    ASSERT_TRUE(iui_is_modal_active(ctx));

    iui_close_modal(ctx);
    ASSERT_FALSE(iui_is_modal_active(ctx));

    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_modal_input_blocking(void)
{
    TEST(modal_input_blocking);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Background", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    iui_update_mouse_pos(ctx, 50, 50);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_rect_t test_bounds = {40, 40, 100, 30};
    iui_state_t state_before = iui_get_component_state(ctx, test_bounds, false);
    ASSERT_EQ(state_before, IUI_STATE_PRESSED);

    iui_begin_modal(ctx, "blocking_test");
    iui_end_modal(ctx);

    iui_state_t state_blocked =
        iui_get_component_state(ctx, test_bounds, false);
    ASSERT_EQ(state_blocked, IUI_STATE_DEFAULT);

    iui_close_modal(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_modal_inside_interaction(void)
{
    TEST(modal_inside_interaction);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Background", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    iui_update_mouse_pos(ctx, 50, 50);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_rect_t test_bounds = {40, 40, 100, 30};

    iui_begin_modal(ctx, "interaction_test");

    iui_state_t state_inside = iui_get_component_state(ctx, test_bounds, false);
    ASSERT_EQ(state_inside, IUI_STATE_PRESSED);

    iui_end_modal(ctx);
    iui_close_modal(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_modal_clicked_inside_tracking(void)
{
    TEST(modal_clicked_inside_tracking);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    iui_begin_modal(ctx, "click_test");

    iui_update_mouse_pos(ctx, 50, 50);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_rect_t test_bounds = {40, 40, 100, 30};

    iui_state_t state = iui_get_component_state(ctx, test_bounds, false);
    ASSERT_EQ(state, IUI_STATE_PRESSED);

    iui_end_modal(ctx);

    ASSERT_FALSE(iui_modal_should_close(ctx));

    iui_close_modal(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_modal_nested_guard(void)
{
    TEST(modal_nested_guard);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    iui_begin_modal(ctx, "first_modal");
    ASSERT_TRUE(iui_is_modal_active(ctx));

    iui_begin_modal(ctx, "second_modal");
    ASSERT_TRUE(iui_is_modal_active(ctx));

    iui_end_modal(ctx);
    iui_close_modal(ctx);

    ASSERT_FALSE(iui_is_modal_active(ctx));

    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_modal_extended_functions(void)
{
    TEST(modal_extended_functions);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test modal active check */
    ASSERT_FALSE(iui_is_modal_active(ctx));

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Test modal functions */
    iui_begin_modal(ctx, "test_modal");
    ASSERT_TRUE(iui_is_modal_active(ctx));

    /* Test modal should close (should be false initially) */
    ASSERT_FALSE(iui_modal_should_close(ctx));

    iui_end_modal(ctx);
    iui_close_modal(ctx);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Snackbar Tests */

static void test_snackbar_show_hide(void)
{
    TEST(snackbar_show_hide);

    iui_snackbar_state snackbar = {0};

    ASSERT_FALSE(snackbar.active);

    iui_snackbar_show(&snackbar, "Test message", 3.f, NULL);
    ASSERT_TRUE(snackbar.active);
    ASSERT_TRUE(!strcmp(snackbar.message, "Test message"));
    ASSERT_NEAR(snackbar.duration, 3.f, 0.001f);
    ASSERT_NEAR(snackbar.timer, 3.f, 0.001f);
    ASSERT_TRUE(snackbar.action_label == NULL);

    iui_snackbar_hide(&snackbar);
    ASSERT_FALSE(snackbar.active);
    ASSERT_NEAR(snackbar.timer, 0.f, 0.001f);

    PASS();
}

static void test_snackbar_with_action(void)
{
    TEST(snackbar_with_action);

    iui_snackbar_state snackbar = {0};

    iui_snackbar_show(&snackbar, "Item deleted", 5.f, "Undo");
    ASSERT_TRUE(snackbar.active);
    ASSERT_TRUE(!strcmp(snackbar.action_label, "Undo"));
    ASSERT_NEAR(snackbar.duration, 5.f, 0.001f);

    PASS();
}

static void test_snackbar_auto_dismiss(void)
{
    TEST(snackbar_auto_dismiss);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_snackbar_state snackbar = {0};
    iui_snackbar_show(&snackbar, "Timed message", 1.f, NULL);

    for (int i = 0; i < 50; i++) {
        iui_begin_frame(ctx, 0.05f);
        bool result = iui_snackbar(ctx, &snackbar, 800.f, 600.f);
        iui_end_frame(ctx);
        (void) result;
        if (!snackbar.active)
            break;
    }

    ASSERT_FALSE(snackbar.active);

    free(buffer);
    PASS();
}

static void test_snackbar_persistent(void)
{
    TEST(snackbar_persistent);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_snackbar_state snackbar = {0};

    iui_snackbar_show(&snackbar, "Persistent message", 0.f, NULL);
    ASSERT_TRUE(snackbar.active);

    for (int i = 0; i < 100; i++) {
        iui_begin_frame(ctx, 0.05f);
        iui_snackbar(ctx, &snackbar, 800.f, 600.f);
        iui_end_frame(ctx);
    }

    ASSERT_TRUE(snackbar.active);

    iui_snackbar_hide(&snackbar);
    ASSERT_FALSE(snackbar.active);

    free(buffer);
    PASS();
}

static void test_snackbar_null_safety(void)
{
    TEST(snackbar_null_safety);

    iui_snackbar_show(NULL, "Test", 1.f, NULL);
    iui_snackbar_hide(NULL);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    bool result = iui_snackbar(ctx, NULL, 800.f, 600.f);
    iui_end_frame(ctx);
    ASSERT_FALSE(result);

    iui_snackbar_state snackbar = {0};
    iui_begin_frame(ctx, 0.016f);
    result = iui_snackbar(ctx, &snackbar, 800.f, 600.f);
    iui_end_frame(ctx);
    ASSERT_FALSE(result);

    free(buffer);
    PASS();
}

/* Elevation/Shadow Tests */

static void test_elevation_enum_values(void)
{
    TEST(elevation_enum_values);

    ASSERT_EQ(IUI_ELEVATION_0, 0);
    ASSERT_EQ(IUI_ELEVATION_1, 1);
    ASSERT_EQ(IUI_ELEVATION_2, 2);
    ASSERT_EQ(IUI_ELEVATION_3, 3);
    ASSERT_EQ(IUI_ELEVATION_4, 4);
    ASSERT_EQ(IUI_ELEVATION_5, 5);

    PASS();
}

static void test_draw_shadow_level_zero(void)
{
    TEST(draw_shadow_level_zero);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    reset_counters();
    iui_rect_t bounds = {100, 100, 100, 50};
    iui_draw_shadow(ctx, bounds, 8.f, IUI_ELEVATION_0);

    ASSERT_EQ(g_draw_box_calls, 0);

    iui_end_window(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_draw_shadow_multi_layer(void)
{
    TEST(draw_shadow_multi_layer);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    reset_counters();
    iui_rect_t bounds = {100, 100, 100, 50};
    iui_draw_shadow(ctx, bounds, 8.f, IUI_ELEVATION_3);

    ASSERT_EQ(g_draw_box_calls, 5);

    iui_end_window(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_draw_elevated_box(void)
{
    TEST(draw_elevated_box);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    reset_counters();
    iui_rect_t bounds = {50, 50, 80, 40};
    iui_draw_elevated_box(ctx, bounds, 12.f, IUI_ELEVATION_2, 0xFFAABBCC);

    ASSERT_EQ(g_draw_box_calls, 6);

    ASSERT_NEAR(g_last_box_x, 50.f, 0.1f);
    ASSERT_NEAR(g_last_box_y, 50.f, 0.1f);
    ASSERT_NEAR(g_last_box_w, 80.f, 0.1f);
    ASSERT_NEAR(g_last_box_h, 40.f, 0.1f);
    ASSERT_EQ(g_last_box_color, 0xFFAABBCC);

    iui_end_window(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_IUI_CARD_ELEVATED_has_shadow(void)
{
    TEST(IUI_CARD_ELEVATED_has_shadow);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    reset_counters();
    iui_card_begin(ctx, 100.f, 100.f, 150.f, 100.f, IUI_CARD_ELEVATED);
    iui_card_end(ctx);

    ASSERT_TRUE(g_draw_box_calls >= 6);

    iui_end_window(ctx);
    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_shadow_null_safety(void)
{
    TEST(shadow_null_safety);

    iui_rect_t bounds = {0, 0, 100, 50};
    iui_draw_shadow(NULL, bounds, 8.f, IUI_ELEVATION_3);
    iui_draw_elevated_box(NULL, bounds, 8.f, IUI_ELEVATION_2, 0xFFFFFFFF);

    PASS();
}

/* TextField Icons Tests */

static void test_textfield_basic(void)
{
    TEST(textfield_basic);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    static char text[64] = "Hello";
    static size_t cursor = 5;

    iui_textfield_result result =
        iui_textfield(ctx, text, sizeof(text), &cursor, NULL);
    ASSERT_FALSE(result.value_changed);
    ASSERT_FALSE(result.submitted);
    ASSERT_FALSE(result.leading_icon_clicked);
    ASSERT_FALSE(result.trailing_icon_clicked);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_textfield_with_icons(void)
{
    TEST(textfield_with_icons);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    reset_counters();
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    static char text[64] = "";
    static size_t cursor = 0;

    iui_textfield_options opts = {
        .style = IUI_TEXTFIELD_FILLED,
        .placeholder = "Search...",
        .leading_icon = IUI_TEXTFIELD_ICON_SEARCH,
        .trailing_icon = IUI_TEXTFIELD_ICON_CLEAR,
    };

    iui_textfield_result result =
        iui_textfield(ctx, text, sizeof(text), &cursor, &opts);
    ASSERT_FALSE(result.value_changed);
    ASSERT_FALSE(result.leading_icon_clicked);
    ASSERT_FALSE(result.trailing_icon_clicked);

    ASSERT_TRUE(g_draw_box_calls > 0);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_textfield_password_mode(void)
{
    TEST(textfield_password_mode);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    static char password[64] = "secret123";
    static size_t cursor = 9;

    iui_textfield_options opts = {
        .style = IUI_TEXTFIELD_OUTLINED,
        .placeholder = "Password",
        .trailing_icon = IUI_TEXTFIELD_ICON_VISIBILITY_OFF,
        .password_mode = true,
    };

    iui_textfield_result result =
        iui_textfield(ctx, password, sizeof(password), &cursor, &opts);
    ASSERT_FALSE(result.value_changed);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_textfield_null_safety(void)
{
    TEST(textfield_null_safety);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    size_t cursor = 0;
    iui_textfield_result result = iui_textfield(ctx, NULL, 64, &cursor, NULL);
    ASSERT_FALSE(result.value_changed);

    char text[64] = "test";
    result = iui_textfield(ctx, text, sizeof(text), NULL, NULL);
    ASSERT_FALSE(result.value_changed);

    result = iui_textfield(ctx, text, 0, &cursor, NULL);
    ASSERT_FALSE(result.value_changed);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Menu Component Tests */

static void test_menu_open_close(void)
{
    TEST(menu_open_close);

    iui_menu_state menu = {0};

    ASSERT_FALSE(iui_menu_is_open(&menu));
    ASSERT_FALSE(menu.open);

    iui_menu_open(&menu, "test_menu", 100.f, 50.f);
    ASSERT_TRUE(iui_menu_is_open(&menu));
    ASSERT_TRUE(menu.open);
    ASSERT_NEAR(menu.x, 100.f, 0.1f);
    ASSERT_NEAR(menu.y, 50.f, 0.1f);
    ASSERT_EQ(menu.hovered_index, -1);
    ASSERT_EQ(menu.frames_since_open, 0);

    iui_menu_close(&menu);
    ASSERT_FALSE(iui_menu_is_open(&menu));
    ASSERT_FALSE(menu.open);

    PASS();
}

static void test_menu_null_safety(void)
{
    TEST(menu_null_safety);

    iui_menu_open(NULL, "id", 0, 0);
    iui_menu_close(NULL);
    ASSERT_FALSE(iui_menu_is_open(NULL));

    iui_menu_state menu = {0};
    iui_menu_open(&menu, NULL, 0, 0);
    ASSERT_FALSE(menu.open);

    PASS();
}

static void test_menu_begin_end(void)
{
    TEST(menu_begin_end);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);

    iui_menu_state menu = {0};

    ASSERT_FALSE(iui_menu_begin(ctx, &menu, NULL));

    iui_menu_open(&menu, "test", 10.f, 20.f);
    ASSERT_TRUE(iui_menu_begin(ctx, &menu, NULL));
    /* Frame counter is 0 after begin (incremented in end for click protection)
     */
    ASSERT_EQ(menu.frames_since_open, 0);

    iui_menu_end(ctx, &menu);
    /* After end, frame counter is incremented */
    ASSERT_EQ(menu.frames_since_open, 1);

    iui_end_frame(ctx);
    free(buffer);

    PASS();
}

static void test_menu_item_basic(void)
{
    TEST(menu_item_basic);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);

    iui_menu_state menu = {0};
    iui_menu_open(&menu, "test", 10.f, 20.f);
    ASSERT_TRUE(iui_menu_begin(ctx, &menu, NULL));

    float initial_height = menu.height;

    iui_menu_item item = {.text = "Test Item"};
    bool clicked = iui_menu_add_item(ctx, &menu, &item);
    ASSERT_FALSE(clicked);

    ASSERT_TRUE(menu.height > initial_height);

    iui_menu_end(ctx, &menu);
    iui_end_frame(ctx);
    free(buffer);

    PASS();
}

/* Dialog Component Tests */

static void test_dialog_show_close(void)
{
    TEST(dialog_show_close);

    iui_dialog_state dialog = {0};

    ASSERT_FALSE(iui_dialog_is_open(&dialog));

    iui_dialog_show(&dialog, "Title", "Message", "OK");
    ASSERT_TRUE(iui_dialog_is_open(&dialog));
    ASSERT_NOT_NULL(dialog.title);
    ASSERT_NOT_NULL(dialog.message);
    ASSERT_NOT_NULL(dialog.buttons);
    ASSERT_EQ(dialog.button_count, 1);
    ASSERT_EQ(dialog.selected_button, -1);

    iui_dialog_close(&dialog);
    ASSERT_FALSE(iui_dialog_is_open(&dialog));
    ASSERT_NULL(dialog.title);
    ASSERT_NULL(dialog.message);
    ASSERT_NULL(dialog.buttons);
    ASSERT_EQ(dialog.button_count, 0);

    PASS();
}

static void test_dialog_button_count(void)
{
    TEST(dialog_button_count);

    iui_dialog_state dialog = {0};

    iui_dialog_show(&dialog, "T", "M", "OK");
    ASSERT_EQ(dialog.button_count, 1);
    iui_dialog_close(&dialog);

    iui_dialog_show(&dialog, "T", "M", "Cancel;OK");
    ASSERT_EQ(dialog.button_count, 2);
    iui_dialog_close(&dialog);

    iui_dialog_show(&dialog, "T", "M", "Cancel;Discard;Save");
    ASSERT_EQ(dialog.button_count, 3);
    iui_dialog_close(&dialog);

    iui_dialog_show(&dialog, "T", "M", "");
    ASSERT_EQ(dialog.button_count, 0);
    iui_dialog_close(&dialog);

    iui_dialog_show(&dialog, "T", "M", NULL);
    ASSERT_EQ(dialog.button_count, 0);
    iui_dialog_close(&dialog);

    PASS();
}

static void test_dialog_null_safety(void)
{
    TEST(dialog_null_safety);

    iui_dialog_show(NULL, "Title", "Message", "OK");
    iui_dialog_close(NULL);
    ASSERT_FALSE(iui_dialog_is_open(NULL));

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_begin_frame(ctx, 0.016f);

    int result = iui_dialog(ctx, NULL, 800.f, 600.f);
    ASSERT_EQ(result, -1);

    iui_dialog_state dialog = {0};
    iui_dialog_show(&dialog, "T", "M", "OK");
    result = iui_dialog(NULL, &dialog, 800.f, 600.f);
    ASSERT_EQ(result, -1);

    iui_dialog_close(&dialog);
    result = iui_dialog(ctx, &dialog, 800.f, 600.f);
    ASSERT_EQ(result, -1);

    iui_end_frame(ctx);
    free(buffer);

    PASS();
}

static void test_dialog_render(void)
{
    TEST(dialog_render);

    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_dialog_state dialog = {0};
    iui_dialog_show(&dialog, "Confirm", "Are you sure?", "Cancel;OK");

    iui_begin_frame(ctx, 0.016f);

    reset_counters();
    int result = iui_dialog(ctx, &dialog, 800.f, 600.f);

    ASSERT_EQ(result, -1);

    ASSERT_TRUE(g_draw_box_calls >= 8);

    iui_end_frame(ctx);
    iui_dialog_close(&dialog);
    free(buffer);

    PASS();
}

/* Input Layer System Tests */

static void test_input_layer_initial_state(void)
{
    TEST(input_layer_initial_state);
    void *buffer = malloc(iui_min_memory_size());
    const iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    /* Initial state: no active layers */
    ASSERT_FALSE(iui_has_active_layer(ctx));
    ASSERT_EQ(iui_get_current_layer(ctx), 0);

    free(buffer);
    PASS();
}

static void test_input_layer_push_pop(void)
{
    TEST(input_layer_push_pop);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);

    /* Push first layer */
    int layer1 = iui_push_layer(ctx, 100);
    ASSERT_TRUE(layer1 > 0);
    ASSERT_TRUE(iui_has_active_layer(ctx));
    ASSERT_EQ(iui_get_current_layer(ctx), layer1);

    /* Push second layer */
    int layer2 = iui_push_layer(ctx, 200);
    ASSERT_TRUE(layer2 > 0);
    ASSERT_TRUE(layer2 != layer1);
    ASSERT_EQ(iui_get_current_layer(ctx), layer2);

    /* Pop second layer */
    iui_pop_layer(ctx);
    ASSERT_TRUE(iui_has_active_layer(ctx));

    /* Pop first layer */
    iui_pop_layer(ctx);
    ASSERT_FALSE(iui_has_active_layer(ctx));
    ASSERT_EQ(iui_get_current_layer(ctx), 0);

    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_input_layer_register_region(void)
{
    TEST(input_layer_register_region);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);

    /* Push a layer and register a blocking region */
    int layer = iui_push_layer(ctx, 100);
    ASSERT_TRUE(layer > 0);

    iui_rect_t region = {100, 100, 200, 150};
    iui_register_blocking_region(ctx, region);

    /* Region registered - test passes if no crash */
    iui_pop_layer(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_input_layer_should_process(void)
{
    TEST(input_layer_should_process);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    /* Frame 1: Register blocking region */
    iui_begin_frame(ctx, 0.016f);
    (void) iui_push_layer(ctx, 100);
    iui_rect_t blocking_region = {100, 100, 200, 150};
    iui_register_blocking_region(ctx, blocking_region);

    /* Widget inside blocking region should process input */
    iui_rect_t inside_widget = {150, 125, 50, 30};
    ASSERT_TRUE(iui_should_process_input(ctx, inside_widget));

    iui_pop_layer(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Now blocking region is active (double-buffered) */
    iui_begin_frame(ctx, 0.016f);

    /* No layer active now, all widgets should process */
    iui_rect_t any_widget = {50, 50, 50, 30};
    ASSERT_TRUE(iui_should_process_input(ctx, any_widget));

    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_input_layer_stack_overflow_guard(void)
{
    TEST(input_layer_stack_overflow_guard);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);

    /* Push layers up to limit */
    int layers[IUI_MAX_INPUT_LAYERS + 2];
    int pushed = 0;
    for (int i = 0; i < IUI_MAX_INPUT_LAYERS + 2; i++) {
        layers[i] = iui_push_layer(ctx, i * 10);
        if (layers[i] > 0)
            pushed++;
    }

    /* Should have pushed exactly IUI_MAX_INPUT_LAYERS */
    ASSERT_EQ(pushed, IUI_MAX_INPUT_LAYERS);

    /* Pop all layers */
    for (int i = 0; i < IUI_MAX_INPUT_LAYERS; i++) {
        iui_pop_layer(ctx);
    }

    /* Extra pops should be safe */
    iui_pop_layer(ctx);
    iui_pop_layer(ctx);

    ASSERT_FALSE(iui_has_active_layer(ctx));

    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_input_layer_with_modal_compat(void)
{
    TEST(input_layer_with_modal_compat);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    iui_begin_frame(ctx, 0.016f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_end_window(ctx);

    /* Legacy modal blocking should still work */
    iui_begin_modal(ctx, "test_modal");
    iui_end_modal(ctx);

    /* Widget outside modal should be blocked by legacy modal system */
    iui_rect_t widget_bounds = {50, 50, 100, 30};
    ASSERT_FALSE(iui_should_process_input(ctx, widget_bounds));

    iui_close_modal(ctx);

    /* After closing modal, widget should process input */
    ASSERT_TRUE(iui_should_process_input(ctx, widget_bounds));

    iui_end_frame(ctx);
    free(buffer);
    PASS();
}

static void test_input_layer_double_buffer(void)
{
    TEST(input_layer_double_buffer);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_TRUE(ctx != NULL);

    /* Frame 1: Register region */
    iui_begin_frame(ctx, 0.016f);
    (void) iui_push_layer(ctx, 100);
    iui_rect_t region = {100, 100, 200, 150};
    iui_register_blocking_region(ctx, region);
    iui_pop_layer(ctx);
    iui_end_frame(ctx);

    /* Frame 2: Region should now be blocking */
    iui_begin_frame(ctx, 0.016f);
    /* Without active layer, should still process */
    iui_rect_t widget = {50, 50, 30, 30};
    ASSERT_TRUE(iui_should_process_input(ctx, widget));
    iui_end_frame(ctx);

    /* Frame 3: Old regions should be cleared */
    iui_begin_frame(ctx, 0.016f);
    ASSERT_TRUE(iui_should_process_input(ctx, widget));
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_input_layer_system(void)
{
    TEST(input_layer_system);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test initial state */
    ASSERT_FALSE(iui_has_active_layer(ctx));
    ASSERT_EQ(iui_get_current_layer(ctx), 0);
    ASSERT_EQ(iui_get_layer_depth(ctx), 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Test layer pushing */
    int layer = iui_push_layer(ctx, 100);
    ASSERT_TRUE(layer > 0);

    ASSERT_TRUE(iui_has_active_layer(ctx));
    ASSERT_TRUE(iui_get_current_layer(ctx) > 0);
    ASSERT_EQ(iui_get_layer_depth(ctx), 1);

    /* Test blocking region registration */
    iui_rect_t bounds = {0, 0, 100, 100};
    bool success = iui_register_blocking_region(ctx, bounds);
    ASSERT_TRUE(success);

    /* Test input processing */
    bool should_process = iui_should_process_input(ctx, bounds);
    /* Should process since rendering is in the same layer */
    ASSERT_TRUE(should_process);

    iui_pop_layer(ctx);

    ASSERT_FALSE(iui_has_active_layer(ctx));
    ASSERT_EQ(iui_get_current_layer(ctx), 0);
    ASSERT_EQ(iui_get_layer_depth(ctx), 0);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test Suite Runners */

void run_input_layer_tests(void)
{
    SECTION_BEGIN("Input Layer System");
    test_input_layer_initial_state();
    test_input_layer_push_pop();
    test_input_layer_register_region();
    test_input_layer_should_process();
    test_input_layer_stack_overflow_guard();
    test_input_layer_with_modal_compat();
    test_input_layer_double_buffer();
    test_input_layer_system();
    SECTION_END();
}

void run_modal_tests(void)
{
    SECTION_BEGIN("Modal Blocking");
    test_modal_blocking_state();
    test_modal_input_blocking();
    test_modal_inside_interaction();
    test_modal_clicked_inside_tracking();
    test_modal_nested_guard();
    test_modal_extended_functions();
    SECTION_END();
}

void run_snackbar_tests(void)
{
    SECTION_BEGIN("Snackbar");
    test_snackbar_show_hide();
    test_snackbar_with_action();
    test_snackbar_auto_dismiss();
    test_snackbar_persistent();
    test_snackbar_null_safety();
    SECTION_END();
}

void run_elevation_tests(void)
{
    SECTION_BEGIN("Elevation/Shadow");
    test_elevation_enum_values();
    test_draw_shadow_level_zero();
    test_draw_shadow_multi_layer();
    test_draw_elevated_box();
    test_IUI_CARD_ELEVATED_has_shadow();
    test_shadow_null_safety();
    SECTION_END();
}

void run_textfield_icon_tests(void)
{
    SECTION_BEGIN("TextField Icons");
    test_textfield_basic();
    test_textfield_with_icons();
    test_textfield_password_mode();
    test_textfield_null_safety();
    SECTION_END();
}

void run_menu_tests(void)
{
    SECTION_BEGIN("Menu Component");
    test_menu_open_close();
    test_menu_null_safety();
    test_menu_begin_end();
    test_menu_item_basic();
    SECTION_END();
}

void run_dialog_tests(void)
{
    SECTION_BEGIN("Dialog Component");
    test_dialog_show_close();
    test_dialog_button_count();
    test_dialog_null_safety();
    test_dialog_render();
    SECTION_END();
}
