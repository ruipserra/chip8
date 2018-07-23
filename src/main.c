#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "chip8.h"
#include "miniterm.h"


#include <ctype.h>

#define ROM "./rocket.ch8"

void render_framebuffer(chip8_t *ch8) {
  uint8_t *framebuffer_end = &ch8->framebuffer[CHIP8_FRAMEBUFFER_SIZE];
  uint8_t *framebuffer_ptr = &ch8->framebuffer[0];
  uint8_t row = 0, col = 0;

  while (framebuffer_ptr != framebuffer_end) {
    uint8_t byte = *framebuffer_ptr;

    for (int i = 7; i >= 0; i--) {
      uint8_t bit = (byte >> i) & 0x01;
      fprintf(stdout, "\x1b[%dm  ", bit ? 43 : 49);

      col++;
      if (col == CHIP8_FRAMEBUFFER_MAX_X + 1) {
        row++;
        col = 0;
        fprintf(stdout, "\r\n");
      }
    }

    framebuffer_ptr++;
  }

  fprintf(stdout, "\x1b[49m  ");
  fflush(stdout);
}

void render_debug(const chip8_t* ch8) {
  printf("---- Chip8 Debug ----\r\n");
  printf("  ip: %x\r\n", ch8->ip);
  printf("  reg_i: %x\r\n", ch8->reg_i);
  for (int i = 0; i < CHIP8_REGISTER_COUNT; i++)
    printf("  reg_v[%d]: %x\r\n", i, ch8->reg_v[i]);
  printf("  timer: %x\r\n", ch8->timer);
  printf("  tone_clock: %x\r\n", ch8->tone_clock);
  printf("---------------------\r\n");


  for (int i = -2; i < 6; i++) {
    uint16_t ip = ch8->ip + i * 2;

    printf(
      "%s %04X: %02X%02X\r\n",
      ip + i == ch8->ip ? ">" : " ",
      ip + i,
      ch8->mem[ip + i],
      ch8->mem[ip + i + 1]
    );
  }
}

enum {
  RENDER_DEBUG,
  RENDER_FRAMEBUFFER
};

int main(void) {
  srand(time(NULL));

  chip8_t ch8;
  chip8_init(&ch8);
  chip8_load_rom(&ch8, ROM);

  mterm_init();
  mterm_clear_screen();
  mterm_show_cursor(false);

  char c;
  int mode = RENDER_FRAMEBUFFER;
  while (true) {
    int bytes_read = read(STDIN_FILENO, &c, 1);
    if (bytes_read == 0) { /* timeout */ }

    chip8_run_instruction(&ch8);

    mterm_clear_screen();
    mterm_set_cursor_pos(0, 0);

    if (mode == RENDER_DEBUG) {
      render_debug(&ch8);
    } else if (mode == RENDER_FRAMEBUFFER) {
      render_framebuffer(&ch8);
    }

    if (c == 'q') {
      break;
    } else if (c == 'm') {
      mode = (mode + 1) % (RENDER_FRAMEBUFFER + 1);
    } else if (c == 'r') {
      chip8_init(&ch8);
      chip8_load_rom(&ch8, ROM);
    }

    c = '\0';
    usleep(1000 / 10 * 1000);
  }

  mterm_teardown();
}

