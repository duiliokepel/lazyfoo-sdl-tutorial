# Clear all default suffixes
.SUFFIXES:

#This specifies targets that are not files
.PHONY: all clean

################################################################
# Toolchain and flags
################################################################
CC = gcc

LANGUAGE_FLAGS = -std=c11
WARNING_FLAGS  = -Wall -Wextra -Wpedantic \
	-Wmissing-prototypes -Wstrict-prototypes \
	-Wdeclaration-after-statement \
	-Wshadow -Wconversion -Wcast-qual -Wcast-align \
	-Wundef -Wuninitialized \
	-Wpointer-arith -Wwrite-strings -Wvla \
	-Waggregate-return -Wfloat-equal \
	-Wswitch-enum -Wswitch-default -Wbad-function-cast \
	-Wformat=2 -Wformat-security \
	-Wunused-result

ANALYZER_FLAGS = -fanalyzer -fstack-protector-strong -fno-common \
	-fno-strict-overflow -fno-strict-aliasing

OPTIMIZATION_FLAGS = -O3 -ffunction-sections -fdata-sections -fno-omit-frame-pointer
DEBUG_FLAGS        = -g3 -ggdb

INCLUDE_FLAGS = -I./

CFLAGS = $(LANGUAGE_FLAGS) $(WARNING_FLAGS) \
	$(ANALYZER_FLAGS) $(OPTIMIZATION_FLAGS) $(DEBUG_FLAGS) $(INCLUDE_FLAGS)

LDFLAGS = \
	-Wl,-z,noexecstack \
	-Wl,-z,relro \
	-Wl,-z,now \
	-Wl,--as-needed \
	-Wl,--gc-sections

################################################################
# Pretty-print
################################################################

# Usage: $(call PRINT_RULE)
PRINT_RULE = \
	echo ""; \
	echo "\#---------------------------------------------------------------"; \
	echo "\#                   Target=[$@]"; \
	if [ -n "$^" ]; then \
	echo "\#            Prerequisites=[$^]"; \
	fi; \
	if [ -n "$|" ]; then \
	echo "\# Order-only Prerequisites=[$|]"; \
	fi; \
	echo "\#---------------------------------------------------------------"; \
	echo ""

################################################################
# Directories
################################################################
# Source directories
SRC_DIR = src
ASSETS_DIR = assets

# Generated directories
DEPS_DIR = deps
BUILD_DIR = build
INCLUDE_DIR = include
EMBED_DIR = embed
BIN_DIR = bin

################################################################
# Program-specific object lists and libraries
################################################################

HELLOWORLD_OBJS = $(BUILD_DIR)/helloworld.o
HELLOWORLD_LIBS =
PROGRAMS += $(BIN_DIR)/helloworld
ALL_OBJS += $(HELLOWORLD_OBJS)

HELLOEMBED_OBJS = $(BUILD_DIR)/helloembed.o $(EMBED_DIR)/lorem-ipsum.txt.o
HELLOEMBED_LIBS =
PROGRAMS += $(BIN_DIR)/helloembed
ALL_OBJS += $(HELLOEMBED_OBJS)

01_hello_sdl_OBJS = $(BUILD_DIR)/01_hello_sdl.o $(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o
01_hello_sdl_LIBS = -lSDL2
PROGRAMS += $(BIN_DIR)/01_hello_sdl
ALL_OBJS += $(01_hello_sdl_OBJS)

02_image_on_screen_OBJS = $(BUILD_DIR)/02_image_on_screen.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/hello_world.bmp.o
02_image_on_screen_LIBS = -lSDL2
PROGRAMS += $(BIN_DIR)/02_image_on_screen
ALL_OBJS += $(02_image_on_screen_OBJS)

03_event_driven_programming_OBJS = $(BUILD_DIR)/03_event_driven_programming.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/press_x_to_close.bmp.o
03_event_driven_programming_LIBS = -lSDL2
PROGRAMS += $(BIN_DIR)/03_event_driven_programming
ALL_OBJS += $(03_event_driven_programming_OBJS)

04_key_presses_OBJS = $(BUILD_DIR)/04_key_presses.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/press_default.bmp.o \
	$(EMBED_DIR)/press_up.bmp.o $(EMBED_DIR)/press_down.bmp.o \
	$(EMBED_DIR)/press_left.bmp.o $(EMBED_DIR)/press_right.bmp.o
04_key_presses_LIBS = -lSDL2
PROGRAMS += $(BIN_DIR)/04_key_presses
ALL_OBJS += $(04_key_presses_OBJS)

05_optimized_surface_and_soft_stretching_OBJS = $(BUILD_DIR)/05_optimized_surface_and_soft_stretching.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/stretching_to_window.bmp.o
05_optimized_surface_and_soft_stretching_LIBS = -lSDL2 -lm
PROGRAMS += $(BIN_DIR)/05_optimized_surface_and_soft_stretching
ALL_OBJS += $(05_optimized_surface_and_soft_stretching_OBJS)

06_extension_libraries_OBJS = $(BUILD_DIR)/06_extension_libraries.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/png_loaded.png.o
06_extension_libraries_LIBS = -lSDL2 -lSDL2_image -lm
PROGRAMS += $(BIN_DIR)/06_extension_libraries
ALL_OBJS += $(06_extension_libraries_OBJS)

07_texture_loading_and_rendering_OBJS = $(BUILD_DIR)/07_texture_loading_and_rendering.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/rendering_texture.png.o
07_texture_loading_and_rendering_LIBS = -lSDL2 -lSDL2_image -lm
PROGRAMS += $(BIN_DIR)/07_texture_loading_and_rendering
ALL_OBJS += $(07_texture_loading_and_rendering_OBJS)

08_geometry_rendering_OBJS = $(BUILD_DIR)/08_geometry_rendering.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o
08_geometry_rendering_LIBS = -lSDL2 -lm
PROGRAMS += $(BIN_DIR)/08_geometry_rendering
ALL_OBJS += $(08_geometry_rendering_OBJS)

09_the_viewport_OBJS = $(BUILD_DIR)/09_the_viewport.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/viewport.png.o
09_the_viewport_LIBS = -lSDL2 -lSDL2_image -lm
PROGRAMS += $(BIN_DIR)/09_the_viewport
ALL_OBJS += $(09_the_viewport_OBJS)

10_color_keying_OBJS = $(BUILD_DIR)/10_color_keying.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o \
	$(EMBED_DIR)/earth_background.png.o $(EMBED_DIR)/space_shuttle_colorkey.png.o
10_color_keying_LIBS = -lSDL2 -lSDL2_image -lm
PROGRAMS += $(BIN_DIR)/10_color_keying
ALL_OBJS += $(10_color_keying_OBJS)

11_clip_rendering_OBJS = $(BUILD_DIR)/11_clip_rendering.o \
	$(BUILD_DIR)/trace.o $(BUILD_DIR)/assert.o $(EMBED_DIR)/sprite_sheet.png.o
11_clip_rendering_LIBS = -lSDL2 -lSDL2_image
PROGRAMS += $(BIN_DIR)/11_clip_rendering
ALL_OBJS += $(11_clip_rendering_OBJS)

################################################################
# Master target
################################################################

all: $(PROGRAMS)

################################################################
# Dependency inclusion
################################################################

# Generate dependencies for objects
ALL_OBJS := $(sort $(ALL_OBJS))
DEPS := $(patsubst $(BUILD_DIR)/%.o,$(DEPS_DIR)/%.d, $(filter $(BUILD_DIR)/%.o,$(ALL_OBJS)))
-include $(DEPS)

# Embed header dependencies
$(BUILD_DIR)/helloembed.o: $(EMBED_DIR)/lorem-ipsum.txt.h

$(BUILD_DIR)/02_image_on_screen.o: $(EMBED_DIR)/hello_world.bmp.h

$(BUILD_DIR)/03_event_driven_programming.o: $(EMBED_DIR)/press_x_to_close.bmp.h

$(BUILD_DIR)/04_key_presses.o: $(EMBED_DIR)/press_default.bmp.h \
	$(EMBED_DIR)/press_up.bmp.h $(EMBED_DIR)/press_down.bmp.h \
	$(EMBED_DIR)/press_left.bmp.h $(EMBED_DIR)/press_right.bmp.h

$(BUILD_DIR)/05_optimized_surface_and_soft_stretching.o: $(EMBED_DIR)/stretching_to_window.bmp.h

$(BUILD_DIR)/06_extension_libraries.o: $(EMBED_DIR)/png_loaded.png.h

$(BUILD_DIR)/07_texture_loading_and_rendering.o: $(EMBED_DIR)/rendering_texture.png.h

$(BUILD_DIR)/09_the_viewport.o: $(EMBED_DIR)/viewport.png.h

$(BUILD_DIR)/10_color_keying.o: $(EMBED_DIR)/earth_background.png.h $(EMBED_DIR)/space_shuttle_colorkey.png.h

$(BUILD_DIR)/11_clip_rendering.o: $(EMBED_DIR)/sprite_sheet.png.h

################################################################
# Targets Build rules
################################################################

$(BIN_DIR)/helloworld: $(HELLOWORLD_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(HELLOWORLD_LIBS)

$(BIN_DIR)/helloembed: $(HELLOEMBED_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(HELLOEMBED_LIBS)

$(BIN_DIR)/01_hello_sdl: $(01_hello_sdl_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(01_hello_sdl_LIBS)

$(BIN_DIR)/02_image_on_screen: $(02_image_on_screen_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(02_image_on_screen_LIBS)

$(BIN_DIR)/03_event_driven_programming: $(03_event_driven_programming_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(03_event_driven_programming_LIBS)

$(BIN_DIR)/04_key_presses: $(04_key_presses_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(04_key_presses_LIBS)

$(BIN_DIR)/05_optimized_surface_and_soft_stretching: $(05_optimized_surface_and_soft_stretching_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(05_optimized_surface_and_soft_stretching_LIBS)

$(BIN_DIR)/06_extension_libraries: $(06_extension_libraries_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(06_extension_libraries_LIBS)

$(BIN_DIR)/07_texture_loading_and_rendering: $(07_texture_loading_and_rendering_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(07_texture_loading_and_rendering_LIBS)

$(BIN_DIR)/08_geometry_rendering: $(08_geometry_rendering_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(08_geometry_rendering_LIBS)

$(BIN_DIR)/09_the_viewport: $(09_the_viewport_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(09_the_viewport_LIBS)

$(BIN_DIR)/10_color_keying: $(10_color_keying_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(10_color_keying_LIBS)

$(BIN_DIR)/11_clip_rendering: $(11_clip_rendering_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(11_clip_rendering_LIBS)

################################################################
# Generic Build rules
################################################################

# Generic compile rule
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) $(DEPS_DIR)
	@$(call PRINT_RULE)
	$(CC) $(CFLAGS) -MD -MP -MF $(DEPS_DIR)/$*.d -c -o $@ $<

# Build embedded object + header
$(EMBED_DIR)/%.o $(EMBED_DIR)/%.h: $(ASSETS_DIR)/% | $(EMBED_DIR)
	@$(call PRINT_RULE)
	./embed.sh "$(EMBED_DIR)" "$(ASSETS_DIR)" "$<"

################################################################
# Directory creation rules
################################################################

$(DEPS_DIR) $(BUILD_DIR) $(INCLUDE_DIR) $(EMBED_DIR) $(BIN_DIR):
	@$(call PRINT_RULE)
	mkdir -vp $@

################################################################
# Phony rules
################################################################

clean:
	@$(call PRINT_RULE)
	rm -rfv $(DEPS_DIR) $(BUILD_DIR) $(INCLUDE_DIR) $(EMBED_DIR) $(BIN_DIR)
