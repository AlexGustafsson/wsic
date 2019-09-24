#!/usr/bin/env bash

# The first parameter is the path to the test build of wsic
wsic="$1"

if [[ -z "$wsic" ]]; then
  echo -e "\e[31mRequired path to WSIC missing\e[0m"
  exit 1
fi

tests=0
failed=0
passed=0

function assert() {
  if [[ $1 -eq 1 ]]; then
    echo -e "\e[32m$2: PASSED\e[0m"
  else
    echo -e "\e[31m$2: FAILED\e[0m"
  fi
}
export -f assert

function assertEquals() {
  result="$([[ "$1" == "$2" ]] && echo 1 || echo 0)"
  assert "$result" "$3"
}
export -f assertEquals

# Clean up on exit
function cleanup {
  if [[ ! -z "$wsicPID" ]]; then
    echo "Killing WSIC"
    kill "$wsicPID" > /dev/null 2>&1
  fi
}
trap cleanup EXIT

function runTest() {
  output="$("$1")"
  echo "$output"
  currentPassed="$(echo "$output" | grep -c ': PASSED')"
  currentFailed="$(echo "$output" | grep -c ': FAILED')"
  currentTests="$(($passed + $failed))"

  tests=$(($tests + $currentTests))
  failed=$(($failed + $currentFailed))
  passed=$(($passed + $currentPassed))
}

function runTests() {
  # Create report directories
  mkdir -p build/reports/integration

  # Start wsic in the background
  $wsic -p 8080 > "build/reports/integration/log.txt" 2>&1 &
  wsicPID="$!"

  # Wait for WSIC to start
  echo "Waiting for WSIC to start"
  sleep 5

  while read -r file; do
    echo "$file"
    echo "$(yes '=' | head -n "$(echo -n "$file" | wc -c)" | tr -d '\n')"
    runTest "$file"
  done <<<"$(find ci/integration/tests -type f -name 'test-*.sh')"
}

function printSummary() {
  echo "Integration test summary"
  summary="$(echo -e "$tests tests     $failed failed     $passed succeeded")"
  # Print ====.... the same width as the summary
  echo "$(yes '=' | head -n "$(echo -n "$summary" | wc -c)" | tr -d '\n')"
  echo -e "$summary"
  if [[ $failed -gt 0 ]]; then
    echo -e "\e[31mTest failed\e[0m"
  else
    echo -e "\e[32mTest succeeded\e[0m"
  fi
}

runTests
printSummary
