#include <bitset>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if CHAT_WINDOWS
#include <Windows.h>
#endif

#include <form.h>
#include <ncurses.h>

int main() {
	initscr();
	// raw();
	cbreak();
	noecho();
	// nodelay(stdscr, TRUE);
	//  nonl();
	nonl();
	notimeout(stdscr, TRUE);
	keypad(stdscr, TRUE);

	int ch;
	// Main loop for handling input
	while ((ch = getch()) != '\033') { // Press 'q' to quit
		// auto is_ctrl = GetAsyncKeyState(VK_LCONTROL);
		auto bits = std::bitset<8 * sizeof(ch)>(ch).to_string();
		printw("pressed {%d 0%o} = %s\n", ch, ch, bits.c_str());
	}

	endwin();

	return 0;
}
