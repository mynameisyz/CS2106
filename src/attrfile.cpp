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
		printf("av[1] : %s\n", av[1]);
		//		int index = openFile(av[1], MODE_CREATE);
		initFS("part.dsk", "password");
		int index = findFile(av[1]);

		if(index == FS_FILE_NOT_FOUND) {
			printf("File NOT FOUND\n");
		} 
		string attr = av[2];
		unsigned int attrStored = getattr(av[1]);
		if(attr == "R" || attr == "r"){
//			printf("before R attr: %d\n", attrStored);
//			printf("before R 2attr: %d\n", (attrStored >> 2) & 1);
			attrStored |= 1 << 2;
			setattr(av[1], attrStored);
//			printf("attr: %d\n", getattr(av[1]));
//			printf("2attr: %d\n", (getattr(av[1]) >> 2) & 1);
		} else if(attr == "w" || attr  == "W") {
//			printf("before W attr: %d\n", attrStored);
//			printf("before w 2attr: %d\n", (attrStored >> 2) & 1);
			attrStored &= ~(1 << 2);
			//set bit 2 to 0
			setattr(av[1], attrStored);
//			printf("attr: %d\n", attrStored);
		}
		updateDirectory();
	}

	return 0;
}
