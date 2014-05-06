/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>


//////////////////////////Added this
#include <copyinout.h>
#include <thread.h>
#include <synch.h>
////////////////////////////////////


static struct semaphore *first_sem = NULL;



//////////////////////////Added this
static void init_first_sem()
{
    if (first_sem == NULL)
    {
        first_sem = sem_create("firstsem", 0);
        if (first_sem == NULL)
        {
            panic("Panic! Semaphore creation failed!\n");
        }
    }
}





/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */


//argv = character pointer pointer, command line arguments
//argc = total number of arguments between spaces.


int
runprogram(char ** args, unsigned long nargs)
{

//Allocating space
    struct addrspace *as;
    struct vnode *v;
    vaddr_t entrypoint, stackptr, argv;
    int result;

//////////////////////////Added this

//Size of the stack
    char kstack[1024];
//Refers to virtual address in kernel's address space, kernel's command line arguments take in the number of arguments
    vaddr_t kargv[nargs+1];
    char progname[128];

//unsigned integer of at least 16 bits, stackSize is of this type
    size_t stackSize;

    char * args2[nargs];

    for (unsigned int i = 0; i < nargs; i++)
    {
       args2[i] = args[i];
 }

//Copies the program name into the the first index
    strcpy(progname, args[0]);


//////////////////////////////////////


    /* Open the file. */


//Predefined buffer, everything is stored from userland
    result = vfs_open(progname, O_RDONLY, 0, &v);
    if (result)
    {
        return result;
    }

    /* We should be a new process. */
    KASSERT(curproc_getas() == NULL);
    /* Create a new address space. */
    as = as_create();
    if (as ==NULL)
    {
        vfs_close(v);
        return ENOMEM;
    }

    /* Switch to it and activate it. */
    curproc_setas(as);
    as_activate();

    /* Load the executable. */
    result = load_elf(v, &entrypoint);
    if (result)
    {
        /* p_addrspace will go away when curproc is destroyed */
        vfs_close(v);
        return result;
    }

    /* Done with the file now. */
    vfs_close(v);


    /* Define the user stack in the address space */
    result = as_define_stack(as, &stackptr);
    if (result)
    {
        /* p_addrspace will go away when curproc is destroyed */
        return result;
    }


//////////////////////////Added this
    init_first_sem();
    int padding = 0;
    int paddingCount;
    int length;

//Only way to get padding/spacing information when used with the kstack(takes a bianry value) We can't use % and - signs which are important with getting padding info
    stackSize = 0;
    for (unsigned int i = 0; i < nargs; i++)
    {
  strcpy(kstack+stackSize,args2[i]);


  length = strlen(kstack+stackSize)+1;


        kargv[i] = length;
        stackSize += length;

        if(paddingCount = stackSize%4)
        {
           memcpy(kstack+stackSize, &padding, paddingCount=4-paddingCount);


           stackSize += paddingCount;
            kargv[i] += paddingCount;
        }
    }

    kargv[nargs]=stackptr;
    for (int i = nargs-1; i >= 0; i--)
    {
        kargv[i] = kargv[i+1] - kargv[i];
    }
 argv = stackptr - (stackSize + (nargs+1)*4);


//Copies data from the virtual kernal-space address (src) into the user-sapce address (userdest).
//Takes kaddr, uaddr, count
   copyout(kstack, stackptr-stackSize, stackSize);

//Both addresses of the userland and kernal have to match
    copyout(kargv, argv, (nargs+1)*4);

//////////////////////////////////////





    /* Warp to user mode. */
    //enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,stackptr, entrypoint);


//////////////////////////Added this

    enter_new_process(nargs, argv, stackptr-stackSize, entrypoint);

//////////////////////////////////////

    /* enter_new_process does not return. */
    panic("enter_new_process returned\n");

    return EINVAL;

}



