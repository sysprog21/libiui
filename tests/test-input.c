/*
 * Input Handling Tests
 *
 * Tests for mouse, keyboard, and button state handling.
 */

#include "common.h"

/* Mouse Input Tests */

static void test_mouse_negative_coords(void)
{
    TEST(mouse_negative_coords);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_update_mouse_pos(ctx, -100.0f, -50.0f);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

static void test_mouse_large_coords(void)
{
    TEST(mouse_large_coords);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    iui_update_mouse_pos(ctx, 100000.0f, 100000.0f);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Keyboard Input Tests */

static void test_keyboard_edit_navigation(void)
{
    TEST(keyboard_edit_navigation);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 2;

    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Test Home key */
    iui_update_key(ctx, IUI_KEY_HOME);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, 0);

    /* Test End key */
    iui_update_key(ctx, IUI_KEY_END);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, strlen(text_buf));

    free(buffer);
    PASS();
}

static void test_keyboard_word_navigation(void)
{
    TEST(keyboard_word_navigation);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "hello world test";
    size_t cursor = 0;

    /* Focus the edit field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Test Ctrl+Right: move to next word boundary */
    iui_update_modifiers(ctx, IUI_MOD_CTRL);
    iui_update_key(ctx, IUI_KEY_RIGHT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, 6); /* After "hello " */

    /* Test Ctrl+Right again: move to next word */
    iui_update_key(ctx, IUI_KEY_RIGHT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, 12); /* After "world " */

    /* Test Ctrl+Left: move to previous word boundary */
    iui_update_key(ctx, IUI_KEY_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, 6); /* Back to start of "world" */

    /* Clear modifiers and verify normal navigation */
    iui_update_modifiers(ctx, IUI_MOD_NONE);
    iui_update_key(ctx, IUI_KEY_RIGHT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_EQ(cursor, 7); /* Single character movement */

    free(buffer);
    PASS();
}

static void test_keyboard_delete_at_end(void)
{
    TEST(keyboard_delete_at_end);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 5;

    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    iui_update_key(ctx, IUI_KEY_DELETE);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(strcmp(text_buf, "Hello"), 0);

    free(buffer);
    PASS();
}

static void test_keyboard_backspace_at_start(void)
{
    TEST(keyboard_backspace_at_start);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[32] = "Hello";
    size_t cursor = 0;

    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    iui_update_key(ctx, IUI_KEY_BACKSPACE);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield(ctx, text_buf, sizeof(text_buf), &cursor, NULL);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(strcmp(text_buf, "Hello"), 0);
    ASSERT_EQ(cursor, 0);

    free(buffer);
    PASS();
}

/* Text Selection Tests */

static void test_edit_state_initialization(void)
{
    TEST(edit_state_initialization);
    iui_edit_state state = {0};

    /* Verify zero initialization */
    ASSERT_EQ(state.cursor, 0);
    ASSERT_EQ(state.selection_start, 0);
    ASSERT_EQ(state.selection_end, 0);
    ASSERT_EQ(state.scroll_offset, 0.0f);
    ASSERT_EQ(state.last_click_count, 0);
    ASSERT_EQ(state.is_dragging, false);

    PASS();
}

static void test_edit_with_selection_basic(void)
{
    TEST(edit_with_selection_basic);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};
    state.cursor = 5;

    /* Focus the edit field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Verify state is maintained */
    ASSERT_EQ(strcmp(text_buf, "Hello World"), 0);

    free(buffer);
    PASS();
}

static void test_shift_arrow_selection(void)
{
    TEST(shift_arrow_selection);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};
    state.cursor = 5;
    state.selection_start = 5;
    state.selection_end = 5;

    /* Focus the edit field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Test Shift+Right: extend selection right */
    iui_update_modifiers(ctx, IUI_MOD_SHIFT);
    iui_update_key(ctx, IUI_KEY_RIGHT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Selection should now span one character */
    ASSERT_TRUE(state.selection_start != state.selection_end ||
                state.cursor != 5);

    /* Test Shift+Left: extend selection left */
    state.cursor = 5;
    state.selection_start = 5;
    state.selection_end = 5;
    iui_update_key(ctx, IUI_KEY_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(state.selection_start != state.selection_end ||
                state.cursor != 5);

    iui_update_modifiers(ctx, IUI_MOD_NONE);

    free(buffer);
    PASS();
}

static void test_shift_home_end_selection(void)
{
    TEST(shift_home_end_selection);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};
    state.cursor = 5;
    state.selection_start = 5;
    state.selection_end = 5;

    /* Focus the edit field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    /* Test Shift+End: select to end */
    iui_update_modifiers(ctx, IUI_MOD_SHIFT);
    iui_update_key(ctx, IUI_KEY_END);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Selection should span from original cursor to end */
    ASSERT_EQ(state.cursor, strlen(text_buf));
    ASSERT_EQ(state.selection_end, strlen(text_buf));

    /* Test Shift+Home: select to start */
    state.cursor = 5;
    state.selection_start = 5;
    state.selection_end = 5;
    iui_update_key(ctx, IUI_KEY_HOME);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(state.cursor, 0);
    ASSERT_EQ(state.selection_start, 0);

    iui_update_modifiers(ctx, IUI_MOD_NONE);

    free(buffer);
    PASS();
}

static void test_delete_selection(void)
{
    TEST(delete_selection);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};

    /* Focus the edit field first */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Release mouse to end click and clear mouse_held/is_dragging */
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Now set selection state after focus is established and mouse released */
    state.cursor = 8;
    state.selection_start = 5; /* Select " Wor" */
    state.selection_end = 9;

    /* Press backspace to delete selection */
    iui_update_key(ctx, IUI_KEY_BACKSPACE);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    bool modified =
        iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(modified);
    ASSERT_EQ(strcmp(text_buf, "Hellold"), 0);
    ASSERT_EQ(state.cursor, 5);
    ASSERT_EQ(state.selection_start,
              state.selection_end); /* Selection cleared */

    free(buffer);
    PASS();
}

static void test_type_replaces_selection(void)
{
    TEST(type_replaces_selection);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};

    /* Focus the edit field first */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Release mouse to end click and clear mouse_held/is_dragging */
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Now set selection state after focus is established and mouse released */
    state.cursor = 5;
    state.selection_start = 0; /* Select entire "Hello" */
    state.selection_end = 5;

    /* Type 'X' to replace selection */
    iui_update_char(ctx, 'X');
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    bool modified =
        iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_TRUE(modified);
    ASSERT_EQ(strcmp(text_buf, "X World"), 0);
    ASSERT_EQ(state.cursor, 1);
    ASSERT_EQ(state.selection_start, state.selection_end);

    free(buffer);
    PASS();
}

static void test_selection_constants(void)
{
    TEST(selection_constants);

    /* Verify MD3 selection highlight opacity (40%) */
    ASSERT_EQ(IUI_SELECTION_ALPHA, 0x66); /* 40% ≈ 102/255 */

    /* Verify double/triple click timing */
    ASSERT_TRUE(IUI_DOUBLE_CLICK_TIME > 0.0f);
    ASSERT_TRUE(IUI_TRIPLE_CLICK_TIME > 0.0f);

    PASS();
}

static void test_textfield_with_selection(void)
{
    TEST(textfield_with_selection);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Test Input";
    iui_edit_state state = {0};
    state.cursor = 4;

    iui_textfield_options opts = {
        .style = IUI_TEXTFIELD_FILLED,
        .placeholder = "Enter text...",
    };

    /* Focus the edit field */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_textfield_result result = iui_textfield_with_selection(
        ctx, text_buf, sizeof(text_buf), &state, &opts);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* Verify field was rendered without crash */
    ASSERT_EQ(strcmp(text_buf, "Test Input"), 0);
    (void) result; /* Suppress unused warning */

    free(buffer);
    PASS();
}

/* UTF-8 String Handling Tests */

static void test_utf8_codepoint_length(void)
{
    TEST(utf8_codepoint_length);

    /* ASCII: 1 byte */
    ASSERT_EQ(iui_utf8_codepoint_len('A'), 1);
    ASSERT_EQ(iui_utf8_codepoint_len(' '), 1);
    ASSERT_EQ(iui_utf8_codepoint_len(0x7F), 1);

    /* 2-byte sequences (110xxxxx): Latin Extended, Greek, etc. */
    ASSERT_EQ(iui_utf8_codepoint_len(0xC3), 2); /* é, ñ, etc. */
    ASSERT_EQ(iui_utf8_codepoint_len(0xCE), 2); /* Greek */

    /* 3-byte sequences (1110xxxx): CJK, most Unicode */
    ASSERT_EQ(iui_utf8_codepoint_len(0xE4), 3); /* Chinese */
    ASSERT_EQ(iui_utf8_codepoint_len(0xE3), 3); /* Japanese */

    /* 4-byte sequences (11110xxx): Emoji, rare scripts */
    ASSERT_EQ(iui_utf8_codepoint_len(0xF0), 4); /* Emoji */

    /* Continuation bytes and invalid: treat as single */
    ASSERT_EQ(iui_utf8_codepoint_len(0x80), 1); /* Continuation */
    ASSERT_EQ(iui_utf8_codepoint_len(0xFE), 1); /* Invalid */

    PASS();
}

static void test_utf8_continuation_detection(void)
{
    TEST(utf8_continuation_detection);

    /* Continuation bytes: 10xxxxxx (0x80-0xBF) */
    ASSERT_TRUE(iui_utf8_is_continuation(0x80));
    ASSERT_TRUE(iui_utf8_is_continuation(0x8F));
    ASSERT_TRUE(iui_utf8_is_continuation(0xBF));

    /* Non-continuation bytes */
    ASSERT_FALSE(iui_utf8_is_continuation(0x00)); /* ASCII NUL */
    ASSERT_FALSE(iui_utf8_is_continuation('A'));  /* ASCII */
    ASSERT_FALSE(iui_utf8_is_continuation(0x7F)); /* DEL */
    ASSERT_FALSE(iui_utf8_is_continuation(0xC0)); /* 2-byte lead */
    ASSERT_FALSE(iui_utf8_is_continuation(0xE0)); /* 3-byte lead */
    ASSERT_FALSE(iui_utf8_is_continuation(0xF0)); /* 4-byte lead */

    PASS();
}

static void test_utf8_navigation(void)
{
    TEST(utf8_navigation);

    /* Test string: "Café日本" = C a f é (2b) 日 (3b) 本 (3b) */
    /* Bytes: C(1) a(1) f(1) é(C3 A9) 日(E6 97 A5) 本(E6 9C AC) */
    const char *s = "Caf\xC3\xA9\xE6\x97\xA5\xE6\x9C\xAC";
    size_t len = strlen(s); /* 12 bytes */

    /* Forward navigation */
    size_t pos = 0;
    pos = iui_utf8_next(s, pos, len); /* After C */
    ASSERT_EQ(pos, 1);
    pos = iui_utf8_next(s, pos, len); /* After a */
    ASSERT_EQ(pos, 2);
    pos = iui_utf8_next(s, pos, len); /* After f */
    ASSERT_EQ(pos, 3);
    pos = iui_utf8_next(s, pos, len); /* After é (skip 2 bytes) */
    ASSERT_EQ(pos, 5);
    pos = iui_utf8_next(s, pos, len); /* After 日 (skip 3 bytes) */
    ASSERT_EQ(pos, 8);
    pos = iui_utf8_next(s, pos, len); /* After 本 (skip 3 bytes) */
    ASSERT_EQ(pos, 11);
    pos = iui_utf8_next(s, pos, len); /* At end - stays at len */
    ASSERT_EQ(pos, len);

    /* Backward navigation */
    pos = len;
    pos = iui_utf8_prev(s, pos); /* Before 本 */
    ASSERT_EQ(pos, 8);
    pos = iui_utf8_prev(s, pos); /* Before 日 */
    ASSERT_EQ(pos, 5);
    pos = iui_utf8_prev(s, pos); /* Before é */
    ASSERT_EQ(pos, 3);
    pos = iui_utf8_prev(s, pos); /* Before f */
    ASSERT_EQ(pos, 2);
    pos = iui_utf8_prev(s, pos); /* Before a */
    ASSERT_EQ(pos, 1);
    pos = iui_utf8_prev(s, pos); /* Before C */
    ASSERT_EQ(pos, 0);
    pos = iui_utf8_prev(s, pos); /* At start - stays at 0 */
    ASSERT_EQ(pos, 0);

    PASS();
}

static void test_utf8_decode(void)
{
    TEST(utf8_decode);

    /* ASCII */
    const char *ascii = "ABC";
    ASSERT_EQ(iui_utf8_decode(ascii, 0, 3), 'A');
    ASSERT_EQ(iui_utf8_decode(ascii, 1, 3), 'B');

    /* 2-byte: é = U+00E9 */
    const char *two = "\xC3\xA9"; /* é */
    ASSERT_EQ(iui_utf8_decode(two, 0, 2), 0x00E9);

    /* 3-byte: 日 = U+65E5 */
    const char *three = "\xE6\x97\xA5"; /* 日 */
    ASSERT_EQ(iui_utf8_decode(three, 0, 3), 0x65E5);

    /* 4-byte: 😀 = U+1F600 */
    const char *four = "\xF0\x9F\x98\x80"; /* 😀 */
    ASSERT_EQ(iui_utf8_decode(four, 0, 4), 0x1F600);

    /* Out of bounds returns U+FFFD */
    ASSERT_EQ(iui_utf8_decode(ascii, 5, 3), 0xFFFD);

    /* Truncated sequences return U+FFFD */
    ASSERT_EQ(iui_utf8_decode(two, 0, 1), 0xFFFD);   /* 2-byte truncated */
    ASSERT_EQ(iui_utf8_decode(three, 0, 2), 0xFFFD); /* 3-byte truncated */
    ASSERT_EQ(iui_utf8_decode(four, 0, 3), 0xFFFD);  /* 4-byte truncated */

    PASS();
}

static void test_utf8_encode(void)
{
    TEST(utf8_encode);
    char buf[5] = {0};

    /* ASCII */
    ASSERT_EQ(iui_utf8_encode('A', buf), 1);
    ASSERT_EQ(buf[0], 'A');

    /* 2-byte: é = U+00E9 */
    ASSERT_EQ(iui_utf8_encode(0x00E9, buf), 2);
    ASSERT_EQ((unsigned char) buf[0], 0xC3);
    ASSERT_EQ((unsigned char) buf[1], 0xA9);

    /* 3-byte: 日 = U+65E5 */
    ASSERT_EQ(iui_utf8_encode(0x65E5, buf), 3);
    ASSERT_EQ((unsigned char) buf[0], 0xE6);
    ASSERT_EQ((unsigned char) buf[1], 0x97);
    ASSERT_EQ((unsigned char) buf[2], 0xA5);

    /* 4-byte: 😀 = U+1F600 */
    ASSERT_EQ(iui_utf8_encode(0x1F600, buf), 4);
    ASSERT_EQ((unsigned char) buf[0], 0xF0);
    ASSERT_EQ((unsigned char) buf[1], 0x9F);
    ASSERT_EQ((unsigned char) buf[2], 0x98);
    ASSERT_EQ((unsigned char) buf[3], 0x80);

    /* Surrogates (U+D800..U+DFFF) encode as U+FFFD replacement char */
    ASSERT_EQ(iui_utf8_encode(0xD800, buf), 3); /* Low surrogate start */
    ASSERT_EQ((unsigned char) buf[0], 0xEF);
    ASSERT_EQ((unsigned char) buf[1], 0xBF);
    ASSERT_EQ((unsigned char) buf[2], 0xBD);

    ASSERT_EQ(iui_utf8_encode(0xDFFF, buf), 3); /* High surrogate end */
    ASSERT_EQ((unsigned char) buf[0], 0xEF);
    ASSERT_EQ((unsigned char) buf[1], 0xBF);
    ASSERT_EQ((unsigned char) buf[2], 0xBD);

    /* Values > U+10FFFF encode as U+FFFD */
    ASSERT_EQ(iui_utf8_encode(0x110000, buf), 3);
    ASSERT_EQ((unsigned char) buf[0], 0xEF);
    ASSERT_EQ((unsigned char) buf[1], 0xBF);
    ASSERT_EQ((unsigned char) buf[2], 0xBD);

    PASS();
}

static void test_utf8_strlen(void)
{
    TEST(utf8_strlen);

    /* ASCII only */
    ASSERT_EQ(iui_utf8_strlen("Hello"), 5);

    /* Mixed: "Café日本" = 6 code points */
    ASSERT_EQ(iui_utf8_strlen("Caf\xC3\xA9\xE6\x97\xA5\xE6\x9C\xAC"), 6);

    /* Empty string */
    ASSERT_EQ(iui_utf8_strlen(""), 0);

    /* Emoji: 😀 = 1 code point */
    ASSERT_EQ(iui_utf8_strlen("\xF0\x9F\x98\x80"), 1);

    PASS();
}

static void test_utf8_word_char(void)
{
    TEST(utf8_word_char);

    /* ASCII word chars */
    ASSERT_TRUE(iui_utf8_is_word_char('a'));
    ASSERT_TRUE(iui_utf8_is_word_char('Z'));
    ASSERT_TRUE(iui_utf8_is_word_char('0'));
    ASSERT_TRUE(iui_utf8_is_word_char('_'));

    /* ASCII non-word chars */
    ASSERT_FALSE(iui_utf8_is_word_char(' '));
    ASSERT_FALSE(iui_utf8_is_word_char('.'));
    ASSERT_FALSE(iui_utf8_is_word_char('-'));

    /* Latin Extended (accented letters) */
    ASSERT_TRUE(iui_utf8_is_word_char(0x00E9)); /* é */
    ASSERT_TRUE(iui_utf8_is_word_char(0x00F1)); /* ñ */

    /* CJK (Chinese/Japanese/Korean) */
    ASSERT_TRUE(iui_utf8_is_word_char(0x65E5)); /* 日 */
    ASSERT_TRUE(iui_utf8_is_word_char(0x672C)); /* 本 */

    /* Hiragana */
    ASSERT_TRUE(iui_utf8_is_word_char(0x3042)); /* あ */

    /* Katakana */
    ASSERT_TRUE(iui_utf8_is_word_char(0x30A2)); /* ア */

    PASS();
}

static void test_selection_left_arrow(void)
{
    TEST(selection_left_arrow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};

    /* Focus and set selection */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    state.cursor = 8;
    state.selection_start = 5;
    state.selection_end = 8; /* " Wo" selected */

    /* Press Left: should move cursor to selection_start and clear selection */
    iui_update_key(ctx, IUI_KEY_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    /* OLD behavior: cursor = 5
       NEW behavior: cursor = 4 (because it moved left AGAIN) */
    ASSERT_EQ(state.cursor, 5);
    ASSERT_EQ(state.selection_start, 5);
    ASSERT_EQ(state.selection_end, 5);

    free(buffer);
    PASS();
}

static void test_selection_right_arrow(void)
{
    TEST(selection_right_arrow);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    char text_buf[64] = "Hello World";
    iui_edit_state state = {0};

    /* Focus and set selection */
    iui_update_mouse_pos(ctx, 200.0f, 150.0f);
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);

    state.cursor = 5;
    state.selection_start = 5;
    state.selection_end = 8; /* " Wo" selected */

    /* Press Right: should move cursor to selection_end and clear selection */
    iui_update_key(ctx, IUI_KEY_RIGHT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    iui_edit_with_selection(ctx, text_buf, sizeof(text_buf), &state);
    iui_end_window(ctx);
    iui_end_frame(ctx);

    ASSERT_EQ(state.cursor, 8);
    ASSERT_EQ(state.selection_start, 8);
    ASSERT_EQ(state.selection_end, 8);

    free(buffer);
    PASS();
}

/* Button State Tests */

static void test_button_state_sequence(void)
{
    TEST(button_state_sequence);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    bool clicked = false;

    iui_update_mouse_pos(ctx, 250.0f, 150.0f);

    /* Idle state */
    iui_update_mouse_buttons(ctx, 0, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    clicked = iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_FALSE(clicked);

    /* Pressed state */
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT, 0);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    clicked = iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_TRUE(clicked);

    /* Released state */
    iui_update_mouse_buttons(ctx, 0, IUI_MOUSE_LEFT);
    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 100, 100, 300, 200, 0);
    clicked = iui_button(ctx, "Test", IUI_ALIGN_CENTER);
    iui_end_window(ctx);
    iui_end_frame(ctx);
    ASSERT_FALSE(clicked);

    free(buffer);
    PASS();
}

static void test_input_update_functions(void)
{
    TEST(input_update_functions);
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    ASSERT_NOT_NULL(ctx);

    /* Test mouse button update with multiple buttons */
    iui_update_mouse_buttons(ctx, IUI_MOUSE_LEFT | IUI_MOUSE_RIGHT, 0);

    /* Test character update */
    iui_update_char(ctx, 'A');

    /* Test modifier update */
    iui_update_modifiers(ctx, IUI_MOD_CTRL | IUI_MOD_SHIFT);

    iui_begin_frame(ctx, 1.0f / 60.0f);
    iui_begin_window(ctx, "Test", 0, 0, 400, 300, 0);

    /* Use the updated input state */
    iui_button(ctx, "Test", IUI_ALIGN_CENTER);

    iui_end_window(ctx);
    iui_end_frame(ctx);

    free(buffer);
    PASS();
}

/* Test Suite Runner */

void run_input_tests(void)
{
    SECTION_BEGIN("Input Handling");
    test_mouse_negative_coords();
    test_mouse_large_coords();
    test_keyboard_edit_navigation();
    test_keyboard_word_navigation();
    test_keyboard_delete_at_end();
    test_keyboard_backspace_at_start();
    test_selection_left_arrow();
    test_selection_right_arrow();
    test_button_state_sequence();
    test_input_update_functions();
    SECTION_END();

    SECTION_BEGIN("Text Selection");
    test_edit_state_initialization();
    test_edit_with_selection_basic();
    test_shift_arrow_selection();
    test_shift_home_end_selection();
    test_delete_selection();
    test_type_replaces_selection();
    test_selection_constants();
    test_textfield_with_selection();
    SECTION_END();

    SECTION_BEGIN("UTF-8 String Handling");
    test_utf8_codepoint_length();
    test_utf8_continuation_detection();
    test_utf8_navigation();
    test_utf8_decode();
    test_utf8_encode();
    test_utf8_strlen();
    test_utf8_word_char();
    SECTION_END();
}
