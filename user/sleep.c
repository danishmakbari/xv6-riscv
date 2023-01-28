#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		return 0;	
	}

	for (int i = 1; i < argc; i++) {
		sleep(atoi(argv[i]));
	}
	
	return 0;
}

