/* Date and Time Picker components implementation */

#include "internal.h"

/* Calendar helpers */

static bool iui_is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* Get days in month (1-12) */
static int iui_days_in_month(int year, int month)
{
    static const int days[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
    };
    if (month < 1 || month > 12)
        return 0;
    if (month == 2 && iui_is_leap_year(year))
        return 29;
    return days[month];
}

/* Get day of week for given date (0=Sunday, 6=Saturday)
 * Uses Zeller's congruence algorithm
 */
static int iui_day_of_week(int year, int month, int day)
{
    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100, j = year / 100,
        h = (day + (13 * (month + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    /* Convert from Zeller's (0=Saturday) to standard (0=Sunday) */
    return ((h + 6) % 7);
}

/* Month names for display */
static const char *iui_month_names[] = {
    "",     "January", "February",  "March",   "April",    "May",      "June",
    "July", "August",  "September", "October", "November", "December",
};

/* Short weekday names */
static const char *iui_weekday_short[] = {"S", "M", "T", "W", "T", "F", "S"};

/* Date Picker */

void iui_date_picker_show(iui_date_picker_state *picker,
                          int year,
                          int month,
                          int day)
{
    if (!picker)
        return;

    /* Validate and clamp input */
    if (year < 1900)
        year = 1900;
    if (year > 2100)
        year = 2100;
    if (month < 1)
        month = 1;
    if (month > 12)
        month = 12;
    int max_day = iui_days_in_month(year, month);
    if (day < 1)
        day = 1;
    if (day > max_day)
        day = max_day;

    picker->year = year;
    picker->month = month;
    picker->day = day;
    picker->view_year = year;
    picker->view_month = month;
    picker->is_open = true;
    picker->frames_since_open = 0;
    picker->confirmed = false;
}

void iui_date_picker_close(iui_date_picker_state *picker)
{
    if (!picker)
        return;

    picker->is_open = false;
    picker->frames_since_open = 0;
    picker->confirmed = false;
}

bool iui_date_picker_is_open(const iui_date_picker_state *picker)
{
    return picker && picker->is_open;
}

bool iui_date_picker(iui_context *ctx,
                     iui_date_picker_state *picker,
                     float screen_width,
                     float screen_height)
{
    if (!ctx || !picker || !picker->is_open)
        return false;

    /* Capture frame count BEFORE incrementing for click protection.
     * This ensures the first frame after opening (frames_active == 0) blocks
     * clicks, preventing click-through from the button that opened this picker.
     */
    int frames_active = picker->frames_since_open++;

    /* Calculate dialog dimensions using MD3 specs */
    float day_w = IUI_DATE_PICKER_DAY_WIDTH;
    float day_h = IUI_DATE_PICKER_DAY_HEIGHT;
    float day_corner = IUI_DATE_PICKER_DAY_CORNER;
    float padding = IUI_DATE_PICKER_PADDING;
    float weekday_h = IUI_DATE_PICKER_WEEKDAY_HEIGHT;
    float nav_h = IUI_DATE_PICKER_NAV_HEIGHT;
    float nav_btn_size = IUI_DATE_PICKER_NAV_BUTTON_SIZE;
    float touch_target = IUI_DATE_PICKER_TOUCH_TARGET;
    float button_h = 40.f; /* Confirmation button height */

    /* Grid: 7 columns (days) x 6 rows max - use touch target for cell spacing
     */
    float cell_w = touch_target; /* MD3: 48dp touch target per cell */
    float cell_h = touch_target;
    float grid_w = cell_w * 7.f;
    float grid_h = cell_h * 6.f;

    float dialog_w = grid_w + padding * 2.f;
    float dialog_h =
        padding + nav_h + weekday_h + grid_h + padding + button_h + padding;

    /* Center on screen */
    float dialog_x = (screen_width - dialog_w) * 0.5f;
    float dialog_y = (screen_height - dialog_h) * 0.5f;

    /* Begin modal blocking */
    iui_begin_modal(ctx, "date_picker_modal");

    /* Register blocking region for input layer system */
    iui_rect_t dialog_rect = {dialog_x, dialog_y, dialog_w, dialog_h};
    iui_register_blocking_region(ctx, dialog_rect);

    /* Draw scrim */
    iui_rect_t scrim_rect = {0, 0, screen_width, screen_height};
    ctx->renderer.draw_box(scrim_rect, 0, ctx->colors.scrim,
                           ctx->renderer.user);

    /* Draw dialog shadow */
    float corner = IUI_DIALOG_CORNER_RADIUS;
    iui_draw_shadow(ctx, dialog_rect, corner, IUI_ELEVATION_3);

    /* Draw dialog background */
    ctx->renderer.draw_box(dialog_rect, corner,
                           ctx->colors.surface_container_high,
                           ctx->renderer.user);

    /* Navigation: Month Year with arrows (MD3 uses nav_h for this row) */
    float nav_y = dialog_y + padding;
    float nav_x = dialog_x + padding;

    /* Month/Year text */
    char header_text[64];
    snprintf(header_text, sizeof(header_text), "%s %d",
             iui_month_names[picker->view_month], picker->view_year);
    float header_text_w = iui_get_text_width(ctx, header_text);
    float header_text_x = dialog_x + (dialog_w - header_text_w) * 0.5f;
    float header_text_y = nav_y + (nav_h - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, header_text_x, header_text_y, header_text,
                           ctx->colors.on_surface);

    /* Previous month button (left arrow) */
    iui_rect_t prev_btn = {nav_x, nav_y + (nav_h - nav_btn_size) * 0.5f,
                           nav_btn_size, nav_btn_size};
    iui_state_t prev_state = iui_get_component_state(ctx, prev_btn, false);
    iui_draw_state_layer(ctx, prev_btn, nav_btn_size * 0.5f,
                         ctx->colors.on_surface, prev_state);
    /* Draw left arrow */
    float cx_prev = prev_btn.x + nav_btn_size * 0.5f;
    float cy_prev = prev_btn.y + nav_btn_size * 0.5f;
    float arrow_size = 8.f;
    iui_draw_line_soft(ctx, cx_prev + arrow_size * 0.3f, cy_prev - arrow_size,
                       cx_prev - arrow_size * 0.5f, cy_prev, 2.f,
                       ctx->colors.on_surface);
    iui_draw_line_soft(ctx, cx_prev - arrow_size * 0.5f, cy_prev,
                       cx_prev + arrow_size * 0.3f, cy_prev + arrow_size, 2.f,
                       ctx->colors.on_surface);

    /* Next month button (right arrow) */
    iui_rect_t next_btn = {dialog_x + dialog_w - padding - nav_btn_size,
                           nav_y + (nav_h - nav_btn_size) * 0.5f, nav_btn_size,
                           nav_btn_size};
    iui_state_t next_state = iui_get_component_state(ctx, next_btn, false);
    iui_draw_state_layer(ctx, next_btn, nav_btn_size * 0.5f,
                         ctx->colors.on_surface, next_state);
    /* Draw right arrow */
    float cx_next = next_btn.x + nav_btn_size * 0.5f;
    float cy_next = next_btn.y + nav_btn_size * 0.5f;
    iui_draw_line_soft(ctx, cx_next - arrow_size * 0.3f, cy_next - arrow_size,
                       cx_next + arrow_size * 0.5f, cy_next, 2.f,
                       ctx->colors.on_surface);
    iui_draw_line_soft(ctx, cx_next + arrow_size * 0.5f, cy_next,
                       cx_next - arrow_size * 0.3f, cy_next + arrow_size, 2.f,
                       ctx->colors.on_surface);

    /* Handle navigation clicks - use pre-increment value to ensure first frame
     * after opening is protected against click-through
     */
    if (frames_active >= 1) {
        if (prev_state == IUI_STATE_PRESSED) {
            picker->view_month--;
            if (picker->view_month < 1) {
                picker->view_month = 12;
                picker->view_year--;
            }
        }
        if (next_state == IUI_STATE_PRESSED) {
            picker->view_month++;
            if (picker->view_month > 12) {
                picker->view_month = 1;
                picker->view_year++;
            }
        }
    }

    /* Weekday labels */
    float weekday_y = nav_y + nav_h;
    for (int i = 0; i < 7; i++) {
        float wx = dialog_x + padding + i * cell_w;
        float label_w = iui_get_text_width(ctx, iui_weekday_short[i]);
        float label_x = wx + (cell_w - label_w) * 0.5f;
        float label_y = weekday_y + (weekday_h - ctx->font_height) * 0.5f;
        iui_internal_draw_text(ctx, label_x, label_y, iui_weekday_short[i],
                               ctx->colors.on_surface_variant);
    }

    /* Calendar grid */
    float grid_y = weekday_y + weekday_h;
    int first_day = iui_day_of_week(picker->view_year, picker->view_month, 1);
    int days_in_month =
        iui_days_in_month(picker->view_year, picker->view_month);
    int clicked_day = -1;

    for (int week = 0; week < 6; week++) {
        for (int dow = 0; dow < 7; dow++) {
            int cell_index = week * 7 + dow;
            int day_num = cell_index - first_day + 1;

            if (day_num < 1 || day_num > days_in_month)
                continue; /* Empty cell */

            /* Cell position uses touch target size for proper spacing */
            float touch_x = dialog_x + padding + dow * cell_w;
            float touch_y = grid_y + week * cell_h;
            iui_rect_t touch_rect = {touch_x, touch_y, cell_w, cell_h};

            /* Visual circle centered within touch target */
            float vis_x = touch_x + (cell_w - day_w) * 0.5f;
            float vis_y = touch_y + (cell_h - day_h) * 0.5f;
            iui_rect_t vis_rect = {vis_x, vis_y, day_w, day_h};

            bool is_selected = (day_num == picker->day &&
                                picker->view_month == picker->month &&
                                picker->view_year == picker->year);

            /* Use touch target rect for hit testing (MD3: 48dp) */
            iui_state_t cell_state =
                iui_get_component_state(ctx, touch_rect, false);

            /* Draw selection background OR state layer (mutually exclusive) */
            if (is_selected) {
                /* Selected day: filled primary rounded rect */
                ctx->renderer.draw_box(vis_rect, day_corner,
                                       ctx->colors.primary, ctx->renderer.user);
            } else if (cell_state == IUI_STATE_HOVERED ||
                       cell_state == IUI_STATE_PRESSED) {
                /* State layer covers full touch target for proper feedback */
                uint8_t alpha = (cell_state == IUI_STATE_PRESSED)
                                    ? IUI_STATE_PRESS_ALPHA
                                    : IUI_STATE_HOVER_ALPHA;
                /* circular touch feedback */
                float touch_corner = cell_w * 0.5f;
                ctx->renderer.draw_box(
                    touch_rect, touch_corner,
                    iui_state_layer(ctx->colors.on_surface, alpha),
                    ctx->renderer.user);
            }

            /* Draw day number centered in visual rect */
            char day_str[4];
            snprintf(day_str, sizeof(day_str), "%d", day_num);
            float text_w = iui_get_text_width(ctx, day_str);
            float text_x = vis_x + (day_w - text_w) * 0.5f;
            float text_y = vis_y + (day_h - ctx->font_height) * 0.5f;
            uint32_t day_color =
                is_selected ? ctx->colors.on_primary : ctx->colors.on_surface;
            iui_internal_draw_text(ctx, text_x, text_y, day_str, day_color);

            /* Handle click */
            if (frames_active >= 1 && cell_state == IUI_STATE_PRESSED) {
                clicked_day = day_num;
            }
        }
    }

    /* Update selected day on click */
    if (clicked_day > 0) {
        picker->day = clicked_day;
        picker->month = picker->view_month;
        picker->year = picker->view_year;
    }

    /* Confirmation buttons (Cancel / OK) */
    float btn_y = grid_y + grid_h + padding;
    float btn_spacing = 8.f;
    float btn_padding_h = 24.f;

    /* Calculate button widths */
    const char *cancel_label = "Cancel";
    const char *ok_label = "OK";
    float cancel_w =
        iui_get_text_width(ctx, cancel_label) + btn_padding_h * 2.f;
    float ok_w = iui_get_text_width(ctx, ok_label) + btn_padding_h * 2.f;

    /* Position buttons right-aligned */
    float ok_x = dialog_x + dialog_w - padding - ok_w;
    float cancel_x = ok_x - btn_spacing - cancel_w;

    /* Cancel button (text style) */
    iui_rect_t cancel_rect = {cancel_x, btn_y, cancel_w, button_h};
    iui_state_t cancel_state = iui_get_component_state(ctx, cancel_rect, false);
    iui_draw_state_layer(ctx, cancel_rect, button_h * 0.5f, ctx->colors.primary,
                         cancel_state);
    float cancel_text_x =
        cancel_rect.x +
        (cancel_rect.width - iui_get_text_width(ctx, cancel_label)) * 0.5f;
    float cancel_text_y =
        cancel_rect.y + (cancel_rect.height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, cancel_text_x, cancel_text_y, cancel_label,
                           ctx->colors.primary);

    /* OK button (filled style) */
    iui_rect_t ok_rect = {ok_x, btn_y, ok_w, button_h};
    iui_state_t ok_state = iui_get_component_state(ctx, ok_rect, false);
    ctx->renderer.draw_box(ok_rect, button_h * 0.5f, ctx->colors.primary,
                           ctx->renderer.user);
    iui_draw_state_layer(ctx, ok_rect, button_h * 0.5f, ctx->colors.on_primary,
                         ok_state);
    float ok_text_x =
        ok_rect.x + (ok_rect.width - iui_get_text_width(ctx, ok_label)) * 0.5f;
    float ok_text_y = ok_rect.y + (ok_rect.height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, ok_text_x, ok_text_y, ok_label,
                           ctx->colors.on_primary);

    /* Handle button clicks - use frames_active for consistent protection */
    if (frames_active >= 1) {
        if (cancel_state == IUI_STATE_PRESSED) {
            picker->confirmed = false;
            picker->is_open = false;
            iui_close_modal(ctx);
            iui_end_modal(ctx);
            return false;
        }
        if (ok_state == IUI_STATE_PRESSED) {
            picker->confirmed = true;
            picker->is_open = false;
            iui_close_modal(ctx);
            iui_end_modal(ctx);
            return true;
        }
    }

    iui_end_modal(ctx);
    return false;
}

/* Time Picker */

void iui_time_picker_show(iui_time_picker_state *picker,
                          int hour,
                          int minute,
                          bool use_24h)
{
    if (!picker)
        return;

    /* Clamp input values */
    if (hour < 0)
        hour = 0;
    if (hour > 23)
        hour = 23;
    if (minute < 0)
        minute = 0;
    if (minute > 59)
        minute = 59;

    picker->use_24h = use_24h;

    if (use_24h) {
        picker->hour = hour;
        picker->is_pm = false;
    } else {
        /* Convert to 12H format */
        picker->is_pm = (hour >= 12);
        picker->hour = hour % 12;
        if (picker->hour == 0)
            picker->hour = 12;
    }

    picker->minute = minute;
    picker->is_open = true;
    picker->frames_since_open = 0;
    picker->confirmed = false;
    picker->selecting_minute = false; /* Start with hour selection */
}

void iui_time_picker_close(iui_time_picker_state *picker)
{
    if (!picker)
        return;

    picker->is_open = false;
    picker->frames_since_open = 0;
    picker->confirmed = false;
}

bool iui_time_picker_is_open(const iui_time_picker_state *picker)
{
    return picker && picker->is_open;
}

/* Helper: convert polar coordinates to cartesian */
static void iui_polar_to_cart(float cx,
                              float cy,
                              float radius,
                              float angle_rad,
                              float *out_x,
                              float *out_y)
{
    *out_x = cx + radius * cosf(angle_rad);
    *out_y = cy + radius * sinf(angle_rad);
}

bool iui_time_picker(iui_context *ctx,
                     iui_time_picker_state *picker,
                     float screen_width,
                     float screen_height)
{
    if (!ctx || !picker || !picker->is_open)
        return false;

    /* Capture frame count BEFORE incrementing for click protection.
     * This ensures the first frame after opening (frames_active == 0) blocks
     * clicks, preventing click-through from the button that opened this picker.
     */
    int frames_active = picker->frames_since_open++;

    /* Calculate dialog dimensions */
    float dial_size = IUI_TIME_PICKER_DIAL_SIZE;
    float padding = IUI_TIME_PICKER_PADDING;
    float header_h = IUI_TIME_PICKER_HEADER_HEIGHT;
    float selector_size = IUI_TIME_PICKER_SELECTOR_SIZE;
    float center_dot = IUI_TIME_PICKER_CENTER_DOT;
    float button_h = 40.f;

    float dialog_w = dial_size + padding * 2.f;
    float dialog_h =
        padding + header_h + padding + dial_size + padding + button_h + padding;

    /* Add space for AM/PM toggle if using 12H format */
    float ampm_width = 0.f;
    if (!picker->use_24h) {
        ampm_width = IUI_TIME_PICKER_AMPM_WIDTH; /* 52dp */
        dialog_w += ampm_width + padding;
    }

    /* Center on screen */
    float dialog_x = (screen_width - dialog_w) * 0.5f;
    float dialog_y = (screen_height - dialog_h) * 0.5f;

    /* Begin modal blocking */
    iui_begin_modal(ctx, "time_picker_modal");

    /* Register blocking region for input layer system */
    iui_rect_t dialog_rect = {dialog_x, dialog_y, dialog_w, dialog_h};
    iui_register_blocking_region(ctx, dialog_rect);

    /* Draw scrim */
    iui_rect_t scrim_rect = {0, 0, screen_width, screen_height};
    ctx->renderer.draw_box(scrim_rect, 0, ctx->colors.scrim,
                           ctx->renderer.user);

    /* Draw dialog shadow */
    float corner = IUI_SHAPE_EXTRA_LARGE;
    iui_draw_shadow(ctx, dialog_rect, corner, IUI_ELEVATION_3);

    /* Draw dialog background */
    ctx->renderer.draw_box(dialog_rect, corner,
                           ctx->colors.surface_container_high,
                           ctx->renderer.user);

    /* Header: Time display (HH:MM) */
    float header_y = dialog_y + padding;

    /* Format time display */
    char hour_str[4], minute_str[4];
    if (picker->use_24h) {
        snprintf(hour_str, sizeof(hour_str), "%02d", picker->hour);
    } else {
        snprintf(hour_str, sizeof(hour_str), "%d", picker->hour);
    }
    snprintf(minute_str, sizeof(minute_str), "%02d", picker->minute);

    /* Calculate time display position (MD3: material_clock_display_width =
     * 96dp) */
    float time_box_h = header_h; /* Use full header height (80dp) */
    float time_box_w = IUI_TIME_PICKER_DISPLAY_WIDTH; /* 96dp */
    float colon_w = iui_get_text_width(ctx, ":");
    float time_display_w = time_box_w * 2.f + colon_w;

    float time_center_x = dialog_x + padding + dial_size * 0.5f;
    float hour_box_x = time_center_x - time_display_w * 0.5f;
    float minute_box_x = hour_box_x + time_box_w + colon_w;
    float time_box_y = header_y + (header_h - time_box_h) * 0.5f;

    /* Hour box (selectable) */
    iui_rect_t hour_rect = {hour_box_x, time_box_y, time_box_w, time_box_h};
    iui_state_t hour_state = iui_get_component_state(ctx, hour_rect, false);
    bool hour_active = !picker->selecting_minute;

    uint32_t hour_bg = hour_active ? ctx->colors.primary_container
                                   : ctx->colors.surface_container_highest;
    uint32_t hour_text =
        hour_active ? ctx->colors.on_primary_container : ctx->colors.on_surface;
    ctx->renderer.draw_box(hour_rect, 8.f, hour_bg, ctx->renderer.user);
    iui_draw_state_layer(ctx, hour_rect, 8.f, hour_text, hour_state);
    float hour_text_w = iui_get_text_width(ctx, hour_str);
    float hour_text_x = hour_rect.x + (hour_rect.width - hour_text_w) * 0.5f;
    float hour_text_y =
        hour_rect.y + (hour_rect.height - ctx->font_height * 2.f) * 0.5f;
    iui_internal_draw_text(ctx, hour_text_x, hour_text_y, hour_str, hour_text);

    /* Colon */
    float colon_x = hour_box_x + time_box_w +
                    (colon_w - iui_get_text_width(ctx, ":")) * 0.5f;
    float colon_y = time_box_y + (time_box_h - ctx->font_height * 2.f) * 0.5f;
    iui_internal_draw_text(ctx, colon_x, colon_y, ":", ctx->colors.on_surface);

    /* Minute box (selectable) */
    iui_rect_t minute_rect = {minute_box_x, time_box_y, time_box_w, time_box_h};
    iui_state_t minute_state = iui_get_component_state(ctx, minute_rect, false);
    bool minute_active = picker->selecting_minute;

    uint32_t min_bg = minute_active ? ctx->colors.primary_container
                                    : ctx->colors.surface_container_highest;
    uint32_t min_text = minute_active ? ctx->colors.on_primary_container
                                      : ctx->colors.on_surface;
    ctx->renderer.draw_box(minute_rect, 8.f, min_bg, ctx->renderer.user);
    if (minute_state == IUI_STATE_HOVERED ||
        minute_state == IUI_STATE_PRESSED) {
        uint8_t alpha = (minute_state == IUI_STATE_PRESSED)
                            ? IUI_STATE_PRESS_ALPHA
                            : IUI_STATE_HOVER_ALPHA;
        ctx->renderer.draw_box(minute_rect, 8.f,
                               iui_state_layer(min_text, alpha),
                               ctx->renderer.user);
    }
    float min_text_w = iui_get_text_width(ctx, minute_str);
    float min_text_x = minute_rect.x + (minute_rect.width - min_text_w) * 0.5f;
    float min_text_y =
        minute_rect.y + (minute_rect.height - ctx->font_height * 2.f) * 0.5f;
    iui_internal_draw_text(ctx, min_text_x, min_text_y, minute_str, min_text);

    /* Handle hour/minute selection toggle */
    if (frames_active >= 1) {
        if (hour_state == IUI_STATE_PRESSED)
            picker->selecting_minute = false;
        if (minute_state == IUI_STATE_PRESSED)
            picker->selecting_minute = true;
    }

    /* AM/PM toggle (12H only) */
    if (!picker->use_24h) {
        float ampm_total_height = IUI_TIME_PICKER_AMPM_HEIGHT; /* 96dp total */
        float ampm_x = dialog_x + dialog_w - padding - ampm_width;
        /* MD3: total height 96dp with 12dp gap = 42dp per button */
        float ampm_gap = 12.f; /* material_clock_period_toggle_vertical_gap */
        float ampm_h = (ampm_total_height - ampm_gap) * 0.5f; /* 42dp each */

        /* AM button - vertically centered with time display */
        float ampm_start_y =
            time_box_y + (time_box_h - ampm_total_height) * 0.5f;
        iui_rect_t am_rect = {ampm_x, ampm_start_y, ampm_width, ampm_h};
        iui_state_t am_state = iui_get_component_state(ctx, am_rect, false);
        bool am_selected = !picker->is_pm;

        uint32_t am_bg = am_selected ? ctx->colors.tertiary_container
                                     : ctx->colors.surface_container_highest;
        uint32_t am_text_color = am_selected ? ctx->colors.on_tertiary_container
                                             : ctx->colors.on_surface_variant;
        ctx->renderer.draw_box(am_rect, 8.f, am_bg, ctx->renderer.user);
        iui_draw_state_layer(ctx, am_rect, 8.f, am_text_color, am_state);
        float am_w = iui_get_text_width(ctx, "AM");
        iui_internal_draw_text(
            ctx, am_rect.x + (am_rect.width - am_w) * 0.5f,
            am_rect.y + (am_rect.height - ctx->font_height) * 0.5f, "AM",
            am_text_color);

        /* PM button */
        iui_rect_t pm_rect = {ampm_x, ampm_start_y + ampm_h + ampm_gap,
                              ampm_width, ampm_h};
        iui_state_t pm_state = iui_get_component_state(ctx, pm_rect, false);
        bool pm_selected = picker->is_pm;

        uint32_t pm_bg = pm_selected ? ctx->colors.tertiary_container
                                     : ctx->colors.surface_container_highest;
        uint32_t pm_text_color = pm_selected ? ctx->colors.on_tertiary_container
                                             : ctx->colors.on_surface_variant;
        ctx->renderer.draw_box(pm_rect, 8.f, pm_bg, ctx->renderer.user);
        iui_draw_state_layer(ctx, pm_rect, 8.f, pm_text_color, pm_state);
        float pm_w = iui_get_text_width(ctx, "PM");
        iui_internal_draw_text(
            ctx, pm_rect.x + (pm_rect.width - pm_w) * 0.5f,
            pm_rect.y + (pm_rect.height - ctx->font_height) * 0.5f, "PM",
            pm_text_color);

        /* Handle AM/PM toggle */
        if (frames_active >= 1) {
            if (am_state == IUI_STATE_PRESSED)
                picker->is_pm = false;
            if (pm_state == IUI_STATE_PRESSED)
                picker->is_pm = true;
        }
    }

    /* Clock dial */
    float dial_x = dialog_x + padding;
    float dial_y = header_y + header_h + padding;
    float dial_cx = dial_x + dial_size * 0.5f;
    float dial_cy = dial_y + dial_size * 0.5f;
    float dial_r = dial_size * 0.5f;

    /* Draw dial background */
    iui_rect_t dial_bg_rect = {dial_x, dial_y, dial_size, dial_size};
    ctx->renderer.draw_box(dial_bg_rect, dial_r,
                           ctx->colors.surface_container_highest,
                           ctx->renderer.user);

    /* Draw center dot */
    iui_rect_t center_dot_rect = {dial_cx - center_dot * 0.5f,
                                  dial_cy - center_dot * 0.5f, center_dot,
                                  center_dot};
    ctx->renderer.draw_box(center_dot_rect, center_dot * 0.5f,
                           ctx->colors.primary, ctx->renderer.user);

    /* Calculate number positions and draw */
    int num_count = 12;                /* 12 numbers for both hour and minute */
    float num_radius = dial_r * 0.75f; /* Position numbers at 75% of radius */

    int selected_value;
    if (picker->selecting_minute) {
        selected_value = picker->minute / 5; /* 0-11 for minute markers */
    } else {
        if (picker->use_24h) {
            selected_value = picker->hour % 12;
            if (picker->hour == 0)
                selected_value = 0;
        } else {
            selected_value = picker->hour % 12;
        }
    }

    /* Track clicked number */
    int clicked_num = -1;

    for (int i = 0; i < num_count; i++) {
        /* Angle: 0 = 12 o'clock, clockwise */
        float angle = (i - 3) * (2.f * IUI_PI /
                                 12.f); /* -3 to start at 12 o'clock position */

        float num_x, num_y;
        iui_polar_to_cart(dial_cx, dial_cy, num_radius, angle, &num_x, &num_y);

        /* Number label */
        char num_str[4];
        if (picker->selecting_minute) {
            snprintf(num_str, sizeof(num_str), "%02d", i * 5);
        } else {
            int display_num = (i == 0) ? 12 : i;
            snprintf(num_str, sizeof(num_str), "%d", display_num);
        }

        float num_w = iui_get_text_width(ctx, num_str);
        iui_rect_t num_rect = {num_x - selector_size * 0.5f,
                               num_y - selector_size * 0.5f, selector_size,
                               selector_size};

        /* Check if this number is selected */
        bool is_selected;
        if (picker->selecting_minute) {
            is_selected = (picker->minute / 5 == i);
        } else {
            int check_val = (picker->hour == 12) ? 0 : picker->hour;
            is_selected = (check_val == i);
        }

        iui_state_t num_state = iui_get_component_state(ctx, num_rect, false);

        /* Draw selection circle */
        if (is_selected) {
            ctx->renderer.draw_box(num_rect, selector_size * 0.5f,
                                   ctx->colors.primary, ctx->renderer.user);
        } else if (num_state == IUI_STATE_HOVERED ||
                   num_state == IUI_STATE_PRESSED) {
            uint8_t alpha = (num_state == IUI_STATE_PRESSED)
                                ? IUI_STATE_PRESS_ALPHA
                                : IUI_STATE_HOVER_ALPHA;
            ctx->renderer.draw_box(
                num_rect, selector_size * 0.5f,
                iui_state_layer(ctx->colors.on_surface, alpha),
                ctx->renderer.user);
        }

        /* Draw number text */
        uint32_t num_color =
            is_selected ? ctx->colors.on_primary : ctx->colors.on_surface;
        float text_x = num_x - num_w * 0.5f;
        float text_y = num_y - ctx->font_height * 0.5f;
        iui_internal_draw_text(ctx, text_x, text_y, num_str, num_color);

        /* Handle click */
        if (frames_active >= 1 && num_state == IUI_STATE_PRESSED) {
            clicked_num = i;
        }
    }

    /* Draw selector hand (line from center to selected) */
    if (ctx->renderer.draw_line) {
        float sel_angle = (selected_value - 3) * (2.f * IUI_PI / 12.f);
        float sel_x, sel_y;
        iui_polar_to_cart(dial_cx, dial_cy, num_radius - selector_size * 0.3f,
                          sel_angle, &sel_x, &sel_y);

        ctx->renderer.draw_line(dial_cx, dial_cy, sel_x, sel_y, 2.f,
                                ctx->colors.primary, ctx->renderer.user);
    }

    /* Update selected value on click */
    if (clicked_num >= 0) {
        if (picker->selecting_minute) {
            picker->minute = clicked_num * 5;
        } else {
            picker->hour = (clicked_num == 0) ? 12 : clicked_num;
            /* Auto-advance to minute selection after hour is selected */
            picker->selecting_minute = true;
        }
    }

    /* Confirmation buttons */
    float btn_y = dial_y + dial_size + padding;
    float btn_spacing = 8.f;
    float btn_padding_h = 24.f;

    const char *cancel_label = "Cancel";
    const char *ok_label = "OK";
    float cancel_w =
        iui_get_text_width(ctx, cancel_label) + btn_padding_h * 2.f;
    float ok_w = iui_get_text_width(ctx, ok_label) + btn_padding_h * 2.f;

    float ok_x = dialog_x + dialog_w - padding - ok_w;
    float cancel_x = ok_x - btn_spacing - cancel_w;

    /* Cancel button */
    iui_rect_t cancel_rect = {cancel_x, btn_y, cancel_w, button_h};
    iui_state_t cancel_state = iui_get_component_state(ctx, cancel_rect, false);
    iui_draw_state_layer(ctx, cancel_rect, button_h * 0.5f, ctx->colors.primary,
                         cancel_state);
    float cancel_text_x =
        cancel_rect.x +
        (cancel_rect.width - iui_get_text_width(ctx, cancel_label)) * 0.5f;
    float cancel_text_y =
        cancel_rect.y + (cancel_rect.height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, cancel_text_x, cancel_text_y, cancel_label,
                           ctx->colors.primary);

    /* OK button */
    iui_rect_t ok_rect = {ok_x, btn_y, ok_w, button_h};
    iui_state_t ok_state = iui_get_component_state(ctx, ok_rect, false);
    ctx->renderer.draw_box(ok_rect, button_h * 0.5f, ctx->colors.primary,
                           ctx->renderer.user);
    iui_draw_state_layer(ctx, ok_rect, button_h * 0.5f, ctx->colors.on_primary,
                         ok_state);
    float ok_text_x =
        ok_rect.x + (ok_rect.width - iui_get_text_width(ctx, ok_label)) * 0.5f;
    float ok_text_y = ok_rect.y + (ok_rect.height - ctx->font_height) * 0.5f;
    iui_internal_draw_text(ctx, ok_text_x, ok_text_y, ok_label,
                           ctx->colors.on_primary);

    /* Handle button clicks - use frames_active for consistent protection */
    if (frames_active >= 1) {
        if (cancel_state == IUI_STATE_PRESSED) {
            picker->confirmed = false;
            picker->is_open = false;
            iui_close_modal(ctx);
            iui_end_modal(ctx);
            return false;
        }
        if (ok_state == IUI_STATE_PRESSED) {
            picker->confirmed = true;
            /* Convert 12H back to 24H if needed for output */
            if (!picker->use_24h) {
                if (picker->hour == 12) {
                    picker->hour = picker->is_pm ? 12 : 0;
                } else if (picker->is_pm) {
                    picker->hour += 12;
                }
            }
            picker->is_open = false;
            iui_close_modal(ctx);
            iui_end_modal(ctx);
            return true;
        }
    }

    iui_end_modal(ctx);
    return false;
}
