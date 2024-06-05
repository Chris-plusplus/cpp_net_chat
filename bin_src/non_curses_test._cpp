// #include <stdio.h>
// #include <windows.h>
//
// void getConsoleSize(SMALL_RECT* windowSize) {
//	CONSOLE_SCREEN_BUFFER_INFO csbi;
//	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
//		*windowSize = csbi.srWindow;
//	}
// }
//
// int main() {
//	SMALL_RECT oldSize, newSize;
//	getConsoleSize(&oldSize);
//
//	printf("Initial console size: (%d, %d)\n", oldSize.Right - oldSize.Left + 1, oldSize.Bottom - oldSize.Top + 1);
//
//	while (1) {
//		getConsoleSize(&newSize);
//		if (newSize.Right != oldSize.Right || newSize.Bottom != oldSize.Bottom || newSize.Left != oldSize.Left ||
//			newSize.Top != oldSize.Top) {
//			printf(
//				"Console resized to: (%d, %d)\n",
//				newSize.Right - newSize.Left + 1,
//				newSize.Bottom - newSize.Top + 1
//			);
//			oldSize = newSize; // Update the old size to the new size
//		}
//		Sleep(100); // Sleep for a short period to avoid excessive CPU usage
//	}
//
//	return 0;
// }
