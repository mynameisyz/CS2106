#include "libefs.h"

// FS Descriptor
TFileSystemStruct *_fs;

// Open File Table
TOpenFile *_oft;

// Open file table counter
int _oftCount=0;

int setupOftEntry (const char* filename, int inode, long filesize, int mode);

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword)
{
	mountFS(fsPartitionName, fsPassword);
	_fs = getFSInfo();

	//max number files = number of available blocks
	_oft = (TOpenFile*)calloc(_fs->maxFiles, sizeof(TOpenFile));

}

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.
int openFile(const char *filename, unsigned char mode)
{
	int inode = findFile(filename);
	switch(mode) {
	case MODE_NORMAL: {
		if (_result == FS_OK) {
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		} else {
			printf("ERR CODE: %lu\n", _result);
			return -1;
		}
		break;
	}
	case MODE_CREATE: {
		if (_result == FS_OK ) {
			//file exists
			printf("FILE EXISTS\n");
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		}
		else {
			//attempt to create new file
			inode = makeDirectoryEntry(filename, 128, 0);
			if (_result == FS_OK) {
				//success
				updateDirectory();
				return setupOftEntry(filename, inode, 0, mode);
			} else {
				//error encountered
				printf("ERR CODE: %lu\n", _result);
				return -1;
			}
		}
		break;
	}
	case MODE_READ_ONLY: {
		if (_result == FS_OK) {
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		} else {
			printf("ERR CODE: %lu\n", _result);
			return -1;
		}
		break;
	}
	case MODE_READ_APPEND: {
		if(_result == FS_OK) {
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		}
		else {
			printf("ERR CODE: %lu\n", _result);
			return -1;
		}
		break;
	}
	default:
		return -1;
	}
}

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
void writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
	if (_oft[fp].openMode == MODE_READ_ONLY) {
		printf("ERR : READ_ONLY");
	}

	unsigned long leftToWrite = dataCount;
	unsigned long currentBlock;

	printf("WRITING TO FILE");
	while (leftToWrite > 0) {

		currentBlock = returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);
		printf("Writing to block# %lu\n", currentBlock);
		if (currentBlock == 0) {
			//assign new block
			currentBlock = findFreeBlock();
			markBlockBusy(currentBlock);
			setBlockNumInInode(_oft[fp].inodeBuffer, _oft[fp].filePtr, currentBlock);
			printf("Assigning  block# %lu\n", currentBlock);
		}
		unsigned int written, remainingBlockSpace;
		remainingBlockSpace = _fs->blockSize - _oft[fp].writePtr;
		//if we have enough space left in buffer
		if ( remainingBlockSpace > leftToWrite) {
			written = leftToWrite;
			memcpy(_oft[fp].buffer + _oft[fp].writePtr,
					(char*)buffer+(dataSize*(dataCount-leftToWrite)), written);
			leftToWrite -= written;

			//update ptr
			_oft[fp].writePtr += written;
			_oft[fp].filePtr += written;
			printf("Written to buffer size : %u left : %lu\n", written, leftToWrite);
		}
		else {
			written = remainingBlockSpace;
			memcpy(_oft[fp].buffer + _oft[fp].writePtr,
					(char*)buffer+(dataSize*(dataCount-leftToWrite)), written);
			leftToWrite -= written;

			//update ptr
			_oft[fp].writePtr += written - _fs->blockSize;
			_oft[fp].filePtr += written;

			printf("Written to block# %lu size : %u left : %lu\n", currentBlock, written, leftToWrite);

			//assign new block
			writeBlock(_oft[fp].buffer, currentBlock);
			currentBlock = findFreeBlock();
			markBlockBusy(currentBlock);
			setBlockNumInInode(_oft[fp].inodeBuffer, _oft[fp].filePtr, currentBlock);
			printf("Assigning  block# %lu\n", currentBlock);
		}
	}
	printf("FILE WRITTEN!\n");
	updateDirectoryFileLength(_oft[fp].filename, _oft[fp].filePtr);
	saveInode(_oft[fp].inodeBuffer, _oft[fp].inode);

}
// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
	printf("Flushing file\n");
	if (_oft[fp].openMode == MODE_READ_ONLY) {
		return;
	}

	if (_oft[fp].writePtr != 1) {
		memset(_oft[fp].buffer+_oft[fp].writePtr, 0, _fs->blockSize - _oft[fp].writePtr);
		writeBlock(_oft[fp].buffer, returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].filePtr));
	}
	updateDirectory();
	updateFreeList();
	saveInode(_oft[fp].inodeBuffer, _oft[fp].inode);

}


// Read data from the file.
//buffer: blockSize
int readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
	long remainingToRead = dataCount;
	unsigned long blockNum;
	unsigned long totalRead = 0;

	//if there are less than dataCount left to read
	if(dataCount > _oft[fp].filePtr - _oft[fp].readPtr) {
		remainingToRead = _oft[fp].filePtr - _oft[fp].readPtr;
	}


	printf("Reading File from Block\tReadPtr : %lu\tFilePtr : %lu\n",
			_oft[fp].readPtr, _oft[fp].filePtr);

	//stop until desired count or end of file reached
	while(remainingToRead > 0 && _oft[fp].readPtr < _oft[fp].filePtr){
		//get the blockNum
		blockNum = returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].readPtr);
		//read 
		readBlock(_oft[fp].buffer, blockNum);
		unsigned int read;
		if (remainingToRead < dataCount ) {
			read = remainingToRead;
		} else {
			read = dataCount;
		}
		memcpy(buffer, _oft[fp].buffer, read);

		//update curr pos
		_oft[fp].readPtr += read;
		remainingToRead -= read;
		printf("ReadPtr : %u\tRead: %u\tBufferPtr: %d\n",
				_oft[fp].readPtr, read, buffer + sizeof(char)*_oft[fp].readPtr);
		totalRead += read;
	}
	return totalRead;
}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set. 
// See TDirectory structure.
void delFile(const char *filename) {

}

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp) {
	//flush modified buffers to disk
	flushFile(fp);
	
	// release buffer
	free(_oft[fp].buffer);
	free(_oft[fp].inodeBuffer);
	
	//update file descriptor
	updateDirectoryFileLength(_oft[fp].filename, _oft[fp].filePtr);
	// Write the free list
	updateFreeList();
	// Write the diretory
	updateDirectory();

	//free OFT entry
	for (int i = fp; i < _oftCount; ++i)
		_oft[i] = _oft[i + 1];
	
	_oftCount--;
}

// Unmount file system.
void closeFS() {
	for (int i = 0; i++; i < _oftCount) {
		closeFile(i);
	}
	unmountFS();
	free(_oft);
}

int setupOftEntry (const char* filename, int inode, long filesize, int mode) {
	int i = _oftCount;

	_oft[i].filename = filename;
	_oft[i].openMode = mode;
	_oft[i].blockSize = _fs->blockSize;

	unsigned long* inodeBuffer = makeInodeBuffer();
	loadInode(inodeBuffer, inode);
	_oft[i].inode = inode;
	_oft[i].inodeBuffer = inodeBuffer;
	_oft[i].buffer = makeDataBuffer();
	_oft[i].readPtr = 0;
	if (mode == MODE_READ_APPEND) {
		_oft[i].writePtr = filesize % _fs->blockSize;
		//set buffer to last block
		readBlock(_oft[i].buffer, returnBlockNumFromInode(_oft[i].inodeBuffer, filesize));
	} else {
		_oft[i].writePtr = 0;
	}
	_oft[i].filePtr = filesize;
	_oftCount++;
	return i;
}
