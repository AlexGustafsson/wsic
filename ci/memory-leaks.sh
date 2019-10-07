#!/usr/bin/env bash

# The first parameter is the path to a debug build of wsic
wsic="$1"

if [[ -z "$wsic" ]]; then
  echo -e "\e[31mRequired path to WSIC missing\e[0m"
  exit 1
fi

# Clean up on exit
function cleanup {
  if [[ ! -z "$wsicPID" ]]; then
    kill "$wsicPID" &> /dev/null
  fi
}
trap cleanup EXIT

# Create report directories
mkdir -p build/reports/memory-leaks

# Start wsic in the background with leak detection enabled
export ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=asan-ignores.txt
$wsic start &> "build/reports/memory-leaks/log.txt" &
wsicPID="$!"

# Wait for WSIC to start
echo "Waiting for WSIC to start"
sleep 5

# Test currently available paths (simulate user actions)
curl http://localhost:8080 > /dev/null
curl -I http://localhost:8080 > /dev/null
curl --insecure https://localhost:8443 > /dev/null
curl -I --insecure https://localhost:8443 > /dev/null

curl http://localhost:8080/cgi > /dev/null
curl --insecure https://localhost:8443/cgi > /dev/null

curl http://localhost:8080/cgi/environment > /dev/null
curl --insecure https://localhost:8443/cgi/environment > /dev/null

curl http://localhost:8080/cgi/environment > /dev/null
curl --insecure https://localhost:8443/cgi/environment > /dev/null

curl -X POST -d "foo=bar" http://localhost:8080/cgi/post > /dev/null
curl --insecure -X POST -d "foo=bar" https://localhost:8443/cgi/post > /dev/null

kill "$wsicPID" &> /dev/null

echo "Waiting for WSIC to exit"
sleep 5

leaks="$(grep --text -e '    #' -e 'leak of' -e 'detected memory leaks' "build/reports/memory-leaks/log.txt")"
exitCode="$(echo "$?")"

if [[ "$exitCode" == 0 ]]; then
  echo "$leaks"
  echo -e "\e[31mMemory leaks detected\e[0m"
  exit 1
else
  echo -e "\e[32mNo memory leaks detected\e[0m"
  exit 0
fi
