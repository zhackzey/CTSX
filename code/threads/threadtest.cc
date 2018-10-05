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
	printf("*** thread name %s userID %d threadID %d looped %d times\n", currentThread->getName(),currentThread->getUserID(),currentThread->getThreadID(),num);
        currentThread->Yield();
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

    Thread *t1 = new Thread("thread1");
    Thread *t2 = new Thread("thread2");
    Thread *t3 = new Thread("thread3");

    //t->Fork(SimpleThread, (void*)1);
    t1->Fork(PrintAllThreads,(void*)1);
    t2->Fork(PrintAllThreads,(void*)1);
    t3->Fork(PrintAllThreads,(void*)1);
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
    default:
	printf("No test specified.\n");
	break;
    }
}

