#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "chip8.h"
#include "miniterm.h"

#define ROM "./rocket.ch8"

#define KEY_0 ','
#define KEY_1 '7'
#define KEY_2 '8'
#define KEY_3 '9'
#define KEY_4 'u'
#define KEY_5 'i'
#define KEY_6 'o'
#define KEY_7 'j'
#define KEY_8 'k'
#define KEY_9 'l'
#define KEY_A 'm'
#define KEY_B '.'
#define KEY_C '0'
#define KEY_D 'p'
#define KEY_E ';'
#define KEY_F '/'

enum { RENDER_DEBUG, RENDER_FRAMEBUFFER };

void render(const chip8_t *ch8, int render_mode);
void render_framebuffer(const chip8_t *ch8);
void render_debug(const chip8_t *ch8);
bool process_input(chip8_t *ch8, int *render_mode);

int main(void) {
  srand(time(NULL));

  chip8_t ch8;
  chip8_init(&ch8);
  chip8_load_rom(&ch8, ROM);

  mterm_init();
  mterm_clear_screen();
  mterm_show_cursor(false);

  int render_mode = RENDER_FRAMEBUFFER;
  bool running = true;

  while (running) {
    running = process_input(&ch8, &render_mode);

    for (int i = 0; i < 20; i++) {
      chip8_run_instruction(&ch8);
    }

    render(&ch8, render_mode);

    usleep(1.0 / 60.0 * 1000.0 * 1000.0);
  }
}

void render(const chip8_t *ch8, int render_mode) {
  mterm_clear_screen();
  mterm_set_cursor_pos(0, 0);

  if (render_mode == RENDER_DEBUG) {
    render_debug(ch8);
  } else if (render_mode == RENDER_FRAMEBUFFER) {
    render_framebuffer(ch8);
  }
}

void render_framebuffer(const chip8_t *ch8) {
  const uint8_t *framebuffer_end = &ch8->framebuffer[CHIP8_FRAMEBUFFER_SIZE];
  const uint8_t *framebuffer_ptr = &ch8->framebuffer[0];
  uint8_t row = 0, col = 0;

  while (framebuffer_ptr != framebuffer_end) {
    uint8_t byte = *framebuffer_ptr;

    for (int i = 7; i >= 0; i--) {
      uint8_t bit = (byte >> i) & 0x01;
      fprintf(stdout, "\x1b[%dm  ", bit ? 42 : 49);

      col++;
      if (col == CHIP8_FRAMEBUFFER_X_LEN && row < CHIP8_FRAMEBUFFER_MAX_Y) {
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

void render_debug(const chip8_t *ch8) {
  printf("---- Chip8 Debug ----\r\n");
  printf("  ip: %04X\r\n", ch8->ip);
  printf("  reg_i: %04X\r\n", ch8->reg_i);
  for (int i = 0; i < CHIP8_REGISTER_COUNT; i++)
    printf("  reg_v[%d]: %02X\r\n", i, ch8->reg_v[i]);
  printf("  timer: %02X\r\n", ch8->timer);
  printf("  tone_clock: %02X\r\n", ch8->tone_clock);
  printf("  keypress: %02X\r\n", ch8->keypress);
  printf("---------------------\r\n");

  for (int i = -2; i < 6; i++) {
    uint16_t ip = ch8->ip + i * 2;

    printf("%s %04X: %02X%02X\r\n", i == 0 ? ">" : " ", ip, ch8->mem[ip + i],
           ch8->mem[ip + i + 1]);
  }
}

bool process_input(chip8_t *ch8, int *render_mode) {
  char c;
  int bytes_read = read(STDIN_FILENO, &c, 1);
  if (bytes_read == 0) { /* timeout */ }

  ch8->keypress = CHIP8_NO_KEY_PRESSED;

  switch (c) {
    case KEY_0:
      ch8->keypress = 0x00;
      break;
    case KEY_1:
      ch8->keypress = 0x01;
      break;
    case KEY_2:
      ch8->keypress = 0x02;
      break;
    case KEY_3:
      ch8->keypress = 0x03;
      break;
    case KEY_4:
      ch8->keypress = 0x04;
      break;
    case KEY_5:
      ch8->keypress = 0x05;
      break;
    case KEY_6:
      ch8->keypress = 0x06;
      break;
    case KEY_7:
      ch8->keypress = 0x07;
      break;
    case KEY_8:
      ch8->keypress = 0x08;
      break;
    case KEY_9:
      ch8->keypress = 0x09;
      break;
    case KEY_A:
      ch8->keypress = 0x0A;
      break;
    case KEY_B:
      ch8->keypress = 0x0B;
      break;
    case KEY_C:
      ch8->keypress = 0x0C;
      break;
    case KEY_D:
      ch8->keypress = 0x0D;
      break;
    case KEY_E:
      ch8->keypress = 0x0E;
      break;
    case KEY_F:
      ch8->keypress = 0x0F;
      break;

    case 'd':  // Switch render mode
      *render_mode = (*render_mode + 1) % (RENDER_FRAMEBUFFER + 1);
      break;

    case 'r':  // Reset
      chip8_init(ch8);
      chip8_load_rom(ch8, ROM);
      break;

    case 'q':  // Quit
      return false;
  }

  return true;
}
