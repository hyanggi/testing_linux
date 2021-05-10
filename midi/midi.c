#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

ssize_t msg_write(int fd, const void *data, size_t len) {
	ssize_t res = write(fd, data, len);
	if (res < 0) {
		perror("write failed");
		return -1;
	}
	else if ((size_t)res != len) {
		fprintf(stderr, "Incomplete write\n");
		return -1;
	}
	return 0;
}

int run_with_dev_opened(int fd) {
	unsigned char data[3];
	data[2] = 127; // maximum velocity

	data[0] = 0x90; // note-on
	data[1] = 60; // C4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	usleep(300*1000);
	data[1] = 64; // E4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	usleep(300*1000);
	data[1] = 67; // G4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	usleep(700*1000);

	data[0] = 0x80; // note-off
	data[1] = 60; // C4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 64; // E4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 67; // G4
	if (msg_write(fd, data, 3) < 0)
		return -1;

	data[0] = 0x90; // note-on
	data[1] = 60; // C4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 64; // E4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 67; // G4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	usleep(1800*1000);

	data[0] = 0x80; // note-off
	data[1] = 60; // C4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 64; // E4
	if (msg_write(fd, data, 3) < 0)
		return -1;
	data[1] = 67; // G4
	if (msg_write(fd, data, 3) < 0)
		return -1;

	return 0;
}

int main(void) {
	const char *dev = "/dev/snd/midiC1D0";

	int fd = open(dev, O_WRONLY, 0);
	if (fd < 0) {
		perror("open failed");
		return -1;
	}

	if (run_with_dev_opened(fd) < 0) {
		if (close(fd) < 0)
			perror("close failed");
		return -1;
	}

	if (close(fd) < 0) {
		perror("close failed");
		return -1;
	}

	return 0;
}
