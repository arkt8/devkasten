#define _POSIX_SOURCE 199309L
#define _GNU_SOURCE
#include <stdio.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>


static inline
void twrite(int fd, char *s, size_t slen) {
  FILE *f = fdopen(fd, "w");
  fwrite(s, slen, sizeof(*s), f);
  fflush(f);
  fclose(f);
}

static inline
int fcntl_addflag(int fd, int flag) {
  int f = fcntl(fd, F_GETFL);
  if (f == -1) return -1;
  f |= flag;
  return fcntl(fd, F_SETFL, f);
}

static inline
int mkpipe(int *io) {
  return socketpair(AF_UNIX, SOCK_STREAM, 0, io);
}


static inline
pid_t run(char *const *cmd, int fdin, int fdout, int fderr) {
  pid_t pid = fork();
  if (pid != 0) return pid;
  if ( dup2(fdin,  STDIN_FILENO)  == STDIN_FILENO
    && dup2(fdout, STDOUT_FILENO) == STDOUT_FILENO
    && dup2(fderr, STDERR_FILENO) == STDERR_FILENO
    && execvp(cmd[0], cmd) != -1 )
      exit(EXIT_SUCCESS);
  exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main() {

  int pipe[3], res, readpid;
  char msg[BUFSZ];
  mkpipe(pipe);
  pipe[2] = open("/dev/null", O_RDWR);

  if (!(readpid = fork())) {
    fcntl_addflag(pipe[1], O_NONBLOCK);
    char c;
    FILE *f = fdopen(pipe[0], "r");
    while ((c = getc(f))) {
      printf("%c",c);
    }
    exit(EXIT_SUCCESS);
  }

  char *const cmd[2][4] = {
    { "tr", "a", "c", NULL },
    { "tr", "c", "C", NULL }
  };


  run(cmd[0], pipe[1], pipe[1], pipe[0]);
  strcpy(msg, "ola\n");
  FILE *f = fdopen(pipe[0], "w");
  fwrite(msg, strlen(msg), sizeof(char), f);
  fflush(f);
  printf("Pos write\n");

  strcpy(msg, "bola\n");
  FILE *h = fdopen(pipe[0], "w");
  fwrite(msg, strlen(msg), sizeof(char), h);
  fflush(h);

  if (!fork()) {
    close(pipe[1]);
    strcpy(msg, "boa\n");
    twrite(pipe[0], msg, strlen(msg));
    twrite(pipe[0], msg, strlen(msg));
    close(pipe[0]);
    exit(EXIT_SUCCESS);
  }
  waitpid(readpid, NULL, 0);
  return 0;
}

/*
 * Copyright © 2023 - Thadeu A. C de Paula
 * Licensed under MIT License
 */
