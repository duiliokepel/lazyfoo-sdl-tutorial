#!/bin/bash
set -u
fail=0

# Safety Critical Coding Rule 01
rule_name=$'Safety Critical Code Rule 01'
rule_description=$'Restrict all code to very simple control flow constructs - do not use goto statements, setjmp or longjmp constructs, and direct or indirect recursion.'
rule_rationale=$'Simpler control flow translates into stronger capabilities for verification and often results in improved code clarity. The banishment of recursion is perhaps the biggest surprise here. Without recursion, though, we are guaranteed to have an acyclic function call graph, which can be exploited by code analyzers, and can directly help to prove that all executions that should be bounded are in fact bounded. (Note that this rule does not require that all functions have a single point of return - although this often also simplifies control flow. There are enough cases, though, where an early error return is the simpler solution.)'

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
grep_flags="-R -n -I -E -H -s --color=always"
pattern_goto='(^|[^A-Za-z0-9_])goto([^A-Za-z0-9_]|$)'
pattern_jmp='(^|[^A-Za-z0-9_])(sigsetjmp|setjmp|_setjmp|siglongjmp|longjmp|_longjmp)([^A-Za-z0-9_]|$)'

# check 1: goto
out="$(grep $grep_flags "$pattern_goto" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
    echo "${bold}${rule_name} ${red_bold}failed${reset}: found 'goto'"
    echo "$out"
    fail=1
fi

# check 2: setjmp/longjmp family
out="$(grep $grep_flags "$pattern_jmp" "$@" 2>/dev/null || true)"
if [ -n "$out" ]; then
    echo "${bold}${rule_name} ${red_bold}failed${reset}: found setjmp/longjmp usage"
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
