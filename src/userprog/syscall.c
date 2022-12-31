#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // thread_exit ();

  int *data_pointer = f->esp;

  /*

  f->esp  -----------------------------
          |          SYS_CODE         |
          -----------------------------
          | arg[0] ptr -OR- int_value |
          -----------------------------
          | arg[1] ptr -OR- int_value |
          -----------------------------
          | arg[2] ptr -OR- int_value |
          -----------------------------
          
  */

  int systemCall = *(int *)f->esp;

  switch(systemCall){
    case SYS_HALT:
      shutdown_power_off();
      break;
    
    case SYS_EXIT:
      thread_current()->parent->ex = true;
      thread_exit();
      break;
    
    case SYS_WRITE:
    {
      int fd = *(data_pointer + 1);
      void *buffer = (void *)(*(data_pointer + 2));
      unsigned size = *(data_pointer + 3);
      putbuf(buffer, size);
    }
  }
}
