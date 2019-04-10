// CSE344 - System Programming HW3, 141044013, Fatih KOÇ, 01.04.2019

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>



int makeArgArr(const char *inputStr, const char *delimiters, char ***argArrPtr);
void freeArgArr(char **argArr);
void help();
void exitFromShell();
void changeDirectory(char* path);
int handleRedirection(char* commandLineBuffer);
int handlePipe(char* commandLineBuffer);
void executePipe(char *s);

// Shell and utility executables directory path
static char workingDirectory[PATH_MAX];


int main (void) {
	char commandLineBuffer[MAX_CANON];
	int commandLength;
	char **arguments;
	int argumentCount;
	pid_t child;
	char path[PATH_MAX];
	char currentDirectory[PATH_MAX];
	char commandHistory[10][MAX_CANON];
	int commandCounter = 0;
    // struct sigaction action;

    // memset(&action, 0, sizeof(struct sigaction));
    // action.sa_handler = exitFromShell;
    // sigaction(SIGTERM, &action, NULL);

	// Main directory to run the utilities and BenShello
	if (getcwd(workingDirectory, PATH_MAX) == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	
	while (1) {
		// Flush the buffers
		fflush(stdin);
		fflush(stdout);

		// Prompt string: BenShello@:PATH>>
		getcwd(currentDirectory, PATH_MAX);
		fprintf(stdout, "BenShello@:%s>>", currentDirectory); 

		// Keep getting input from prompt screen
		if (fgets(commandLineBuffer, MAX_CANON, stdin) == NULL)
			continue;

		if (strcmp(commandLineBuffer, "\n") == 0) {
			continue;
		}

		// Reset the length after read an input
		commandLength = strlen(commandLineBuffer);
		if (commandLineBuffer[commandLength - 1] == '\n')
			commandLineBuffer[commandLength - 1] = 0;

		// exit command of the BenShello
		if (strcmp(commandLineBuffer, "exit") == 0) { 
			exitFromShell();
			exit(0);
		}

		// !n command of the BenShello
		else if (commandLineBuffer[0] == '!') { 
			strcpy(commandLineBuffer, commandHistory[atoi(&commandLineBuffer[1]) - 1]);
		}

		// Keep previous command records at the commandHistory
		else {
			strcpy(commandHistory[commandCounter], commandLineBuffer);
			commandCounter++;
		}

		// Parse the commandLineBuffer and create an argument array
		if ((argumentCount = makeArgArr(commandLineBuffer, " ", &arguments)) <= 0) {
			fprintf(stderr, "Failed to parse command line!\n");
			continue;
		}

		// Is input command runs an utility?
		if ((strcmp(arguments[0], "pwd") == 0) || (strcmp(arguments[0], "lsf") == 0) || (strcmp(arguments[0], "cat") == 0) || (strcmp(arguments[0], "wc") == 0) || (strcmp(arguments[0], "bunedu") == 0))
		{
			// Modify the utility executable path to make it absolute path
			strcpy(path, workingDirectory);
			strcat(path, "/");
			strcat(path, arguments[0]);

			switch (child = fork()) {
				case -1:
					fprintf(stderr, "Error at fork()!\n");
					exit(1);
				case 0:
					// Is there any pipe command?
					if (strchr(commandLineBuffer, '|') != NULL) {
						#ifdef DEBUG
							fprintf(stdout, "DEBUG:\tpipe commandLineBuffer:[%s]\n", commandLineBuffer);
						#endif
						// Perform the pipe operation and execute command
						if (handlePipe(commandLineBuffer) != 0) {
							fprintf(stderr, "Failed to handle pipe for '%s'\n", commandLineBuffer);
							exit(1);
						}
						exit(0);
					}
					// Is there any redirection command?
					else if ((strchr(commandLineBuffer, '<') != NULL) || (strchr(commandLineBuffer, '>') != NULL)) {
						// Perform the redirection
						if (handleRedirection(commandLineBuffer) != 0) {
							fprintf(stderr, "Failed to handle redirection for '%s'\n", commandLineBuffer);
							exit(1);
						}
						// After removal of the redirection character(if there is), reconstruct the argument array
						if ((argumentCount = makeArgArr(commandLineBuffer, " ", &arguments)) <= 0) {
							fprintf(stderr, "Failed to parse command line\n");
							exit(1);
						}
					}
					// Execute the command
					execvp(path, arguments);
					fprintf(stderr, "Failed to execute command in main!\n");
					exit(1);
				default:
					waitpid(child, NULL, 0);
					break;
			}
		}
		// Other commands are built-in to the BenShello
		else {
			if (strcmp(arguments[0], "cd") == 0) {
				changeDirectory(arguments[1]);
			}
			else if (strcmp(arguments[0], "help") == 0) {
				help();
			}
			else
				fprintf(stderr, "Unsupported action!\n");
		}
		freeArgArr(arguments);
	}

	return 0;
}

// Function to create an argument array from the inputStr by using given delimiters
// I took some parts of this function from the book Unix System Programming, Chapter 2, Program 2.2, makeargv function.
int makeArgArr(const char *inputStr, const char *delimiters, char ***argArrPtr) {
	int error;
	int argumentCount = 0;
	char* argument;
	char* temp;
	char tempInputStr[strlen(inputStr)];
	int i;
	
	// Parameter validation
	if ((inputStr == NULL) || (delimiters == NULL) || (argArrPtr == NULL)) {
		errno = EINVAL;
		fprintf(stderr, "makeArgArr error for '%s'\n%s\n", inputStr, strerror(errno));
		return -1;
	}
	
	*argArrPtr = NULL;
	strcpy(tempInputStr, inputStr);
	argument = tempInputStr + strspn(inputStr, delimiters);
	
	if ((temp = malloc(strlen(argument) + 1)) == NULL)
		return -1;
	
	strcpy(temp, argument);
	
	// count the number of tokens in inputStr
	if (strtok(temp, delimiters) != NULL)
		for (argumentCount = 1; strtok(NULL, delimiters) != NULL; argumentCount++) ;
	
	// create argument array for argArrPtr to the tokens
	if ((*argArrPtr = malloc((argumentCount + 1) * sizeof(char *))) == NULL) {
		error = errno;
		free(temp);
		errno = error;
		return -1;
	}
	// insert pointers to tokens into the argument array
	if (argumentCount == 0)
		free(temp);
	else {
		strcpy(temp, argument);
		**argArrPtr = strtok(temp, delimiters);
		for (i = 1; i < argumentCount; i++)
			*((*argArrPtr) + i) = strtok(NULL, delimiters);
	}
	
	// put in final NULL pointer
	*((*argArrPtr) + argumentCount) = NULL; 
	
	return argumentCount;
}

void freeArgArr(char **argArr) {
	if (argArr == NULL)
		return;
	if (*argArr != NULL)
		free(*argArr);
	free(argArr);
}

// help command of the BenShello
void help() {
	fprintf(stdout, "Supported commands by BenShello:\n");
	fprintf(stdout, "lsf 	: Lists all files in the present working directory.\n");
	fprintf(stdout, "pwd 	: Prints the path of present working directory.\n");
	fprintf(stdout, "cd  	: Changes the present working directory to the location.\n");
	fprintf(stdout, "cat 	: Prints the contents of the file to the standart output.\n");
	fprintf(stdout, "wc  	: Prints the number of lines in the file to the standard output.\n");
	fprintf(stdout, "bunedu : Displays the sizes of the subdirectories of the tree rooted at the given directory.\n");
	fprintf(stdout, "help   : Prints the list of supported commands.\n");
	fprintf(stdout, "exit   : Exits from the BenShello.\n");
}

//Function to do stuff that needs to be done before quit from BenShello
void exitFromShell() {
	fprintf(stdout, "See you!\n");
}

// cd command of the BenShello
void changeDirectory(char* path) {
	char currentDirectory[PATH_MAX];
	
	// If the given path is not an absolute path, modify it
	if (path[0] != '/') {
		getcwd(currentDirectory, PATH_MAX);
		strcat(currentDirectory, "/");
		strcat(currentDirectory, path);
		
		if (chdir(currentDirectory) == -1) {
			fprintf(stderr, "Error at cd: %s\n", strerror(errno));
		}
	}
	else {
		if (chdir(path) == -1) {
			fprintf(stderr, "Error at cd: %s\n", strerror(errno));
		}
	}

	getcwd(currentDirectory, PATH_MAX);
}

// Function to handle redirection if there is a redirection character in the given commandLineBuffer
int handleRedirection(char* commandLineBuffer) {
	char* charPositionPtr = NULL;
	int redirectionFileDescriptor;

	// Input redirection
	if ((charPositionPtr = strchr(commandLineBuffer, '<')) != NULL) {
		// Remove the redirection character from commandLineBuffer, to prevent it being sent to the arguments
		*charPositionPtr = '\0';
		if ((charPositionPtr = strtok(charPositionPtr + 1, " ")) != NULL) {
			if ((redirectionFileDescriptor = open(charPositionPtr, O_RDONLY)) == -1) {
				fprintf(stderr, "Cannot open file %s!\n", charPositionPtr);
				return -1;
			}
			if (dup2(redirectionFileDescriptor, STDIN_FILENO) == -1) {
				fprintf(stderr, "Cannot duplicate the stdin file descriptor!\n");
				close(redirectionFileDescriptor);
				return -1;
			}
			close(redirectionFileDescriptor);
		}
	}

	// Output redirection
	else if ((charPositionPtr = strchr(commandLineBuffer, '>')) != NULL) {
		// Remove the redirection character from commandLineBuffer, to prevent it being sent to the arguments
		*charPositionPtr = '\0';
		if ((charPositionPtr = strtok(charPositionPtr + 1, " ")) != NULL) {
			if ((redirectionFileDescriptor = open(charPositionPtr, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
				fprintf(stderr, "Cannot open file %s!\n", charPositionPtr);
				return -1;
			}
			if (dup2(redirectionFileDescriptor, STDOUT_FILENO) == -1) {
				fprintf(stderr, "Cannot duplicate the stdout file descriptor!\n");
				close(redirectionFileDescriptor);
				return -1;
			}
			close(redirectionFileDescriptor);
		}
	}

	return 0;
}

// Function to handle pipe if there is a pipe character in the given commandLineBuffer
int handlePipe(char* commandLineBuffer) {
	int pipeChild;
	int count;
	int fds[2];
	char **pipelist;

	count = makeArgArr(commandLineBuffer, "|", &pipelist);
	if (count < 1) {
		fprintf(stderr, "Failed to find any commands\n");
		exit(1);
	}

	#ifdef DEBUG
	fprintf(stdout, "DEBUG:\tpipe pipelist:\n");
	for (int i = 0; i < count; ++i) {
		fprintf(stdout, "argv[%d]:%s\n", i, pipelist[i]);
	}
	#endif

	if (pipe(fds) == -1)
		fprintf(stderr, "Failed to create pipes");
	switch (pipeChild = fork()) {
		case -1:
			fprintf(stderr, "Error at fork()!\n");
			exit(1);
		case 0:
			if (dup2(fds[1], STDOUT_FILENO) == -1)                   
			    fprintf(stderr, "Failed to connect last component");
		    close(fds[0]);
		    close(fds[1]);
		    #ifdef DEBUG
				fprintf(stdout, "handlePipe Child created.\n");
				fprintf(stdout, "DEBUG:\tpipe pipelist[0]:[%s]\n", pipelist[0]);
			#endif
		    executePipe(pipelist[0]);
			exit(0);
		default:
			if (dup2(fds[0], STDIN_FILENO) == -1)
		    	fprintf(stderr, "Failed to connect pipeline");
		    close(fds[0]);
		    close(fds[1]);
			#ifdef DEBUG
				fprintf(stdout, "DEBUG:\tpipe pipelist[1]:[%s]\n", pipelist[1]);
			#endif
			executePipe(pipelist[1]);
			waitpid(pipeChild, NULL, 0);
			break;
	}
	freeArgArr(pipelist);
	return 0;
}

void executePipe(char *s) {
	char **arguments;
	char path[PATH_MAX];
	int argc;
	#ifdef DEBUG
		fprintf(stdout, "DEBUG:\nexecutePipe s:[%s]\n", s);
	#endif
	if ((argc = makeArgArr(s, " ", &arguments)) <= 0) {
		fprintf(stderr, "Failed to parse command line\n");
		freeArgArr(arguments);
	}

	else {
		#ifdef DEBUG
			fprintf(stdout, "DEBUG:\nexecutePipe arguments:");
			for (int i = 0; i < argc; ++i) {
				fprintf(stdout, "arguments[%d]:%s\n", i, arguments[i]);
			}
		#endif
		strcpy(path, workingDirectory);
		strcat(path, "/");
		strcat(path, arguments[0]);
		#ifdef DEBUG
			fprintf(stdout, "DEBUG:\nexecutePipe path:[%s]\n", path);
		#endif
		execvp(path, arguments);
		fprintf(stderr, "Failed to execute command in executePipe!\n");
		exit(1);
	}
}
