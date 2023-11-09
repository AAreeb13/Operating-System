#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/stddef.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

/* Process identifier. */
typedef int pid_t;

void syscall_init (void);

/* File descriptor helper functions. */
int allocate_fd(struct thread *);
struct file *fd_to_file(int, struct thread *);

#endif /* userprog/syscall.h */
