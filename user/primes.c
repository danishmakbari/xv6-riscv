#include "kernel/types.h"
#include "user/user.h"

static int isprime(int n)
{
	if (n < 2) {
		return 0;
	}
	for (int i = 2; i < n; i++) {
		if (!(n % i)) {
			return 0;
		}
	}
	return 1;
}

static int nextprime(int n)
{
	while (!isprime(++n));
	return n;
}

int main(int argc, char *argv[])
{
	int pid;
	int prime = 2;
	int fds[2];

	while (1) {
		if (pipe(fds) < 0) {
			return 1;
		}

		if ((pid = fork()) > 0) {
			close(fds[0]);
			if (prime > 35) {
				close(fds[1]);
				break;
			}

			printf("prime %d\n", prime);
			prime = nextprime(prime);
			write(fds[1], &prime, sizeof(prime));
			close(fds[1]);
			wait(0);
			break;
		} else if (!pid) {
			close(fds[1]);
			read(fds[0], &prime, 1);
			close(fds[0]);
		} else {
			close(fds[0]);
			close(fds[1]);
			return 1;
		}
	}

	return 0;
}

