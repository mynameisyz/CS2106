#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 2)
	{
		printf("\nUsage: %s <file to delete>\n\n", av[0]);
		return -1;
<<<<<<< HEAD
=======
	} else {
		initFS("part.dsk", "whatever");

		delFile(av[1]);

		if(_result == FS_FILE_NOT_FOUND)
		{
			printf("FILE NOT FOUND\n");
		}
		
>>>>>>> branch 'master' of https://github.com/mynameisyz/CS2106.git
	}

	initFS("part.dsk", "whatever");

	delFile(av[1]);

	return 1;
}
