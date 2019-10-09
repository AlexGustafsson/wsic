#!/usr/bin/env bash

function canRespondToGETRequest {
  output="$(curl -sv localhost:8080/index.html 2>&1 | tr -d '\r')"
  isOK="$(grep -e '< HTTP/1.1 200 OK' <<<"$output" > /dev/null && echo 1 || echo 0)"
  contentLength="$(grep -e '< Content-Length:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  contentType="$(grep -e '< Content-Type:' <<<"$output" | sed -e 's/.*: \(.*\)/\1/')"
  hasBody="$(grep -e '<html>' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$isOK" "Response to GET request was 200 OK"
  assert "$hasBody" "Response to GET request has html content"
  assertExists "$contentLength" "Response to GET request has a content length specified"
  assertEquals "text/html" "$contentType" "Response to GET request has the content type text/html"
}

canRespondToGETRequest
