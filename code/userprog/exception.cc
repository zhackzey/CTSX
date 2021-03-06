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

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) 
    {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	printf("user program calls syscall halt\n");
    printf("current thread : %s\n",currentThread->getName());

       interrupt->Halt();
    }
    else if((which==SyscallException)&&(type == SC_Exit))
    {
        printf("current thread : %s\n",currentThread->getName());
        printf("user program calls syscall Exit\n");
        // we should deallocate the physical pages allocated for this user program
        printf("Before deallocate...\n");
        machine->PrintPageTable();
        
        machine->clear();
        printf("After deallocate...\n");
        machine->PrintPageTable();

        machine->PrintBitmap();
        //update PC
        AdvancePC();
        currentThread->Finish();
    } 
    else if(which==PageFaultException)
    {
        if(machine->tlb!=NULL)
        // the page is not in TLB
        {
            printf("TLB pageFault\n");
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
        // the page is not in pageTable
        {
            
            OpenFile *openfile = fileSystem->Open("virtual_memory");
            ASSERT(openfile!=NULL);
            // vpn causing PageFault
            int vpn = (unsigned) machine->registers[BadVAddrReg] /PageSize;
            printf("Page %d is not in pageTable\n",vpn);
            
            int pos = -1;       // pageTable entry number , the pageTable entry to be changed            
            // find the unused physical page
            pos = machine->find();
            if(pos==-1)
            //all physical pages are allocated
            // we need to replace some physical page out
            {
                // For simplity, just choose to switch out the first physical page
                pos = 0;
                for (int i=0;i<machine->pageTableSize;++i)
                {
                    if(machine->pageTable[i].physicalPage == pos)
                    {
                       if(machine->pageTable[i].dirty==TRUE)
                       // this physical page is modified
                       // we need to write it back into disk
                       {
                           openfile->WriteAt(&(machine->mainMemory[pos*PageSize]),
                            PageSize,machine->pageTable[i].virtualPage*PageSize);
                            machine->pageTable[i].valid =FALSE;
                            break;
                       } 
                    }
                }
            }

            // this is for traditional pageTable
            // now we have chosen the physical page to replace
            // we should copy the disk physical page into memory
            /*
            openfile->ReadAt(&(machine->mainMemory[pos*PageSize]),PageSize,vpn* PageSize);
            machine->pageTable[vpn].valid = TRUE;
            printf("Copy virtual page %d into memory, corresponding ppn is %d \n",vpn,pos);

            // pageTable 's index is its vpn
            machine->pageTable[vpn].virtualPage = vpn;
            machine->pageTable[vpn].physicalPage = pos;
            machine->pageTable[vpn].use =FALSE;
            machine->pageTable[vpn].dirty = FALSE;
            machine->pageTable[vpn].readOnly = FALSE;
            */

           // this is for inverted pageTable
            openfile->ReadAt(&(machine->mainMemory[pos*PageSize]),PageSize,vpn* PageSize);
            // ppn is index into inverted PageTable !
            machine->pageTable[pos].valid = TRUE;
            printf("Copy virtual page %d into memory, corresponding ppn is %d \n",vpn,pos);
            machine->pageTable[pos].virtualPage = vpn;
            machine->pageTable[pos].use = FALSE;
            machine->pageTable[pos].dirty = FALSE;
            machine->pageTable[pos].readOnly = FALSE;
            machine->pageTable[pos].threadID = currentThread->getThreadID();
            delete openfile;
            //ASSERT(FALSE);
        }
    }
    else  {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
