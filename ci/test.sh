#!/usr/bin/env bash

# The first parameter is the path to the test build of wsic
wsic="$1"

if [[ -z "$wsic" ]]; then
  echo -e "\e[31mRequired path to WSIC missing\e[0m"
  exit 1
fi

# Create report directory
mkdir -p build/reports/test

# Run tests and save the result
$wsic &> build/reports/test/report.txt

# Output the results with appropriate colors
echo -e "$(sed -e 's/^\(.*:PASS\)/\\e[32m\1\\e[0m/' -e 's/^\(.*:FAIL\)/\\e[31m\1\\e[0m/' -e 's/^\(.*:IGNORE\)/\\e[33m\1\\e[0m/' build/reports/test/report.txt)"

# Extract summary
summary=$(grep build/reports/test/report.txt -e '[0-9]\+ Tests [0-9]\+ Failures [0-9]\+ Ignored' | sed -e 's/\([0-9]\+\) Tests \([0-9]\+\) Failures \([0-9]\+\) Ignored/\1 \2 \3/')
tests=$(echo "$summary" | awk '{print $1}')
failures=$(echo "$summary" | awk '{print $2}')
ignored=$(echo "$summary" | awk '{print $3}')

if [[ $failures -gt 0 ]]; then
  echo -e "\e[31m$failures tests out of $tests failed ($ignored ignored)\e[0m"
  exit 1
else
  echo -e "\e[32mAll $tests tests succeeded ($ignored ignored)\e[0m"
fi
