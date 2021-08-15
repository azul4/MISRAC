#include <stdio.h>
#define DEBUG
#define MAXLINE 120

static FILE *fpi;
static FILE *fpo;
static char line[MAXLINE];
static int lineIndex = -1;

char nextChar()
{
    char ch;
    if(lineIndex == -1) {
        if(fgets(line, MAXLINE, fpi) != NULL) {
            fputs(line, fpo);
            lineIndex = 0;
        } else {
            printf("EOF\n");
            return 0;
        }
    }

    if( (ch = line[lineIndex++]) == '\n') {
        lineIndex = -1;
        return ' ';
    }
    return ch;
}

void read_voca()
{
    while(ch == ' ') ch = nextChar();
    state1:
        k = 0;
        if(charClassT[ch] == letter)
            goto state2;
        if(charClassT[ch] == digit)
            goto state3;
        if(charClass[ch] == delimiter)
            goto state4;
        else
            error();
    
    state2:
        //a[k++] = ch;
        ch = nextChar();
        if(charClassT[ch] == letter || charClassT[ch] == digit)
            goto state2;
        else
            goto state5;
    state3:
        if(charClassT[ch] == digit)
            goto state3;
        else
            goto state5;
    state4:
        ch = nextChar();
        goto state5;
    
    state5:

}

void backChar()
{
    lineIndex--;
}


#ifdef DEBUG
int main()
{
    fpi = fopen("../totest.c", "r");
    fpo = fopen("../compiler.txt", "w");
    nextChar();
}
#endif

