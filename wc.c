// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 05.04.2019
// wc utility for BenShello

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>


int main(int argc, const char* argv[]) {
	int fileDescriptor;
	char absolutePath[PATH_MAX];
	char buffer;
	int lineCount = 0;
	int byteRead;

	#ifdef DEBUG
		fprintf(stdout, "\nwc DEBUG:\n");
		for (int i = 0; i < argc; ++i) {
			fprintf(stdout, "argv[%d]:%s\n", i, argv[i]);
		}
	#endif

	if (argc > 2) {
		fprintf(stderr, "Wrong argument usage! Usage -> wc PATH\n");
		return -1;
	}

	if (argc == 1) {
		byteRead = read(STDIN_FILENO, absolutePath, MAX_CANON);
		for (int i = 0; i < byteRead; ++i) {
			if (absolutePath[i] == '\n') {
				absolutePath[i] = '\0';
				break;
			}
		}
	}
	else {
		if (argv[1][0] != '/') {
			getcwd(absolutePath, PATH_MAX);
			strcat(absolutePath, "/");
			strcat(absolutePath, argv[1]);
		}
		else 
			strcpy(absolutePath, argv[1]);
	}

	fileDescriptor = open(absolutePath, O_RDONLY);
	if (fileDescriptor == -1) {
		fprintf(stderr, "Cannot open the file '%s'!\n", absolutePath);
		return -1;
	}

	while (read(fileDescriptor, &buffer, sizeof(buffer)) != 0) {
		if (lineCount == 0)
			lineCount++;
		if (buffer == '\n')
			lineCount++;
	}

	close(fileDescriptor);

	fprintf(stdout, "%d\n", lineCount);

	return 0;
}
