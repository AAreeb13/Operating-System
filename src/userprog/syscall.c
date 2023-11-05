#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

/* Max number value for system calls. */
const int SYSCALL_NUM = 19;

/* System call functions. */
static void sys_halt(void);
static void sys_exit(int status);
static pid_t sys_exec(const char *file);
static int sys_wait(pid_t pid);
static bool sys_create(const char *file, unsigned initial_size);
static bool sys_remove(const char *file);
static int sys_open(const char *file);
static int sys_filesize(int fd);
static int sys_read(int fd, void *buffer, unsigned size);
static int sys_write(int fd, const void *buffer, unsigned size);
static void sys_seek(int fd, unsigned position);
static unsigned sys_tell(int fd);
static void sys_close(int fd);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  syst();
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* Creates a pointer to 32 bit system call number from SP and dereferences it. */
  uint32_t *syscall_num = (uint32_t *) f->esp;
  uint32_t syscall_number = *syscall_num;

  /* Checks to see if pointer is a user virtual address. */
  if (!is_user_vaddr(syscall_num)) {
    sys_exit(-1);
  }

  /* Checks to see if pointer does not point into kernel memory. */
  if (is_kernel_vaddr(syscall_num)) {
    sys_exit(-1);
  }

  /* Verifies the pointer is not NULL. */
  if (syscall_num == NULL) {
    sys_exit(-1);
  }

  /* Checks if the system call value is within range. */
  if (syscall_num < 0 || syscall_num > SYSCALL_NUM) {
    sys_exit(-1);
  }

  // TODO: Check for first 3 arguments when necessary
  
  /* Switch statement to handle each system call depending on syscall_number value. */
  // Values currently NULL as each call has yet to be implemented (delete comment once done).
  switch (syscall_number) {
    case SYS_HALT:
      sys_halt();
      break;
    case SYS_EXIT:
      sys_exit(NULL);
      break;
    case SYS_EXEC:
      sys_exec(NULL);
      break;
    case SYS_WAIT:
      sys_wait(NULL);
      break;
    case SYS_CREATE:
      sys_create(NULL, NULL);
      break;
    case SYS_REMOVE:
      sys_remove(NULL);
      break;
    case SYS_OPEN:
      sys_open(NULL);
      break;
    case SYS_FILESIZE:
      sys_filesize(NULL);
      break;
    case SYS_READ:
      sys_read(NULL, NULL, NULL);
      break;
    case SYS_WRITE:
      sys_write(NULL, NULL, NULL);
      break;
    case SYS_SEEK:
      sys_seek(NULL, NULL);
      break;
    case SYS_TELL:
      sys_tell(NULL);
      break;
    case SYS_CLOSE:
      sys_close(NULL);
      break;  
    default:
      sys_exit(-1); // Terminates the current program, -1 to indicate errors as it is invalid.
  }

  // TODO: Store value into f->eax (if call returns value).
}
