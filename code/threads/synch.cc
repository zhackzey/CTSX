// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) 
{
    name = debugName;
    semaphore = new Semaphore(name,1);
    lockHelder = NULL;
}

Lock::~Lock() 
{
    delete semaphore;
}

void Lock::Acquire() 
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    semaphore->P();
    lockHelder = currentThread;
    (void) interrupt->SetLevel(oldlevel);
}

void Lock::Release() 
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    ASSERT(lockHelder==currentThread);
    lockHelder = NULL;
    semaphore->V();
    (void) interrupt->SetLevel(oldlevel);
}

bool Lock::isHeldByCurrentThread()
{
    if(lockHelder == currentThread)
        return true;
    return false; 
}
Condition::Condition(char* debugName)
{
    name = debugName;
    queue = new List;
}
Condition::~Condition() 
{
    delete queue;
}
void Condition::Wait(Lock* conditionLock) 
{ 
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    conditionLock->Release();
    queue->Append((void*)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldlevel);
}
void Condition::Signal(Lock* conditionLock) 
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    Thread *thread = (Thread*)queue->Remove();
    if(thread)
        scheduler->ReadyToRun(thread);
    (void) interrupt->SetLevel(oldlevel);
}
void Condition::Broadcast(Lock* conditionLock) 
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    Thread *thread = (Thread*)queue->Remove();
    while(thread)
        {
            scheduler->ReadyToRun(thread);
            thread = (Thread*) queue->Remove();
        }
    (void) interrupt->SetLevel(oldlevel);
}


Barrier::Barrier(char* debugName,int _total )
{
    name = debugName;
    queue = new List;
    total = _total;
    already =0;
}

Barrier::~Barrier()
{
    delete queue;
}

void Barrier::Wait()
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    ++already;
    if(already < total)
    {
        queue->Append((void*) currentThread);
        currentThread->Sleep();
        (void) interrupt->SetLevel(oldlevel);
        return;
    }

    // already == total
    Thread * thread = (Thread*) queue->Remove();
    while(thread)
    {
        scheduler->ReadyToRun(thread);
        thread = (Thread*) queue->Remove();
    }
    (void) interrupt->SetLevel(oldlevel);
}

RW_Lock::RW_Lock(char * debugName)
{
    name = debugName;
    r_queue = new List;
    w_queue = new List;
    is_writing = false;
    is_reading = false;
    reader_cnt = 0;
}

RW_Lock::~RW_Lock()
{
    delete r_queue;
    delete w_queue;
}

void RW_Lock::getWLock()
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    if(is_writing||is_reading)
    {
        w_queue->Append((void*) currentThread);
        currentThread->Sleep();
    }
    is_writing = true;
    (void) interrupt->SetLevel(oldlevel);
}

void RW_Lock::getRLock()
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    if (is_writing)
    {
        r_queue->Append((void*)currentThread);
        currentThread->Sleep();
    }
    is_reading = true;
    reader_cnt++;
    printf("Now Reader_CNT :%d\n",reader_cnt);
    (void) interrupt->SetLevel(oldlevel);
}

void RW_Lock::ReleaseWLock()
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    if(w_queue->IsEmpty()!=true)
    {
        Thread * thread = (Thread*) w_queue->Remove();
        scheduler->ReadyToRun(thread);
        is_writing = true;
    }
    else if(r_queue->IsEmpty()!=true)
    {
        Thread * thread = (Thread*) r_queue->Remove();
        scheduler->ReadyToRun(thread);
        is_reading = true;
    }
    is_writing = false;
    (void) interrupt->SetLevel(oldlevel);
}

void RW_Lock::ReleaseRLock()
{
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    reader_cnt --;
    printf("Now Reader_CNT :%d\n",reader_cnt);
   
    if(reader_cnt==0)
    {
        if(w_queue->IsEmpty()!=true)
        {
            Thread * thread = (Thread*) w_queue->Remove();
            scheduler->ReadyToRun(thread);
            is_writing = true;
        }
    }
    if(r_queue->IsEmpty()!=true)
    {
        Thread * thread = (Thread*) r_queue->Remove();
        scheduler->ReadyToRun(thread);
        is_reading = true;
    }

    is_reading = false;
    (void) interrupt->SetLevel(oldlevel);
}

