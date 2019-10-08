#!/usr/bin/env bash

output="$(make clean analyze)"
echo "$output"

bugs="$(grep -c 'scan-build: [0-9]\+ bug' <<<"$output" | sed 's/.* \([0-9]\+\) .*/\1/')"

if [[ "$bugs" == "" ]] || [[ "$bugs" -eq 0 ]]; then
  echo -e "\e[32mNo bugs found\e[0m"
  exit 0
else
  echo -e "\e[31mFound $bugs bug(s)\e[0m"
  exit 1
fi
