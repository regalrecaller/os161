
#include <types.h>
#include <test.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>


static struct semaphore *first_sem = NULL;

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


void print_message(void *data1 , unsigned long data2)
{

    kprintf("\nThis is a newly spawned thread!\n\n");

//This enables the test to run to completion.
    V(first_sem);
}

int lab5test(int nargs, char **args)
{
    init_first_sem();
    kprintf("\nHello! Just printing to the console; nothing fancy. " );
    kprintf("\n\nHere is the command you entered: ");


    for (int i = 0; i < nargs; i++)
    {
        kprintf(args[i]);
        kprintf(" ");
    }


    kprintf("\n");
    thread_fork("print_message_thread", NULL, print_message, NULL, NULL);

//This helps numbers print in order.
    P(first_sem);
    return 0;



}

