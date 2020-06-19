/*
 * Copyright (c) 2001, 2002, 2009
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
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/* Semaphores needed for the matchmaking problem*/
struct semaphore *males = NULL;
struct lock *male_lock = NULL;
struct semaphore *females = NULL;
struct lock *female_lock = NULL;
struct semaphore *matchmakers = NULL;
struct lock *matchmaker_lock = NULL;
/*
 * Called by the driver during initialization.
 */

void whalemating_init() {
	males = sem_create("Males",0);
	if (males == NULL){
		panic("Couldn't create the males semaphore!\n");
	}
	
	females = sem_create("Females",0);
	if (females == NULL){
		kfree(males);
		panic("Couldn't create the females semaphore!\n");
	}
	
	matchmakers = sem_create("Matchmakers",0);
	if (matchmakers == NULL){
		kfree(males);
		kfree(females);
		panic("Couldn't create the matchmakers semaphore!\n");
	}

	male_lock = lock_create("Male lock");
	if (male_lock == NULL) {
		kfree(males);
		kfree(females);
		kfree(matchmakers);
		panic("Couldn't create the male lock!\n");
	}

	female_lock = lock_create("Female lock");
	if (female_lock == NULL) {
		kfree(males);
		kfree(females);
		kfree(matchmakers);
		kfree(male_lock);
		panic("Couldn't create the female lock!\n");
	}

	matchmaker_lock = lock_create("Matchmaker lock");
	if (matchmaker_lock == NULL) {
		kfree(males);
		kfree(females);
		kfree(matchmakers);
		kfree(male_lock);
		kfree(female_lock);
		panic("Couldn't create the Matchmaker lock!\n");
	}
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	sem_destroy(males);
	sem_destroy(females);
	sem_destroy(matchmakers);
        lock_destroy(male_lock);
	lock_destroy(female_lock);
	lock_destroy(matchmaker_lock);
	return;
}

void
male(uint32_t index)
{
        (void) index;
	male_start((uint32_t)index);
	V(males);
	V(males);
	lock_acquire(male_lock);
	P(females);
	P(matchmakers);
	lock_release(male_lock);
	male_end((uint32_t)index);
	return;
}

void
female(uint32_t index)
{
        (void) index;
	female_start((uint32_t)index);
	V(females);
	V(females);
	lock_acquire(female_lock);
	P(males);
	P(matchmakers);
	lock_release(female_lock);
	female_end((uint32_t)index);
	return;
}

void
matchmaker(uint32_t index)
{
	(void) index;
	matchmaker_start((uint32_t)index);
	V(matchmakers);
	V(matchmakers);
	lock_acquire(matchmaker_lock);
	P(females);
	P(males);
	lock_release(matchmaker_lock);
	matchmaker_end((uint32_t)index);
	return;
}
