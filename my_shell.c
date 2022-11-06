#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

char **history;
int cmd_num = 0;
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_echo(char **args);
int lsh_record(char **args);
int lsh_replay(char **args);
int lsh_mypid(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "echo",
    "record",
    "replay",
    "mypid"};

int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_echo,
    &lsh_record,
    &lsh_replay,
    &lsh_mypid};

int lsh_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

int lsh_cd(char **args)
{
  if (args[1] == NULL)
  { // if second argument not exist
    fprintf(stderr, "Error:Expected argument to \"cd\"\n");
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      perror("Error");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  printf("----------------------------------------------------------------\n");
  printf("my little shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");
  printf("1: help: show all build-in function info\n");
  printf("2: cd: change directory\n");
  printf("3: echo: echo the strings to standard output\n");
  printf("4: record: show last-16 cmds you typed in\n");
  printf("5: replay: re-execute the cmd showed in record\n");
  printf("6: mypaid: find and print process-ids\n");
  printf("7: exit: exit shell\n");
  printf("\n");
  printf("Use the man command for information on other programs.\n");
  printf("----------------------------------------------------------------\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

int lsh_echo(char **args)
{
  int i = 1;
  if (strcmp(args[1], "-n") == 0)
  {
    if (args[i + 1] != NULL)
    {
      printf("%s", args[i + 1]);
      i = i + 2;
      while (args[i] != NULL)
      {
        printf(" ");
        printf("%s", args[i]);
        i++;
      }
    }
  }
  else
  {
    if (args[i] != NULL)
    {
      printf("%s", args[i]);
      i = i + 1;
      while (args[i] != NULL)
      {
        printf(" ");
        printf("%s", args[i]);
        i++;
      }
    }
    printf("\n");
  }
  return 1;
}

int lsh_record(char **args)
{
  if (cmd_num == 16)
  {
    for (int i = 0; i < 16; i++)
    {
      printf("%d: %s\n", i + 1, history[i]);
    }
  }
  else
  {
    for (int i = 0; i < cmd_num; i++)
    { // cmd_num from 0 to count
      printf("%d: %s\n", i + 1, history[i]);
    }
  }
  return 1;
}

int lsh_replay(char **args)
{
  return 1;
}

int visitDir(const char *addr, const char *args) // check in "/proc/"" have this pid or not
{
  DIR *dir = opendir(addr);
  if (dir != NULL)
  {
    struct dirent *ent = NULL;
    while ((ent = readdir(dir)) != NULL)
    {
      if (strcmp(ent->d_name, args) == 0)
      {
        return 1;
      }
    }
    closedir(dir);
  }
}

void print_ppid(const char *args){
  FILE *file;
  char line[32];
  char path[32];
  char *p;
  //char* str1 = "/proc/";
  strcpy(path, "/proc/");
  strcat(path, args);
  strcat(path, "/status");
  file = fopen(path, "r");
  if (file == NULL){
    printf("failed to open ");// Failure to open /proc/
  }else{
    while (fgets(line, sizeof(line), file) != NULL){
      p = strstr(line, "PPid:");
      if(p != NULL){
        printf("%s", p+5); //length of ("PPid:") is 5
      }
    }
  }
  fclose(file);
}

void print_cpid(const char *args){//不會印出自己所在process那串的pid
  char path[64];
  char *p;
  strcpy(path, "/proc/");
  strcat(path, args);
  strcat(path, "/task/");
  strcat(path, args);
  strcat(path, "/children");
  DIR *dir = opendir(path);
  if (dir != NULL)
  {
    struct dirent *ent = NULL;
    while ((ent = readdir(dir)) != NULL)
    {
      if (strcmp(ent->d_name, args) != 0 && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
      {
        printf("%s\n", ent->d_name);
      }
    }
    closedir(dir);
  }
}

int lsh_mypid(char **args)
{
  if (strcmp(args[1], "-i") == 0)
  {
    printf("%d\n", getpid());
  }
  else if (strcmp(args[1], "-p") == 0)
  {
    if (visitDir("/proc", args[2]) == 1)
    {
      print_ppid(args[2]);
    }
    else
    {
      printf("mypid -p: process id not exist\n");
    }
  }
  else if (strcmp(args[1], "-c") == 0)
  {
    if (visitDir("/proc", args[2]) == 1)
    {
      print_cpid(args[2]);
    }
    else
    {
      printf("mypid -c: process id not exist\n");
    }
  }
  return 1;
}

int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("Error");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Error forking
    perror("Error");
  }
  else
  {
    // Parent process
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL)
  { // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0)
    { // if have
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1)
  {
    if (feof(stdin))
    {
      exit(EXIT_SUCCESS); // We received an EOF
    }
    else
    {
      perror("Error: getline\n");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens)
  {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  printf("=======================\n");
  printf("*Wellcome to my little shell: \n");
  printf("*Type \"help\" to see builtin funtions\n");
  printf("=======================\n");

  do
  {
    printf(">>> $ ");
    line = lsh_read_line();
    if (strstr(line, "replay") == NULL)
    { //////////////////////    record history
      char *line1 = malloc(sizeof(line));
      strcpy(line1, line);
      line1[strlen(line1) - 1] = NULL; // delete the "\n"
      if (cmd_num == 16)
      {
        for (int i = 0; i < 15; i++)
        {
          history[i] = history[i + 1];
        }
        history[15] = line1;
        args = lsh_split_line(line);
      }
      else
      {
        history[cmd_num] = line1;
        cmd_num++;
        args = lsh_split_line(line);
      }
    }
    else
    {
      args = lsh_split_line(line);
      if (atoi(args[1]) > 0 && atoi(args[1]) < cmd_num)
      { // legal argument
        char *line2 = malloc(16 * sizeof(char *));
        char *line3 = malloc(16 * sizeof(char *));
        strcpy(line2, history[(atoi(args[1]) - 1)]);
        strcpy(line3, history[(atoi(args[1]) - 1)]);
        if (cmd_num == 16)
        {
          for (int i = 0; i < 15; i++)
          {
            history[i] = history[i + 1];
          }
          history[15] = line3;
          args = lsh_split_line(line2);
        }
        else
        {
          history[cmd_num] = line3;
          cmd_num++;
          args = lsh_split_line(line2);
        }
      }
      else
      {
        printf("replay: wrong args\n");
      }
    }

    status = lsh_execute(args);
    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  // Load config files, if any.
  // Run command loop.
  history = malloc(128 * sizeof(char *));
  lsh_loop();

  // Perform any shutdown/cleanup.
  return EXIT_SUCCESS;
}
