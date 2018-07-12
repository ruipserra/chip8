#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "chip8.h"

int main(void) {
  srand(time(NULL));

  chip8_t ch8;
  chip8_init(&ch8);
  chip8_load_rom(&ch8, "./rocket.ch8");

  while (true) {
    printf("\x1b[2J"); // clear
    printf("\x1b[H"); // cursorhome

    chip8_run_instruction(&ch8);
    chip8_debug(&ch8);
    usleep(1000 / 10 * 1000);
  }
}
