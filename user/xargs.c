#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int i = 0, args_i = 0, pid;
	char ch;
	char buf[512];
	char *args[MAXARG];

	for (int i = 0; i < 11; i++) {
		args[i] = 0;
	}

	for (int i = 1; i < argc; i++) {
		int len = strlen(argv[i]) + 1;
		args[i - 1] = malloc(len);
		strcpy(args[i - 1], argv[i]);
		args_i++;
	}

	while (1) {
		if (!read(0, &ch, 1)) {
			if (i) {
				buf[i] = '\0';
				i++;
				args[args_i] = malloc(i);
				strcpy(args[args_i], buf);
			}
			break;
		} else if (ch == ' ' || ch == '\n') {
			if (!i) {
				continue;
			} else {
				buf[i] = '\0';
				i++;
				args[args_i] = malloc(i);
				strcpy(args[args_i], buf);
				args_i++;
				i = 0;
			}
		} else {
			buf[i] = ch;
			i++;
		}
	}
	
	if ((pid = fork()) > 0) {
		wait(0);
	} else if (!pid) {
		exec(args[0], args);
	} else {
		return 1;
	}

	return 0;
}

