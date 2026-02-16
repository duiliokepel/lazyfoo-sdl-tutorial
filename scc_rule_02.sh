#!/bin/bash
set -u
fail=0

# Safety Critical Code Rule 02
rule_name=$'Safety Critical Code Rule 02'
rule_description=$''
rule_rationale=$''

# Styles
bold=$'\033[1m'
red_bold=$'\033[1;31m'
green_bold=$'\033[1;32m'
reset=$'\033[0m'

# Usage
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
grep_flags="-R -P -n -I -E -H -s --color=always"
pattern_while='(^|[^A-Za-z0-9_])(while[[:space:]]*\([[:space:]]*(1|true)[[:space:]]*\))'
pattern_for='(^|[^A-Za-z0-9_])(for[[:space:]]*\([[:space:]]*;[[:space:]]*;[[:space:]]*\))'

# check 1: while (1) / while (true) / do { ... } while (1| true}
out="$(grep $grep_flags "$pattern_while" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
    echo "---- ${bold}Rule 02${red_bold} failed${reset}: found while(1)/while(true)"
    echo "$out"
    fail=1
fi

# check 2: for(;;)
out="$(grep $grep_flags "$pattern_for" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
    echo "---- ${bold}Rule 02${red_bold} failed${reset}: found for(;;)"
    echo "$out"
    fail=1
fi

# Result
if [ "$fail" -eq 0 ]; then
    echo "${bold}${rule_name} ${green_bold}passed${reset}."
else
    echo "${bold}${rule_name}${reset}:"
    echo "${rule_description}"
    echo "${bold}Rationale:${reset}:"
    echo "${rule_rationale}"
fi

exit "$fail"
