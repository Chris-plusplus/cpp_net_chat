#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

int main() {
	// Initialize curses
	initscr();
	// std::cout << has_colors() << '\n';
	// start_color();
	// std::cout << can_change_color() << '\n';
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	refresh();

	// init_color(8, 0, 500, 1'000);

	// init_pair(1, 8, COLOR_BLACK);

	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);

	// Create a new window
	WINDOW* my_win = newwin(max_y - 2, max_x, 0, 0);
	box(my_win, 0, 0);
	scrollok(my_win, TRUE);

	mvwprintw(my_win, 0, 0, "xd");

	// Create a vector to store lines
	std::vector<std::string> lines;
	for (size_t i = 0; i != 100; ++i) {
		lines.push_back("line " + std::to_string(i));
	}

	// Variables for scrolling
	int top_line = 0;
	int ch;

	// Function to refresh window content
	auto refresh_window = [&]() {
		werase(my_win);
		box(my_win, 0, 0);
		for (int i = 0; i < max_y - 4; ++i) {
			// wattron(my_win, COLOR_PAIR(1));
			if (top_line + i < lines.size()) {
				mvwprintw(my_win, i + 1, 1, "%s", lines[top_line + i].c_str());
			}
			// wattroff(my_win, COLOR_PAIR(1));
		}
		wrefresh(my_win);
	};

	// Initial refresh to display the window
	refresh_window();

	// Main loop for handling input
	while ((ch = getch()) != 'q') { // Press 'q' to quit
		switch (ch) {
			case KEY_UP:
				if (top_line > 0) {
					top_line--;
					refresh_window();
				}
				break;
			case KEY_DOWN:
				if (top_line + max_y - 4 < lines.size()) {
					top_line++;
					refresh_window();
				}
				break;
			case '\033': goto _end;
		}
	}
_end:
	// Clean up
	delwin(my_win);
	endwin();
	return 0;
}
