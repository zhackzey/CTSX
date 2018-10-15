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
    default:
	printf("No test specified.\n");
	break;
    }
}

