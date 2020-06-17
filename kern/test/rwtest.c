/*
 * All the contents of this file are overwritten during automated
 * testing. Please consider this before changing anything in this file.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <kern/test161.h>
#include <spinlock.h>

#define CREATELOOPS 8
#define RWLOOPS 10

/*
 * Use these stubs to test your reader-writer locks.
 */

struct spinlock status_lock;
static struct rwlock *testrw = NULL;
static struct rwlock *donerw = NULL;
static struct semaphore *donesem = NULL;
static struct semaphore *testsem = NULL;
static bool test_status = TEST161_FAIL;

/*static
bool
failif(bool condition) {
	if (condition) {
		spinlock_acquire(&status_lock);
		test_status = TEST161_FAIL;
		spinlock_release(&status_lock);
	}
	return condition;
	}*/



int rwtest(int nargs, char **args) {
	(void)nargs;
	(void)args;
	
	struct spinlock status_lock;

	for(int i = 0; i < CREATELOOPS; i++) {
		kprintf(".");
		testrw = rwlock_create("testrwlock");
		if(testrw == NULL) {
			panic("rw1: rwlock_create failed\n");
		}
		donerw = rwlock_create("donerwlock");
		if(donerw == NULL) {
			panic("rw1: rwlock_create failed\n");
		}
		if (i != CREATELOOPS - 1) {
			rwlock_destroy(testrw);
			rwlock_destroy(donerw);
		}
	}
	spinlock_init(&status_lock);
	test_status = TEST161_SUCCESS;

	kprintf_n("Two writers are acquiring the lock\nIf this hangs, it's broken :");
	rwlock_acquire_write(testrw);
       	rwlock_acquire_write(donerw);
	kprintf_n("Ok\n");
	kprintf_t(".");
	kprintf_n("First writer is releasing the lock, second should release after\n If this hangs, it's broken:");
	rwlock_release_write(testrw);
       	rwlock_release_write(donerw);

	rwlock_destroy(testrw);
	rwlock_destroy(donerw);
	testrw = donerw = NULL;

	kprintf_t("\n");
	success(test_status, SECRET, "rwt1");
	
	return 0;
}
static
void
rwtestthread( void *junk, unsigned long i)
{
	(void)junk;

	if(RWLOOPS-i < RWLOOPS/5){
		kprintf_n("Thread #%ld requesting the write lock\n",i);
		rwlock_acquire_write(testrw);
		kprintf_n("Thread #%ld granted the write lock!\n",i);
       	        kprintf_n("Ok\n");
	        kprintf_t(".");
		kprintf_n("Thread #%ld is releasing the write lock\n",i);
		rwlock_release_write(testrw);
	        kprintf_n("Thread #%ld released the write lock.\n",i);
	} else {
		kprintf_n("Thread #%ld requesting the read lock!\n",i);
		rwlock_acquire_read(testrw);
		kprintf_n("Thread #%ld granted the read lock\n",i);
       	        kprintf_n("Ok\n");
	        kprintf_t(".");
		kprintf_n("Thread #%ld is releasing the read lock!\n",i);
		rwlock_release_read(testrw);
	        kprintf_n("Thread #%ld released the read lock!\n",i);
	}
       	V(donesem);
	return;
}

int rwtest2(int nargs, char **args) {
	(void)nargs;
	(void)args;
	
	int result, i;
	testrw = rwlock_create("testrwlock");
	donesem = sem_create("donesem",0);
	testsem = sem_create("testsem",0);
	rwlock_acquire_write(testrw);
	kprintf("FIRST WRITER ACQUIRED THE LOCK\n");
	/* Fork a number of writers and reader threads */
	for(i = 0; i < RWLOOPS; i++)
	{
		result = thread_fork("rwtest2", NULL, rwtestthread, NULL, i);
		if (result) {
			panic("rwt2: thread_fork failed: %s\n", strerror(result));
		}
	}
	kprintf("RELEASINGGG\n");
	rwlock_release_write(testrw);
	for(int i = 0; i < RWLOOPS; i++){
		P(donesem);
	}
	rwlock_destroy(testrw);
	sem_destroy(donesem);
	sem_destroy(testsem);
	donesem = testsem = NULL;
	testrw = NULL;

	kprintf_t("\n");
	success(test_status, SECRET, "rwt2");

	return 0;
}

int rwtest3(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt3 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt3");

	return 0;
}

int rwtest4(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt4 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt4");

	return 0;
}

int rwtest5(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt5 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt5");

	return 0;
}
