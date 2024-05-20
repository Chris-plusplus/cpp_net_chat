#include <ncurses.h>

int main() {
    // Initialize ncurses
    initscr();
    // Enable color mode if supported
    if(has_colors())
        start_color();
    // Clear the screen
    clear();
    // Print "Hello, World!" at the center of the screen
    mvprintw(LINES / 2, (COLS - 13) / 2, "Hello, World!");
    // Refresh the screen to display changes
    refresh();
    // Wait for a key press
    getch();
    // End ncurses
    endwin();
    return 0;
}
