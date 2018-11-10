// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
void ForkThread(int which)
{
    printf("Second thread started!\n");
    machine->Run();
}
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable1 = fileSystem->Open(filename);
    OpenFile *executable2 = fileSystem->Open(filename);
    AddrSpace *space1,*space2;
    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space1 = new AddrSpace(executable1);    
    printf("First thread's addrspace initialized\n");
    Thread * thread = new Thread("second thread");
    space2 = new AddrSpace(executable2);
    printf("Second thread's addrspace initialized\n");
    currentThread->space = space1;

    // set second thread's addrspace
    space2->InitRegisters();
    space2->RestoreState();
    thread->space=space2;
    thread->Fork(ForkThread,(void*)1);
    // let second thread take cpu
    currentThread->Yield();
    delete executable2;			// close file
    delete executable1;
    space1->InitRegisters();		// set the initial register values
    space1->RestoreState();		// load page table register
    printf("First thread started!\n");
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
