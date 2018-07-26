#include "miniterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define MTERM_SEND(...)         \
  fprintf(stdout, __VA_ARGS__); \
  fflush(stdout);

static struct termios orig_termios;

void mterm_init(void) {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(mterm_teardown);

  struct termios raw_termios;
  cfmakeraw(&raw_termios);
  raw_termios.c_cc[VMIN] = 0;
  raw_termios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);
}

void mterm_teardown(void) {
  mterm_clear_screen();
  mterm_set_cursor_pos(0, 0);
  mterm_show_cursor(true);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void mterm_clear_screen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);  // Erase entire screen
  write(STDOUT_FILENO, "\x1b[3J", 4);  // Erase scrollback
}

void mterm_show_cursor(bool val) { MTERM_SEND("\x1b[?25%s", val ? "h" : "l"); }

void mterm_set_cursor_pos(int line, int column) {
  MTERM_SEND("\x1b[%d;%dH", line, column);
}
