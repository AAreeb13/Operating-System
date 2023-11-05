#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

/* Max and min number value for system calls. */
const int SYSCALL_MAX = 19;
const int SYSCALL_MIN = 0;

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

/* System calls handler that reroutes to correct system calls function depending on the value in
   the 32 bit word at the caller's stack pointer. */
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* Creates a pointer to 32 bit system call number from SP and dereferences it. */
  uint32_t *syscall_number_address = (uint32_t *) f->esp;
  uint32_t syscall_number = *syscall_number_address;

  /* Parameter values grabbed for system calls. */
  uint32_t *parameter_1 = syscall_number_address + 1;
  uint32_t *parameter_2 = syscall_number_address + 2;
  uint32_t *parameter_3 = syscall_number_address + 3;

  /* Result to be stored within eax register if returning a value. */
  uint32_t result = NULL; 

  /* Checks to see if pointer is a valid user virtual address. */
  if (!is_user_vaddr(syscall_number_address)) {
    sys_exit(-1);
  }

  /* Checks to see if pointer does not point into kernel memory. */
  if (is_kernel_vaddr(syscall_number_address)) {
    sys_exit(-1);
  }

  /* Verifies the pointer is not NULL. */
  if (syscall_number_address == NULL) {
    sys_exit(-1);
  }

  /* Checks if the system call value is within range. */
  if (syscall_number_address < SYSCALL_MIN || syscall_number_address > SYSCALL_MAX) {
    sys_exit(-1);
  }
  
  /* Switch statement to handle each system call depending on syscall_number value. */
  switch (syscall_number) {
    case SYS_HALT:
      sys_halt();
      break;

    case SYS_EXIT:
      syscall_args_check(syscall_number_address, 1);
      sys_exit(parameter_1);
      break;

    case SYS_EXEC:
      syscall_args_check(syscall_number_address, 1);
      result = sys_exec(parameter_1);
      break;

    case SYS_WAIT:
      syscall_args_check(syscall_number_address, 1);
      result = sys_wait(parameter_1);
      break;

    case SYS_CREATE:
      syscall_args_check(syscall_number_address, 2);
      result = sys_create(parameter_1, parameter_2);
      break;

    case SYS_REMOVE:
      syscall_args_check(syscall_number_address, 1);
      result = sys_remove(parameter_1);
      break;

    case SYS_OPEN:
      syscall_args_check(syscall_number_address, 1);
      result = sys_open(parameter_1);
      break;

    case SYS_FILESIZE:
      syscall_args_check(syscall_number_address, 1);
      result = sys_filesize(parameter_1);
      break;

    case SYS_READ:
      syscall_args_check(syscall_number_address, 3);
      result = sys_read(parameter_1, parameter_2, parameter_3);
      break;

    case SYS_WRITE:
      syscall_args_check(syscall_number_address, 3);
      result = sys_write(parameter_1, parameter_2, parameter_3);
      break;

    case SYS_SEEK:
      syscall_args_check(syscall_number_address, 2);
      sys_seek(parameter_1, parameter_2);
      break;

    case SYS_TELL:
      syscall_args_check(syscall_number_address, 1);
      result = sys_tell(parameter_1);
      break;

    case SYS_CLOSE:
      syscall_args_check(syscall_number_address, 1);
      sys_close(parameter_1);
      break;

    default:
      sys_exit(-1);
  }

  /* NULL tells us if result has been modified or not, and therefore whether the eax register 
     needs to be changed. */
  if (result != NULL) {
    f->eax = result;
  }

  return;
}

/* Helper function for syscall_handler() to check if the arguments are valid memory addresses. */
void syscall_args_check(uint32_t *syscall_num, int args) {
  switch (args) {
    case 1:
      if (!(is_user_vaddr(syscall_num + 1))) {
        sys_exit(-1);
      }
    break;

    case 2:
      if (!((is_user_vaddr(syscall_num + 1)) && is_user_vaddr(syscall_num + 2))) {
        sys_exit(-1);
      }
      break;
    
    default:
      if (!((is_user_vaddr(syscall_num + 1)) && is_user_vaddr(syscall_num + 2) && 
          is_user_vaddr(syscall_num + 3))) {
        sys_exit(-1);
      }
  }
}
