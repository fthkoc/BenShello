// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 01.04.2019
// pwd utility for BenShello

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>


int main(void) {
	char path[PATH_MAX];
	if (getcwd(path, PATH_MAX) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	fprintf(stdout, "%s\n", path);
	return 0;
}
