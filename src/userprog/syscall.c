#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "lib/string.h"

static void syscall_handler (struct intr_frame *);
static void access_user_mem (const void *);

/* Standard input and output fd values respectively. */
const int STDIN_FILENUM = 0;
const int STDOUT_FILENUM = 1;

/* Max and min number value for system calls. */
const int SYSCALL_MAX = 19;
const int SYSCALL_MIN = 0;

/* System call functions. */
static void sys_halt(void);
static void sys_exit(int);
static pid_t sys_exec(const char *);
static int sys_wait(pid_t);
static bool sys_create(const char *, unsigned);
static bool sys_remove(const char *);
static int sys_open(const char *);
static int sys_filesize(int);
static int sys_read(int, void *, unsigned);
static int sys_write(int, const void *, unsigned);
static void sys_seek(int, unsigned);
static unsigned sys_tell(int);
static void sys_close(int);

static struct lock *filesys_lock;

/* Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
written, which may be less than size if some bytes could not be written.
Writing past end-of-file would normally extend the file, but file growth is not implemented
by the basic file system. The expected behavior is to write as many bytes as possible up to
end-of-file and return the actual number written, or 0 if no bytes could be written at all.
Fd 1 writes to the console. Your code to write to the console should write all of buffer in
one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is
reasonable to break up larger buffers.) Otherwise, lines of text output by different processes
may end up interleaved on the console, confusing both human readers and our grading scripts.*/

/* Questions :
- What can cause less bytes to be written?
- How do I check for this?
- Is it fine I return 0 if charBuffer is null

Writes the N characters in BUFFER to the console.
void
putbuf (const char *buffer, size_t n)
{
  acquire_console ();
  while (n-- > 0)
    putchar_have_lock (*buffer++);
  release_console ();
}
*/

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  filesys_lock = (struct lock *) malloc(sizeof(struct lock));
  lock_init(filesys_lock);
}

/* System calls handler that reroutes to correct system calls function depending on the value in
   the 32 bit word at the caller's stack pointer. */
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* Creates a pointer to 32 bit system call number from SP and dereferences it. */
  uint32_t *syscall_number_address = (uint32_t *) f->esp;
  access_user_mem(syscall_number_address);
  
  uint32_t syscall_number = *syscall_number_address;

  /* Parameter values grabbed for system calls. */
  uint32_t *arg1 = syscall_number_address + 1;
  uint32_t *arg2 = syscall_number_address + 2;
  uint32_t *arg3 = syscall_number_address + 3;

  /* Result to be stored within eax register if returning a value if sucess is true; */
  uint32_t result = 0;
  bool success;

  /* Checks if the system call value is within range. */
  if (syscall_number < SYSCALL_MIN || syscall_number > SYSCALL_MAX) {
    sys_exit(-1);
  }
  
  // TODO: Use tables to hold functions and args over switch statements and passing args.

  /* Switch statement to handle each system call depending on syscall_number value. */
  switch (syscall_number) {
    case SYS_HALT:
      sys_halt();
      break;

    case SYS_EXIT:
      syscall_args_check(syscall_number_address, 1);
      sys_exit(*arg1);
      break;

    case SYS_EXEC:
      syscall_args_check(syscall_number_address, 1);
      result = sys_exec((void *) *arg1);
      success = true;
      break;

    case SYS_WAIT:
      syscall_args_check(syscall_number_address, 1);
      result = sys_wait(*arg1);
      success = true;
      break;

    case SYS_CREATE:
      syscall_args_check(syscall_number_address, 2);
      result = sys_create((void *) *arg1, *arg2);
      success = true;
      break;

    case SYS_REMOVE:
      syscall_args_check(syscall_number_address, 1);
      result = sys_remove((void *) *arg1);
      success = true;
      break;

    case SYS_OPEN:
      syscall_args_check(syscall_number_address, 1);
      result = sys_open((void *) *arg1);
      success = true;
      break;

    case SYS_FILESIZE:
      syscall_args_check(syscall_number_address, 1);
      result = sys_filesize(*arg1);
      success = true;
      break;

    case SYS_READ:
      syscall_args_check(syscall_number_address, 3);
      result = sys_read(*arg1, (void *) *arg2, *arg3);
      success = true;
      break;

    case SYS_WRITE:
      syscall_args_check(syscall_number_address, 3);
      result = sys_write(*arg1, (void *) *arg2, *arg3);
      success = true;
      break;

    case SYS_SEEK:
      syscall_args_check(syscall_number_address, 2);
      sys_seek(*arg1, *arg2);
      break;

    case SYS_TELL:
      syscall_args_check(syscall_number_address, 1);
      result = sys_tell(*arg1);
      success = true;
      break;

    case SYS_CLOSE:
      syscall_args_check(syscall_number_address, 1);
      sys_close(*arg1);
      break;

    default:
      sys_exit(-1);
  }

  /* NULL tells us if result has been modified or not, and therefore whether the eax register 
     needs to be changed. */
  if (success) {
    f->eax = result;
  } else {
    return;
  }
}

/* Helper function for syscall_handler() to check if the arguments are valid memory addresses. */
void syscall_args_check(uint32_t *syscall_num, int args) {
  for (int i = 1; i <= args; i++) {
    access_user_mem (syscall_num + i);
  }
}

static void access_user_mem (const void *uaddr) {
  if (!is_user_vaddr(uaddr) || pagedir_get_page(thread_current()->pagedir, uaddr) == NULL) {
    sys_exit(-1);
  }
}

/* Terminates Pintos. */
static void sys_halt(void) {
  free(filesys_lock);
  shutdown_power_off();
}

static void sys_exit(int status) {
  thread_current()->manager->exit_status = status;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}

/* Runs the executable given. */
static pid_t sys_exec(const char *file) {
  access_user_mem(file);
  lock_acquire(filesys_lock);
  int result = process_execute(file);
  lock_release(filesys_lock);

  return result;
}

static int sys_wait(pid_t pid) {
  return process_wait(pid);
};

/* Creates a new file named by input with a specified size. */
static bool sys_create(const char *file, unsigned initial_size) {
  access_user_mem(file);
  lock_acquire(filesys_lock);
  bool result = filesys_create(file, initial_size);
  lock_release(filesys_lock);

  return result;
}

/* Deletes specified file if possible, returning a value depending on success or failure. */
static bool sys_remove(const char *file) {
  access_user_mem(file);
  lock_acquire(filesys_lock);
  bool result = filesys_remove(file);
  lock_release(filesys_lock);

  return result;
}

/* Opens the file specified. */
static int sys_open(const char *file) {
  access_user_mem(file);
  lock_acquire(filesys_lock);
  struct file *f = filesys_open(file);
  lock_release(filesys_lock);

  // Returns -1 if file does not exist.
  if (f == NULL) {
    return -1;
  }

  struct file_descriptor *fd_elem = malloc(sizeof(struct file_descriptor));

  // Returns -1 if memory could not be allocated for this file (descriptor).
  if (fd_elem == NULL) {
    file_close(f);
    return -1;
  }

  fd_elem->file = f;
  fd_elem->fd = allocate_fd();
  list_push_back(thread_current()->file_descriptors, &fd_elem->elem);
  return fd_elem->fd;
}

/* Returns filesize for a specified file descriptor. */
int sys_filesize(int fd) {
  struct file *file = fd_to_file(fd);

  if (file != NULL) {
    return file_length(file);
  } else {
    return -1;
  }
}

/* Reads "size" bytes from file open as fd into buffer. */
static int sys_read(int fd, void *buffer, unsigned size) {
  access_user_mem(buffer);
  // Handles reading from keyboard if fd value is 0 or greater than 1.
  if (fd == STDIN_FILENUM) {
    uint8_t *buf = (uint8_t *) buffer;

    for (unsigned i = 0; i < size; i++) {
      buf[i] = input_getc();
    }
    return size;
  } else if (fd > STDOUT_FILENUM) {
    struct file *file = fd_to_file(fd);

    if (file != NULL) {
      int bytes_read = file_read(file, buffer, size);
      return bytes_read;
    } else {
      return -1;
    }
  }
  // Handles invalid fd values.
  return -1;
}

/* Writes to file or console depending on fd value. */
static int sys_write(int fd, const void *buffer, unsigned size) {
  access_user_mem(buffer);
  // Handles standard output fd value to write to console or anything greater.
  if (fd == STDOUT_FILENUM) {
    const char *charBuffer = (const char *) buffer;

    // Checking buffer is not NULL, if NULL return 0 since no byte was written.
    if (charBuffer == NULL) {
      return 0;
    }

    /* Handles sizes that are greater than 400 bytes and breaks it down. */
    int sizeCount = size;
    while (sizeCount != 0) {
      if (sizeCount < 400) {
        putbuf(charBuffer, sizeCount);
        sizeCount = 0;
      } else {
        putbuf(charBuffer, 400);
        sizeCount -= 400;
        charBuffer += 400;
      }
    }

    return size;
  } else if (fd > STDOUT_FILENUM) {
  
    struct file *file = fd_to_file(fd);
    if (file != NULL) {
      int bytes_written = file_write(file, buffer, size);
      return bytes_written;
    }
  }

  // Handles invalid fd values.
  return -1;
}

/* Changes the next byte to be read in a file to "position". */
void sys_seek (int fd, unsigned position) {
  struct file *file = fd_to_file(fd);

  // If the fd value is not valid or the file is NULL, it exits.
  if (file != NULL && fd > STDOUT_FILENUM) {
    file_seek(file, position);
  } else {
    sys_exit(-1);
  }
}

/* Returns the poisiton of the next byte to be written for fd. */
static unsigned sys_tell(int fd) {
  struct file *file = fd_to_file(fd);

  // If the fd value is not valid or the file is NULL, it exits.
  if (file != NULL && fd > STDOUT_FILENUM) {
    return file_tell(file);
  } else {
    sys_exit(-1);
  }

  // Should not reach this line, for compiler warning supression.
  return -1;
}

/* Closes file descriptor fd. */
static void sys_close(int fd) {
  struct file *file = fd_to_file(fd);

  if (file != NULL && fd > STDOUT_FILENUM) {
    file_close(file);
    struct file_descriptor *file_descriptor = fd_to_file_descriptor(fd);
    if (file_descriptor != NULL) {
      list_remove(&file_descriptor->elem);
      free(file_descriptor);
      return;
    }
  }
}

/* Finds an available fd value by iterating through file_descriptors of thread. */
int allocate_fd(void) {
  int fd = 2; // Starts from 2 to avoid conflicts with standard input/output values.

  while (fd_to_file_descriptor(fd) != NULL) {
    fd++;
  }
  
  return fd;
}

/* Grabs the file associated with an fd value from some thread. */
struct file_descriptor *fd_to_file_descriptor(int fd) {
  struct list_elem *e;
  struct thread *current_thread = thread_current();

  for (e = list_begin(current_thread->file_descriptors); 
       e != list_end(current_thread->file_descriptors); 
       e = list_next(e)) {
    struct file_descriptor *fd_elem = list_entry(e, struct file_descriptor, elem);

    if (fd_elem->fd == fd) {
      return fd_elem;
    }
  }

  return NULL; // If the file descriptor is not found, return NULL.
}

struct file *fd_to_file(int fd) {
  struct file_descriptor *file_descriptor = fd_to_file_descriptor(fd);

  if (file_descriptor == NULL) {
    return NULL;
  } else {
    return file_descriptor->file;
  }
}
