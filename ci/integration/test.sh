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

currentPassed=0
currentFailed=0

netcatHasTimeout="$(command nc 2>&1 | grep -e "-w" > /dev/null && echo 1 || echo 0)"
export netcatHasTimeout
netcatHasReceiveTimeout="$(command nc 2>&1 | grep -e "-q" > /dev/null && echo 1 || echo 0)"
export netcatHasReceiveTimeout

function nc() {
  parameters=""
  if [[ "$netcatHasTimeout" -eq 1 ]]; then
    parameters="-w 10 $parameters"
  fi
  if [[ "$netcatHasReceiveTimeout" -eq 1 ]]; then
    parameters="-q 10 $parameters"
  fi
  command nc $parameters "$@"
}
export -f nc

function beginTest() {
  echo -e "\n$1"
  # Print ====.... the same width as the summary
  echo "$(echo -n "$1" | tr '[:alnum:]-./' '=')"

  tests="$((tests + 1))"
  currentFailed=0
}
export -f beginTest

function endTest() {
  if [[ "$currentFailed" -gt  0 ]]; then
    echo -e "\e[31mTest $1: FAILED\e[0m"
  else
    echo -e "\e[32mTest $1: PASSED\e[0m"
  fi

  currentFailed=0
}
export -f endTest

function assert() {
  if [[ $1 -eq 1 ]]; then
    echo -e "\e[32m$2: PASSED\e[0m"
    currentPassed="$(($currentPassed + 1))"
  else
    echo -ne "\e[31m$2: FAILED\e[0m"
    currentFailed="$(($currentFailed + 1))"
    if [[ ! -z "$3" ]]; then
      echo -ne " - $3"
    fi
    echo ""
  fi
}
export -f assert

function assertEquals() {
  assert "$([[ "$1" == "$2" ]] && echo 1 || echo 0)" "$3" "expected '$1', got '$2'"
}
export -f assertEquals

function assertExists() {
  assert "$([[ -z "$1" ]] && echo 0 || echo 1)" "$2"
}
export -f assertExists

# Clean up on exit
function cleanup {
  if [[ ! -z "$wsicPID" ]]; then
    echo "Killing WSIC"
    kill "$wsicPID" > /dev/null 2>&1
  fi
}
trap cleanup EXIT

function runTest() {
  beginTest "$1"
  $1
  endTest "$1"
}
export -f runTest

function runTests() {
  # Create report directories
  mkdir -p build/reports/integration

  # Start wsic in the background
  $wsic start --verbose > "build/reports/integration/log.txt" 2>&1 &
  wsicPID="$!"

  # Wait for WSIC to start
  echo "Waiting for WSIC to start"
  sleep 5

  while read -r file; do
    echo -e "\n\e[1m$file"
    # Print ====.... the same width as the summary
    echo "$(echo -n "$file" | tr '[:alnum:]-./' '=')"
    echo -e "\e[0m"

    output="$($file)"
    echo "$output"
    currentFailed="$(grep -ce 'Test [a-zA-Z0-9]\+: FAILED' <<<"$output")"
    currentPassed="$(grep -ce 'Test [a-zA-Z0-9]\+: PASSED' <<<"$output")"
    tests="$(($tests + $currentFailed + $currentPassed))"
    failed="$(($failed + $currentFailed))"
    passed="$(($passed + $currentPassed))"

    isAlive="$(kill -0 $wsicPID &> /dev/null && echo 1 || echo 0)"
    if [[ $isAlive -eq 0 ]]; then
      echo -e "\e[31mWSIC has been terminated - cannot proceed test\e[0m"
      exit 1
    fi
  done <<<"$(find ci/integration/tests -type f -name 'test-*.sh')"
}

function printSummary() {
  echo -e "\nIntegration test summary"
  summary="$(echo -e "$tests tests     $failed failed     $passed succeeded")"
  # Print ====.... the same width as the summary
  echo "$(echo -n "$summary" | tr '[:alnum:] ' '=')"
  echo -e "$summary"
  if [[ $failed -gt 0 ]]; then
    echo -e "\e[31mTest failed\e[0m"
    exit 1
  else
    hasCrashed="$(grep -e '==ERROR' "build/reports/integration/log.txt" &> /dev/null && echo 1 || echo 0)"
    if [[ "$hasCrashed" -eq 1 ]]; then
      echo -e "\e[31mTest failed. WSIC crashed\e[0m"
      exit 1
    else
      echo -e "\e[32mTest succeeded\e[0m"
      exit 0
    fi
  fi
}

runTests
printSummary
