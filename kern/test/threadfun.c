
#include <types.h>
#include <test.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>


static int counter;

static int threadNum; 
static struct semaphore *first_sem = NULL;
static struct lock *counter_lock = NULL;


static void init_first_sem(){
    if (first_sem == NULL){
        first_sem = sem_create("firstsem", 0);
        if (first_sem == NULL){
            panic("Panic! Semaphore creation failed!\n");
        }
    }
}


static void init_counter_lock(){
    if (counter_lock == NULL){
        counter_lock = lock_create("counter_lock");
        if (counter_lock == NULL){
            panic("Panic! There is no counter_lock!\n");
        }
    }
}


static void safeincrementcounter(void *data, unsigned long msg){
    (void)data;

    for (unsigned int i = 0; i < msg; i++){
        lock_acquire(counter_lock);
        counter++;
        lock_release(counter_lock);
    }

//This enables the test to run to completion.
    V(first_sem);
}


static void unsafeincrementcounter(void *data, unsigned long msg){
    (void)data;

    for (unsigned int i = 0; i < msg; i++){
        counter++;
    }

//This enables the test to run to completion.
    V(first_sem);
}


int unsafethreadcounter(int nargs, char** args){

    int k, result, numincrements;
    threadNum = 0;
    counter = 0;

    if (nargs < 3){
        kprintf("This command requires 2 arguments. Example: stc <int> <int>\n");
        return 0;
    }

    if (atoi(args[1]) > 20){
        kprintf("\nYou must use less than or equal to 20 threads!\n\n");
        return 0;
    }

    k = atoi(args[1]), numincrements = atoi(args[2]);
    init_first_sem();
    init_counter_lock();

    kprintf("\nBeginning Unsafe Thread Test...\n");

    for (int i = 0; i < k; i++){
        result = thread_fork("Running Unsafe Thread Counter", NULL,unsafeincrementcounter, NULL, numincrements);

        if (result){
            panic("Panic! thread_fork failed! %s)\n", strerror(result));
        }
    }

    for (int i = 0; i < k; i++){
        threadNum++;
//This helps numbers print in order.
        P(first_sem);
    }
    kprintf("\nTotal threads: %d", threadNum);
    kprintf("\nTotal counter value should be: %d", k*numincrements);
	 kprintf("\nFinal counter value: %d", counter);
    kprintf("\nUnsafe Thread Test finished!\n");
    return 0;
}


int safethreadcounter(int nargs, char** args){

    int k, result, numincrements;
    threadNum = 0;
    counter = 0;

    if (nargs < 3){
        kprintf("This command requires 2 arguments. Example: stc <int> <int>\n");
        return 0;
    }
    if (atoi(args[1]) > 20){
        kprintf("\nYou must use less than or equal to 20 threads!\n\n");
        return 0;
    }

    k = atoi(args[1]), numincrements = atoi(args[2]);
    init_first_sem();
    init_counter_lock();

    kprintf("\nBeginning Safe Thread Test...\n");

    for (int i = 0; i < k; i++){
        result = thread_fork("Running Safe Thread Counter", NULL, safeincrementcounter, NULL, numincrements);

        if (result){
            panic("Panic! thread_fork failed! %s)\n", strerror(result));
        }
    }

    for (int i = 0; i < k; i++){
        threadNum++;
//This helps numbers print in order.
        P(first_sem);
    }
    kprintf("\nTotal threads: %d", threadNum);
    kprintf("\nTotal counter value should be: %d", k*numincrements);
	 kprintf("\nFinal counter value: %d", counter);    
    kprintf("\nSafe Thread Test finished!\n");
    return 0;
}


static void printnum(void* data, unsigned long number){
    (void)data;

//%ld used for "long". "long" must be used as a compatible pointer type for thread_fork(...)
    kprintf("Thread number: %ld\n",number+1);

//This enables the test to run to completion.
    V(first_sem);
}


int threadfun(int nargs, char** args){
    threadNum = 0;
    int k, result;

    if (nargs < 2){
        kprintf("This command requires 1 argument. Example: tft <int>\n");
        return 0;
    }

    if (atoi(args[1]) > 20){
        kprintf("\nYou must use less than or equal to 20 threads!\n\n");
        return 0;
    }

    k = atoi(args[1]);
    init_first_sem();

    kprintf("\nBeginning Thread Fun Test...\n\n");

    for (int i = 0; i < k; i++){
        result = thread_fork("Running Safe Thread Counter", NULL, printnum, NULL, i);

        if (result){
            panic("Panic! thread_fork failed! %s)\n", strerror(result));
        }
    }

    for (int i = 0; i < k; i++){
//This helps numbers print in order.
        P(first_sem);
    }

    kprintf("\nThread Fun Test finished!\n\n");
    return 0;
}
