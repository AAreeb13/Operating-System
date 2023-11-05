#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
static void access_user_mem (uint32_t *, const void *);
static void exit(void);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}

static void access_user_mem (const void *uaddr) {
  if (uaddr != NULL && is_user_vaddr (uaddr)) {
    pagedir_get_page(thread_current()->pagedir, uaddr);
  } else {
    exit();
  }
}
