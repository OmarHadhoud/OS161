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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Locks and semaphores needed for the solution
 */
struct semaphore *intersection_sem = NULL;
struct lock *quad0 = NULL;
struct lock *quad1 = NULL;
struct lock *quad2 = NULL;
struct lock *quad3 = NULL;
struct lock *quads[4];

/*
 * Called by the driver during initialization.
 */

void
stoplight_init() {
	
	intersection_sem = sem_create("Intersection semaphore",3);
	if (intersection_sem == NULL) {
		panic("Couldn't create the intersection sem\n");
	}

	quad0 = lock_create("Quad 0 lock");
	if (quad0 == NULL) {
		kfree(intersection_sem);
		panic("Couldn't create the quad 0 lock");
	}

	quad1 = lock_create("Quad 1 lock");
	if (quad1 == NULL) {
		kfree(intersection_sem);
		kfree(quad0);
		panic("Couldn't create the quad 1 lock");
	}

	quad2 = lock_create("Quad 2 lock");
	if (quad2 == NULL) {
		kfree(intersection_sem);
		kfree(quad0);
		kfree(quad1);
		panic("Couldn't create the quad 2 lock");
	}

	quad3 = lock_create("Quad 3 lock");
	if (quad3 == NULL) {
		kfree(intersection_sem);
		kfree(quad0);
		kfree(quad1);
		kfree(quad2);
		panic("Couldn't create the quad 3 lock");
	}
	quads[0] = quad0;
	quads[1] = quad1;
	quads[2] = quad2;
	quads[3] = quad3;
	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
	sem_destroy(intersection_sem);
	lock_destroy(quad0);
	lock_destroy(quad1);
	lock_destroy(quad2);
	lock_destroy(quad3);
	return;
}

void
turnright(uint32_t direction, uint32_t index)
{
	//Make sure there's at most 2 cars before entering intersection
	P(intersection_sem);
	//Acquire the quad we're going to if it is empty and update it in driver
	lock_acquire(quads[direction]);
	inQuadrant(direction, index);
	//Leave the inresection as we turn right, after turning right we leave the lock for the quad for other cars.
	leaveIntersection(index);
	lock_release(quads[direction]);
	//Increment the semaphore to indicate a car has left the intersection
	V(intersection_sem);
	return;
}
void
gostraight(uint32_t direction, uint32_t index)
{
	uint32_t passing_direction = (direction + 3) % 4;
	//Make sure there's at most 2 cars before entering intersection
	P(intersection_sem);
	//Acquire the quad we're going to if is is empty, update it in the driver.
	lock_acquire(quads[direction]);
	inQuadrant(direction, index);
	//Request to acquire next quad if it is empty, if not, we wait until it is empty.
	lock_acquire(quads[passing_direction]);
	//Update the quad.
	inQuadrant(passing_direction, index);
	//Release the lock of previous quad for other cars as we moved.
	lock_release(quads[direction]);
	//Leave the intersection and release the lock for other cars.
	leaveIntersection(index);
	lock_release(quads[passing_direction]);
	//Increment the semaphore to indicate a car has left the intersection
	V(intersection_sem);
	return;
}
void
turnleft(uint32_t direction, uint32_t index)
{
	uint32_t passing_direction = (direction + 3) % 4;
	uint32_t passing_direction2 = (direction + 2) % 4;
	//Make sure there's at most 2 cars before entering intersection
	P(intersection_sem);
	//Acquire the quad we're going to if is is empty, update it in the driver.
	lock_acquire(quads[direction]);
	inQuadrant(direction, index);
	lock_acquire(quads[passing_direction]);
	//Update the quad.
	inQuadrant(passing_direction, index);
	//Release the lock of previous quad for other cars as we moved.
	lock_release(quads[direction]);
	lock_acquire(quads[passing_direction2]);
	//Update the quad.
	inQuadrant(passing_direction2, index);
	//Release the lock of previous quad for other cars as we moved.
	lock_release(quads[passing_direction]);
	//Leave the intersection and release the lock for other cars.
	leaveIntersection(index);
	lock_release(quads[passing_direction2]);
	//Increment the semaphore to indicate a car has left the intersection
	V(intersection_sem);
	return;
}
