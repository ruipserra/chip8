#include "../src/chip8.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static void test_that_it_works() { assert(true); }

int main() {
  test_that_it_works();

  printf("\33[1;32m🎉 Tests passed! 🎉\33[m\n");
}
