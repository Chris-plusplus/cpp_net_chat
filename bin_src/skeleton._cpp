#include <csignal>
#include <iostream>

#include <ncurses.h>

int main() {
	// initialize ncurses

	initscr();

	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	for (;;) {
		int y, x;
		getmaxyx(stdscr, y, x);
		std::cout << y << ' ' << x << '\n';
		if (getch() == 'q') {
			goto _end;
		}
	}

_end:;
	// end ncurses
	endwin();
}
