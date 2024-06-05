// #include <iostream>
//
// #include <termios.h>
// #include <unistd.h>
//
// bool isCtrlPressed() {
//	struct termios oldt, newt;
//	tcgetattr(STDIN_FILENO, &oldt);
//	newt = oldt;
//	newt.c_lflag &= ~(ICANON | ECHO);
//	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//
//	int ch = getchar();
//	bool ctrlPressed = (ch == 3); // ASCII code for Ctrl+C
//	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//
//	return ctrlPressed;
// }
//
// int main() {
//	std::cout << "Press Ctrl to check: ";
//	if (isCtrlPressed()) {
//		std::cout << "Ctrl is pressed.\n";
//	} else {
//		std::cout << "Ctrl is not pressed.\n";
//	}
//	return 0;
// }
//
