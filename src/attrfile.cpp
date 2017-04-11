#include <string>
#include "libefs.h"

using namespace std;
int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check set attrbute> <attribute>\n", av[0]);
		printf("Attribute: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	} else {
		int index = findFile(av[1]);

		if(index == FS_FILE_NOT_FOUND) {
			printf("File NOT FOUND\n");
		} 
		string attr = av[2];
		unsigned int attrStored = getattr(av[1]);
		if(attr == "R" || attr == "r"){
			attrStored |= 1 << 2;
			setattr(av[1], attrStored);
		} else if(attr == "w" || attr  == "W") {
			attrStored &= ~(1 << 2);
			//set bit 2 to 0
			setattr(av[1], attrStored);

		}
		
	}

	return 0;
}
