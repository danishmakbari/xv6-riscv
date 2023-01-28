#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int pid;
	char byte;
	int fds[2];

	if (pipe(fds) < 0) {
		return 1;
	}

	if ((pid = fork()) > 0) {
		byte = 0;
		write(fds[1], &byte, 1);
		read(fds[0], &byte, 1);
		printf("%d: received pong\n", getpid());
	} else if (!pid) {
		read(fds[0], &byte, 1);
		printf("%d: received ping\n", getpid());
		write(fds[1], &byte, 1);
	} else {
		close(fds[0]);
		close(fds[1]);
		return 1;
	}

	return 0;
}

