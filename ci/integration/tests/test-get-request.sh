#!/usr/bin/env bash

# Test that the reserver allows a connection and responds with something
function canRespondToGETRequest {
  output="$(curl -v localhost:8080/index.html 2>&1)"
  # The connection is left intact by curl if the request was finished
  leftIntact="$(echo "$output" | grep -ce 'host localhost left intact')"
  assert "$leftIntact" "Can respond to GET request"
}

canRespondToGETRequest
