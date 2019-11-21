#include "get_path.h"

int pid;
int sh( int argc, char **argv, char **envp);
char **stringToArray(char *s, char **args, int *argCount);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void freePathList(struct pathelement *pathlist);

#define PROMPTMAX 32
#define MAXARGS 10
