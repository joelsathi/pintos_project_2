#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "list.h"

static void syscall_handler (struct intr_frame *);
static struct lock file_lock;

// Helper functions
bool isValidPointer(const void * pointer);

struct file_details{
  int fd;
  struct file *cur_file;
  struct list_elem elem;
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
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

  uint32_t *argv0 = data_pointer + 1;
  uint32_t *argv1 = data_pointer + 2;
  uint32_t *argv2 = data_pointer + 3;

  switch(systemCall){
    case SYS_HALT:
      shutdown_power_off();
      break;
    
    case SYS_EXIT:
      thread_current()->parent->ex = true;
      thread_current()->exit_code = *argv0;
      thread_exit();
      break;
    
    case SYS_EXEC:
      isValidPointer(argv0);
      break;
    
    case SYS_WAIT:
      break;
    
    case SYS_CREATE:
    {
      isValidPointer(argv0);
      isValidPointer(*argv1);

      lock_acquire(&file_lock);

      // File name and the size of the file
      f->eax = filesys_create((char *)*argv0, *argv1);

      lock_release(&file_lock);
      break;
    }
    
    case SYS_REMOVE:
      break;
    
    case SYS_OPEN:
      break;
    
    case SYS_FILESIZE:
      break;

    case SYS_READ:
      break;
    
    case SYS_WRITE:
    {
      // The buffer and the size
      putbuf((void *)*argv1, *argv2);
    }

    case SYS_SEEK:
      
      break;
    
    case SYS_TELL:
      isValidPointer(argv0);
      lock_acquire(&file_lock);
      
      struct list_elem *e;
      struct list files = thread_current()->files;

      int fd = *argv0;

      for (e = list_begin (&files); e != list_end (&files); e = list_next (e)){
        struct file_details *currentFileDet = list_entry (e, struct file_details, elem);
        if (currentFileDet->fd == fd){
          f->eax = file_tell(currentFileDet->cur_file);
          break;
        }
      }
      
      lock_release(&file_lock);
      break;
    
    case SYS_CLOSE:
      {
        isValidPointer(argv0);
        lock_acquire(&file_lock);

        struct list_elem *e;
        struct list files = thread_current()->files;

        int fd = *argv0;

        for (e = list_begin (&files); e != list_end (&files); e = list_next (e))
          {
            struct file_details *currentFileDet = list_entry (e, struct file_details, elem);
            if (currentFileDet->fd == fd){
              file_close(currentFileDet->cur_file);
              list_remove(e);
            }
          }

        lock_release(&file_lock);
      }
      break;
    
    default:
      printf("Interrupt not defined!!!\n");
  }
}

bool isValidPointer(const void * pointer){
  // Pointer is NULL
  if(pointer == NULL){
    return false;
  }

  // Check if it is a user (address in the user section not in kernel) virtual address
  if (!is_user_vaddr(pointer)){
    return false;
  }

  // Returns the kernel virtual address corresponding to that physical address, 
  // or a null pointer if UADDR is unmapped.
  // i.e pointer should be mapped to a virtual address
  if (pagedir_get_page(thread_current()->pagedir, pointer)){
    return false;
  }

  return true;
}
