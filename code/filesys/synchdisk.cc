// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"

static Semaphore *readAvail = new Semaphore("read avail",0);
static Semaphore * writeDone = new Semaphore("write done",0);
static void ReadAvail(int arg) { readAvail->V();}
static void WriteDone(int arg) { writeDone->V();}

//----------------------------------------------------------------------
// DiskRequestDone
// 	Disk interrupt handler.  Need this to be a C routine, because 
//	C++ can't handle pointers to member functions.
//----------------------------------------------------------------------

static void
DiskRequestDone (int arg)
{
    SynchDisk* disk = (SynchDisk *)arg;

    disk->RequestDone();
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new Semaphore("synch disk", 0);
    lock = new Lock("synch disk lock");
    disk = new Disk(name, DiskRequestDone, (int) this);
    
    readerLock = new Lock("reader lock");
    for (int i=0;i<NumSectors;++i)
    {
        mutex[i] = new Semaphore("sector mutex",1);
        numReaders[i] = 0;
        numVisitors[i] = 0;
    }
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
    delete readerLock;
    delete disk;
    delete lock;
    delete semaphore;
    for (int i=0;i<NumSectors;++i)
        delete mutex[i];
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
    disk->ReadRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
    disk->WriteRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::RequestDone
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------

void
SynchDisk::RequestDone()
{ 
    semaphore->V();
}

void 
SynchDisk::PlusReader(int sector)
{
    readerLock->Acquire();
    numReaders[sector] ++;
    if (numReaders[sector] == 1)
        mutex[sector]->P();
    printf("Sector %d reader count : %d\n", sector,numReaders[sector]);
    readerLock->Release();
}
void
SynchDisk::MinusReader(int sector)
{
    readerLock->Acquire();
    numReaders[sector] --;
    if(numReaders[sector] == 0)
        mutex[sector]->V();
    printf("Sector %d reader count : %d\n", sector, numReaders[sector]);
    readerLock->Release();
}
void
SynchDisk::BeginWrite(int sector)
{
    mutex[sector]->P();
}
void
SynchDisk::EndWrite(int sector)
{
    mutex[sector]->V();
}
SynchConsole::SynchConsole(char * readFile, char * writeFile)
{
    lock = new Lock("Console");
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole()
{
    delete lock;
    delete console;
}

void SynchConsole::PutChar(char ch)
{
    lock->Acquire();
    console->PutChar(ch);
    writeDone->P();
    lock->Release();
}

char SynchConsole::GetChar()
{
    lock->Acquire();
    readAvail->P();
    char ch= console->GetChar();
    lock->Release();
    return ch;
}