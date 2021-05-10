// This program creates a child process and communicates with it through UNIX domain sockets.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

int child_process(int socket_fd)
{
	char buffer[10];
	ssize_t length = recv(socket_fd, buffer, 9, 0);
	if (length < 0) {
		perror("Receiving failed");
		return -1;
	}
	else if (length == 0) {
		fprintf(stderr, "Nothing received in socket.\n");
		return -1;
	}
	buffer[length] = '\0';
	printf("Child received: %s\n", buffer);

	getchar(); // Wait for the user to signal continuation

	if (send(socket_fd, "OK!", 3, 0) < 0) {
		perror("Sending failed");
		return -1;
	}

	return 0;
}

int parent_process(int socket_fd)
{
	getchar(); // Wait for the user to signal continuation

	if (send(socket_fd, "OK?", 3, 0) < 0) {
		perror("Sending failed");
		return -1;
	}

	char buffer[10];
	ssize_t length = recv(socket_fd, buffer, 9, 0);
	if (length < 0) {
		perror("Receiving failed");
		return -1;
	}
	else if (length == 0) {
		fprintf(stderr, "Nothing received in socket.\n");
		return -1;
	}
	buffer[length] = '\0';
	printf("Parent received: %s\n", buffer);

	return 0;
}

int main(void)
{
	int socket_fd[2];
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, socket_fd) < 0) {
		perror("Creating socket pair failed");
		return EXIT_FAILURE;
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("Folk failed");
		if (close(socket_fd[0]) < 0)
			perror("Closing socket 0 failed");
		if (close(socket_fd[1]) < 0)
			perror("Closing socket 1 failed");
		return EXIT_FAILURE;
	}
	else if (pid == 0) { // in the child process
		printf("Child started.\n");

		if (close(socket_fd[1]) < 0) {
			perror("Closing socket 1 failed");
			if (close(socket_fd[0]) < 0)
				perror("Closing socket 0 failed");
			return EXIT_FAILURE;
		}

		if (child_process(socket_fd[0]) < 0) {
			if (close(socket_fd[0]) < 0)
				perror("Closing socket 0 failed");
			return EXIT_FAILURE;
		}

		if (close(socket_fd[0]) < 0) {
			perror("Closing socket 0 failed");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	else {
		if (close(socket_fd[0]) < 0) {
			perror("Closing socket 0 failed");
			if (close(socket_fd[1]) < 0)
				perror("Closing socket 1 failed");
			if (waitpid(pid, NULL, 0) < 0)
				perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		if (parent_process(socket_fd[1]) < 0) {
			if (close(socket_fd[1]) < 0)
				perror("Closing socket 1 failed");
			if (waitpid(pid, NULL, 0) < 0)
				perror("Waiting for child process failed");
			return EXIT_FAILURE;
		}

		if (close(socket_fd[1]) < 0) {
			perror("Closing socket 1 failed");
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
