#!/bin/bash
set -u
fail=0

# Safety Critical Coding Rule 06
rule_name=$'Safety Critical Code Rule 06'
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
