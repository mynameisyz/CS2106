#include "libefs.h"

int main(int ac, char **av)
{
	int debug = 1;
	if (ac != 3)
	{
		printf("\nUsage: %s <filename> <password>\n\n", av[0]);
		exit(-1);
	}

	// Load the file system
	initFS("part.dsk", av[2]);
	TFileSystemStruct *fs = getFSInfo();
	unsigned int index = findFile(av[1]);
	if (index != FS_FILE_NOT_FOUND) {
		printf("DUPLICATE FILE\n");
		return -1;
	}

	FILE *fp = fopen(av[1], "r");
	int efp = openFile(av[1], MODE_CREATE);

	if (efp == -1)
	{
		printf("\nUnable to open source file %s\n\n", av[1]);
		return -1;
	}

	// Allocate the buffer for reading
	char *buffer = (char*)calloc(sizeof(char), fs->blockSize);

	//write file
	unsigned long len;
	int result;
	while((len= fread(buffer, sizeof(char), fs->blockSize, fp)) > 0)
	{
		if(debug)printf("Calling write with length : %lu\n", len);
		result = writeFile(efp, buffer, sizeof(char), len);
		if (result==-1){
			break;
		}
	}
	if(result==-1){
		printf("FS FULL");
		delFile(av[1]);
	}

	// Free data and inode buffer
	closeFile(efp);

	// Unmount
	closeFS();
	fclose(fp);
	free(buffer);
	return 0;
}
