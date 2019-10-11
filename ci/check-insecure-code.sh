#!/usr/bin/env bash

sourceFolder="$1"
if [[ -z "$sourceFolder" ]]; then
  echo -e "\e[31mRequired path to source missing\e[0m"
  exit 1
fi

testFailed="false"
function testFunctionUse {
  function="$1"
  echo "Checking for uses of $function"
  echo "--------------------------------------"
  grep -Rn "$function" "$sourceFolder"
  if [[ "$?" == 0 ]]; then
    echo -e "\e[31m$function is used\e[0m"
    testFailed="true"
  else
    echo -e "\e[32m$function is not used\e[0m"
  fi
  echo "--------------------------------------"
}

testFunctionUse "strcpy"
testFunctionUse "strcat"
testFunctionUse "sprintf"
testFunctionUse "snprintf"
testFunctionUse "vsprintf"
testFunctionUse "gets"
testFunctionUse "makepath"
testFunctionUse "_splitpath"
testFunctionUse "scanf"
testFunctionUse "sscanf"
testFunctionUse "snscanf"
testFunctionUse "strtok"
testFunctionUse "itoa"
testFunctionUse "strncpy"

if [[ "$testFailed" = "true" ]]; then
  echo -e "\e[31mTest failed\e[0m"
  exit 1
else
  echo -e "\e[32mTest succeeded\e[0m"
fi
