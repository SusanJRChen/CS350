#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include "opt-A2.h"

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

#if OPT_A2
int sys_fork(struct trapframe * tf, pid_t * retval) {
  // Create a new process structure for the child process.
  struct proc * new_proc = proc_create_runprogram(curproc->p_name);
  if (new_proc == NULL) {
    // proc_create_runprogram failed, probably not enough memory
    return ENOMEM;
  }

  // Create and copy the address space (and data) from the parent to the child.
  struct addspace * new_as;
  as_copy(curproc_getas(), &new_as));
  if (new_as == NULL) {
    // as_copy failed, not enough memory
    return ENOMEM;
  }

  // Attach the newly created address space to the child process structure.
	spinlock_acquire(&new_proc->p_lock);
	new_proc->p_addrspace = new_as;
	spinlock_release(&new_proc->p_lock);

  // Assign a PID to the child process and create the parent/child relationship.
  new_proc->p_pid = abs(int(&new_as));
  new_proc->p_parent = curproc;
  array_add(curproc->p_children, new_proc, NULL);

  // Create a thread for a child process. The OS needs a safe way to pass the trapframe to the child thread.
  struct trapframe * new_tf;
  memcpy(&new_tf, &tf, sizeof(struct trapframe));
  KASSERT(new_tf != NULL);
  thread_fork(curproc->p_name, new_proc, enter_forked_process, new_tf, 1);

  // Set the return value to new process pid
  *retval = new_proc->p_pid;

  return 0;
}
#else
#endif /* OPT_A2 */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);

  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
  #if OPT_A2
    *retval = curproc->pid;
    // code you created or modified for ASST2 goes here
  #else
    *retval = 1;
    // old (pre-A2) version of the code goes here
  #endif /* OPT_A2 */
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */
  #if OPT_A2
    for (int i = 0; i < array_num(curproc->p_children); i++) {
      struct proc * cur = array_get(curproc->p_children, i);
      if (cur->p_pid == pid) {
        lock_acquire(cur->p_lock);
        while(!cur->p_has_exited) {
          cv_wait(cur->p_cv, cur->p_lock)
        }
        lock_release(cur->p_lock);
      }
      else if (i == array_num(cur->p_children) - 1) {
        // no children with pid exist
        *retval = -1;
        return ECHILD;
      }
    }
    // code you created or modified for ASST2 goes here
  #else
    if (options != 0) {
      return(EINVAL);
    }
    /* for now, just pretend the exitstatus is 0 */
    exitstatus = 0;
    result = copyout((void *)&exitstatus,status,sizeof(int));
    if (result) {
      return(result);
    }
    *retval = pid;
    // old (pre-A2) version of the code goes here
  #endif /* OPT_A2 */

  return(0);
}

