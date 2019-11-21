#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"
#include <wordexp.h>

#define BUFFER 128

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args;
  int uid, i, status, argsct=0, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  
  uid = getuid();
  password_entry = getpwuid(uid);   /* get passwd info */
  homedir = password_entry->pw_dir; /* Home directory to start
				       out with*/
  
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
    {
      perror("getcwd");
      exit(2);
    }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';
  
  /* Put PATH into a linked list */
  pathlist = get_path();
  
  while ( go )
    {
      /* print your prompt */
      printf("%s [%s]> ", prompt, pwd);
      /* get command line and process */
      arg = fgets(commandline, BUFFER, stdin);
      int len = strlen(arg);
      arg[len-1]='\0';
      argsct=0;
      args=stringToArray(arg, argv, &argsct);
      if(argsct>0){
	/* check for each built in command and implement */
	
	/* EXIT */
	/* no mem leaks, works as intended */
	if(strcmp(args[0], "exit")==0){
	  printf("Executing built-in exit:\n");
	  go = 0;
	  free(args[0]);
	  free(args);
	  free(owd);
	  free(pwd);
	}
	/* WHERE */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "where")==0){
	  if(argsct==1){
	    printf("where: too few arguments\n");
	    free(args[0]);
	    free(args);
	  }
	  else {
	    printf("Executing built-in where:\n");
	    int temp = argsct;
	    while(temp > 1){
	      char *command = which(args[temp-1], pathlist);
	      printf("%s",command);
	      free(command);
	      free(args[temp-1]);
	      temp--;
	    }
	    free(args[0]);
	    free(args);
	  }
	}
	/* WHICH */
	/* no mem leaks, works as intended*/
	else if(strcmp(args[0], "which")==0){
	  if(argsct==1){
	    printf("which: too few arguments\n");
	    free(args[0]);
	    free(args);
	  }
	  else{
	    printf("Executing built-in which:\n");
	    int temp = argsct;
	    while(temp > 1){
	      char *command = which(args[temp-1], pathlist);
	      /*if(command){
		printf("%s", command);
	      }
	      else{
		printf("%s: command not found\n", command);
	      }*/
	      printf("%s",command);
	      free(command);
	      free(args[temp-1]);
	      temp--;
	    }
	    free(args[0]);
	    free(args);
	  }
	}
	/* CD */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "cd")==0){
	  if(argsct > 2){
	    printf("cd: too many arguments\n");
	  }
	  else if(argsct==1) {
	    if(chdir(homedir) == 0){
	      printf("Executing built-in cd:\n");
	      chdir(homedir);
	      free(owd);
	      owd = pwd;
	      pwd = getcwd(NULL, PATH_MAX+1);
	    }
	    else{
	      printf("cd: invalid directory\n");
	    }
	  }
	  else if(strcmp(args[1], "-")==0) {
	    if(chdir(owd)==0){
	      printf("Executing built-in cd:\n");
	      chdir(owd);
	      free(owd);
	      owd = pwd;
	      pwd = getcwd(NULL, PATH_MAX+1);
	    }
	    else {
	      printf("cd: invalid directory\n");
	    }
	  }
	  else{
	    if(chdir(args[1])==0){
	      printf("Executing built-in cd:\n");
	      chdir(args[1]);
	      free(owd);
	      owd=pwd;
	      pwd=getcwd(NULL, PATH_MAX+1);
	    }
	    else{
	      printf("cd: invalid directory\n");
	    }
	  }
	  int temp = argsct;
	  while(temp>0){
	    free(args[temp-1]);
	    temp--;
	  }
	  free(args);
	}
	/* PWD */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "pwd")==0){
	  printf("Executing built-in pwd:\n");
	  printf("[%s]\n", pwd);
	  free(args[0]);
	  free(args);
	}
	/* LIST */
	/* no mem leaks, works as intendend. Question about part with args */
	else if(strcmp(args[0], "list")==0){
	  printf("Executing built-in list:\n");
	  if(argsct==1){
	    list(pwd);
	    free(args[0]);
	    free(args);
	  }
	  else{
	    int temp = argsct;
	    while(temp >1){
	      list(args[temp-1]);
	      free(args[temp-1]);
	      temp--;
	    }
	    free(args[0]);
	    free(args);
	  }
	}
	/* PID */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "pid")==0){
	  printf("Executing built-in pid:\n");
	  printf("pid of shell: %d\n", getpid());
	  free(args[0]);
	  free(args);
	}
	/* KILL */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "kill")==0){
	  if(argsct==1){
	    printf("kill: must have at least one argument\n");
	    free(args[0]);
	    free(args);
	  }
	  else if(argsct > 3){
	    printf("kill: too many arguments\n");
	    int temp = argsct;
	    while(temp > 0){
	      free(args[temp-1]);
	      temp--;
	    }
	    free(args);
	  }
	  else if(argsct == 2){
	    printf("Executing built-in kill:\n");
	    pid_t apid = atoi(args[1]);
	    kill(apid, SIGTERM);
	    free(args[1]);
	    free(args[0]);
	    free(args);
	  }
	  else{
	    printf("Executing built-in kill:\n");
	    pid_t apid = atoi(args[2]);
	    int sig = -1 * atoi(args[1]);
	    printf("%d\n", sig);
	    kill(apid, sig);
	    free(args[2]);
	    free(args[1]);
	    free(args[0]);
	    free(args);
	  }
	}
	
	/* PROMPT */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "prompt")==0){
	  if(argsct==1){
	    printf("Executing built-in prompt:\n"); 
	    printf("input prompt prefix: ");
	    fgets(prompt, BUFFER, stdin);
	    int plen = strlen(prompt);
	    if(prompt[plen-1]=='\n')
	      prompt[plen-1]='\0';
	  }
	  else if(argsct >2){
	    printf("prompt: too many arguments\n");
	    /*free(args[2]);
	      free(args[1]);*/
	    int i = argsct;
	    while(i>1){
	      free(args[i-1]);
	      i--;
	    }
	  }
	  else{
	    printf("Executing built-in prompt:\n"); 
	    strcpy(prompt, args[1]);
	    free(args[1]);
	  }
	  free(args[0]);
	  free(args);
	}
	/* PRINTENV */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "printenv")==0){
	  if(argsct==1){
	    printf("Executing built-in printenv:\n");
	    int j;
	    for(j = 0; envp[j] != NULL; j++){
	      printf("%s\n", envp[j]);
	    }
	  }
	  else if(argsct==2){
	    if(getenv(args[1])){
	      printf("Executing built-in printenv:\n");
	      char *p = getenv(args[1]);
	      printf("%s\n", p);
	    }
	    else{
	      printf("printenv: env not found\n");
	    }
	  }
	  else{
	    printf("printenv: too many arguments\n");
	  }
	  int temp = argsct;
	  while(temp>0){
	    free(args[temp-1]);
	    temp--;
	  }
	  free(args);
	}
	/* SETENV */
	/* no mem leaks, works as intended */
	else if(strcmp(args[0], "setenv")==0){
	  if(argsct==1){
	    printf("Executing built-in setenv:\n");
	    int k;
	    for(k = 0; envp[k] != NULL; k++){
	      printf("%s\n", envp[k]);
	    }
	    
	  }
	  else if(argsct==2){
	    printf("Executing built-in setenv:\n");
	    setenv(args[1],"/",1);
	    if(strcmp(args[1], "HOME")==0){
	      homedir=getenv("HOME");
	    }
	    else if(strcmp(args[1],"PATH")==0){
	      free(pathlist->element);
	      freePathList(pathlist);
	      pathlist = get_path();
	    }
	  }
	  else if(argsct==3){
	    printf("Executing built-in setenv:\n");
	    setenv(args[1],args[2],1);
	    if(strcmp(args[1], "HOME")==0){
	      homedir=getenv("HOME");
	    }
	    else if(strcmp(args[1], "PATH")==0){
	      freePathList(pathlist);
	      pathlist = get_path();
	    }
	  }
	  else{
	    printf("setenv: too many arguments\n");
	  }
	  int temp = argsct;
	  while(temp>0){
	    free(args[temp-1]);
	    temp--;
	  }
	  free(args);
	}

	else if(strchr(arg, '*') !=NULL || strchr(arg, '?') !=NULL){
	  wordexp_t p;
	  char **w;
	  int position;
	  wordexp(arg, &p, 0);
	  w = p.we_wordv;
	  for(position = argsct; position < p.we_wordc; position++){
	    printf("%s\n", w[position]);
	  }
	  wordfree(&p);
	  int temp = argsct;
	  while(temp>0){
	    free(args[temp-1]);
	    temp--;
	  }
	  free(args);
	}
	
	/* else  program to exec */
	/* no mem leaks, works as intended */
	else
	  {
	    char **newArgs = malloc((argsct+1) * (sizeof(char*)));
	    int j = 0;
	    while(j <=argsct){
	      if(j==argsct){
		newArgs[j] = '\0';
	      }
	      else {
		newArgs[j]=args[j];
	      }
	      j++;
	    }
	    if(args[0][0]=='/' || args[0][0] == '.'){
	      char *cmd;
	      cmd = (char *) malloc((strlen(args[0])+1)*sizeof(char));
	      strcpy(cmd, args[0]);
	      pid_t apid;
	      pid = fork();
	      if(pid == 0){
		execve(args[0], newArgs, envp);
	      }
	      else if(pid != 0){
		waitpid(pid, NULL, 0);
	      }
	      else{
		printf("%s: command not found\n", cmd);
	      }
	      free(cmd);
	    }
	    else{
	      char *abs = which(args[0], pathlist);
	      int len = strlen(abs);
	      abs[len-1]= '\0';
	      
	      if(abs == NULL){
		printf("%s: command not found\n", args[0]);
	      }
	      else if(access(abs, X_OK) == 0){
		printf("Executing %s:\n",args[0]);
		pid_t apid = fork();
		if(apid ==0){ //child
		  execve(abs, newArgs, envp);
		}
		else{//parent
		  waitpid(apid,NULL,0);
		}
	      }
	      else{
		printf("cannot acccess file\n");
	      }
	    
	      free(abs);
	      //free(newArgs);
	    }
	    int temp = argsct;
	    while(temp > 0){
	      free(args[temp-1]);
	      temp--;
	    }
	    free(newArgs);
	    free(args);
	  }
      }
      else{ //user hits only enter
	free(args);
      }
    }
  free(pathlist->element);
  freePathList(pathlist);
  free(commandline);
  free(prompt);
  return 0;
} /* sh() */

char **stringToArray(char *s, char **args, int *argCount){
  char buffer[BUFFER];
  char delim[2] = " ";
  strcpy(buffer, s);
  char *token = strtok(buffer, " ");

  /*determine length of s*/
  int count = 0;
  while(strtok(NULL, delim)){
    count++;
  }
  args = malloc((count+1) * sizeof(char*));
  args[count]=0;

  /*buffer was modified through strtok, so redefine*/
  strcpy(buffer, s);
  count = 0; //reset count
  token = strtok(buffer, delim);
  while(token != NULL){
    args[count] = (char*)malloc((strlen(token)+1) * sizeof(char*));
    strcpy(args[count],token);
    count++;
    *argCount= count;
    token = strtok(NULL, delim);
  }
  return args;
}

char *which(char *command, struct pathelement *pathlist )
{
  /* loop through pathlist until finding command and return it.  Return
     NULL when not found. */
  char *findcmd = malloc(200*sizeof(char));
  int cmdFound = 0; //0 if not found, 1 if found
  while(pathlist){
    sprintf(findcmd, "%s/%s", pathlist->element, command);
    if(access(findcmd, X_OK)==0){ //found
      cmdFound=1;
      sprintf(findcmd, "%s/%s\n", pathlist->element, command);
      break;
    }
    pathlist=pathlist->next;
  }
  if(cmdFound==0)
    sprintf(findcmd, "%s: command not found\n", command);
  return findcmd;
  
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  char* findcmd = malloc(200*sizeof(char));
  char* test = malloc(600*sizeof(char));
  int cmdFound = 0; //0 if not found, 1 if found
  int builtin = 0;

  if(strcmp(command, "exit")==0 ||
     strcmp(command,"which")==0 ||
     strcmp(command, "where")==0 ||
     strcmp(command, "cd")==0 ||
     strcmp(command, "pwd")==0 ||
     strcmp(command, "list")==0 ||
     strcmp(command, "pid")==0 ||
     strcmp(command, "kill")==0 ||
     strcmp(command, "prompt")==0 ||
     strcmp(command, "printenv")==0 ||
     strcmp(command, "setenv")==0){
    builtin=1;
    sprintf(findcmd, "%s: built-in command\n", command);
  }

  while(pathlist){
    sprintf(test, "%s/%s", pathlist->element, command);
    if(access(test, X_OK)==0){
      char *add = malloc(200*sizeof(char));
      sprintf(add, "%s%s\n", findcmd, test);
      sprintf(findcmd, "%s", add);
      cmdFound = 1;
      free(add);
    }
    pathlist = pathlist->next;
  }
  free(test);
  if(builtin==0 && cmdFound == 0){
    sprintf(findcmd, "%s: command not found\n", command);
    return findcmd;
  }
  return findcmd;
	
} /* where() */


void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
     the directory passed */
  DIR *adir = opendir(dir);
  struct dirent *file;

  if(adir != NULL){
    file=readdir(adir);
    printf("%s:\n", dir);
    while(file){
      printf("\t%s\n",file->d_name);
      file=readdir(adir);
    }
  }
  else{
    printf("list: cannot access %s: no such file or directory\n", dir);
  }
  free(adir);
} /* list() */

void freePathList(struct pathelement *pathlist){
  struct pathelement *temp;
  while(pathlist){
    temp = pathlist;
    pathlist = pathlist->next;
    free(temp);
  }
  free(pathlist);
}
