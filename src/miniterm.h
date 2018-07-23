#ifndef __MINITERM_H__
#define __MINITERM_H__

#include <stdbool.h>

void mterm_init(void);
void mterm_teardown(void);

void mterm_clear_screen(void);
void mterm_show_cursor(bool val);
void mterm_set_cursor_pos(int line, int column);

#endif // __MINITERM_H__

