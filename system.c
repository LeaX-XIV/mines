#include "system.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <windows.h>
#include <winbase.h>
#include <wincon.h>

#include <conio.h>

void system_init() { }

void system_teardown() { }

int conio_getch() {
	return getch();
}

// https://gist.github.com/jtriley/1108174
int get_terminal_size(int* width, int* height) {
	*width = -1;
	*height = -1;
	// stdin handle is -10
	// stdout handle is -11
	// stderr handle is -12
	HANDLE h = GetStdHandle(-12);
	PCONSOLE_SCREEN_BUFFER_INFO csbi = malloc(sizeof *csbi);
	BOOL res = GetConsoleScreenBufferInfo(h, csbi);
	if (res) {
		*width = csbi->srWindow.Right - csbi->srWindow.Left;
		*height = csbi->srWindow.Bottom - csbi->srWindow.Top;
		return 1;
	}

	return 0;
}

#elif __linux__

#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void system_init() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	
	// Called both exiting the program and in ansi_teardown()
	atexit(system_teardown);

	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	raw.c_oflag &= ~(OPOST);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void system_teardown() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int CONIO_BUFFER[16] = { 0 };
int conio_getch() {
	static int idx = 0;
	char c;
	size_t n;

	if (idx > 0) {
		goto send_output;
	}

	n = read(STDIN_FILENO, &c, 1);
	if (n != 1) {
		exit(2);
	}

	// Start of escape sequence
	if (c == 27) {
		CONIO_BUFFER[idx++] = 224;

		n = read(STDIN_FILENO, &c, 1);
		if (n != 1) {
			exit(2);
		}

		if (c != '[') {
			CONIO_BUFFER[idx++] = c;
			goto send_output;
		}

		n = read(STDIN_FILENO, &c, 1);
		if (n != 1) {
			exit(2);
		}

		switch (c) {
			case 'A': CONIO_BUFFER[idx++] = 72; break;
			case 'B': CONIO_BUFFER[idx++] = 80; break;
			case 'C': CONIO_BUFFER[idx++] = 77; break;
			case 'D': CONIO_BUFFER[idx++] = 75; break;
			// If not an arrow key we don't care to return an accurate result
			default : CONIO_BUFFER[idx++] = c; break;
		}
		goto send_output;
	}

	CONIO_BUFFER[idx++] = c;

send_output:
	return CONIO_BUFFER[--idx];
}

int get_terminal_size(int* width, int* height) {
	*width = -1;
	*height = -1;

	struct winsize w = { 0 };
	int res = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	if (res == 0) {
		*width = w.ws_col;
		*height = w.ws_row;
		return 1;
	}

	return 0;
}

#else
	#error "Unknown compiler"
#endif
