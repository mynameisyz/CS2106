#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
		return -1;
	}
	initFS("part.dsk", av[2]);
	unsigned int fp = openFile(av[1], MODE_READ_ONLY);
	FILE *toWrite = fopen(av[1], "w+");
	char *buffer = (char*)calloc(sizeof(char), getFileLength(av[1]));


	if (_result == FS_FILE_NOT_FOUND) {
		printf("FILE NOT FOUND\n");
		return -1;
	}
	readFile(fp, buffer, sizeof(char), getFileLength(av[1]));
	int written = fwrite(buffer, sizeof(char), getFileLength(av[1]), toWrite);
	printf("Size written: %d\n", written);

	//DELETE FILE?

	fclose(toWrite);
	free(buffer);
	closeFS();
	printf("Checked Out: %d\n", fp);

	return 0;
}
