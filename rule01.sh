#!/usr/bin/env bash
set -u

fail=0

# rule01: forbid goto + setjmp/longjmp

# Styles
bold=$'\033[1m'
red_bold=$'\033[1;31m'
green_bold=$'\033[1;32m'
reset=$'\033[0m'


# usage:
#   ./rule01.sh <paths...>
#   ./rule01.sh src include
#   ./rule01.sh src/*.c src/*.h
if [ $# -lt 1 ]; then
  echo "usage: $0 <paths...>" >&2
  exit 2
fi

# -R    recursive
# -n    show line numbers
# -I    ignore binary files
# -E    extended regex
# -H    always show filename
# -s    suppress "No such file" etc. (optional; remove if you prefer noise)
grep_flags="-R -n -I -E -H -s --color=always"
pattern_goto='(^|[^A-Za-z0-9_])goto([^A-Za-z0-9_]|$)'
pattern_jmp='(^|[^A-Za-z0-9_])(sigsetjmp|setjmp|_setjmp|siglongjmp|longjmp|_longjmp)([^A-Za-z0-9_]|$)'

# check 1: goto
out="$(grep $grep_flags "$pattern_goto" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
  echo "---- ${bold}Rule 01${red_bold} failed${reset}: found 'goto'"
  echo "$out"
  fail=1
fi

# check 2: setjmp/longjmp family
out="$(grep $grep_flags "$pattern_jmp" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
  echo "---- ${bold}Rule 01${red_bold} failed${reset}: found setjmp/longjmp usage"
  echo "$out"
  fail=1
fi

if [ "$fail" -eq 0 ]; then
  echo "---- ${bold}Rule 01${green_bold} passed${reset}."
fi

exit "$fail"
