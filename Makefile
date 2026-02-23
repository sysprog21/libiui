# Modular build with Kconfig support

.DEFAULT_GOAL := all

# Load configuration (safe include for unconfigured builds)
-include .config

# Configuration validation

# Targets that don't require .config
CONFIG_TARGETS := config defconfig oldconfig savedefconfig clean distclean indent check libiui_test

# Targets that generate .config (skip validation if these are in goals)
CONFIG_GENERATORS := config defconfig oldconfig

# Require .config if ANY build target is requested, unless a config generator is also present
# This allows: 'make defconfig all' to work (defconfig creates .config before all runs)
BUILD_GOALS := $(filter-out $(CONFIG_TARGETS),$(MAKECMDGOALS))
HAS_CONFIG_GEN := $(filter $(CONFIG_GENERATORS),$(MAKECMDGOALS))
ifneq ($(BUILD_GOALS),)
ifeq ($(HAS_CONFIG_GEN),)
ifneq "$(CONFIG_CONFIGURED)" "y"
    $(info )
    $(info *** Configuration file ".config" not found!)
    $(info *** Please run 'make config' or 'make defconfig' first.)
    $(info )
    $(error Configuration required)
endif
endif
endif

# Check if src/iui_config.h exists when building (after .config check passes)
ifeq ($(wildcard src/iui_config.h),)
  ifneq ($(BUILD_GOALS),)
    ifeq ($(HAS_CONFIG_GEN),)
      $(info )
      $(info *** Header file "src/iui_config.h" not found!)
      $(info *** Please run 'make config' or 'make defconfig' first to generate it.)
      $(info )
      $(error Missing configuration header)
    endif
  endif
endif

# Include build framework
include mk/toolchain.mk
include mk/deps.mk
include mk/test.mk

# Core Library

target.a-y := libiui.a

libiui.a_includes-y := include src
libiui.a_cflags-y :=
libiui.a_depends-y := $(MD3_GENERATED)

# Core modules (always included)
libiui.a_files-y := \
    src/core.c \
    src/event.c \
    src/layout.c \
    src/draw.c \
    src/font.c

# Optional modules based on config
libiui.a_files-$(CONFIG_MODULE_BASIC)     += src/basic.c
libiui.a_files-$(CONFIG_MODULE_INPUT)     += src/input.c
libiui.a_files-$(CONFIG_MODULE_CONTAINER) += src/container.c
libiui.a_files-$(CONFIG_MODULE_LIST)      += src/list.c
# Compound widget modules (functional grouping)
libiui.a_files-$(CONFIG_MODULE_NAVIGATION) += src/appbar.c src/tabs.c src/navigation.c
libiui.a_files-$(CONFIG_MODULE_OVERLAY)    += src/menu.c src/dialog.c
libiui.a_files-$(CONFIG_MODULE_SELECTION)  += src/chips.c
libiui.a_files-$(CONFIG_MODULE_PICKER)     += src/pickers.c
libiui.a_files-$(CONFIG_MODULE_SEARCH)     += src/searchbar.c
libiui.a_files-$(CONFIG_MODULE_ACTION)     += src/fab.c
libiui.a_files-$(CONFIG_MODULE_MODAL)     += src/modal.c
libiui.a_files-$(CONFIG_FEATURE_ICONS)    += src/icons.c

# Backend Selection

PORT := none
TARGET_LIBS :=
GLOBAL_EXTRA_CFLAGS :=

ifeq ($(CONFIG_PORT_SDL2),y)
    PORT := sdl2
    libiui.a_files-y += ports/sdl2.c
    libiui.a_cflags-y += $(call dep,cflags,sdl2)
    TARGET_LIBS += $(call dep,libs,sdl2)
    # Export SDL2 cflags globally for example.c
    GLOBAL_EXTRA_CFLAGS += $(call dep,cflags,sdl2)
endif

ifeq ($(CONFIG_PORT_HEADLESS),y)
    PORT := headless
    libiui.a_files-y += ports/headless.c
endif

# Pure WebAssembly backend (no SDL dependency)
ifeq ($(CONFIG_PORT_WASM),y)
    PORT := wasm
    libiui.a_files-y += ports/wasm.c
    # Emscripten size optimization
    libiui.a_cflags-$(CC_IS_EMCC) += -Oz
endif

# Emscripten base flags (PORT_WASM only - SDL2+Emscripten not supported)
ifeq ($(CC_IS_EMCC),1)
    TARGET_LIBS += -sWASM=1 -sINITIAL_MEMORY=33554432
endif

# Demo Application

# Generate nyancat data if demo enabled
ifeq ($(CONFIG_DEMO_NYANCAT),y)
NYANCAT_DATA := tests/nyancat-data.h
$(NYANCAT_DATA):
	@echo "  GEN     $@"
	@python3 scripts/gen-nyancat-data.py -o $@
endif

# MD3 validation code generation from DSL
MD3_GEN_FLAGS := src/md3-flags-gen.inc
MD3_GEN_HEADER := src/md3-validate-gen.inc
MD3_GEN_TESTS := tests/test-md3-gen.inc
MD3_DSL := src/md3-spec.dsl
MD3_GEN_SCRIPT := scripts/gen-md3-validate.py
MD3_GENERATED := $(MD3_GEN_FLAGS) $(MD3_GEN_HEADER) $(MD3_GEN_TESTS)

.PHONY: gen-md3
gen-md3: $(MD3_GENERATED)

$(MD3_GENERATED): $(MD3_DSL) $(MD3_GEN_SCRIPT)
	@echo "  GEN     MD3 validation (from DSL)"
	@python3 $(MD3_GEN_SCRIPT)

ifeq ($(CONFIG_DEMO_EXAMPLE),y)
    target-y += libiui_example

    libiui_example_files-y := tests/example.c
    libiui_example_files-$(CONFIG_DEMO_PIXELWALL) += tests/pixelwall-demo.c
    libiui_example_includes-y := include ports externals tests
    libiui_example_depends-y := libiui.a $(NYANCAT_DATA)
    libiui_example_ldflags-y := libiui.a $(TARGET_LIBS)

    # Backend-specific flags for example
    ifeq ($(CONFIG_PORT_SDL2),y)
        libiui_example_cflags-y := $(call dep,cflags,sdl2)
    endif

    # Emscripten-specific linker flags for WebAssembly
    ifeq ($(CC_IS_EMCC),1)
        libiui_example_cflags-y += -Oz
        libiui_example_ldflags-y += \
            -sEXPORTED_FUNCTIONS='["_main","_iui_wasm_mouse_motion","_iui_wasm_mouse_button","_iui_wasm_scroll","_iui_wasm_key","_iui_wasm_char","_iui_wasm_get_framebuffer","_iui_wasm_get_width","_iui_wasm_get_height","_iui_wasm_shutdown"]' \
            -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","HEAPU32","wasmMemory"]' \
            -sALLOW_MEMORY_GROWTH=1 \
            -sASYNCIFY
    endif
endif

# Set generated file prerequisites for compilation rules
PREREQ_GENERATED := $(MD3_GENERATED)

# Include generic build rules
include mk/common.mk

# Kconfig targets

KCONFIG_DIR := tools/kconfig
KCONFIG := configs/Kconfig
CONFIG_HEADER := src/iui_config.h

# Kconfig tool source (kconfiglib)
KCONFIGLIB_REPO := https://github.com/sysprog21/Kconfiglib

# Download and setup Kconfig tools if missing
$(KCONFIG_DIR)/kconfiglib.py:
	@echo "  CLONE   Kconfiglib"
	@git clone --depth=1 -q $(KCONFIGLIB_REPO) $(KCONFIG_DIR)
	@echo "Kconfig tools installed to $(KCONFIG_DIR)"

# Ensure all Kconfig tools exist
$(KCONFIG_DIR)/menuconfig.py $(KCONFIG_DIR)/defconfig.py $(KCONFIG_DIR)/genconfig.py \
$(KCONFIG_DIR)/oldconfig.py $(KCONFIG_DIR)/savedefconfig.py: $(KCONFIG_DIR)/kconfiglib.py

# Detect library availability for Kconfig
export HAVE_SDL2 := $(call pkg-exists,sdl2)

config: $(KCONFIG_DIR)/menuconfig.py
	@python3 $(KCONFIG_DIR)/menuconfig.py $(KCONFIG)
	@python3 $(KCONFIG_DIR)/genconfig.py --header-path $(CONFIG_HEADER) $(KCONFIG)
	@echo "Configuration saved to .config and $(CONFIG_HEADER)"

defconfig: $(KCONFIG_DIR)/defconfig.py
	@python3 $(KCONFIG_DIR)/defconfig.py --kconfig $(KCONFIG) configs/defconfig
	@python3 $(KCONFIG_DIR)/genconfig.py --header-path $(CONFIG_HEADER) $(KCONFIG)
	@echo "Default configuration applied (SDL2, all features)"
ifneq ($(BUILD_GOALS),)
	@echo "Rebuilding with new configuration..."
	@$(MAKE) $(BUILD_GOALS)
endif

oldconfig: $(KCONFIG_DIR)/oldconfig.py
	@python3 $(KCONFIG_DIR)/oldconfig.py $(KCONFIG)
	@python3 $(KCONFIG_DIR)/genconfig.py --header-path $(CONFIG_HEADER) $(KCONFIG)

savedefconfig: $(KCONFIG_DIR)/savedefconfig.py
	@python3 $(KCONFIG_DIR)/savedefconfig.py --kconfig $(KCONFIG) --out configs/defconfig
	@echo "Configuration saved to configs/defconfig"

# ============================================================================
# Standard targets
# ============================================================================

# Check if configuration header exists before building
config-checked:
	@if [ ! -f src/iui_config.h ]; then \
		echo ""; \
		echo "*** Header file \"src/iui_config.h\" not found!"; \
		echo "*** Please run 'make config' or 'make defconfig' first to generate it."; \
		echo ""; \
		exit 1; \
	fi

# When config generator handles BUILD_GOALS via recursive make, skip here
ifneq ($(HAS_CONFIG_GEN),)
ifneq ($(BUILD_GOALS),)
all: config-checked
	@: # Already built by recursive make in config generator
else
all: config-checked $(target.a-y) $(target-y)
	@echo "Build complete: $(target.a-y) $(target-y)"
endif
else
all: config-checked $(target.a-y) $(target-y)
	@echo "Build complete: $(target.a-y) $(target-y)"
endif

# Explicit dependency: example.o requires nyancat-data.h if enabled
ifdef NYANCAT_DATA
$(BUILD_DIR)/example.o: $(NYANCAT_DATA)
endif

# Test target (built on demand, via 'make check')
# Rules defined in mk/test.mk, evaluated here after BUILD_DIR is set

$(eval $(TEST_RULES))

indent:
	@echo "Formatting C source files..."
	@clang-format -i include/*.h
	@clang-format -i src/*.c src/*.h
	@clang-format -i ports/*.c ports/*.h
	@clang-format -i tests/*.c tests/*.h
	@echo "Done."

clean: clean-build
	rm -rf .build/test
	rm -f src/*.o tests/*.o ports/*.o

distclean: clean
	rm -f .config $(CONFIG_HEADER) libiui.a libiui_example libiui_test
	rm -f src/md3-flags-gen.inc src/md3-validate-gen.inc tests/test-md3-gen.inc
	rm -f tests/nyancat-data.h
	rm -rf $(KCONFIG_DIR)

# WebAssembly Installation Target

.PHONY: wasm-install
wasm-install:
	@if [ "$(CC_IS_EMCC)" = "1" ]; then \
		echo "Installing WebAssembly files to assets/web/..."; \
		if [ ! -f libiui_example ] || [ ! -f libiui_example.wasm ]; then \
			echo "Error: WASM build artifacts not found"; \
			echo "Expected: libiui_example and libiui_example.wasm"; \
			exit 1; \
		fi; \
		mkdir -p assets/web; \
		cp -f libiui_example assets/web/libiui_example.js; \
		cp -f libiui_example.wasm assets/web/; \
		echo ""; \
		echo "WebAssembly build installed to assets/web/"; \
		echo "To test, run:"; \
		echo "  \033[1;34mpython3 scripts/serve-wasm.py\033[0m"; \
		echo "Then open http://localhost:8000 in your browser."; \
	else \
		echo "wasm-install requires Emscripten toolchain."; \
		echo "Build with: make"; \
	fi

# Add wasm-install as post-build step for Emscripten
# wasm-install depends on the actual build targets to avoid race conditions
ifeq ($(CC_IS_EMCC), 1)
wasm-install: $(target-y)
all: wasm-install
endif

.PHONY: all config defconfig oldconfig savedefconfig check check-unit check-headless indent clean distclean wasm-install
