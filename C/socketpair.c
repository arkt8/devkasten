/*
 * Process pipelining example using socketpair()
 */

#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

inline static
void assert(int cmp, const char *msg, ...) {
  if (!cmp) {
    va_list va;
    va_start(va, msg);
    vprintf(msg, va);
    exit(EXIT_FAILURE);
  }
}
inline static
void dieif(int cmp) {
  if (cmp) {
    printf("Error: %s\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
}

#define BUFSZ 1024

int main() {

  int pipe[2], res, pid;
  dieif (socketpair(AF_UNIX, SOCK_STREAM, 0, pipe) == -1);

  char *cmd[] = { "tr", "a", "c", NULL };
  char msg[BUFSZ];

  if ((pid = fork()) == 0) {
    assert(dup2(pipe[1], STDIN_FILENO) == STDIN_FILENO, "dup stdin error");
    assert(dup2(pipe[1], STDOUT_FILENO) == STDOUT_FILENO, "dup stdout error");

    execvp(cmd[0], cmd);
    return 0;
  }

  strcpy(msg, "ola\n");
  write(pipe[0], msg, strlen(msg));

  // Needed to send the EOF
  shutdown(pipe[0],SHUT_WR);

  res = read(pipe[0], msg, BUFSZ);
  msg[ (res > 0 ? res : 0) ] = '\0';
  printf("RES (%d): %s\n", res, msg);
  return 0;
}


/*
 * Copyright © 2023 - Thadeu A. C de Paula
 * Licensed under MIT License
 */
