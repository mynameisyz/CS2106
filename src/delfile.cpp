#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 2)
	{
		printf("\nUsage: %s <file to delete>\n\n", av[0]);
		return -1;
	}

	initFS("part.dsk", "whatever");

	delFile(av[1]);

	return 1;
}
