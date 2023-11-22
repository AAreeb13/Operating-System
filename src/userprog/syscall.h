#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdint.h>
#include "lib/stddef.h"
#include "filesys/file.h"

/* Process identifier. */
typedef int pid_t;

void syscall_init (void);
void syscall_args_check(uint32_t *, int);

/* File descriptor helper functions. */
int allocate_fd(void);
struct file_descriptor *fd_to_file_descriptor(int);
struct file *fd_to_file(int);

#endif /* userprog/syscall.h */
