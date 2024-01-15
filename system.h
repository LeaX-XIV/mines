#ifndef _SYSTEM_H
#define _SYSTEM_H

void system_init();
void system_teardown();

int conio_getch();

int get_terminal_size(int* width, int* height);

#endif
