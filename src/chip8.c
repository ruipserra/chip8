#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "chip8.h"

void chip8_init(chip8_t* ch8) {
  ch8->ip = 0;
  ch8->reg_i = 0;
  for (int i = 0; i < CHIP8_REGISTER_COUNT; i++) ch8->reg_v[i] = 0;
  ch8->timer = 0;
  ch8->tone_clock = 0;
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
