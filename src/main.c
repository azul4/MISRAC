#include <stdio.h>
#include "getSource.h"

int main()
{
    char fileName[30] = "program.latex";
    printf("program name = %s\n", fileName);
    if (!openSource(fileName)) {
        printf("open error\n");
        return 1;
    } 
    if (compile()) {
        execute(); 
    }
    closeSource();
}