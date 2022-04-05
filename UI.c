#include "UI.h"


void draw_borders(WINDOW *screen) 
{ 
	int x, y, i; getmaxyx(screen, y, x);

	mvwprintw(screen, 0, 0, "+"); 
	mvwprintw(screen, y - 1, 0, "+"); 
	mvwprintw(screen, 0, x - 1, "+"); 
	mvwprintw(screen, y - 1, x - 1, "+"); 


	for (i = 1; i < (x - 1); i++) 
	{ 
		mvwprintw(screen, 0, i, "-"); 
		mvwprintw(screen, y - 1, i, "-"); 
	} 
	
}

void CleanUP_UI()
{
	delwin(Output);
	delwin(Input);
	endwin(); 
}

void UI() 
{ 
	int parent_x, parent_y;
	int score_size = 3;

	initscr();
	curs_set(FALSE); 
	cbreak();
	
	getmaxyx(stdscr, parent_y, parent_x); 

	Output = newwin(parent_y , parent_x, 0,  0);
	Input = newwin(score_size, parent_x, parent_y - score_size, 0);

	draw_borders(Output);
	draw_borders(Input);

	mvwprintw(Output, 0, parent_x / 2, "Message"); 

	wrefresh(Output); 
	wrefresh(Input); 
}