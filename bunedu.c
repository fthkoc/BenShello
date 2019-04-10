// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 01.04.2019
// bunedu utility for BenShello, buNeDu program from the HW1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>



int depthFirstApply(char *path, int pathfun (char *path1));
int sizepathfun(char *path);

// Flag for '-z' argument
static int FLAG = 0;


int main(int argc, const char* argv[]) {
	struct stat fileStat;
	char path[PATH_MAX];
	int byteRead;

	#ifdef DEBUG
		fprintf(stdout, "\nbunedu DEBUG:\n");
		for (int i = 0; i < argc; ++i) {
			fprintf(stdout, "argv[%d]:%s\n", i, argv[i]);
		}
	#endif

	// Command-line argument validation
	switch (argc){
		case 1:
			byteRead = read(STDIN_FILENO, path, MAX_CANON);
			for (int i = 0; i < byteRead; ++i) {
				if (path[i] == '\n') {
					path[i] = '\0';
					break;
				}
			}
			break;
		case 2:
			if (strcmp(argv[1], "-z") == 0) {
				byteRead = read(STDIN_FILENO, path, MAX_CANON);
				for (int i = 0; i < byteRead; ++i) {
					if (path[i] == '\n') {
						path[i] = '\0';
						break;
					}
				}
				break;
			}
			else if ((lstat(argv[1], &fileStat) < 0) || !(S_ISDIR(fileStat.st_mode))) {
				fprintf(stderr, "Wrong argument usage!\nUsage -> ./buNeDu PATH\nUsage -> ./buNeDu -z PATH\n");
				return -1;
			}
			break;
		case 3:
			if ((strcmp(argv[1], "-z") != 0) || (lstat(argv[2], &fileStat) < 0) || !(S_ISDIR(fileStat.st_mode))) {
				fprintf(stderr, "Wrong argument usage!\nUsage -> ./buNeDu PATH\nUsage -> ./buNeDu -z PATH\n");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "Invalid number of arguments!\nUsage -> ./buNeDu PATH\nUsage -> ./buNeDu -z PATH\n");
			return -1;
	}

	switch (argc){
		case 1:
			FLAG = 2;
			break;
		case 2:
			if (strcmp(argv[1], "-z") == 0)
				FLAG = 3;
			else {
				FLAG = 2;
				strcpy(path, argv[argc - 1]);
			}
			break;
		case 3:
			strcpy(path, argv[argc - 1]);
			break;
		default:
			fprintf(stderr, "Invalid number of arguments!\nUsage -> ./buNeDu PATH\nUsage -> ./buNeDu -z PATH\n");
			return -1;
	}

	if (depthFirstApply(path, sizepathfun) == -1) {
		fprintf(stderr, "Error at depthFirstApply() for [%s]!\n", path);
		return -1;
	}
	
	return 0;
}

/*
 * Traverses the tree, starting at the path. It applies the pathfun function to each subdirectory that it encounters in the traversal.
 * The function returns the sum of the positive return values of pathfun, or -1 if it failed to traverse any subdirectory.
 */
int depthFirstApply(char *path, int pathfun (char *path1)) {
	#ifdef DEBUG
		fprintf(stdout, "\t\t%s\t[%s]\n", "depthFirstApply called:", path);
	#endif

	struct stat fileStat;
    DIR *pDir;
    struct dirent *pDirEnt;
    char filePath[PATH_MAX];
	int totalSize = 0, size = 0, recSize = 0;

	if (lstat(path, &fileStat) < 0) {
		fprintf(stderr, "lstat() error at depthFirstApply() for [%s]!\n", path);
		return -1;
	}

	if (S_ISDIR(fileStat.st_mode)) {
		if ((pDir = opendir(path)) == NULL) {
			fprintf(stderr, "Cannot open directory '%s'!\n", path);
			return -1;
		}
		while ((pDirEnt = readdir(pDir)) != NULL) {
			// [.] and [..] directories avoided to stay in the given path
			if ((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0))
	            continue;
	        // filePath is updated for every file in the directory
			sprintf(filePath, "%s/%s", path, pDirEnt->d_name);
			// Current file size
			size = pathfun(filePath);
			if (size == -1) {
				fprintf(stderr, "Error at pathfun() for '%s'!\n", path);
				return -1;
			}
			// Subdirectory size (recursive call)
			recSize = depthFirstApply(filePath, pathfun);
			if (recSize == -1) {
				fprintf(stderr, "Error at depthFirstApply() for '%s'!\n", path);
				return -1;
			}
			// Calculation of total size depends on the "-z" flag
			if (FLAG == 1 || FLAG == 2) {
				totalSize += size;
			} 
			else if (FLAG == 3){
				totalSize += recSize;
			}

			#ifdef DEBUG
				fprintf(stdout, "\t%d\t[%s]\n", totalSize, filePath);
			#endif
		}
		closedir (pDir);
		// Prints each directories size and name to console
		fprintf(stdout, "%d\t%s\n", totalSize, path);
	}
	// If the command-line argument path is not a directory
	else {
		totalSize += pathfun(path);
	}

	#ifdef DEBUG
		fprintf(stdout, "\t\tdepthFirstApply returns:\t[%d]:[%s]\n", totalSize, path);
	#endif
    
	return totalSize;
}

/*
 * Outputs path along with other information obtained by calling lstat for path.
 * Returns the size in blocks of the file given by path or -1 if path does not corresponds to an ordinary file.
 */
int sizepathfun(char* path) {
	struct stat fileStat;
	int result = 0;

	#ifdef DEBUG
		fprintf(stdout, "\t\t%s\t[%s]\n", "sizepathfun called:", path);
	#endif

	if ((lstat(path, &fileStat) < 0)) {
		fprintf(stderr, "lstat() error at sizepathfun() for [%s]!\n", path);
		return -1;
	}
	if (S_ISDIR(fileStat.st_mode)) {
		result = 0;
	}
	// if 'path' is not directory or regular file, but defined by system, it is considered as a special file
	else if (S_ISLNK(fileStat.st_mode) || S_ISFIFO(fileStat.st_mode) || S_ISBLK(fileStat.st_mode) || S_ISCHR(fileStat.st_mode) || S_ISSOCK(fileStat.st_mode)) {
		fprintf(stdout, "Special file %s\n", path);
		result = 0;
	}
	else if (S_ISREG(fileStat.st_mode)) {
		result = fileStat.st_size;
	}
	else {
		fprintf(stderr, "Unknown type of file [%s]\n", path);
		return -1;
	}

	#ifdef DEBUG
		fprintf(stdout, "\t\tsizepathfun returns:\t[%d]:[%s]\n", result, path);
	#endif

	return result;
}
