#!/usr/bin/env bash

# Test that the reserver allows a POST request and responds with something
function canRespondToGETRequest {
  # Create a temporary file of some bytes
  dd if=/dev/urandom of=512k.data bs=1M count=0.5
  output="$(curl -v -X POST -d @512k.data localhost:8080 2>&1)"
  echo "$output"
  # The connection is left intact by curl if the request was finished
  leftIntact="$(echo "$output" | grep -ce 'host localhost left intact')"
  assert "$leftIntact" "Can respond to GET request"
  # Remove the temporary file
  rm 512k.data
}

canRespondToGETRequest
