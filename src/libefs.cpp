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
	_oft = calloc(sizeof(TOpenFile), _fs->maxFiles);
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
			printf("ERR CODE: %s" + _result);
			return -1;
		}
		break;
	}
	case MODE_CREATE: {
		if (_result == FS_OK ) {
			//file exists
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
				printf("ERR CODE: %s" + _result);
				return -1;
			}
		}
		break;
	}
	case MODE_READ_ONLY: {
		if (_result == FS_OK) {
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		} else {
			printf("ERR CODE: %s" + _result);
			return -1;
		}
		break;
	}
	case MODE_READ_APPEND: {
		if(_result == FS_OK) {
			return setupOftEntry(filename, inode, getFileLength(filename), mode);
		}
		else {
			printf("ERR CODE: %s" + _result);
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

	unsigned long leftToWrite = dataSize * dataCount;
	unsigned long currentBlock;

	while (leftToWrite > 0) {
		currentBlock = returnBlockNumFromInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);

		if (currentBlock == 0) {
			//assign new block
			currentBlock = findFreeBlock();
			markBlockBusy(currentBlock);
			setBlockNumInInode(_oft[fp].inodeBuffer, _oft[fp].filePtr, currentBlock);
		}
		unsigned int written, remainingBlockSpace;
		remainingBlockSpace = _fs->blockSize - _oft[fp].writePtr;
		//if we have enough space left in buffer
		if ( remainingBlockSpace > leftToWrite) {
			written = leftToWrite;
			strncpy(_oft[fp].buffer + _oft[fp].writePtr,
					(char*)buffer+(dataSize*dataCount-leftToWrite), written);
			leftToWrite -= written;

			//update ptr
			_oft[fp].writePtr += written;
			_oft[fp].filePtr += written;
		}
		else {
			written = remainingBlockSpace;
			strncpy(_oft[fp].buffer + _oft[fp].writePtr,
					(char*)buffer+(dataSize*dataCount-leftToWrite), written);
			leftToWrite -= written;

			//update ptr
			_oft[fp].writePtr += written - _fs->blockSize;
			_oft[fp].filePtr += written;

			//assign new block
			writeBlock(_oft[fp].buffer, currentBlock);
			currentBlock = findFreeBlock();
			markBlockBusy(currentBlock);
			setBlockNumInInode(_oft[fp].inodeBuffer, _oft[fp].filePtr, currentBlock);
		}
	}
	updateDirectoryFileLength(_oft[fp].filename, _oft[fp].filePtr);
	saveInode(_oft[fp].inodeBuffer, _oft[fp].inode);

}
// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
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
		_oft[i].filePtr = filesize;
		//set buffer to last block
		readBlock(_oft[i].buffer, returnBlockNumFromInode(_oft[i].inodeBuffer, filesize));
	} else {
		_oft[i].writePtr = 0;
		_oft[i].filePtr = 0;
	}
	_oftCount++;
	return 1;
}
