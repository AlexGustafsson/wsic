#!/usr/bin/env bash

function cannotSendLargeFiles() {
  # Create a temporary file of some bytes
  dd if=/dev/urandom of=5M.data bs=1M count=5 &>/dev/null
  output="$(curl -sv -X POST -d @5M.data localhost:8080/cgi/form 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 417' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to POST request with large body was 417"
  assert "$hasBody" "Response to POST request has html content"
  rm 5M.data
}

function cannotSendLargeFiles2() {
  # Create a temporary file of some bytes
  dd if=/dev/urandom of=5M.data bs=1M count=5 &>/dev/null
  output="$(echo -e "POST /cgi/form HTTP/1.1\r\nContent-Length: 100\r\nHost: localhost:8080\r\n\r\n$(cat 5M.data | tr -d '\0')" | nc -w 10 -G 10 localhost 8080 | tr -d '\r')"
  isOK="$(grep -e 'HTTP/1.1 200' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$isOK" "Response to POST request with large body with small Content-Length was 200"
  assert "$hasBody" "Response to POST request has html content"
  rm 5M.data
}

function cannotSendLargeFiles3() {
  # Create a temporary file of some bytes
  dd if=/dev/urandom of=5M.data bs=1M count=5 &>/dev/null
  output="$(echo -e "POST /cgi/form HTTP/1.1\r\nContent-Length: 100\r\nHost: localhost:8080\r\nContent-Length: 5242880\r\n\r\n$(cat 5M.data | tr -d '\0')" | nc -w 10 -G 10 localhost 8080 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 413' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to POST request with large body with correct Content-Length was 413"
  assert "$hasBody" "Response to POST request has html content"
  rm 5M.data
}

function cannotKeepConnectionOpenForever() {
  output="$(nc -d -w 10 -G 10 localhost 8080 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotKeepConnectionOpenForever2() {
  output="$(echo -ne "GET / HTTP/1.1\r\n" | nc -w 10 -G 10 localhost 8080 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

runTest cannotSendLargeFiles
runTest cannotSendLargeFiles2
runTest cannotSendLargeFiles3

runTest cannotKeepConnectionOpenForever
runTest cannotKeepConnectionOpenForever2
