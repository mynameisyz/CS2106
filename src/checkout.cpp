#include "libefs.h"

int main(int ac, char **av)
{
	int debug = 1;
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
		return -1;
	}
	//setup
	initFS("part.dsk", av[2]);
	TFileSystemStruct *fs = getFSInfo();
	unsigned int index = openFile(av[1], MODE_READ_ONLY);

	if (_result == FS_FILE_NOT_FOUND) {
			printf("FILE NOT FOUND\n");
			return -1;
	}


	FILE *fp = fopen(av[1], "a");
	char *buffer = (char*)calloc(sizeof(char), fs->blockSize);
	long remaining = getFileLength(av[1]);
	unsigned long read, written;

	//read from efs
	while (remaining > 0) {
		read = readFile(index, buffer, sizeof(char), fs->blockSize);
		written = fwrite(buffer, sizeof(char), read, fp);
		remaining -= read;
		if(debug)
			printf("Checkout - Written: %lu\tRead: %lu\t Remaining: %ld\n", written, read, remaining);
		if(read < fs->blockSize) {
			continue;
		}
	}
	delFile(av[1]);

	//Close
	fclose(fp);
	free(buffer);
	closeFS();

	return 0;
}
