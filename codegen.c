#include <stdio.h>
#include "codegen.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXMEM      2000    // 실행할 때 스택의 최대 크기
#define MAXLEVEL    5       // 블록 최대 깊이
#define MAXCODE     200     // 목적 코드의 최대 길이

//====================================================================================
// 구조체명 : Inst
// 명령어의 형태를 기록합니다.
//====================================================================================
typedef struct inst {
    Opcode opCode;
    union {
        RelAddr addr;
        int value;
        Operator optr;
    } u;
} Inst;

static Inst code[MAXCODE];  // 목적 코드의 집합

//====================================================================================
// 함수명 : execute
// 목적 코드(명령어)를 수행합니다.
//====================================================================================
void execute()
{
    int stack[MAXMEM]           = {0};   // 실행 스택
    int display [MAXLEVEL]      = {0};   // 현재 보이는 블록 맨 앞 주소의 디스플레이
    int pc                      = 0;     // Program Counter
    int top                     = 0;     // Stack Top
    int lev                     = 0;        
    int temp                    = 0;        
    Inst i                      = {0};     // 실행할 명령

    printf("start execution\n");
    top         = 0;
    pc          = 0;
    stack[0]    = 0;
    stack[1]    = 0;
    display[0]  = 0;

    //====================================================================================
    // Program Counter가 0이 될때까지 반복해서 실행합니다.
    //====================================================================================
    do {
        i = code[pc++];
        switch(i.opCode) {
            case lit:
            case lod:
            case sto : {

            }
            case cal :  {

            }
            case ret:
            case ict:
            case jmp:
            case jpc:
            case opr:
            switch(i.u.optr) {
                case neg:
                case add:
                case sub:
                case mul:
                case div:
                case odd:
                case eq:
                case ls:
                case gr:
                case neq:
                case lseq:
                case greq:
                case wrt:
                case wrl:
                default: break;
            }

        }

    }while(pc != 0);
}