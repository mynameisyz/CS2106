#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check>\n", av[0]);
		printf("Prints: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	}
	else {
		unsigned int index = getattr(av[1]);
		unsigned bitswanted = 3;
		if(index != FS_FILE_NOT_FOUND) {
			unsigned *bits = (unsigned*)malloc(sizeof(int) * bitswanted);

			int k;
			for(k=0; k < bitswanted; k++){
				int mask =  1 << k;
				int masked_n = 3 & mask;
				int thebit = masked_n >> k;
				bits[k] = thebit;
			}

			if(bits[2] == 1) {
				printf("R\n");
			} else {
				printf("W\n");
			}
		} else {
			printf("FILE NOT FOUND\n");
		}
	}

	return 0;
}
