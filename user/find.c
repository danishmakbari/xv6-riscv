#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *name)
{
	int len;
	int fd;
	struct stat st;
	struct dirent de;
	char buf[512];

	if ((fd = open(path, 0)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch (st.type) {
	case T_DIR:
		strcpy(buf, path);
		len = strlen(buf);
		buf[len] = '/';
		len++;
		buf[len] = '\0';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, "..")) {
				continue;
			}
			
			strcpy(buf + len, de.name);
			find(buf, name);
		}
		break;

	default:
		len = strlen(path) - strlen(name);
		if (len >= 0 && !strcmp(path + len, name)) {
			printf("%s\n", path);
		}
	}
	
	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		return 1;
	}

	find(argv[1], argv[2]);

	return 0;
}
