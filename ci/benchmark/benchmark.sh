#!/usr/bin/env bash

# The first parameter is the path to a build of wsic
# Use the debug build to detect memory leaks
wsic="$1"

if [[ -z "$wsic" ]]; then
  echo -e "\e[31mRequired path to WSIC missing\e[0m"
  exit 1
fi

# Clean up on exit
function cleanup {
  if [[ ! -z "$wsicPID" ]]; then
    echo "Killing WSIC"
    kill "$wsicPID" > /dev/null 2>&1
  fi
}
trap cleanup EXIT

# Create report directories
mkdir -p build/reports/benchmark/consecutive
mkdir -p build/reports/benchmark/concurrent

testFailed="false"
function benchmark {
  test="$1"
  shift

  echo "Performing test: $test"
  echo "--------------------------------------"

  # Start wsic in the background
  $wsic start > "build/reports/benchmark/$test/log.txt" 2>&1 &
  wsicPID="$!"

  # Wait for WSIC to start
  echo "Waiting for WSIC to start"
  sleep 5

  # Perform benchmark
  ab "$@" -g "build/reports/benchmark/$test/temp-data.p" 127.0.0.1:8080/index.html > "build/reports/benchmark/$test/report.txt"
  benchmarkExitCode="$?"

  # Kill WSIC
  kill "$wsicPID" > /dev/null 2>&1
  wsicPID=""

  if [[ "$benchmarkExitCode" = "0" ]]; then
    # ab always seem to have the last five-or-so requests be slow, no matter the number of requests
    head -n -5 "build/reports/benchmark/$test/temp-data.p" > "build/reports/benchmark/$test/data.p"

    # Create plot if the benchmark created data points for us
    echo "Creating plot"
    gnuplot "ci/benchmark/$test.p"
  else
    # Fail the test if the benchmark failed to urn
    testFailed="true"
    echo -e "\e[31mBenchmark failed for test $test\e[0m: The benchmark was not completed"
    echo "--------------------------------------"
    return
  fi

  failedRequests="$(grep "Failed requests:\s\+[0-9]\+" "build/reports/benchmark/$test/report.txt" | tail -1 | sed 's/\s*Failed requests:\s\+\([0-9]\+\)/\1/')"
  timePerRequest="$(grep "Time per request:\s\+[0-9]\+" "build/reports/benchmark/$test/report.txt" | tail -1 | sed 's/\s*Time per request:\s\+\([0-9\.]\+\).*/\1/')"

  if [[ ! "$failedRequests" = "0" ]]; then
    testFailed="true"
    echo -e "\e[31mBenchmark failed for test $test\e[0m: $failedRequests failed requests"
  else
    echo -e "\e[32mBenchmark for test $test had 0 failed requests\e[0m"
  fi

  if [[ "$(echo "$timePerRequest < 1" | bc)" = "0" ]]; then
    testFailed="true"
    echo -e "\e[31mBenchmark failed for test $test\e[0m: The time per request was $timePerRequest"
  else
    echo -e "\e[32mBenchmark for test $test had a request time of $timePerRequest ms per request\e[0m"
  fi

  echo "--------------------------------------"
}

# Benchmark 5 000 consecutive requests
benchmark consecutive -n 5000

# Benchmark 50 concurrent requests, 5 000 total requests
benchmark concurrent -c 50 -n 5000

if [[ "$testFailed" = "true" ]]; then
  echo -e "\e[31mTest failed\e[0m"
  exit 1
else
  echo -e "\e[32mTest succeeded\e[0m"
fi
