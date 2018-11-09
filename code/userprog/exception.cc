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

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
    else if((which==SyscallException)&&(type == SC_Exit))
    {
        printf("user program calls syscall Exit\n");
        
        // we should deallocate the physical pages allocated for this user program
        for (int i=0;i<machine->pageTableSize;++i)
        {
            int vpn = machine->pageTable[i].physicalPage;
            if(machine->bitmap->Test(vpn)==TRUE)
            // deallocate
                {
                    machine->bitmap->Clear(vpn);
                    machine->bitmap->Print();
                }
        }
        int NextPC = machine->ReadRegister(NextPCReg);
        machine->WriteRegister(PCReg,NextPC);
    } 
    else if(which==PageFaultException)
    {
        if(machine->tlb!=NULL)
        {
            int vpn = (unsigned) machine->registers[BadVAddrReg] /PageSize;
            int pos = -1;       // tlb entry number , the tlb entry to be changed
            for (int i=0;i<TLBSize;++i)
            {
                if(machine->tlb[i].valid==FALSE)
                {
                    pos = i;
                    break;
                }
            }
            // all entries are valid,so we need to replace
            // page replacement algorithm
            if(pos==-1)
            {
                // FIFO
                /*
                   choose to replace the first entry
                   so let entries move forward
                   and the last entry stores the new entry
                */
                // choose to move into the last entry
                /*
                pos = TLBSize -1;
                for (int i=0;i<TLBSize -1;++i)
                    machine->tlb[i] = machine->tlb[i+1];
                */
                // LRU
                
                for (int i=0;i<TLBSize; ++i)
                    if (machine->LRU_cnt [i]==TLBSize)
                    {
                        // choose the least recent visited entry to replace
                        pos = i;
                        break;
                    }
                    
            }

            // Since we encounter the exception of PageFaultException
            // we need to update the LRU_cnt
            machine->LRU_cnt[pos] = 1;
            for (int i=0;i<TLBSize;++i)
            {
                if(i==pos)
                    continue;
                if(machine->LRU_cnt[i]==-1) //invalid
                    continue;
                // update the old LRU_cnt
                machine->LRU_cnt[i] ++;
            }

            // new entry changed into TLB
            machine->tlb[pos].valid=true;
            machine->tlb[pos].virtualPage = vpn;
            machine->tlb[pos].physicalPage = machine->pageTable[vpn].physicalPage;
            machine->tlb[pos].use = FALSE;
            machine->tlb[pos].dirty = FALSE;
            machine->tlb[pos].readOnly = FALSE;
        }
        else
        {
            ASSERT(FALSE);
        }
    }
    else  {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
