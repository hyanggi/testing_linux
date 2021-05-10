#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int setup_port(int port) {
	struct termios settings;

	if (tcgetattr(port, &settings) != 0) {
	    perror("Getting port settings failed");
	    return -1;
	}

	cfmakeraw(&settings);

	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	cfsetispeed(&settings, B38400);
	cfsetospeed(&settings, B38400);

	if (tcsetattr(port, TCSANOW, &settings) != 0) {
	    perror("Saving port settings failed");
	    return -1;
	}

	return 0;
}
