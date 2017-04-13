#include "libefs.h"

int main(int ac, char **av)
{
	if (ac != 2)
	{
		printf("\nUsage: %s <file to check>\n", av[0]);
		printf("Prints: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	}
	else {
		// Mount the file system
	    mountFS("part.dsk", "2106s2");
		unsigned int index = getattr(av[1]);
//		unsigned bitswanted = 3;
		if (index != FS_FILE_NOT_FOUND) {

			int mask = 1 << 2;
			int masked_n = index & mask;
			int thebit = masked_n >> 2;


			if (thebit == 1) {
				printf("R\n");
			}
			else {
				printf("W\n");
			}
		}
		else {
			printf("FILE NOT FOUND\n");
		}
	}

	return 0;
}
