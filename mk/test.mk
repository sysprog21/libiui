# Test build rules for libiui
# Builds and runs the test suite independently of user .config
# Tests use isolated build directory (.build/test) and do NOT produce libiui.a
# to avoid interference with port-specific builds.

# ============================================================================
# Test-mode override: enable ALL modules regardless of .config
# This ensures 'make check' tests the complete API surface
# ============================================================================

ifneq ($(filter check check-unit check-headless libiui_test,$(MAKECMDGOALS)),)
    # Force all modules enabled for comprehensive testing
    CONFIG_MODULE_BASIC := y
    CONFIG_MODULE_INPUT := y
    CONFIG_MODULE_CONTAINER := y
    CONFIG_MODULE_NAVIGATION := y
    CONFIG_MODULE_OVERLAY := y
    CONFIG_MODULE_MODAL := y
    CONFIG_MODULE_SELECTION := y
    CONFIG_MODULE_PICKER := y
    CONFIG_MODULE_SEARCH := y
    CONFIG_MODULE_ACTION := y
    CONFIG_MODULE_LIST := y
    CONFIG_FEATURE_ICONS := y
    CONFIG_FEATURE_VECTOR := y
    CONFIG_FEATURE_ACCESSIBILITY := y
    CONFIG_FEATURE_FOCUS := y
    CONFIG_PORT_HEADLESS := y
    CONFIG_CONFIGURED := y

    # Use isolated build directory for tests
    TEST_BUILD_DIR := .build/test
    TEST_CONFIG_HEADER := $(TEST_BUILD_DIR)/iui_config.h
endif

# ============================================================================
# Test Suite Sources
# ============================================================================

# Core tests (always included)
TEST_SRCS := \
    tests/common.c \
    tests/test-init.c \
    tests/test-layout.c \
    tests/test-box.c \
    tests/test-theme.c \
    tests/test-state.c \
    tests/test-demo.c \
    tests/test-clip.c \
    tests/test-tracking.c \
    tests/test-overflow.c \
    tests/main.c

# Module-dependent tests
TEST_SRCS-$(CONFIG_MODULE_BASIC)         += tests/test-widget.c tests/test-slider.c
TEST_SRCS-$(CONFIG_MODULE_INPUT)         += tests/test-input.c
TEST_SRCS-$(CONFIG_MODULE_CONTAINER)     += tests/test-component.c tests/test-scroll.c tests/test-bottomsheet.c
TEST_SRCS-$(CONFIG_MODULE_SELECTION)     += tests/test-chip.c
TEST_SRCS-$(CONFIG_MODULE_OVERLAY)       += tests/test-modal.c
TEST_SRCS-$(CONFIG_MODULE_NAVIGATION)    += tests/test-navigation.c

# Feature-dependent tests
TEST_SRCS-$(CONFIG_FEATURE_VECTOR)       += tests/test-vector.c
TEST_SRCS-$(CONFIG_FEATURE_ACCESSIBILITY) += tests/test-spec.c
TEST_SRCS-$(CONFIG_FEATURE_FOCUS)        += tests/test-focus.c

# Combine all test sources
TEST_SRCS += $(TEST_SRCS-y)

# ============================================================================
# Test Build Rules (uses isolated TEST_BUILD_DIR)
# ============================================================================

# These rules are evaluated after common.mk sets BUILD_DIR
define TEST_RULES

# Create isolated test build directory
$$(TEST_BUILD_DIR):
	@mkdir -p $$@

# Library object files for test build
# Reuses libiui.a_files-y from Makefile for src/ files, but forces headless port
LIBIUI_TEST_OBJS := $$(patsubst src/%.c,$$(TEST_BUILD_DIR)/%.o,$$(filter src/%.c,$$(libiui.a_files-y)))
LIBIUI_TEST_OBJS += $$(TEST_BUILD_DIR)/headless.o

# Test object files
TEST_OBJS := $$(patsubst tests/%.c,$$(TEST_BUILD_DIR)/%.o,$$(TEST_SRCS))

# Generate test config header into isolated build directory
# This avoids conflicts with user's src/iui_config.h from their .config
$$(TEST_CONFIG_HEADER): $$(KCONFIG_DIR)/defconfig.py $$(KCONFIG_DIR)/genconfig.py | $$(TEST_BUILD_DIR)
	@echo "  GEN     $$@ (test configuration)"
	@KCONFIG_CONFIG=.config.test python3 $$(KCONFIG_DIR)/defconfig.py --kconfig $$(KCONFIG) configs/defconfig
	@KCONFIG_CONFIG=.config.test python3 $$(KCONFIG_DIR)/genconfig.py --header-path $$@ $$(KCONFIG)
	@rm -f .config.test

# Compile rules for test build directory (isolated from main build)
# -I$$(TEST_BUILD_DIR) first ensures test config header takes priority
# -DIUI_MD3_RUNTIME_VALIDATION enables runtime validation of rendered dimensions
TEST_CFLAGS := $$(CFLAGS) -DIUI_MD3_RUNTIME_VALIDATION

$$(TEST_BUILD_DIR)/%.o: src/%.c $$(TEST_CONFIG_HEADER) | $$(TEST_BUILD_DIR) $$(MD3_GENERATED)
	@echo "  CC      $$< (test)"
	$$(Q)$$(CC) $$(TEST_CFLAGS) -MMD -MP -MF $$(TEST_BUILD_DIR)/$$(notdir $$*).d \
	    -I$$(TEST_BUILD_DIR) -Iinclude -Isrc -Itests -Iports -Iexternals \
	    -c -o $$@ $$<

$$(TEST_BUILD_DIR)/%.o: ports/%.c $$(TEST_CONFIG_HEADER) | $$(TEST_BUILD_DIR) $$(MD3_GENERATED)
	@echo "  CC      $$< (test)"
	$$(Q)$$(CC) $$(TEST_CFLAGS) -MMD -MP -MF $$(TEST_BUILD_DIR)/$$(notdir $$*).d \
	    -I$$(TEST_BUILD_DIR) -Iinclude -Isrc -Itests -Iports -Iexternals \
	    -c -o $$@ $$<

$$(TEST_BUILD_DIR)/%.o: tests/%.c $$(TEST_CONFIG_HEADER) | $$(TEST_BUILD_DIR) $$(MD3_GENERATED)
	@echo "  CC      $$< (test)"
	$$(Q)$$(CC) $$(TEST_CFLAGS) -MMD -MP -MF $$(TEST_BUILD_DIR)/$$(notdir $$*).d \
	    -I$$(TEST_BUILD_DIR) -Iinclude -Isrc -Itests -Iports -Iexternals \
	    -c -o $$@ $$<

# Link test executable directly from objects (no libiui.a produced)
libiui_test: $$(TEST_CONFIG_HEADER) $$(MD3_GENERATED) $$(LIBIUI_TEST_OBJS) $$(TEST_OBJS)
	@echo "  LD      $$@"
	$$(Q)$$(CC) $$(CFLAGS) -o $$@ $$(LIBIUI_TEST_OBJS) $$(TEST_OBJS) $$(LDFLAGS)

# Build libiui.a with headless port for Python test harness
# Uses same test config as libiui_test for consistency
$$(TEST_BUILD_DIR)/libiui.a: $$(LIBIUI_TEST_OBJS)
	@echo "  AR      $$@"
	$$(Q)$$(AR) rcs $$@ $$^

# Run C unit tests
check-unit: libiui_test
	@echo "=== Running C unit tests ==="
	@./libiui_test

# Run Python headless tests (widget, interaction, MD3)
check-headless: $$(TEST_BUILD_DIR)/libiui.a
	@echo ""
	@echo "=== Running headless tests ==="
	@python3 scripts/headless-test.py --lib $$(TEST_BUILD_DIR)/libiui.a

# Combined check target: run all tests
check: check-unit check-headless
	@echo ""
	@echo "=== All tests passed ==="

# Include test dependency files
-include $$(wildcard $$(TEST_BUILD_DIR)/*.d)

endef
