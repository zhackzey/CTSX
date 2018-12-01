// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"
#include "time.h"
// used for Exercise 2
//#define NumDirect 	((SectorSize - 4 * sizeof(int) - 75) / sizeof(int))
//#define MaxFileSize 	(NumDirect * SectorSize)

// used for Exercise 3
// (128 - 2 * 4) / 4
#define NumDirect 	30
// 128 / 4
#define SectorInt   32
// provide 1 indirect index
// 29 * 128 + 1 * 32 * 128
#define MaxFileSize 	7808

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    // used for Exercise 2
    /*
    void SetCreateTime();
    void SetLastVisitTime();
    void SetLastModifyTime();
    */
  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file
  
  // used for Exercise 2
  /*
  public:
    char type[4];  // file type
    char create_time[25];  // create time
    char last_visit_time[25];  // last  visit time
    char last_modify_time[25]; // last  modify time
    int  sector;    // sector number, where this fileheader is
  */
};

#endif // FILEHDR_H
