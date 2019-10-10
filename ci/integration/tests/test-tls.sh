#!/usr/bin/env bash

function canUpgradeInsecureRequest() {
  output="$(curl -sv -H "Upgrade-Insecure-Requests: 1" http://localhost:8080 2>&1 | tr -d '\r')"
  isRedirect="$(grep -e '< HTTP/1.1 301' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasSecureLocation="$(grep -e 'Location: https://' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$isRedirect" "Response to Upgrade Insecure Requests request was 301 Moved Permanently"
  assert "$hasSecureLocation" "Response to Upgrade Insecure Requests request has a secure Location set"
}

function cannotUpgradeSecureRequest() {
  output="$(curl --insecure -sv -H "Upgrade-Insecure-Requests: 1" https://localhost:8443 2>&1 | tr -d '\r')"
  isOK="$(grep -e '< HTTP/1.1 200 OK' <<<"$output" > /dev/null && echo 1 || echo 0)"
  hasBody="$(grep -e '<html' <<<"$output" > /dev/null && echo 1 || echo 0)"

  assert "$isOK" "Response to Upgrade Insecure Requests to already secure location was 200 OK"
  assert "$hasBody" "Response to Upgrade Insecure Requests to already secure location has html content"
}

runTest canUpgradeInsecureRequest
runTest cannotUpgradeSecureRequest
