#include "libefs.h"

// FS Descriptor
TFileSystemStruct *_fs;

// Open File Table
TOpenFile *_oft;

// Open file table counter
int _oftCount=0;

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword)
{
	mountFS(fsPartitionName, fsPassword);
	_fs = getFSInfo();
}

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.
int openFile(const char *filename, unsigned char mode)
{
	unsigned int index;
	updateDirectory();

	unsigned int attr = getattr(filename);
	if(mode == MODE_CREATE) {
		index = makeDirectoryEntry(filename, attr, getFileLength(filename));
		_oft->writePtr = 0;
		_oft->readPtr = 0;
		_oft->blockSize = _fs->blockSize;
		_oft->inodeBuffer = makeInodeBuffer();
		loadInode(_oft->inodeBuffer, index);
		_oft->buffer = makeDataBuffer();

	} else {
		index = findFile(filename);
	}

	if(index == FS_FILE_NOT_FOUND)
	{
		return -1;
	}

	_oftCount++;
	
	_oft->openMode = mode;
	_oft->inode = index;
	//updateDirectory();
	return index;
}

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
void writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
	/*
	unsigned long blockNum, count = 0, size;
	int num = 0;
	char* buffer1;
	memcpy(buffer1, buffer, dataCount*dataSize);
	if(!_oft->openMode == MODE_READ_ONLY) {
		while(count < dataCount*dataSize ) {
			if(_oft->openMode == MODE_CREATE) {
				//when full buffer
				if(dataSize*dataCount -count >= _oft->blockSize) {
					size = _oft->blockSize;
				} else {
					size = dataSize*dataCount - count;
				}
				blockNum = findFreeBlock();
				strncpy(_oft->buffer, buffer1+count, size);
				markBlockBusy(blockNum);
				loadInode(_oft->inodeBuffer, fp);
				_oft->inodeBuffer[_oft->writePtr] = blockNum;
				writeBlock(_oft->buffer, blockNum);
				_oft->writePtr = _oft->writePtr + 1;
				saveInode(_oft->inodeBuffer, fp);
				updateFreeList();
				updateDirectory();
				unmountFS();
				count += size;
			} else if(_oft->openMode == MODE_NORMAL) {

			}
		}
	}
	*/
}
// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
}

// Read data from the file.
void readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
	char* buffer1;
	memcpy(buffer1, buffer, dataCount*dataSize);
	long count = 0;
	int index = 0;
	while(count < dataSize*dataCount){
		
		loadInode(_oft->inodeBuffer, fp);
		unsigned long blockNum = _oft->inodeBuffer[index++];
		readBlock(_oft->buffer, blockNum);
		_oft->readPtr += sizeof(_oft->buffer);
		strcat(buffer1, _oft->buffer);
		count += sizeof(_oft->buffer);
	}
	memcpy(buffer, buffer1, dataCount*dataSize);
	_oft->readPtr = 0;

}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set. 
// See TDirectory structure.
void delFile(const char *filename);

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp);

// Unmount file system.
void closeFS();

