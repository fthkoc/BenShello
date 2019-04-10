// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 01.04.2019
// lsf utility for BenShello

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>


char* getFilePermissions(char* fileName);

int main(void) {
	DIR *pDir;
	struct stat mainFileStat;
	struct stat fileStat;
    struct dirent *pDirEnt;
	char mainPath[PATH_MAX];
	char fileType;

	if (getcwd(mainPath, PATH_MAX) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	if (lstat(mainPath, &mainFileStat) < 0) {
		fprintf(stderr, "lstat() error for '%s'!\n", mainPath);
		return -1;
	}

	if ((pDir = opendir(mainPath)) == NULL) {
		fprintf(stderr, "Cannot open directory '%s'!\n", mainPath);
		return -1;
	}

	while ((pDirEnt = readdir(pDir)) != NULL) {
		if ((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0))
            continue;
        if (lstat(pDirEnt->d_name, &fileStat) < 0) {
			fprintf(stderr, "lstat() error for '%s'!\n", pDirEnt->d_name);
			return -1;
		}
		if (S_ISDIR(fileStat.st_mode))
			continue;
		else if (S_ISREG(fileStat.st_mode))
			fileType = 'R';
		else
			fileType = 'S';

		fprintf(stdout, "%c\t%s\t%ld\t%s\n", fileType, getFilePermissions(pDirEnt->d_name), fileStat.st_size, pDirEnt->d_name);
	}

	closedir (pDir);

	return 0;
}

char* getFilePermissions(char* fileName){
    struct stat fileStat;
    char* result = malloc(sizeof(char) * 9 + 1);
    if (stat(fileName, &fileStat) == 0) {
        mode_t permission = fileStat.st_mode;
        result[0] = (permission & S_IRUSR) ? 'r' : '-';
        result[1] = (permission & S_IWUSR) ? 'w' : '-';
        result[2] = (permission & S_IXUSR) ? 'x' : '-';
        result[3] = (permission & S_IRGRP) ? 'r' : '-';
        result[4] = (permission & S_IWGRP) ? 'w' : '-';
        result[5] = (permission & S_IXGRP) ? 'x' : '-';
        result[6] = (permission & S_IROTH) ? 'r' : '-';
        result[7] = (permission & S_IWOTH) ? 'w' : '-';
        result[8] = (permission & S_IXOTH) ? 'x' : '-';
        result[9] = '\0';
        return result;     
    }
    else
        return strerror(errno);   
}
