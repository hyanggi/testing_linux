// This program creates a child process and sends it some information through a pipe (also called anonymous pipe).

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int child_process(int pipe_fd)
{
	char buffer[20];
	size_t num_char_read = 0;
	while (num_char_read < sizeof buffer - 1) {
		ssize_t length = read(pipe_fd, buffer + num_char_read, sizeof buffer - 1 - num_char_read);
		if (length < 0) {
			perror("Reading failed");
			return -1;
		}
		else if (length == 0)
			break;
		else
			num_char_read += (size_t)length;
	}
	buffer[num_char_read] = '\0';
	printf("Received: %s\n", buffer);
	return 0;
}

int full_write(int fd, const char *buf, size_t len)
{
	size_t len_written = 0;
	while (len_written < len) {
		ssize_t res = write(fd, buf + len_written, len - len_written);
		if (res < 0) {
			perror("Writing failed");
			return -1;
		}
		else if (res == 0) {
			fprintf(stderr, "Error: write returned 0\n");
			return -1;
		}
		else
			len_written += (size_t)res;
	}
	return 0;
}

int parent_process(int pipe_fd)
{
	getchar(); // Wait for the user to signal continuation
	if (full_write(pipe_fd, "haha", 4) < 0)
		return -1;
	getchar(); // Wait for the user to signal continuation
	if (full_write(pipe_fd, "hoho", 4) < 0)
		return -1;
	return 0;
}

int main(void)
{
	int pipe_fd[2];
	if (pipe(pipe_fd) < 0) {
		perror("Creating pipe failed");
		return EXIT_FAILURE;
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("Folk failed");
		if (close(pipe_fd[0]) < 0)
			perror("Closing pipe's read end failed");
		if (close(pipe_fd[1]) < 0)
			perror("Closing pipe's write end failed");
		return EXIT_FAILURE;
	}
	else if (pid == 0) { // in the child process
		printf("Child started.\n");

		if (close(pipe_fd[1]) < 0) {
			perror("Closing pipe's write end failed");
			if (close(pipe_fd[0]) < 0)
				perror("Closing pipe's read end failed");
			return EXIT_FAILURE;
		}

		if (child_process(pipe_fd[0]) < 0) {
			if (close(pipe_fd[0]) < 0)
				perror("Closing pipe's read end failed");
			return EXIT_FAILURE;
		}

		if (close(pipe_fd[0]) < 0) {
			perror("Closing pipe's read end failed");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	else {
		if (close(pipe_fd[0]) < 0) {
			perror("Closing pipe's read end failed");
			if (close(pipe_fd[1]) < 0)
				perror("Closing pipe's write end failed");
			if (waitpid(pid, NULL, 0) < 0)
				perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		if (parent_process(pipe_fd[1]) < 0) {
			if (close(pipe_fd[1]) < 0)
				perror("Closing pipe's write end failed");
			if (waitpid(pid, NULL, 0) < 0)
				perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		if (close(pipe_fd[1]) < 0) {
			perror("Closing pipe's write end failed");
			if (waitpid(pid, NULL, 0) < 0)
				perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		if (waitpid(pid, NULL, 0) < 0) {
			perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
}
