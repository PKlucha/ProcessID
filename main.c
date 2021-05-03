/* 
    input args: -a: 		print all active processes
				-n <name>:	print pid of processes (one or more), specified by name
				-u <pid>:	print name of process, specified by pid
				-f <file>:	print to file, not std out. *optional
    
	output: 0 on success, -1 on error
    
	remarks: Prints active processes using /proc/<PID>/ files
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>

const char PROC_DIR_PATH[10] =  "/proc/";

// We need 2 of thoes directory handles,
// one for the "inner" directory
DIR *proc, *innerDir;
// Directory data buffer
struct dirent *dp;
// Where to print the output, when -f argument is specified
char* fileParameter;

// Prototypes
_Bool isNumber(char*);
char* extractName(char*);
void printAll(_Bool);
void printByName(char*, _Bool);
void printByPID(char*, _Bool);

int main(int argc, char* argv[]) {

	// Handling function's arguments
	if(argc == 2) {
		// 1 argument specified, only one option
		if(!strcmp(argv[1], "-a")) {
			printAll(0);
			return 0;
		}

	} else if(argc > 2) {
		if(argv[1][0] == '-') {
			if(argc == 3) {
				// 2 arguments specified, only 2 options:
				// -n <name>
				// -u <PID>
				switch((int)argv[1][1]) {
					case 'n':
						printByName(argv[2], 0);
						return 0;
					case 'u':
						printByPID(argv[2], 0);
						return 0;
				}
			} else if(argc > 3) {
				// Only 1 option with 3 arguments
				// -a -f <file>
				if(argc == 4 && !strcmp(argv[1], "-a")) {
					if(!strcmp(argv[2], "-f")) {
						fileParameter = argv[3];
						printAll(1);
						return 0;
					}
				} else if (argc == 5) {
					// 4 arguments specified, only 2 options:
					// -n <name> -f <file>
					// -u <PID> -f <file>
					if(!strcmp(argv[3], "-f")) {
						fileParameter = argv[4];

						if(!strcmp(argv[1],"-n")) {
							printByName(argv[2], 1);
							return 0;
						} else if (!strcmp(argv[1],"-u")) {
							printByPID(argv[2], 1);
							return 0;
						}
					}
				}
			}
		}
	}

	// Printing the error message when there's something wrong with specified arguments
	printf("Wrong or no arguments were specified!\n");
	printf("Possible arguments:\n");
	printf("-a - Print all active processes;\n");
	printf("-u <pid> - Print name of specified process;\n");
	printf("-n <name> - Print PID of specified process;\n");
	printf("-f <file> - Print output to a specified file. Optional, must be combined with any other argument.\n");
	printf("For example: ./ProcessID -a -f dump.txt\n");

	return -1;
}

// Check if a string is made only from digits
// If not, return false (0)
_Bool isNumber(char* name) {
	for(int i = 0; i < strlen(name); i ++) {
      //ASCII value of 0 = 48, 9 = 57.
      if (name[i] < 48 || name[i] > 57)
         return 0;
   }
   return 1;
}

char* extractName(char* path) {
	char *token, *name, *tofree, *ret;
	tofree = name = strdup(path);
	while ((token = strsep(&name, "/"))) {
		ret = token;
	}
	// Getting rid of some unknown characters
	// This should be needed only on my machine...
	for(int i = 0; ret[i] != '\0'; ++i) {
		// enter the loop if the character is not an alphabet
      	// and not the null character
      	if(!(ret[i] >= 'a' && ret[i] <= 'z') && !(ret[i] >= 'A' && ret[i] <= 'Z') 
			&& !(ret[i] == '\0') && !(ret[i] == '-') && !(ret[i] == '.')) {
			for (int j = i; ret[j] != '\0'; ++j) {
				// Just fill the rest of the buffor with zeros
				ret[j] = '\0';
			}
		}
   	}
	free(tofree);
	free(token);
	return ret;
}

// -a argument handling function
// Print all active processes
void printAll(_Bool TOFILE) {
	FILE *fh;
	if(TOFILE) {
		fh = fopen(fileParameter, "w");
		if(fh == NULL) {
			printf("Couldn't open file to write output!...");
			return;
		}
	}

	// Checking firstly if this process have access to /proc/ directory
	if((proc = opendir(PROC_DIR_PATH)) == NULL ) {
		perror("Cannot open proc file.\n");
		printf("Exiting...\n");
	} else {

		// Printing the top table
		if(TOFILE)
			fprintf(fh, "Process Name	Process ID\n");
		else
			printf("Process Name	Process ID\n");

		// Reading the /proc/ directory, file by file
		while((dp = readdir(proc)) != NULL) {

			// Checking if the file name is only digits
			// Skipp the rest
			if(!isNumber(dp->d_name)) {
				continue;
			}

			// Opening the inner directory (for the process name) 
			char tmpPath[40];
			snprintf(tmpPath, 40, "%s%s/", PROC_DIR_PATH, dp->d_name);
			if((innerDir = opendir(tmpPath)) == NULL) {
				perror("Cannot open inner file!\n");
				printf("Skipping...\n");
				continue;
			}
			
			// We know there's always an "exe" file in /proc/{procID}/
			// readlink will just return an error when there's no process name 
			char buffor[250];
			ssize_t length;
			// Setting new path to exe link file
			snprintf(tmpPath, 40, "%s%s/%s", PROC_DIR_PATH, dp->d_name, "exe");

			// Checking if the process directory's exe file is a symbolic link
			if((length = readlink(tmpPath, buffor, sizeof(buffor)-1)) != -1 ) {
				// Process have a name, display it
				char* tmpName = extractName(buffor);
				if(TOFILE) {
					fprintf(fh, "%s	%s\n", tmpName, dp->d_name);
				} else {
					printf("%s	%s\n", tmpName, dp->d_name);
				}
			}
		}
	}
	free(dp);
	free(proc);
	free(innerDir);
	if(TOFILE)
		fclose(fh);
}

void printByName(char* name, _Bool TOFILE) {
	FILE *fh;
	if(TOFILE) {
		fh = fopen(fileParameter, "w");
		if(fh == NULL) {
			printf("Couldn't open file to write output!...");
			return;
		}
	}

	// Checking firstly if this process have access to /proc/ directory
	if((proc = opendir(PROC_DIR_PATH)) == NULL ) {
		perror("Cannot open proc file.\n");
		printf("Exiting...\n");
	} else {

		// Reading the /proc/ directory, file by file
		while((dp = readdir(proc)) != NULL) {

			// Checking if the file name is only digits
			// Skipp the rest
			if(!isNumber(dp->d_name)) {
				continue;
			}

			// Opening the inner directory (for the process name) 
			char tmpPath[40];
			snprintf(tmpPath, 40, "%s%s/", PROC_DIR_PATH, dp->d_name);
			if((innerDir = opendir(tmpPath)) == NULL) {
				perror("Cannot open inner file!\n");
				printf("Skipping...\n");
				continue;
			}
			
			// We know there's always an "exe" file in /proc/{procID}/
			// readlink will just return an error when there's no process name 
			char buffor[250];
			ssize_t length;
			// Setting new path to exe link file
			snprintf(tmpPath, 40, "%s%s/%s", PROC_DIR_PATH, dp->d_name, "exe");

			// Checking if the process directory's exe file is a symbolic link
			if( (length = readlink(tmpPath, buffor, sizeof(buffor)-1)) != -1 ) {

				// Check if any of found names matches the querry
				// There can be more than one process with the same name
				char* tmpName = extractName(buffor);
				if(!strcmp(name, tmpName)) {
					if(TOFILE)
						fprintf(fh, "%s\n", dp->d_name);
					else 
						printf("%s\n", dp->d_name);
				}
			}
		}
	}
	free(dp);
	free(proc);
	free(innerDir);
	if(TOFILE)
		fclose(fh);
}

void printByPID(char* pid, _Bool TOFILE) {
	FILE *fh;
	if(TOFILE) {
		fh = fopen(fileParameter, "w");
		if(fh == NULL) {
			printf("Couldn't open file to write output!...");
			return;
		}
	}

	// Checking firstly if this process have access to /proc/ directory
	if((proc = opendir(PROC_DIR_PATH)) == NULL ) {
		perror("Cannot open proc file.\n");
		printf("Exiting...\n");
	} else {

		// Reading the /proc/ directory, file by file
		while((dp = readdir(proc)) != NULL) {

			// Checking if the file name is only digits
			// Skipp the rest
			if(!isNumber(dp->d_name)) {
				continue;
			}

			if(!strcmp(dp->d_name, pid)) {
				// Opening the inner directory (for the process name) 
				char tmpPath[40];
				snprintf(tmpPath, 40, "%s%s/", PROC_DIR_PATH, dp->d_name);
				if( (innerDir = opendir(tmpPath)) == NULL) {
					perror("Cannot open inner file!\n");
					printf("Skipping...\n");
					continue;
				}

				char buffor[250];
				ssize_t length;
				// Setting new path to exe link file
				snprintf(tmpPath, 40, "%s%s/%s", PROC_DIR_PATH, dp->d_name, "exe");

				// Checking if the process directory's exe file is a symbolic link
				if((length = readlink(tmpPath, buffor, sizeof(buffor)-1)) != -1 ) {
					if(TOFILE)
						fprintf(fh, "%s\n", extractName(buffor));
					else 
						printf("%s\n", extractName(buffor));
				}
			}
		}

		free(dp);
		free(proc);
		free(innerDir);
		if(TOFILE)
			fclose(fh);
	}
}
