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
valgrind --leak-check=full --show-leak-kinds=all $wsic start &> "build/reports/memory-leaks/log.txt" &
wsicPID="$!"

# Wait for WSIC to start
echo "Waiting for WSIC to start"
sleep 5

# Test currently available paths (simulate user actions)
curl http://localhost:8080 > /dev/null
curl --insecure https://localhost:8443 > /dev/null

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

valgrindLog="$(grep --text -e '==[0-9]\+==' "build/reports/memory-leaks/log.txt")"
grep 'lost:' <<<"$valgrindLog" &> /dev/null
exitCode="$?"

if [[ "$exitCode" == 0 ]]; then
  echo "$valgrindLog"
  echo -e "\e[31mMemory leaks detected\e[0m"
  exit 1
else
  echo -e "\e[32mNo memory leaks detected\e[0m"
  exit 0
fi
