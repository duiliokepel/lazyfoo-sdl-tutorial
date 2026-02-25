#!/bin/bash
set -u
fail=0

# Safety Critical Code Rule 02
rule_name=$'Safety Critical Code Rule 02'
rule_description=$'All loops must have a fixed upper-bound. It must be trivially possible for a checking tool to prove statically that a preset upper-bound on the number of iterations of a loop cannot be exceeded. If the loop-bound cannot be proven statically, the rule is considered violated.'
rule_rationale=$'The absence of recursion and the presence of loop bounds prevents runaway code. This rule does not, of course, apply to iterations that are meant to be non-terminating (e.g., in a process scheduler). In those special cases, the reverse rule is applied: it should be statically provable that the iteration cannot terminate. One way to support the rule is to add an explicit upper-bound to all loops that have a variable number of iterations (e.g., code that traverses a linked list). When the upper-bound is exceeded an assertion failure is triggered, and the function containing the failing iteration returns an error. (See Rule 5 about the use of assertions.)'

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

# -r    recursively scan sub-directories
# -n    print line number with output lines
# -o    show only the part of the line that matched
# -I    treat binary files as not matching (ignore)
# -M    run in multiline mode
# -s    suppress error messages
pcre2grep_flags='-r -n -o -I -M -s --color=always'
pattern_unbounded='(?(DEFINE)(?<P>\((?:[^()]++|(?&P))*\)))(?:\bwhile\s*(?!\(\s*LOOPBOUND\s*(?&P)\s*\)\s*\{)(?&P)\s*\{|\bfor\s*(?!\(\s*(?s:.*?)\s*;\s*LOOPBOUND\s*(?&P)\s*;\s*(?s:.*?)\s*\)\s*\{)(?&P)\s*\{|\}\s*while\s*(?!\(\s*LOOPBOUND\s*(?&P)\s*\)\s*;)(?&P)\s*;)'

out="$(pcre2grep $pcre2grep_flags "$pattern_unbounded" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
    echo "${bold}${rule_name} ${red_bold}failed${reset}: found unbounded while/for loops"
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
