#pragma once
#include "bit_map.h"
#include "diskDriver.h"
#define FREE_BLOCK -1

// occupies the first portion of each block in the disk
// represents a chained list of blocks
typedef struct {
  int previous_block; // previous block index, useless if block_in_file is 0
  int next_block;     // next block index, useless if at the end of block
  int block_in_file;  // position in the file, if first one (0) the struct contains a FCB
} BlockHeader;

//For directories, header -> next_block points to the next FirstDirectoryBlock in current dir


// File (And Directory) Control Block
typedef struct {
  int parent_directory;   //First block of the parent directory
  int block_in_disk;      // repeated position of the block on the disk
  char name[128];
  int size_in_bytes;        //for directories, sum of bytes of all files in it
  int size_in_blocks;      //for directories, number of directoryBlock linked
  int is_dir;             // 0 if file, 1 if dir
} FileControlBlock;

/******************* stuff on disk BEGIN *******************/
typedef struct {
  BlockHeader header;
  FileControlBlock fcb;
  char data[BLOCK_SIZE-sizeof(FileControlBlock) - sizeof(BlockHeader)] ;
} FirstFileBlock;

// this is one of the next physical blocks of a file
typedef struct {
  BlockHeader header;
  char data[BLOCK_SIZE-sizeof(BlockHeader)];
} FileBlock;

// this is the first physical block of a directory
typedef struct {
  BlockHeader header;
  FileControlBlock fcb;
  int num_entries;
  int file_blocks[ (BLOCK_SIZE                                //indexes for FirstFileBlock and FirstDirectoryBlock of all files/dir in directory
		   -sizeof(BlockHeader)
		   -sizeof(FileControlBlock)
		    -sizeof(int))/sizeof(int) ];
} FirstDirectoryBlock;

// this is remainder block of a directory
typedef struct {
  BlockHeader header;
  int file_blocks[ (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int) ];  //indexes for FirstFileBlock of all files in directory
} DirectoryBlock;
/******************* stuff on disk END *******************/





typedef struct {
  DiskDriver* disk;       //DiskDriver the FS uses for storing blocks
} SimpleFS;

// this is a file handle, used to refer to open files
typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstFileBlock* fcb;             // pointer to the first block of the file(read it)
  FirstDirectoryBlock* directory;  // pointer to the directory where the file is stored
  BlockHeader* current_block;      // current block in the file
  int pos_in_file;                 // absolute position of the cursor in the file
} FileHandle;

typedef struct {
  SimpleFS* sfs;                   // pointer to memory file system structure
  FirstDirectoryBlock* dcb;        // pointer to the directory control block
  FirstDirectoryBlock* directory;  // pointer to the parent directory (null if top level)
  BlockHeader* current_block;      // current block in the directory
  int pos_in_dir;                  // absolute position of cursor in dir
  int pos_in_block;                // relative position of the cursor in the block
} DirectoryHandle;



// LC
// initializes fileSystem on given disk (both preallocated)
// Creates the root directory and saves the block in first position on disk
// Returns a DirectoryHandle for root dir
// Returns NULL if anything wrong happens
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk);

// LC
// creates an empty first file block in directorty dir with name filename
// if file exists, returns a file handle for the given file
// returns NULL if anything happens (no free space, etc)
FileHandle* SimpleFS_createFile(DirectoryHandle* dir, const char* filename);

// LC
// writes in the preallocated blocks array the names of all files ,if flag
// equals 0, or all files and dirs if flag equals 1, in dir
// returns 0 if anything is ok, 1 otherwise
int SimpleFS_readDir(char** names, DirectoryHandle* dir, int flag);

// LC
// Creates a FileHandle about an existing file in the directory dir
// Returns NULL if anything happens
FileHandle* SimpleFS_openFile(DirectoryHandle* dir, const char* filename);

// LC
// closes an opened file by destroing the relative FileHandle
// return 0 on success, 1 if anything happens
int SimpleFS_close(FileHandle* fh);

// LC
// writes in the file, from cursor position for size bytes, the data in data
// if necessary, allocates new blocks
// returns the number of bytes written or -1 if anything goes wrong
int SimpleFS_write(FileHandle* f, void* data, int size);


// LC
// reads in the preallocated data array, from current position of cursor,
// data stored in the file provided
// returns the number of bytes read or -1 if anything goes wrong
int SimpleFS_read(FileHandle* file, void* data, int size);

// LC
// changes cursor position from current to pos
// returns pos on success, -1 if anything goes wront (file too short, ecc ecc)
int SimpleFS_seek(FileHandle* file, int pos);

// LC
// makes dir point to given directory inside current directory
// if dirname is .. , dir will go one level up
// returns 0 on success, 1 if anything happens
int SimpleFS_changeDir(DirectoryHandle* dir, char* dirname);

// LC
// creates a new directory in the current one
// returns 0 on success, 1 if anything happens
int SimpleFS_mkDir(DirectoryHandle* dir, char* dirname);

// BEGIN EXPERIMENTAL
// LC
// Removes the given file and all his blocks from his dir
// Returns 0 on success, 1 if anything happens
int SimpleFS_rmFile(DirectoryHandle* dir, char* filename);

// LC
// Removes the given dir and all his contents (recursively) from fs
// returns 0 on success, 1 if anything gows wrong
// Side-effect on dir, it will point to parent directory
int SimpleFS_rmDir(DirectoryHandle* dir);
//END EXPERIMENTAL

// LC
// prints all files and dir in provided directory using SimpleFS_readDir
// returns 0 on success, 1 if anything happens
int SimpleFS_ls(DirectoryHandle* dh);

// LC
// creates a copy of provided directory handler, to use in automatic testing
// returns NULL if anything happens
DirectoryHandle* cloneDh(DirectoryHandle* dh);

// LC
// frees all memory used and removes fileSystem and Disk
// returns 0 on success, 1 if anything goes wrong
// can only be used on root
// final destination
int SimpleFS_rmslash(DirectoryHandle* root);


//****************** DEBUGGING FUNCTIONS **********************//
// LC
// prints info about DirectoryHandle values, checks if anything is correct
void print_info_dh(DirectoryHandle* dh);

// LC
// prints info about blockHeader values, checks if anything is correct
void print_info_block_header(BlockHeader* bh);

// LC
// prints info about fileHandle values, checks if everything is ok
void print_info_fh(FileHandle* fh);
