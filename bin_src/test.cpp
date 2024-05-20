// #include <iostream>
// #include <memory>
//
// #include <curses.h>
//
// int main() {
//	FIELD* field[2];
//	FORM* my_form;
//	int ch;
//
//	/* Initialize curses */
//	initscr();
//	cbreak();
//	noecho();
//	keypad(stdscr, TRUE);
//
//	/* Initialize the fields */
//	int bufsize = 5;
//	field[0] = new_field(3, bufsize, 4, 18, 1, 0);
//	field[1] = NULL;
//
//	/* Set field options */
//	set_field_back(field[0], A_UNDERLINE);
//	field_opts_off(field[0], O_AUTOSKIP);
//	field_opts_off(field[0], O_STATIC);
//
//	/* Create the form and post it */
//	my_form = new_form(field);
//	post_form(my_form);
//	refresh();
//
//	mvprintw(4, 10, "Value 1:");
//
//	for (size_t i = 0; i != 100; ++i) {
//		printw("line %llu\n", i);
//	}
//	int buflen = 0;
//	goto _end;
//	form_driver(my_form, REQ_FIRST_FIELD);
//	refresh();
//
//	/* Loop through to get user requests */
//	while ((ch = getch()) != KEY_F(1)) {
//		refresh();
//		/* Set the colors of the current field */
//		// set_field_fore(current_field(my_form), COLOR_PAIR(1));
//		// set_field_back(current_field(my_form), COLOR_PAIR(2));
//		switch (ch) {
//			case KEY_LEFT:	form_driver(my_form, REQ_PREV_CHAR); break;
//			case KEY_RIGHT: form_driver(my_form, REQ_NEXT_CHAR); break;
//			case KEY_BACKSPACE:
//				int field_pos;
//				field_info(current_field(my_form), NULL, NULL, NULL, &field_pos, NULL, NULL);
//				if (field_pos != getcurx(stdscr)) {
//					form_driver(my_form, REQ_DEL_PREV);
//				}
//				break;
//			default:
//				form_driver(my_form, ch);
//				form_driver(my_form, REQ_VALIDATION);
//				refresh();
//				++buflen;
//				// mvprintw(2, 0, "buf: %s", field_buffer(field[0], 0));
//				//  std::cout << field_buffer(field[0], 0) << '\n';
//				refresh();
//				break;
//		}
//	}
//_end:
//	getch();
//	/* Un post form and free the memory */
//	unpost_form(my_form);
//	free_form(my_form);
//	free_field(field[0]);
//
//	endwin();
//	return 0;
// }
