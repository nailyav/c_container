#ifndef MAIN_H_
#define MAIN_H_
#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#endif


void isolate_mnt() {
  system("dd if=/dev/zero of=loopbackfile.img bs=100M count=10");
  system("losetup -fP loopbackfile.img");
  system("mkfs.ext4 ./loopbackfile.img");
  system("mkdir tmp_mnt");
  system("mount -o loop /dev/loop0 tmp_mnt");

  chdir("tmp_mnt/");
}

int create_container(void)
{
    char *cmd[] = { "/bin/bash", NULL };

    chroot("/home/rootfs");
    chdir("/");
    mount("proc", "proc", "proc", 0, "");
    execv("/bin/bash", cmd);
    perror("exec");
    exit(EXIT_FAILURE);
}

int child(void *args) {
  clearenv();
  isolate_mnt();
  create_container();
  
  return EXIT_SUCCESS;  
}

char* alloc_stack() {
  const int stack_size = 131072;
  char stack[stack_size];
  return stack + stack_size;
}

int main() {
    int namespaces = CLONE_NEWNS |
        CLONE_NEWUTS |
        CLONE_NEWPID |
        CLONE_NEWIPC |
        CLONE_NEWNET |
        SIGCHLD;
    pid_t container;

    container = clone(child, malloc(4096) + 4096, SIGCHLD | namespaces, NULL);
    if (container == -1) {
        perror("clone");
        exit(1);
    }
    waitpid(container, NULL, 0);
    return EXIT_SUCCESS;
}