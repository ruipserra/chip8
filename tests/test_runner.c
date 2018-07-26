#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/chip8.h"

static uint16_t get_instruction_at(chip8_t* ch8, uint16_t addr) {
  return (ch8->mem[addr] << 8) | ch8->mem[addr + 1];
}

static void set_instruction_at(chip8_t* ch8, uint16_t addr, uint16_t instr) {
  ch8->mem[addr] = (instr >> 8);
  ch8->mem[addr + 1] = instr & 0xFF;
}

static void test_loading_rom() {
  chip8_t ch8;
  chip8_init(&ch8);
  assert(chip8_load_rom(&ch8, "./rocket.ch8"));

  assert(get_instruction_at(&ch8, 0x0200) == 0x6100);
  assert(get_instruction_at(&ch8, 0x0202) == 0x6200);
  assert(get_instruction_at(&ch8, 0x027E) == 0x7CD6);
  assert(get_instruction_at(&ch8, 0x0280) == 0x7C00);
  assert(get_instruction_at(&ch8, 0x0282) == 0x0000);
}

static void test_run_instruction() {
  chip8_t ch8;

  // 1MMM - Go to MMM
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x1124);
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0124);

  // BMMM - Go to MMM + V0
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xB124);
  ch8.reg_v[0] = 6;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x012A);

  // 2MMM - Do subroutine at 0MMM (must end with 00EE)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x2124);
  chip8_run_instruction(&ch8);
  assert(ch8.stack[0] == 0x0202);
  assert(ch8.sp == 1);
  assert(ch8.ip == 0x0124);

  // 00EE - Return from subroutine
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x00EE);
  ch8.stack[0] = 0x0124;
  ch8.sp = 1;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0124);

  // 3XKK - Skip next instruction if VX == KK
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x3524);
  ch8.reg_v[5] = 0x24;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x3524);
  ch8.reg_v[5] = 0x25;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  // 4XKK - Skip next instruction if VX != KK
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x4524);
  ch8.reg_v[5] = 0x25;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x4524);
  ch8.reg_v[5] = 0x24;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  // 5XY0 - Skip next instruction if VX == VY
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x5240);
  ch8.reg_v[2] = 0x42;
  ch8.reg_v[4] = 0x42;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x5240);
  ch8.reg_v[2] = 0x42;
  ch8.reg_v[4] = 0x24;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  // 9XY0 - Skip next instruction if VX != VY
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x9240);
  ch8.reg_v[2] = 0x42;
  ch8.reg_v[4] = 0x24;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x9240);
  ch8.reg_v[2] = 0x42;
  ch8.reg_v[4] = 0x42;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  // EX9E - Skip next instruction if VX == hexadecimal key (LSD)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE19E);
  ch8.reg_v[1] = 4;
  ch8.keypress = 4;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE19E);
  ch8.reg_v[1] = 0;
  ch8.keypress = 1;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE19E);
  ch8.reg_v[1] = 0;
  ch8.keypress = CHIP8_NO_KEY_PRESSED;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  // EXA1 - Skip next instruction if VX != hexadecimal key (LSD)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE1A1);
  ch8.reg_v[1] = 4;
  ch8.keypress = 4;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE1A1);
  ch8.reg_v[1] = 0;
  ch8.keypress = 1;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xE1A1);
  ch8.reg_v[1] = 0;
  ch8.keypress = CHIP8_NO_KEY_PRESSED;
  chip8_run_instruction(&ch8);
  assert(ch8.ip == 0x0204);

  // 6XKK - Let VX = KK
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x6A05);
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[10] == 0x05);
  assert(ch8.ip == 0x0202);

  // CXKK - Let VX = random byte (KK = mask)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xCA1A);
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[10] == 0x02);
  assert(ch8.ip == 0x0202);

  // 7XKK - Let VX = VX + KK
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x7BCD);
  ch8.reg_v[11] = 0x02;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[11] == 0xCF);
  assert(ch8.ip == 0x0202);

  // 8XY0 - Let VX = VY
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8120);
  ch8.reg_v[1] = 0x01;
  ch8.reg_v[2] = 0x02;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x02);
  assert(ch8.reg_v[2] == 0x02);
  assert(ch8.ip == 0x0202);

  // 8XY1 - Let VX = VX/VY (VF changed)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8121);
  ch8.reg_v[1] = 0x04;
  ch8.reg_v[2] = 0x02;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x02);
  assert(ch8.reg_v[2] == 0x02);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8121);
  ch8.reg_v[1] = 0x01;
  ch8.reg_v[2] = 0x03;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x00);
  assert(ch8.reg_v[2] == 0x03);
  assert(ch8.ip == 0x0202);

  // 8XY2 - Let VX = VX & VY (VF changed)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8122);
  ch8.reg_v[1] = 0x07;
  ch8.reg_v[2] = 0x0C;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x04);
  assert(ch8.reg_v[2] == 0x0C);
  assert(ch8.ip == 0x0202);

  // 8XY4 - Let VX = VX + VY (VF = 0 if VX + VY <= FF,
  // VF == 01 if VX + VY > FF)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8124);
  ch8.reg_v[1] = 0x07;
  ch8.reg_v[2] = 0x0C;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x13);
  assert(ch8.reg_v[2] == 0x0C);
  assert(ch8.reg_v[15] == 0x00);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8124);
  ch8.reg_v[1] = 0xFE;
  ch8.reg_v[2] = 0x01;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0xFF);
  assert(ch8.reg_v[15] == 0x00);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8124);
  ch8.reg_v[1] = 0xFE;
  ch8.reg_v[2] = 0x02;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x00);
  assert(ch8.reg_v[15] == 0x01);
  assert(ch8.ip == 0x0202);

  // 8XY5 - Let VX = VX - VY (VF = 00 if VX < VY,
  // VF == 01 if VX >= VY)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8125);
  ch8.reg_v[1] = 0x07;
  ch8.reg_v[2] = 0x0C;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0xFB);
  assert(ch8.reg_v[15] == 0x00);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8125);
  ch8.reg_v[1] = 0x07;
  ch8.reg_v[2] = 0x07;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x00);
  assert(ch8.reg_v[15] == 0x01);
  assert(ch8.ip == 0x0202);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x8125);
  ch8.reg_v[1] = 0x02;
  ch8.reg_v[2] = 0x01;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x01);
  assert(ch8.reg_v[15] == 0x01);
  assert(ch8.ip == 0x0202);

  // FX07 - Let VX = current timer value
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF107);
  ch8.reg_v[1] = 0x12;
  ch8.timer = 0x0C;
  chip8_run_instruction(&ch8);
  // assert(ch8.reg_v[1] == 0x0C); // TODO depends on clock speed
  assert(ch8.ip == 0x0202);

  // FX0A - Let VX = hexadecimal key digit (waits for key press)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF10A);
  ch8.reg_v[1] = 0x11;
  ch8.keypress = CHIP8_NO_KEY_PRESSED;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x11);
  assert(ch8.ip == 0x0200);

  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF10A);
  ch8.reg_v[1] = 0x11;
  ch8.keypress = 7;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[1] == 0x07);
  assert(ch8.ip == 0x0202);

  // FX15 - Set timer = VX (01 = 1/60 second)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF115);
  ch8.reg_v[1] = 0xF0;
  chip8_run_instruction(&ch8);
  assert(ch8.timer == 0xF0);
  assert(ch8.ip == 0x0202);

  // FX18 - Set tone duration = VX (01 = 1/60 second)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF118);
  ch8.reg_v[1] = 0xF0;
  chip8_run_instruction(&ch8);
  assert(ch8.tone_clock == 0xF0);
  assert(ch8.ip == 0x0202);

  // AMMM - Set I = 0MMM
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xA123);
  chip8_run_instruction(&ch8);
  assert(ch8.reg_i == 0x0123);
  assert(ch8.ip == 0x0202);

  // FX1E - Let I = I + VX
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF21E);
  ch8.reg_i = 0x0422;
  ch8.reg_v[2] = 0x0A;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_i == 0x042C);
  assert(ch8.ip == 0x0202);

  // FX29 - Let I = 5 byte display pattern for LSD of VX
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xFA29);
  ch8.reg_v[10] = 0xD2;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_i == CHIP8_DIGITS_START_ADDRESS + (2 * 5));
  assert(ch8.ip == 0x0202);

  // FX33 - Let MI = 3 decimal digit equivalent of VX (I unchanged)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF933);
  ch8.reg_i = 0x0422;
  ch8.reg_v[9] = 0xA7;
  chip8_run_instruction(&ch8);
  assert(ch8.mem[0x0422] == 0x01);
  assert(ch8.mem[0x0423] == 0x06);
  assert(ch8.mem[0x0424] == 0x07);
  assert(ch8.ip == 0x0202);

  // FX55 - Let MI = V0 : VX (I = I + X + 1)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF255);
  ch8.reg_i = 0x0422;
  ch8.reg_v[0] = 0x01;
  ch8.reg_v[1] = 0x06;
  ch8.reg_v[2] = 0x07;
  ch8.reg_v[3] = 0xDF;
  chip8_run_instruction(&ch8);
  assert(ch8.mem[0x0422] == 0x01);
  assert(ch8.mem[0x0423] == 0x06);
  assert(ch8.mem[0x0424] == 0x07);
  assert(ch8.mem[0x0425] == 0x00);
  assert(ch8.reg_i == 0x0425);
  assert(ch8.ip == 0x0202);

  // FX65 - Let V0 : VX = MI (I = I + X + 1)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xF265);
  ch8.reg_i = 0x0422;
  ch8.mem[0x0422] = 0x01;
  ch8.mem[0x0423] = 0x06;
  ch8.mem[0x0424] = 0x07;
  chip8_run_instruction(&ch8);
  assert(ch8.reg_v[0] == 0x01);
  assert(ch8.reg_v[1] == 0x06);
  assert(ch8.reg_v[2] == 0x07);
  assert(ch8.reg_i == 0x0425);
  assert(ch8.ip == 0x0202);

  // 00E0 - Erase display (all 0s)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x00E0);
  ch8.framebuffer[0x42] = 0xFF;
  chip8_run_instruction(&ch8);
  assert(ch8.framebuffer[0x42] == 0);
  assert(ch8.ip == 0x0202);

  // DXYN - Show n byte MI pattern at VX - VY coordinates.
  // I unchanged. MI pattern is combined with existing display via exclusive-OR
  // function. VF = 01 if a 1 in MI pattern matches 1 in existing display.
  // TODO
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0xD123);
  ch8.reg_v[1] = 0x20;
  ch8.reg_v[2] = 0x10;
  ch8.reg_i = 0x0300;
  ch8.mem[0x300] = 0x01;
  ch8.mem[0x301] = 0x04;
  ch8.mem[0x302] = 0x08;
  ch8.mem[0x303] = 0xFF;
  ch8.framebuffer[(0x20 + (0x11 * (CHIP8_FRAMEBUFFER_MAX_X + 1))) / 8] = 0XFF;
  chip8_run_instruction(&ch8);
  assert(ch8.framebuffer[(0x20 + (0x10 * (CHIP8_FRAMEBUFFER_MAX_X + 1))) / 8] ==
         0x01);
  assert(ch8.framebuffer[(0x20 + (0x11 * (CHIP8_FRAMEBUFFER_MAX_X + 1))) / 8] ==
         0xFB);
  assert(ch8.framebuffer[(0x20 + (0x12 * (CHIP8_FRAMEBUFFER_MAX_X + 1))) / 8] ==
         0x08);
  assert(ch8.framebuffer[(0x20 + (0x13 * (CHIP8_FRAMEBUFFER_MAX_X + 1))) / 8] ==
         0x00);
  assert(ch8.reg_v[15] == 0x01);
  assert(ch8.ip == 0x0202);

  // 0MMM - Do machine language subroutine at 0MMM (subroutine must end with D4
  // byte)
  chip8_init(&ch8);
  set_instruction_at(&ch8, 0x0200, 0x00D4);
  chip8_run_instruction(&ch8);
  assert(ch8.stack[0] == 0x0202);
  assert(ch8.sp == 1);
  assert(ch8.ip == 0x00D4);
}

int main() {
  test_loading_rom();
  test_run_instruction();

  printf("\33[1;32m🎉 Tests passed! 🎉\33[m\n");
}
