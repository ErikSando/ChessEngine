#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Definitions.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

int GetTimeMS() {
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

int InputWaiting() {
#ifdef WIN32
	static int init = 0, pipe;
	static HANDLE inh;
	DWORD dw;

	if (!init) {
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);

		if (!pipe) {
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}

	if (pipe) {
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return TRUE;
		return dw;
	}
	else {
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? FALSE : dw;
	}
#else
	fd_set readfds;
	struct timeval tv;
	FD_ZERO(&readfds);
	FD_SET(fileno(stdin), &readfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	select(16, &readfds, 0, 0, &tv);

	return FD_ISSET(fileno(stdin), &readfds);
#endif
}

void ReadInput(SearchInfo* info) {
	int bytes;

	char input[256] = "", * endc;

	if (InputWaiting()) {
		info->stopped = True;

		do bytes = read(fileno(stdin), input, 256);
		while (bytes < 0);

		if (endc = strchr(input, '\n')) *endc = 0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4) || !strncmp(input, "stop", 4)) info->stopped = True;
		}
	}
}