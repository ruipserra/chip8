#ifndef __CHIP8_H__
#define __CHIP8_H__

#include <stdint.h>

#define CHIP8_REGISTER_COUNT 16
#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_PROGRAM_START_ADDRESS 0x0200
#define CHIP8_STACK_SIZE 256
#define CHIP8_FRAMEBUFFER_X_LEN 64
#define CHIP8_FRAMEBUFFER_Y_LEN 32
#define CHIP8_FRAMEBUFFER_MAX_X 0x3F
#define CHIP8_FRAMEBUFFER_MAX_Y 0x1F
#define CHIP8_FRAMEBUFFER_SIZE \
  ((CHIP8_FRAMEBUFFER_X_LEN * CHIP8_FRAMEBUFFER_Y_LEN) / 8)
#define CHIP8_DIGITS_START_ADDRESS 0x0000

#define CHIP8_NO_KEY_PRESSED 0xAF

typedef enum status {
  ERR = 0,
  OK = 1,
} status_t;

struct chip8 {
  uint16_t ip;  // instruction pointer
  uint16_t reg_i;
  uint8_t reg_v[CHIP8_REGISTER_COUNT];
  uint8_t timer;
  uint8_t tone_clock;
  uint8_t keypress;
  uint8_t sp;  // stack pointer
  uint16_t stack[CHIP8_STACK_SIZE];
  uint8_t mem[CHIP8_MEMORY_SIZE];
  uint8_t framebuffer[CHIP8_FRAMEBUFFER_SIZE];
};

typedef struct chip8 chip8_t;

void chip8_init(chip8_t* chip8);
void chip8_debug(const chip8_t* chip8);
status_t chip8_load_rom(chip8_t* chip8, const char* filepath);
void chip8_run_instruction(chip8_t* chip8);

#endif  // __CHIP8_H__
