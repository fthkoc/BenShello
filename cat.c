// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 05.04.2019
// cat utility for BenShello

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>


int main(int argc, const char* argv[]) {
	int fileDescriptor;
	char absolutePath[PATH_MAX];
	char commandLineBuffer[MAX_CANON];
	char buffer;

	if (argc > 2) {
		fprintf(stderr, "Wrong argument usage! Usage -> cat PATH\n");
		return -1;
	}

	if (argc == 1) {
		while (1) {
			fgets(commandLineBuffer, MAX_CANON, stdin);
			fprintf(stdout, "%s", commandLineBuffer);
		}
	}

	else { 
		if (argc == 2 && argv[1][0] != '/') {
			getcwd(absolutePath, PATH_MAX);
			strcat(absolutePath, "/");
			strcat(absolutePath, argv[1]);
		}
		else 
			strcpy(absolutePath, argv[1]);

		fileDescriptor = open(absolutePath, O_RDONLY);
		if (fileDescriptor == -1) {
			fprintf(stderr, "Cannot open the file '%s'!\n", argv[1]);
			return -1;
		}

		while (read(fileDescriptor, &buffer, sizeof(buffer)) != 0) {
			fprintf(stdout, "%c", buffer);
		}
		fprintf(stdout, "\n");

		close(fileDescriptor);
	}

	return 0;
}
