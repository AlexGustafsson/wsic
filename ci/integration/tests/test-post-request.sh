#!/usr/bin/env bash

function canRespondToPOSTRequest {
  # Create a temporary file of some bytes
  dd if=/dev/urandom of=512.data bs=512 count=1
  output="$(curl -sv -X POST -d @512.data localhost:8080/cgi/form 2>&1 | tr -d '\r')"
  isOK="$(grep -e '< HTTP/1.1 200 OK' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasBody="$(grep -e '<html>' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$isOK" "Response to POST request was 200 OK"
  assert "$hasBody" "Response to POST request has html content"
  rm 512.data
}

canRespondToPOSTRequest
