#!/bin/sh
# Usage: ./embed.sh <input-file> <output-dir>

set -e

################################################################
### Step 1 - Validate arguments
################################################################

if [ $# -ne 3 ]; then
    echo "Usage: $0 <embed-dir> <assets-dir> <asset-file> " >&2
    exit 1
fi

EMBED_DIR="$1"
ASSETS_DIR="$2"
INPUT_FILE="$3"

echo "Embed dir.........[$EMBED_DIR]"
echo "Assets dir........[$ASSETS_DIR]"
echo "Input file........[$INPUT_FILE]"
echo ""

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file not found $INPUT_FILE" >&2
    exit 1
fi

################################################################
### Step 2 - Compose output files path
################################################################

# Normalize assets dir (strip trailing slash)
ASSETS_DIR_NOPSLASH="${ASSETS_DIR%/}"

# Assert input file resides within assets dir
case "$INPUT_FILE" in
  "$ASSETS_DIR_NOPSLASH"/*) ;;  # OK
  *)
    echo "Error: Input file [$INPUT_FILE] is not inside assets dir [$ASSETS_DIR_NOPSLASH]" >&2
    exit 1
    ;;
esac

# Derive relative path within assets tree
RELATIVE_PATH="${INPUT_FILE#${ASSETS_DIR_NOPSLASH}/}"
FILENAME="$(basename "$RELATIVE_PATH")"
DIR_PART="$(dirname "$RELATIVE_PATH")"

if [ "$DIR_PART" = "." ]; then
    DESTINATION_DIR="$EMBED_DIR"
else
    DESTINATION_DIR="$EMBED_DIR/$DIR_PART"
fi

OBJECT_PATH="$DESTINATION_DIR/${FILENAME}.o"
HEADER_PATH="$DESTINATION_DIR/${FILENAME}.h"

echo "Destination dir...[$DESTINATION_DIR]"
echo "Object path.......[$OBJECT_PATH]"
echo "Header path.......[$HEADER_PATH]"
echo ""


################################################################
### Step 3 - Link binary blob
################################################################

echo "Linking binary blob..."

mkdir -vp "$DESTINATION_DIR"
ld -r -b binary -o "$OBJECT_PATH" "$INPUT_FILE"

nm "$OBJECT_PATH"
echo ""

################################################################
### Step 4 - Rename Symbols
################################################################

echo "Renaming symbols..."

# Original ld symbols are based on the *exact* input path we passed to ld
ORIG_BASE="$(printf '%s' "$INPUT_FILE" | sed 's/[^A-Za-z0-9_]/_/g')"
ORIG_PREFIX="_binary_${ORIG_BASE}"

# New short prefix based on the relative path (assets/ stripped)
REL_BASE="$(printf '%s' "$RELATIVE_PATH" | sed 's/[^A-Za-z0-9_]/_/g')"
RENAMED_PREFIX="_embed_${REL_BASE}"

echo "Symbol prefix.....[$ORIG_PREFIX]"
echo "Renamed prefix....[$RENAMED_PREFIX]"

objcopy \
  --redefine-sym "${ORIG_PREFIX}_start"="${RENAMED_PREFIX}_start" \
  --redefine-sym "${ORIG_PREFIX}_end"="${RENAMED_PREFIX}_end" \
  --redefine-sym "${ORIG_PREFIX}_size"="${RENAMED_PREFIX}_size" \
  "$OBJECT_PATH"

nm "$OBJECT_PATH"
echo ""

################################################################
### Step 5 - Mark stack as non-executable
################################################################

objcopy --add-section .note.GNU-stack=/dev/null \
        --set-section-flags .note.GNU-stack=readonly "$OBJECT_PATH"

################################################################
### Step 6 - Generate header file
################################################################

echo "Generating header file..."

# ----------- Header guard from file name -----------
# Map letters to upper, and map '/', '.', '-' to '_' (put '-' at end to avoid tr range warning)
GUARD_NAME=$(echo "$FILENAME" | tr '[:lower:]/.-' '[:upper:]___')_H

# ----------- Write header -----------
cat > "$HEADER_PATH" <<EOF
#ifndef $GUARD_NAME
#define $GUARD_NAME

/* Embedded file: $RELATIVE_PATH */

#include <stdint.h>

extern const uint8_t ${RENAMED_PREFIX}_start[];
extern const uint8_t ${RENAMED_PREFIX}_end[];
//extern const uint64_t ${RENAMED_PREFIX}_size[];

// Compute at runtime to avoid absolute-symbol relocations
#define ${RENAMED_PREFIX}_size ((uint64_t)(${RENAMED_PREFIX}_end - ${RENAMED_PREFIX}_start))

#endif /* $GUARD_NAME */
EOF



echo "Done."
