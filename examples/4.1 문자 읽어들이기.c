#include <stdio.h>
#define MAXLINE 120

static FILE *fpi; //소스 파일
static FILE *fpo; //컴파일러 출력 파일
static char line[MAXLINE]; //한줄 읽는 입력 버퍼
static int lineIndex = -1;  //다음 읽어들일 문자의 위치

char nextChar()
{
    char ch;

    if (lineIndex == -1) {
        if (fgets(line, MAXLINE, fpi) != NULL) {
            fputs(line, fpo);
            lineIndex = 0;
        } else {
            printf("end of file\n");
            exit(1);
        }
    }

    if ((ch = line[lineIndex++]) == '\n') {
        lineIndex = -1;
        return ' ';
    }

    return ch;
}

