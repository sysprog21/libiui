# MD3 Component Specification DSL
# Format: COMPONENT name { constraints... }
#
# Constraint types:
#   height MIN <dp>           - minimum height in dp
#   height EXACT <dp> [±tol]  - exact height with optional pixel tolerance
#   size EXACT <dp> [±tol]    - exact width=height (square components)
#   width MIN <dp>            - minimum width in dp
#   width EXACT <dp>          - exact width in dp
#   touch_target <dp>         - minimum touch target in dp
#   corner_radius <dp>        - corner radius in dp (or @shape.xxx token)
#   icon_size <dp>            - icon dimensions in dp
#   padding_h <dp>            - horizontal padding in dp
#   padding_v <dp>            - vertical padding in dp
#   padding <dp>              - general padding in dp
#   gap <dp>                  - gap between elements in dp
#   icon_label_gap <dp>       - gap between icon and label in dp
#   action_gap <dp>           - gap between action buttons in dp
#   min_width <dp>            - minimum width in dp
#   max_width <dp>            - maximum width in dp
#   max_height <dp>           - maximum height in dp
#   indicator_height <dp>     - indicator height in dp
#   indicator_width <dp>      - indicator width in dp
#   avatar_size <dp>          - avatar/image size in dp
#   divider_inset <dp>        - divider inset in dp
#   offset <dp>               - positioning offset in dp
#   collapsed_height <dp>     - collapsed state height in dp
#   expanded_width <dp>       - expanded state width in dp
#
# Token references:
#   corner_radius @shape.none         - 0dp
#   corner_radius @shape.extra_small  - 4dp
#   corner_radius @shape.small        - 8dp
#   corner_radius @shape.medium       - 12dp
#   corner_radius @shape.large        - 16dp
#   corner_radius @shape.extra_large  - 28dp
#
# State layer opacity (percentages):
#   hover <percent>
#   focus <percent>
#   press <percent>
#   drag <percent>
#   disable <percent>

# === Core Components ===

COMPONENT button {
    height MIN 40
    touch_target 48
    corner_radius 20
}

COMPONENT segmented {
    height EXACT 40
    icon_size 18
    touch_target 48
}

COMPONENT fab {
    size EXACT 56 ±1
    icon_size 24
    touch_target 48
    corner_radius @shape.large
}

COMPONENT fab_large {
    size EXACT 96 ±1
    icon_size 36
    touch_target 48
    corner_radius @shape.extra_large
}

COMPONENT fab_extended {
    height MIN 56
    icon_size 24
    padding_h 16
    gap 8
    touch_target 48
    corner_radius @shape.large
}

COMPONENT chip {
    height MIN 32
    icon_size 18
    padding_h 16
    icon_label_gap 8
    touch_target 48
    corner_radius @shape.small
}

COMPONENT textfield {
    height MIN 56
    touch_target 48
    corner_radius @shape.extra_small
}

COMPONENT icon_button {
    size EXACT 40 ±1
    icon_size 24
    touch_target 48
    corner_radius 20
}

COMPONENT slider {
    track_height 4
    thumb_idle 20
    thumb_pressed 28
    touch_target 48
}

COMPONENT switch {
    track_width 52
    track_height 32
    thumb_size 24
    touch_target 48
    corner_radius @shape.large
}

COMPONENT tab {
    height MIN 48
    min_width 90
    indicator_height 3
    icon_size 24
    icon_label_gap 4
    padding_h 16
    touch_target 48
}

COMPONENT search_bar {
    height MIN 56
    corner_radius 28
    icon_size 24
    padding_h 16
    icon_label_gap 16
    touch_target 48
}

COMPONENT menu_item {
    height MIN 40
    icon_size 24
}

# === App Bar Components ===

COMPONENT appbar_small {
    height EXACT 64
    collapsed_height 64
    icon_size 24
    padding_h 16
    action_gap 8
    touch_target 48
}

COMPONENT appbar_medium {
    height EXACT 112
    collapsed_height 64
    icon_size 24
    padding_h 16
    action_gap 8
    touch_target 48
}

COMPONENT appbar_large {
    height EXACT 152
    collapsed_height 64
    icon_size 24
    padding_h 16
    action_gap 8
    touch_target 48
}

# === Picker Components ===

COMPONENT date_picker_day {
    size EXACT 40
    corner_radius 20
}

COMPONENT time_picker_dial {
    size EXACT 256
    selector_size 40
    center_dot 8
}

COMPONENT time_picker_display {
    width EXACT 96
    header_height 80
    padding 24
}

# === Search Components ===

COMPONENT search_view {
    header_height 72
    suggestion_height 56
    icon_size 24
    padding 16
}

# === Dialog & Sheet Components ===

COMPONENT dialog {
    min_width 280
    max_width 480
    padding 24
    corner_radius @shape.extra_large
    button_height 36
    action_gap 8
    content_spacing 16
}

COMPONENT fullscreen_dialog {
    header_height 56
    padding 16
}

COMPONENT bottom_sheet {
    corner_radius @shape.extra_large
    drag_handle_width 32
    drag_handle_height 4
    min_height 64
}

# === Container Components ===

COMPONENT card {
    corner_radius @shape.medium
    padding 16
}

COMPONENT snackbar {
    height EXACT 48
    corner_radius @shape.extra_small
    padding_h 16
}

COMPONENT tooltip {
    min_width 32
    min_height 32
    max_width 200
    corner_radius @shape.extra_small
    padding 12
    offset 8
}

COMPONENT banner {
    height MIN 52
    padding 16
    icon_size 24
    action_gap 8
}

# === Navigation Components ===

COMPONENT nav_rail {
    width EXACT 80
    expanded_width 256
    item_height 56
    icon_size 24
    indicator_width 56
    indicator_height 32
    corner_radius @shape.large
}

COMPONENT nav_drawer {
    width EXACT 280
    item_height 56
    icon_size 24
    padding_h 12
    icon_label_gap 12
}

COMPONENT nav_bar {
    height EXACT 80
    icon_size 24
    indicator_width 64
    indicator_height 32
    icon_label_gap 4
}

COMPONENT bottom_app_bar {
    height EXACT 80
    icon_size 24
    icon_container 48
}

# === Menu Components ===

COMPONENT menu {
    min_width 200
    max_width 400
    corner_radius @shape.extra_small
    padding_v 8
    padding_h 12
}

# === List Components ===

COMPONENT list_item_one_line {
    height EXACT 56
    icon_size 24
    padding_h 16
    avatar_size 40
    divider_inset 16
}

COMPONENT list_item_two_line {
    height EXACT 72
    icon_size 24
    padding_h 16
    avatar_size 40
    divider_inset 16
}

COMPONENT list_item_three_line {
    height EXACT 88
    icon_size 24
    padding_h 16
    avatar_size 40
    divider_inset 16
}

# === Data Display Components ===

COMPONENT data_table_header {
    height EXACT 56
    padding 16
}

COMPONENT data_table_row {
    height EXACT 52
    padding 16
}

COMPONENT badge_dot {
    size EXACT 8
}

COMPONENT badge_label {
    height EXACT 16
    padding_h 4
}

COMPONENT progress_bar {
    height EXACT 4
}

# === Dropdown Components ===

COMPONENT dropdown {
    height EXACT 56
    corner_radius @shape.extra_small
    item_height 48
    max_height 280
}

# === Layout Constraints ===

GLOBAL grid_unit 4

GLOBAL window_size_class {
    compact    0      # < 600dp: phone portrait
    medium     600    # 600-839dp: tablet portrait, foldable
    expanded   840    # 840-1199dp: tablet landscape
    large      1200   # 1200-1599dp: desktop
    extra_large 1600  # >= 1600dp: large desktop
}

GLOBAL layout_columns {
    compact    4
    medium     8
    expanded   12
}

GLOBAL layout_margin {
    compact    16
    medium     24
    expanded   24
}

GLOBAL layout_gutter {
    compact    8
    medium     8
    expanded   16
}

GLOBAL state_layer {
    hover 8
    focus 12
    press 12
    drag 16
    disable 38
}

GLOBAL shape {
    none 0
    extra_small 4
    small 8
    medium 12
    large 16
    extra_large 28
}

GLOBAL typography {
    headline_small 24
    title_large 22
    title_medium 16
    title_small 14
    body_large 16
    body_medium 14
    body_small 12
    label_large 14
    label_medium 12
    label_small 11
}
