#include "getSource.h"

int main()
{
    char fileName[30] = {0};
    printf("enter .c file name: ");
    scanf("%s\n", fileName);
    if (!openSource(fileName)) {
        return 1;
    }
    if (compile()) {
        execute();
    }
    closeSource();
}

