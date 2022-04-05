#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <unistd.h>

WINDOW *Output;
WINDOW *Input;

void draw_borders(WINDOW *);
void CleanUP_UI();
void UI();


#endif