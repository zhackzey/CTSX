// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"
// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread name %s userID %d threadID %d priority %d looped %d times\n", currentThread->getName(),currentThread->getUserID(),
    currentThread->getThreadID(), currentThread->getPriority() ,num);
        //currentThread->Yield();
    }
}
//----------------------------------------------------------------------
// ThreadNumLimit
void ThreadNumLimit()
{
    DEBUG('t',"Entering ThreadTest2(Num limit test)");
    for (int i=0;i<128;++i)
    {
        Thread *t = new Thread("thread");
	    printf("*** thread name %s userID %d threadID %d\n",t->getName(),t->getUserID(),t->getThreadID());
    }
}
//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t1 = new Thread("thread1");
    Thread *t2 = new Thread("thread2");
    Thread *t3 = new Thread("thread3");
    Thread *t4 = new Thread("thread4");
    Thread *t5 = new Thread("thread5");

    //t->Fork(SimpleThread, (void*)1);
    t1->Fork(SimpleThread,(void*) t1->getThreadID());
    t2->Fork(SimpleThread,(void*) t2->getThreadID());
    t3->Fork(SimpleThread,(void*) t3->getThreadID());
    t4->Fork(SimpleThread,(void*) t4->getThreadID());
    t5->Fork(SimpleThread,(void*) t5->getThreadID());

    SimpleThread(0);
}
//----------------------------------------------------------------------
// Print all threads
// which: dummy
void PrintAllThreads(int which)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    //Print the thread which is running
    printf("name %s threadID %d status: %s \n",currentThread->getName(),currentThread->getThreadID(),currentThread->getStatus());

    List * ll =new List();
    ll = scheduler->getReadyList();
    if(!ll->IsEmpty())
    {
        ll->Mapcar(ThreadPrint);
    }
    currentThread->Yield();
    interrupt->SetLevel(oldLevel);
}   
//----------------------------------------------------------------------
// ThreadTest3
void ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");

    Thread *t1 = new Thread("thread1",1);
    Thread *t2 = new Thread("thread2",2);
    Thread *t3 = new Thread("thread3",3);

    //t->Fork(SimpleThread, (void*)1);
    t3->Fork(PrintAllThreads,(void*)1);
    t2->Fork(PrintAllThreads,(void*)1);
    t1->Fork(PrintAllThreads,(void*)1);
}
//----------------------------------------------------------------------
// ThreadTest4
// test whether threads are excuted by the order of their priority
void ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    Thread *t1 = new Thread("thread1",1);
    Thread *t2 = new Thread("thread2",3);
    Thread *t3 = new Thread("thread3",7);

    t3->Fork(SimpleThread,(void*)1);
    t1->Fork(SimpleThread,(void*)1);
    t2->Fork(SimpleThread,(void*)1);
}

//----------------------------------------------------------------------
// ThreadTest5
// test if one thread can take over one another
void thread3(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread name %s userID %d threadID %d priority %d looped %d times\n", currentThread->getName(),currentThread->getUserID(),
    currentThread->getThreadID(), currentThread->getPriority() ,num);
        //currentThread->Yield();
    if(num==0)
    {
        Thread *t3 = new Thread("thread3" ,1);
        t3->Fork(SimpleThread,(void*)1);
    }
    }
}
void thread2(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread name %s userID %d threadID %d priority %d looped %d times\n", currentThread->getName(),currentThread->getUserID(),
    currentThread->getThreadID(), currentThread->getPriority() ,num);
        //currentThread->Yield();
    if(num==0)
    {
        Thread *t2 = new Thread("thread2" ,3);
        t2->Fork(thread3,(void*)1);
    }
    }
}
void ThreadTest5()
{
    Thread * t1 = new Thread("thread1",7);
    t1->Fork(thread2,(void*)1);
}

// the following function is used for testing RR
//----------------------------------------------------------------------
// SimpleThread2
void SimpleThread2(int which)
{
    int num;
    for(num=0;num<which +1;++num)
    {
	printf("*** thread %d looped %d times, used %d timeSlices\n", which,num,currentThread->getUsedSlice()+1);
        for(int i=0;i<10;++i)
        {
            interrupt->OneTick();
        }
    }
    currentThread->Yield();
}

void ThreadTest6()
{
    for(int i=0;i<5;++i)
    {
        Thread*t =new Thread("td");
        t->Fork(SimpleThread2,(void*)t->getThreadID());
    }
    SimpleThread2(0);
}
// for the test of producer and consumer problem

const int buffer_size = 3;
// the lock on buffer
Semaphore *mutex = new Semaphore("mutex",1);
// the count of remaining space of buffer
Semaphore *empty = new Semaphore("empty",buffer_size);
// the count of items in buffer
Semaphore *full  = new Semaphore("full",0);
// used for printf since the int value of Semaphore can not be read
int num = 0 ; 
// producer
void Producer(int which)
{
    printf("Now thread %s is waiting to produce\n",currentThread->getName());
    // if any remaining space?
    empty->P();
    // exclude other threads
    mutex->P();
    // enter the region 
    printf("Thread %s has produced one item, now we have %d items in buffer\n",currentThread->getName(),++num);
    // leave the region
    mutex->V();
    full->V();
    // relinguish the cpu
    currentThread->Yield();
}

//consumer
void Consumer(int which)
{
    printf("Now thread %s is waiting to consume\n",currentThread->getName());
    // if ang remaining items in buffer?
    full->P();
    // exclude other threads
    mutex->P();
    // enter the region
    printf("Thread %s has consumed one item, now we have %d items in buffer\n",currentThread->getName(),--num);
    // leave the region
    mutex->V();
    empty->V();
    // relinguish the cpu
    currentThread->Yield();
}

void PC_Test1()
{
    Thread * t1 = new Thread("Consumer1");
    t1->Fork(Consumer,(void*)1);
    Thread * t2 = new Thread("Producer1");
    t2->Fork(Producer,(void*)1);
}

void PC_Test2()
{
    Thread * t1 = new Thread("Consumer1");
    t1->Fork(Consumer,(void*)1);
    Thread * t2 = new Thread("Producer1");
    t2->Fork(Producer,(void*)1);
    Thread * t3 = new Thread("Producer2");
    t3->Fork(Producer,(void*)1);
    Thread * t4 = new Thread("Producer3");
    t4->Fork(Producer,(void*)1);
    Thread * t5 = new Thread("Producer4");
    t5->Fork(Producer,(void*)1);
    Thread * t6 = new Thread("Producer5");
    t6->Fork(Producer,(void*)1);
    Thread * t7 = new Thread("Consumer2");
    t7->Fork(Consumer,(void*)1);
}
//----------------------------------------------------------------------
// For the test of Condition based Consumer and Producer Problem

Condition *Full = new Condition("full");
Condition *Empty = new Condition("empty");
Lock * lock = new Lock("lock");
Lock * full_lock = new Lock("full_lock");
Lock * empty_lock = new Lock("empty_lock");
int item = 0;

void Producer2(int which)
{
    printf("Now thread %s is waiting to produce\n",currentThread->getName());
    lock->Acquire();
    if(item==buffer_size)
    // wait on  full queue
    {
        // require the lock of Full condition
        full_lock->Acquire();
        // before we get to wait on the Full 
        // we need to give up the lock on item
        // otherwise other theads can not acquire that
        lock->Release();
        // get to wait on Full
        Full->Wait(full_lock);

        // we are waked up so we need to acquire the lock
        // on item at first
        lock->Acquire();
        full_lock ->Release();
    }
    printf("Thread %s has produced one item, now we have %d items in buffer\n",currentThread->getName(),++item);
    if(item ==1)
    // we need to wake up consumer
    {
        // require the lock of Empty condition 
        empty_lock->Acquire();
        // signal one consumer
        Empty->Signal(empty_lock);
        empty_lock->Release();
    }
    lock->Release();
}

void Consumer2(int which)
{
    printf("Now thread %s is waiting to consume\n",currentThread->getName());
    lock->Acquire();
    if (item == 0)
    // we need to wait on Empty condtion
    {
        // require the lock of Empty condition
        empty_lock->Acquire();
        lock->Release();
        Empty->Wait(empty_lock);
        lock->Acquire();
        empty_lock->Release();
    }
    printf("Thread %s has consumed one item, now we have %d items in buffer\n",currentThread->getName(),--item);

    if(item==buffer_size-1)
    // wake up producer
    {
        full_lock->Acquire();
        Full->Signal(full_lock);
        full_lock->Release();
    }
    lock->Release();
}


void PC_Test3()
{
    Thread * t1 = new Thread("Consumer1");
    t1->Fork(Consumer2,(void*)1);
    Thread * t2 = new Thread("Producer1");
    t2->Fork(Producer2,(void*)1);
    Thread * t3 = new Thread("Producer2");
    t3->Fork(Producer2,(void*)1);
    Thread * t4 = new Thread("Producer3");
    t4->Fork(Producer2,(void*)1);
    Thread * t5 = new Thread("Producer4");
    t5->Fork(Producer2,(void*)1);
    Thread * t6 = new Thread("Producer5");
    t6->Fork(Producer2,(void*)1);
    Thread * t7 = new Thread("Consumer2");
    t7->Fork(Consumer2,(void*)1);
}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    ThreadNumLimit();
    break;
    case 3:
    ThreadTest3();
    break;
    case 4:
    ThreadTest4();
    break;
    case 5:
    ThreadTest5();
    break;
    case 6:
    ThreadTest6();
    break;
    case 7:
    PC_Test1();
    break;
    case 8:
    PC_Test2();
    break;
    case 9:
    PC_Test3();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

