#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "../src/chip8.h"

static uint16_t get_instruction_at(chip8_t* ch8, uint16_t addr) {
  return (ch8->mem[addr] << 8) | ch8->mem[addr + 1];
}

static void test_loading_rom() {
  chip8_t ch8;
  chip8_init(&ch8);
  assert(chip8_load_rom(&ch8, "./rocket.ch8"));

  assert(get_instruction_at(&ch8, 0x01FE) == 0x0000);
  assert(get_instruction_at(&ch8, 0x0200) == 0x6100);
  assert(get_instruction_at(&ch8, 0x0202) == 0x6200);
  assert(get_instruction_at(&ch8, 0x027E) == 0x7CD6);
  assert(get_instruction_at(&ch8, 0x0280) == 0x7C00);
  assert(get_instruction_at(&ch8, 0x0282) == 0x0000);
}

int main() {
  test_loading_rom();

  printf("\33[1;32m🎉 Tests passed! 🎉\33[m\n");
}
