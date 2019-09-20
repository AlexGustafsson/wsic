#include "unity/unity.h"

#include "list-test.c"

int main() {
    UNITY_BEGIN();

    list_test_run();

    return UNITY_END();
}
