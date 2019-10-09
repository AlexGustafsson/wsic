#!/usr/bin/env bash

function cannotParseIncorrectRequestLine() {
  output="$(echo -ne "GET /in / as\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectRequestLine2() {
  output="$(echo -ne "GET /in / as\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectMethod() {
  output="$(echo -ne "check /index.html HTTP/1.1\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectVersion() {
  output="$(echo -ne "GET /index.html HTP/1.1\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectVersion2() {
  output="$(echo -ne "GET /index.html HTTP 1.1\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectHeaders() {
  output="$(echo -ne "GET /index.html HTTP/1.1\r\nHost localhost\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectHeaders2() {
  output="$(echo -ne "GET /index.html HTTP/1.1\r\nHost:localhost\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

function cannotParseIncorrectHeaders3() {
  output="$(echo -ne "GET /index.html HTTP/1.1\r\nHost:localhost\r\n\r\n" | nc -w 10 -G 10 localhost 8080 2>&1 | tr -d '\r')"
  failed="$(grep -e 'HTTP/1.1 400' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e 'Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e 'Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$failed" "Response to incorrect request was 400"
  assert "$hasBody" "Response to incorrect request has html content"
  assertExists "$contentLength" "Response to incorrect request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to incorrect request has the content type text/html"
}

runTest cannotParseIncorrectRequestLine
runTest cannotParseIncorrectRequestLine2

runTest cannotParseIncorrectMethod

runTest cannotParseIncorrectVersion
runTest cannotParseIncorrectVersion2

runTest cannotParseIncorrectHeaders
runTest cannotParseIncorrectHeaders2
runTest cannotParseIncorrectHeaders3
