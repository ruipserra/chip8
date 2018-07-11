#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdint.h>

#define CHIP8_REGISTER_COUNT 16
#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_PROGRAM_START_ADDRESS 0x0200

typedef enum status {
  ERR = 0,
  OK = 1,
} status_t;

struct chip8 {
  uint16_t ip;
  uint16_t reg_i;
  uint8_t reg_v[CHIP8_REGISTER_COUNT];
  uint8_t timer;
  uint8_t tone_clock;
  uint8_t mem[CHIP8_MEMORY_SIZE];
};

typedef struct chip8 chip8_t;

void chip8_init(chip8_t* chip8);
void chip8_debug(const chip8_t* chip8);
status_t chip8_load_rom(chip8_t* chip8, const char* filepath);

#endif  // __CHIP8_H__
