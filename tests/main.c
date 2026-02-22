/*
 * Test Suite Runner
 */

#include "common.h"

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("Options:\n");
    printf("  -v        Verbose output (show each test name)\n");
    printf("  -vv       Very verbose (show draw calls)\n");
    printf("  -h        Show this help\n");
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(argv[0]);
            return 0;
        }
        if (!strcmp(argv[i], "-v"))
            g_verbose = 1;
        else if (!strcmp(argv[i], "-vv"))
            g_verbose = 2;
    }

    printf("libiui API Test Suite\n");

    /* Create context for demonstration tests */
    void *buffer = malloc(iui_min_memory_size());
    iui_context *ctx = create_test_context(buffer, false);
    if (!ctx) {
        fprintf(stderr, "Failed to create test context\n");
        free(buffer);
        return 1;
    }

    /* Part 1: Demonstration tests */
    run_demo_tests(ctx);
    free(buffer);

    /* Part 2: Unit tests */
    run_init_tests();
    run_bounds_tests();
    run_layout_tests();
    run_widget_tests();
    run_input_tests();
    run_animation_tests();
    run_theme_tests();
    run_vector_tests();
    run_string_tests();
    run_state_machine_tests();
    run_new_component_tests();
    run_modal_tests();
    run_input_layer_tests();
    run_snackbar_tests();
    run_elevation_tests();
    run_scroll_tests();
    run_textfield_icon_tests();
    run_menu_tests();
    run_dialog_tests();
    run_slider_ex_tests();
    run_chip_tests();
    run_spec_tests();
    run_focus_tests();
    run_clip_tests();
    run_field_tracking_tests();
    run_overflow_tests();
    run_navigation_tests();
    run_bottom_sheet_tests();
    run_box_tests();

    /* Summary */
    if (g_tests_failed == 0) {
        printf("  All %d tests passed\n", g_tests_run);
    } else {
        printf("  Results: %d tests, %d passed, %d failed\n", g_tests_run,
               g_tests_passed, g_tests_failed);
    }

    return g_tests_failed > 0 ? 1 : 0;
}
