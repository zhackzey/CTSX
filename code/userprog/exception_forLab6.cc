// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void AdvancePC()
{
    machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg,machine->ReadRegister(NextPCReg)+4);
}
void exec_func(int address)
{
    char name[20];
    int pos = 0;
    int data;
    while(1)
    {
        machine->ReadMem(address + pos, 1, &data);
        if (data == 0)
        {
            name[pos] ='\0';
            break;
        }
        name[pos++] = char(data);
    }
    OpenFile * executable = fileSystem ->Open(name);
    AddrSpace * space;
    space = new AddrSpace (executable);
    currentThread->space = space;
    delete executable;
    space->InitRegisters();
    space->RestoreState();
    machine->Run();
}
struct Info
{
    AddrSpace * space;
    int pc;
};
void fork_func(int address)
{
    Info * info = (Info*) address;
    AddrSpace *space = info->space;
    currentThread->space = space;
    int cur_pc = info->pc;
    space->InitRegisters();
    space->RestoreState();
    machine->WriteRegister(PCReg, cur_pc);
    machine->WriteRegister(NextPCReg, cur_pc + 4);
    machine->Run();
}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) 
    {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	printf("user program calls syscall Halt\n");
    printf("current thread : %s\n",currentThread->getName());

       interrupt->Halt();
    }
    else if ((which == SyscallException) && (type == SC_Create))
    {
   	    printf("user program calls syscall Create\n");
        int address = machine->ReadRegister(4);
        char name[20];
        int pos = 0;
        int data;
        while(1)
        {
            machine->ReadMem(address + pos, 1, &data);
            if(data == 0)
            {
                name[pos] = '\0';
                break;
            }
            name[pos++] = char(data);
        }
        fileSystem->Create(name,128);
        AdvancePC();
    }
    else if ((which == SyscallException) && (type == SC_Open))
    {
   	    printf("user program calls syscall Open\n");
        int address = machine->ReadRegister(4);
        char name[20];
        int pos = 0;
        int data;
        while(1)
        {
            machine->ReadMem(address + pos, 1, &data);
            if(data == 0)
            {
                name[pos] = '\0';
                break;
            }
            name[pos++] = char(data);
        }
        OpenFile * openfile = fileSystem->Open(name);
        machine->WriteRegister(2, int(openfile));
        AdvancePC();
    }
    else if ((which == SyscallException) && (type == SC_Close))
    {
   	    printf("user program calls syscall Close\n");
        int fd = machine->ReadRegister(4);
        OpenFile * openfile = (OpenFile *) fd;
        delete openfile;
        AdvancePC();
    }
    else if ((which == SyscallException) && (type == SC_Read))
    {
   	    printf("user program calls syscall Read\n");
        int address = machine->ReadRegister(4);
        int cnt = machine->ReadRegister(5);
        int fd = machine->ReadRegister(6);
        OpenFile * openfile = (OpenFile *) fd;
        char *content = new char[cnt];
        int result = openfile->Read(content, cnt);
        for (int i = 0; i < result; ++i)
            machine->WriteMem(address + i, 1,int(content[i]));
        machine->WriteRegister(2, result);
        AdvancePC();
    }
    else if ((which == SyscallException) && (type == SC_Write))
    {
   	    printf("user program calls syscall Write\n");
        int address = machine->ReadRegister(4);
        int cnt = machine->ReadRegister(5);
        int fd = machine->ReadRegister(6);
        char *content = new char[cnt];
        int data;
        for (int i = 0; i < cnt; ++i)
        {
            machine->ReadMem(address + i, 1,&data);
            content[i] = char(data);
        }
        OpenFile * openfile = (OpenFile *) fd;
        openfile->Write(content, cnt);
        AdvancePC();
    }
    else if ((which == SyscallException) && (type == SC_Exec))
    {
        printf("user program calls syscall Exec\n");
        int address = machine->ReadRegister(4);
        Thread* newthread = new Thread("New Thread");
        newthread->Fork(exec_func,address);
        machine->WriteRegister(2,newthread->getThreadID());
        AdvancePC();
    }
    else if((which == SyscallException) && (type == SC_Fork))
    {
        printf("user program calls syscall Fork\n");
        int function_pc = machine->ReadRegister(4);
        OpenFile * executable = fileSystem->Open(currentThread->filename);
        AddrSpace * space = new AddrSpace(executable);
        space->CopyAddrSpace(currentThread->space);
        Info * info = new Info;
        info->space = space;
        info->pc = function_pc;
        Thread * newthread = new Thread("New Thread");
        newthread ->Fork (fork_func,int(info));
        AdvancePC();
    }

    else if((which==SyscallException)&&(type == SC_Exit))
    {
        printf("current thread : %s\n",currentThread->getName());
        printf("user program calls syscall Exit\n");
        // we should deallocate the physical pages allocated for this user program
        //printf("Before deallocate...\n");
        //machine->PrintPageTable();
        
        machine->clear();
        //printf("After deallocate...\n");
        //machine->PrintPageTable();

        //machine->PrintBitmap();
        //update PC
        AdvancePC();
        currentThread->Finish();
    } 
    else if((which == SyscallException) && (type == SC_Yield))
    {
        printf("user program calls syscall Yield\n");
        AdvancePC();
        currentThread->Yield();
    }
    else if ((which == SyscallException) && (type == SC_Join))
    {
        printf("user program calls syscall Join\n");
        int threadID = machine->ReadRegister(4);
        while(threadsID_Array[threadID])
            currentThread->Yield();
        AdvancePC();
    }
    else if(which==PageFaultException)
    {
        printf("Page Fault!\n");
    }
    else  {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
