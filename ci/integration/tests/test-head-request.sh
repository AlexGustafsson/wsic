#!/usr/bin/env bash

function canRespondToHEADRequest {
  output="$(curl -sv -I localhost:8080/index.html 2>&1 | tr -d '\r')"
  isOK="$(grep -e '< HTTP/1.1 200 OK' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e '< Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e '< Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasNoBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 0 || echo 1)"

  assert "$isOK" "Response to HEAD request was 200 OK"
  assert "$hasNoBody" "Response to HEAD request has no html content"
  assertExists "$contentLength" "Response to HEAD request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to HEAD request has the content type text/html"
}

runTest canRespondToHEADRequest
