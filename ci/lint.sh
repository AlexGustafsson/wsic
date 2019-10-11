#!/usr/bin/env bash

failed="false"
while [[ "$1" != "" ]]; do
  diffed="$(diff -d --unchanged-line-format="" --old-line-format="line %dn was: %L" --new-line-format="but expected: %L" "$1" <(clang-format -style=file "$1"))"
  if [[ ! -z "$diffed" ]]; then
    echo "Code style violation(s) found for file $1"
    echo "--------------------------------------"
    echo "$diffed"
    echo "--------------------------------------"
    failed="true"
  fi
  shift
done

[[ "$failed" = "true" ]] && exit 1 || exit 0
