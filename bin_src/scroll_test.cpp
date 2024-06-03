#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <Net.h>
#include <form.h>
#include <ncurses.h>

#define KEY_CTRL_UP 567
#define KEY_CTRL_DOWN 526
#define KEY_CTRL_LEFT 546
#define KEY_CTRL_RIGHT 516

#if CHAT_WINDOWS
#include <Windows.h>
#endif

int main() {
	setlocale(LC_ALL, "");
	initscr();
	raw();
	// cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	refresh();

	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);

	// Create a new window
	WINDOW* my_win = derwin(stdscr, max_y - 2, max_x, 0, 0);
	touchwin(stdscr);
	box(my_win, 0, 0);
	scrollok(my_win, TRUE);

	// Create the form fields
	FIELD* fields[2];
	FORM* form;

	fields[0] = new_field(1, max_x - 2, 0, 0, 0, 0);
	set_field_back(fields[0], A_REVERSE);
	field_opts_off(fields[0], O_STATIC);
	curs_set(1);
	fields[1] = nullptr;

	form = new_form(fields);

	// Create a subwindow for the form
	WINDOW* form_sub = derwin(stdscr, 1, max_x - 2, max_y - 1, 1);
	touchwin(stdscr);
	set_form_win(form, stdscr);
	set_form_sub(form, form_sub);

	post_form(form);
	wrefresh(my_win);
	wrefresh(form_sub);

	// Create a vector to store lines
	std::mutex lines_mutex;
	std::vector<std::string> lines;
	/*for (size_t i = 0; i < 100; ++i) {
		lines.push_back(std::format("jakas tam sobie linijka numero: {}", i + 1));
	}*/

	// Variables for scrolling
	int top_line = 0;
	int ch;

	// Function to refresh window content
	auto refresh_window = [&]() {
		werase(my_win);
		box(my_win, 0, 0);
		for (int i = 0; i < max_y - 4; ++i) {
			if (top_line + i < lines.size()) {
				mvwprintw(my_win, i + 1, 1, "%s", lines[top_line + i].c_str());
			}
		}
		wrefresh(my_win);
		wrefresh(form_sub);
	};

	auto writer_thread = std::jthread([&](std::stop_token stop_token) {
		for (size_t i = 0; i != 100 and not stop_token.stop_requested(); ++i) {
			{
				auto lock = std::lock_guard(lines_mutex);
				lines.push_back(std::format("jakas tam sobie linijka numero: {}", i + 1));
				if (top_line + max_y - 4 == lines.size() - 1) {
					++top_line;
				}
				refresh_window();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1'000) / 2);
		}
	});
	auto write_to_win = [&](const std::string& arg) {
		auto lock = std::lock_guard(lines_mutex);
		lines.push_back(arg);
		if (top_line + max_y - 4 == lines.size() - 1) {
			++top_line;
		}
		refresh_window();
	};
	auto fieldLen = [](const std::string& arg) -> size_t {
		for (size_t i = arg.length() - 1; i != (size_t)-1; --i) {
			if (arg[i] != ' ') {
				return i + 1;
			}
		}
		return 0;
	};

	// Initial refresh to display the window
	refresh_window();

	// Main loop for handling input
	while ((ch = getch()) != '\033') { // Press 'q' to quit
		int ch2;
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
			case KEY_LEFT:
				form_driver(form, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(form, REQ_NEXT_CHAR);
				break;
			case '\b':
			case KEY_BACKSPACE:
				form_driver(form, REQ_DEL_PREV);
				break;
			case '\n':
			case '\r':
			case KEY_ENTER:
				{
					form_driver(form, REQ_VALIDATION);
					std::string val = field_buffer(fields[0], 0);
					size_t last_blank = fieldLen(val);
					if (last_blank == 0) {
						break;
					} else if (last_blank != std::string::npos) {
						val.erase(last_blank);
					}
					std::cout << std::format("val: '{}'\n", val);
					write_to_win(val);
					form_driver(form, REQ_CLR_FIELD);
				}
				break;
			default:
				form_driver(form, REQ_VALIDATION);
				if (fieldLen(field_buffer(fields[0], 0)) < 255) {
					form_driver(form, ch);
					wrefresh(form_sub);
				}
				break;
		}
	}

	writer_thread.request_stop();

	// Clean up
	unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	delwin(form_sub);
	delwin(my_win);
	endwin();

	return 0;
}
