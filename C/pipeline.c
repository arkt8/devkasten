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

inline static
void guardm(int cmp, const char *msg, ...) {
  if (!cmp) {
    va_list va;
    va_start(va, msg);
    vprintf(msg, va);
    exit(EXIT_FAILURE);
  }
}
//
//inline static
//void guard(int cmp) {
//  if (!cmp) {
//    printf("Error: %s\n",strerror(errno));
//    exit(EXIT_FAILURE);
//  }
//}

static inline
int mkpipe(int *io) {
  return socketpair(AF_UNIX, SOCK_STREAM, 0, io);
}

typedef int pipes[2];

static inline
int fcntl_addflag(int fd, int flag) {
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | flag);
}

static inline
pid_t run(char *const *cmd, pipes in, pipes out) {
  pid_t pid = fork();
  if (pid == 0) {
    guardm( dup2(in[0],  STDIN_FILENO)  == STDIN_FILENO,  "Error dup/in");
    guardm( dup2(out[1], STDOUT_FILENO) == STDOUT_FILENO, "Error dup/in");
    close(in[0]);
    close(in[1]);
    close(out[1]);
    close(out[0]);
    execvp(cmd[0], cmd);
    exit(EXIT_FAILURE);
  }
  close( in[0]);
  close(out[1]);
  return pid;
}

#define BUFSZ 1024

int main() {

  char msg[BUFSZ];
  pipes pipeA, pipeB, pipeC;
  mkpipe(pipeA);
  mkpipe(pipeB);
  mkpipe(pipeC);
  fcntl_addflag(pipeA[0], O_NONBLOCK);
//  fcntl_addflag(pipeA[1], O_NONBLOCK);
//  fcntl_addflag(pipeB[0], O_NONBLOCK);
//  fcntl_addflag(pipeB[1], O_NONBLOCK);
//  fcntl_addflag(pipeC[0], O_NONBLOCK);
  fcntl_addflag(pipeC[1], O_NONBLOCK);

  char *const cmd[2][4] = {
    { "tr", "a", "c", NULL },
    { "tr", "o", "u", NULL }
  };

  int pids[] = {
    run(cmd[0], pipeA, pipeB),
    run(cmd[1], pipeB, pipeC),
    -1000
  };
  int r;

  READ:
    if (!fork()) {
      printf("--- BEFORE READ --- (fd:%d)\n", pipeC[0]);
//      r = read(pipeB[0], msg, BUFSZ);

        char c;
        FILE *f = fdopen(pipeC[0], "r");
        while ((c = getc(f)) > 0) {
          printf("%c",c);
        }

      printf("read (%d):%s\n", c, strerror(errno));


      printf("--- AFTER READ ---\n");
      exit(EXIT_SUCCESS);
    }

  WRITE:
      printf("--- BEFORE WRITE ---\n");
      FILE *f = fdopen(pipeA[1], "w");
      strcpy(msg, "ola\n");
      r = fwrite(msg, strlen(msg), sizeof(char), f);
      fflush(f);
      //r = write(pipeA[1], msg, strlen(msg));
      strcpy(msg, "oba\n");
      r = fwrite(msg, strlen(msg), sizeof(char), f);
      fflush(f);
      printf("--- AFTER WRITE (%d of %lu) ---\n", r, strlen(msg));
      fclose(f);
      close(pipeA[1]);


  for(int p=0;; p++) {
    if (pids[p] == -1000) break;
    printf("Exiting at P[%d]\n", p);
    waitpid(pids[p], NULL, 0);
  }
  close(pipeA[1]);
  close(pipeC[0]);
  return 0;
}

/*
 * Copyright © 2023 - Thadeu A. C de Paula
 * Licensed under MIT License
 */
