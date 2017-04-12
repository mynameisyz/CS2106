#include "libefs.h"

int main(int ac, char **av)
{
		if (ac != 3)
		{
			printf("\nUsage: %s <filename> <password>\n\n", av[0]);
			exit(-1);
		}

		// Load the file system
		initFS("part.dsk", av[2]);
		TFileSystemStruct *fs = getFSInfo();
		int fp = openFile(av[1], MODE_CREATE);

		if (fp == -1)
		{
			printf("\nUnable to open source file %s\n\n", av[1]);
			return -1;
		}

		if (_result == FS_DUPLICATE_FILE) {
			printf("\nDuplicate file %s exist\n\n", av[1]);

		}
		char *buffer;

		// Allocate the buffer for reading
		buffer = makeDataBuffer();

		// Read the file
		readFile(fp, &buffer, sizeof(char), fs->blockSize);

		//write file
		writeFile(fp, &buffer, sizeof(char), fs->blockSize);
	
		flushFile(fp);

		// Free data and inode buffer
		closeFile(fp);

		// Unmount
		closeFS();
		return 0;
}
