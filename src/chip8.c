#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "chip8.h"

void chip8_init(chip8_t* ch8) {
  ch8->ip = CHIP8_PROGRAM_START_ADDRESS;
  ch8->reg_i = 0;
  for (int i = 0; i < CHIP8_REGISTER_COUNT; i++) ch8->reg_v[i] = 0;
  ch8->timer = 0;
  ch8->tone_clock = 0;
  ch8->sp = 0;
  memset(ch8->stack, 0, CHIP8_MEMORY_SIZE);
  memset(ch8->mem, 0, CHIP8_MEMORY_SIZE);
}

void chip8_debug(const chip8_t* ch8) {
  printf("---- Chip8 Debug ----\n");
  printf("  ip: %x\n", ch8->ip);
  printf("  reg_i: %x\n", ch8->reg_i);
  for (int i = 0; i < CHIP8_REGISTER_COUNT; i++)
    printf("  reg_v[%d]: %x\n", i, ch8->reg_v[i]);
  printf("  timer: %x\n", ch8->timer);
  printf("  tone_clock: %x\n", ch8->tone_clock);
  printf("---------------------\n");


  for (int i = -2; i < 6; i++) {
    uint16_t ip = ch8->ip + i * 2;

    printf(
      "%s %04X: %02X%02X\n",
      ip + i == ch8->ip ? ">" : " ",
      ip + i,
      ch8->mem[ip + i],
      ch8->mem[ip + i + 1]
    );
  }
}

status_t chip8_load_rom(chip8_t* ch8, const char* filepath) {
  FILE* fp = fopen(filepath, "rb");
  if (fp == NULL) {
    return ERR;
  }

  const int max_bytes_to_read = CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START_ADDRESS;

  // Make sure the rom is not too big.
  fseek(fp, 0, SEEK_END);
  const long file_size = ftell(fp);
  rewind(fp);

  if (file_size > max_bytes_to_read) {
    return ERR;
  }

  size_t bytes_read =
      fread(&ch8->mem[CHIP8_PROGRAM_START_ADDRESS], 1, max_bytes_to_read, fp);

  if (bytes_read == 0) {
    return ERR;
  }

  return OK;
}

void chip8_run_instruction(chip8_t* ch8) {
  uint16_t instruction = (ch8->mem[ch8->ip] << 8) | ch8->mem[ch8->ip + 1];

  // TODO
  // if (ch8->timer > 0) ch8->timer--;
  // if (ch8->tone_clock > 0) ch8->tone_clock--;

  if ((instruction & 0xF000) == 0x1000) {
    // 1MMM - Go to MMM
    ch8->ip = 0x0FFF & instruction;
  } else if ((instruction & 0xF000) == 0xB000) {
    // BMMM - Go to MMM + V0
    ch8->ip = (0x0FFF & instruction) + ch8->reg_v[0];
  } else if ((instruction & 0xF000) == 0x2000) {
    // 2MMM - Do subroutine at 0MMM (must end with 00EE)
    ch8->stack[ch8->sp++] = ch8->ip + 2;
    ch8->ip = 0x0FFF & instruction;
  } else if (instruction == 0x00EE) {
    // 00EE - Return from subroutine
    ch8->ip = ch8->stack[--ch8->sp];
  } else if ((instruction & 0xF000) == 0x3000) {
    // 3XKK - Skip next instruction if VX == KK
    uint8_t reg = (instruction >> 8) & 0x0F;
    uint8_t val = instruction & 0xFF;
    if (ch8->reg_v[reg] == val) {
      ch8->ip += 4;
    } else {
      ch8->ip += 2;
    }
  } else if ((instruction & 0xF000) == 0x4000) {
    // 4XKK - Skip next instruction if VX != KK
    uint8_t reg = (instruction >> 8) & 0x0F;
    uint8_t val = instruction & 0xFF;
    if (ch8->reg_v[reg] != val) {
      ch8->ip += 4;
    } else {
      ch8->ip += 2;
    }
  } else if ((instruction & 0xF00F) == 0x5000) {
    // 5XY0 - Skip next instruction if VX == VY
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    if (ch8->reg_v[reg_x] == ch8->reg_v[reg_y]) {
      ch8->ip += 4;
    } else {
      ch8->ip += 2;
    }
  } else if ((instruction & 0xF00F) == 0x9000) {
    // 9XY0 - Skip next instruction if VX != VY
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    if (ch8->reg_v[reg_x] != ch8->reg_v[reg_y]) {
      ch8->ip += 4;
    } else {
      ch8->ip += 2;
    }
  } else if ((instruction & 0xF0FF) == 0xE09E) {
    // EX9E - Skip next instruction if VX == hexadecimal key (LSD)
    // TODO
  } else if ((instruction & 0xF0FF) == 0xE0A1) {
    // EXA1 - Skip next instruction if VX != hexadecimal key (LSD)
    // TODO
  } else if ((instruction & 0xF000) == 0x6000) {
    // 6XKK - Let VX = KK
    uint8_t reg = (instruction >> 8) & 0x0F;
    uint8_t val = instruction & 0xFF;
    ch8->reg_v[reg] = val;
    ch8->ip += 2;
  } else if ((instruction & 0xF000) == 0xC000) {
    // CXKK - Let VX = random byte (KK = mask)
    uint8_t reg = (instruction >> 8) & 0x0F;
    uint8_t mask = instruction & 0xFF;
    ch8->reg_v[reg] = (rand() % 256) & mask;
    ch8->ip += 2;
  } else if ((instruction & 0xF000) == 0x7000) {
    // 7XKK - Let VX = VX + KK
    uint8_t reg = (instruction >> 8) & 0x0F;
    uint8_t val = instruction & 0xFF;
    ch8->reg_v[reg] += val; // TODO will this wrap around?
    ch8->ip += 2;
  } else if ((instruction & 0xF00F) == 0x8000) {
    // 8XY0 - Let VX = VY
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    ch8->reg_v[reg_x] = ch8->reg_v[reg_y];
    ch8->ip += 2;
  } else if ((instruction & 0xF00F) == 0x8001) {
    // 8XY1 - Let VX = VX/VY (VF changed)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    ch8->reg_v[reg_x] /= ch8->reg_v[reg_y];
    ch8->ip += 2;
  } else if ((instruction & 0xF00F) == 0x8002) {
    // 8XY2 - Let VX = VX & VY (VF changed)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    ch8->reg_v[reg_x] &= ch8->reg_v[reg_y];
    ch8->ip += 2;
  } else if ((instruction & 0xF00F) == 0x8004) {
    // 8XY4 - Let VX = VX + VY (VF = 00 if VX + VY <= FF,
    // VF == 01 if VX + VY > FF)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    uint16_t val = ch8->reg_v[reg_x] + ch8->reg_v[reg_y];
    ch8->reg_v[0x0F] = (val > 0xFF ? 1 : 0);
    ch8->reg_v[reg_x] = (val & 0xFF);
    ch8->ip += 2;
  } else if ((instruction & 0xF00F) == 0x8005) {
    // 8XY5 - Let VX = VX - VY (VF = 00 if VX < VY,
    // VF == 01 if VX >= VY)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    uint8_t reg_y = (instruction >> 4) & 0x0F;
    ch8->reg_v[0x0F] = (ch8->reg_v[reg_x] >= ch8->reg_v[reg_y] ? 1 : 0);
    ch8->reg_v[reg_x] -= ch8->reg_v[reg_y];
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF007) {
    // FX07 - Let VX = current timer value
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    ch8->reg_v[reg_x] = ch8->timer;
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF00A) {
    // FX0A - Let VX = hexadecimal key digit (waits for key press)
    // TODO
  } else if ((instruction & 0xF0FF) == 0xF015) {
    // FX15 - Set timer = VX (01 = 1/60 second)
    uint8_t reg = (instruction >> 8) & 0x0F;
    ch8->timer = ch8->reg_v[reg];
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF018) {
    // FX18 - Set tone duration = VX (01 = 1/60 second)
    uint8_t reg = (instruction >> 8) & 0x0F;
    ch8->tone_clock = ch8->reg_v[reg];
    ch8->ip += 2;
  } else if ((instruction & 0xF000) == 0xA000) {
    // AMMM - Set I = 0MMM
    ch8->reg_i = instruction & 0x0FFF;
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF01E) {
    // FX1E - Let I = I + VX
    uint8_t reg = (instruction >> 8) & 0x0F;
    ch8->reg_i += ch8->reg_v[reg];
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF029) {
    // FX29 - Let I = 5 byte display pattern for LSD of VX
    // TODO
  } else if ((instruction & 0xF0FF) == 0xF033) {
    // FX33 - Let MI = 3 decimal digit equivalent of VX (I unchanged)
    uint8_t reg = (instruction >> 8) & 0x0F;
    ch8->mem[ch8->reg_i] = ch8->reg_v[reg] / 100 % 10;
    ch8->mem[ch8->reg_i + 1] = ch8->reg_v[reg] / 10 % 10;
    ch8->mem[ch8->reg_i + 2] = ch8->reg_v[reg] % 10;
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF055) {
    // FX55 - Let MI = V0 : VX (I = I + X + 1)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    for (int i = 0; i <= reg_x; i++) {
      ch8->mem[ch8->reg_i++] = ch8->reg_v[i];
    }
    ch8->ip += 2;
  } else if ((instruction & 0xF0FF) == 0xF065) {
    // FX65 - Let V0 : VX = MI (I = I + X + 1)
    uint8_t reg_x = (instruction >> 8) & 0x0F;
    for (int i = 0; i <= reg_x; i++) {
      ch8->reg_v[i] = ch8->mem[ch8->reg_i++];
    }
    ch8->ip += 2;
  } else if (instruction == 0x00E0) {
    // 00E0 - Erase display (all 0s)
    // TODO
  } else if ((instruction & 0xF000) == 0xD000) {
    // DXYN - Show n byte MI pattern at VX - VY coordinates.
    // I unchanged. MI pattern is combined with existing display via exclusive-OR
    // function. VF = 01 if a 1 in MI pattern matches 1 in existing display.
    // TODO
  } else if ((instruction & 0xF000) == 0x0000) {
    // 0MMM - Do machine language subroutine at 0MMM (subroutine must end with D4 byte)

    // TODO Error handling.
    ch8->stack[ch8->sp++] = ch8->ip + 2;
    ch8->ip = 0x0FFF & instruction;
  } else {
    printf("Error: unknown instruction: %04x\n", instruction);
  }

}
