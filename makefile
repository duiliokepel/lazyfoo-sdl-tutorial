# Clear all default suffixes
.SUFFIXES:

#This specifies targets that are not files
.PHONY: clean

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

OPTIMIZATION_FLAGS = -O3 -ffunction-sections -fdata-sections
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

01_first_window_OBJS = $(BUILD_DIR)/01_first_window.o $(BUILD_DIR)/trace.o
01_first_window_LIBS = -lSDL2
PROGRAMS += $(BIN_DIR)/01_first_window
ALL_OBJS += $(01_first_window_OBJS)

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

################################################################
# Targets Build rules
################################################################

$(BIN_DIR)/helloworld: $(HELLOWORLD_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(HELLOWORLD_LIBS)

$(BIN_DIR)/helloembed: $(HELLOEMBED_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(HELLOEMBED_LIBS)

$(BIN_DIR)/01_first_window: $(01_first_window_OBJS) | $(BIN_DIR)
	@$(call PRINT_RULE)
	$(CC) $(LDFLAGS) -o $@ $^ $(01_first_window_LIBS)

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
