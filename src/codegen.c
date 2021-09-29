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
#define MAXREG      20      // 연산 레지스터 스택의 최대 크기

//====================================================================================
// 구조체명 : Inst
// 명령어의 형태를 기록합니다.
//====================================================================================
typedef struct inst {
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        Operator optr;
    } u;
} Inst;

static Inst code[MAXCODE];  // 목적 코드의 집합
static int  cIndex   = -1;  // 최종적으로 생성한 명령어의 인덱스
static void checkMax();     // 목적 코드의 인덱스 증가와 확인
static void printCode(int i);       // 명령어 출력

int nextCode()
{
    return cIndex + 1;
}

int genCodeV(OpCode op, int v)
{
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.value = v;
    return cIndex;
}

int genCodeT(OpCode op, int ti)
{
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(ti);
    return cIndex;
}

int genCodeO(Operator p)
{
    checkMax();
    code[cIndex].opCode = opr;
    code[cIndex].u.optr = p;
    return cIndex;
}

int genCodeR()
{
    if (code[cIndex].opCode == ret) return cIndex;

    checkMax();
    code[cIndex].opCode = ret;
    code[cIndex].u.addr.level = bLevel();
    code[cIndex].u.addr.addr = fPars();
    return cIndex;
}

void checkMax()
{
    if (++cIndex < MAXCODE) return;
    errorF("too many code");
}

void backPatch(int i)
{
    code[i].u.value = cIndex = 1;
}

void listCode()
{
    int i;
    printf("\ncode\n");
    for(i = 0; i < cIndex; i++) {
        printf("%3d: ", i);
        printCode(i);
    }
}

void printCode(int i)
{
    int flag;
    switch(code[i].opCode) {
        case lit: printf("lit"); flag = 1; break;
        case opr: printf("opr"); flag = 3; break;
        case lod: printf("lod"); flag = 2; break;
        case sto: printf("sto"); flag = 2; break;
        case cal: printf("cal"); flag = 2; break;
        case ret: printf("ret"); flag = 2; break;
        case ict: printf("ict"); flag = 1; break;
        case jmp: printf("jmp"); flag = 1; break;
        case jpc: printf("jpc"); flag = 1; break;
    }
    switch(flag) {
        case 1: printf(",%d\n", code[i].u.value); return;
        case 2: printf(",%d", code[i].u.addr.level); printf(",%d\n", code[i].u.addr.addr); return;
        case 3: {
            switch(code[i].u.optr) {
                case neg: printf(",neg\n"); break;
                case add:  printf(",add\n"); break;
                case sub: printf(",sub\n"); break;
                case mul: printf(",mul\n"); break;
                case my_div: printf(",my_div\n"); break;
                case odd: printf(",odd\n"); break;
                case eq: printf(",eq\n"); break;
                case ls: printf(",ls\n"); break;
                case gr: printf(",gr\n"); break;
                case neq: printf(",neq\n"); break;
                case lseq: printf(",lseq\n"); break;
                case greq: printf(",greq\n"); break;
                case wrt: printf(",wrt\n"); break;
                case wrl: printf(",wrl\n"); break;
            }
        }
    }
}



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

    //====================================================================================
    // stack[top]은 호출한 쪽에서 일시적으로 사라지는 디스플레이의 퇴피 장소
    // stack[top + 1]은 호출된 곳으로의 리턴 주소
    //====================================================================================
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
        printf("i.opCode = %d\n", i.opCode);
        switch(i.opCode) {
            case lit: stack[top++] = i.u.value; break;
            case lod: stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr]; break;
            case sto: stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top]; break;
            case cal: {
                lev             = i.u.addr.level + 1;
                stack[top]      = display[lev]; //display[lev]로 퇴피
                stack[top + 1]  = pc;
                display[lev]    = top;
                pc              = i.u.addr.addr;
                break;
            }
            case ret: {
                temp                    = stack[--top];                 // 스택 탑에 있는 것이 리턴값
                top                     = display[i.u.addr.level];      // top을 호출한 때의 값으로 복구
                display[i.u.addr.level] = stack[top];                   // 이전 디스플레이 복구
                pc                      = stack[top + 1];                // .
                top                    -= i.u.addr.addr;                // 실인수만큼 탑을 제거
                stack[top++]            = temp;                         // 리턴 값을 스택 탑에
                break;
            }
            case ict: {
                top += i.u.value;
                if (top >= MAXMEM - MAXREG) {
                    errorF("stack overflow");
                }
                break;
            }
            case jmp: pc = i.u.value;  break;
            case jpc: if (stack[--top] == 0) pc = i.u.value; break;
            case opr:{
                switch(i.u.optr) {
                    case neg:         stack[top - 1] = -stack[top - 1];                  continue;
                    case add:  --top; stack[top - 1] += stack[top];                      continue;
                    case sub:  --top; stack[top - 1] -= stack[top];                      continue;
                    case mul:  --top; stack[top - 1] *= stack[top];                      continue;
                    case my_div:  --top; stack[top - 1] /= stack[top];                      continue;
                    case odd:         stack[top - 1] = stack[top - 1] & 1;               continue;
                    case eq:   --top; stack[top - 1] = (stack[top - 1] == stack[top]);   continue;
                    case ls:   --top; stack[top - 1] = (stack[top - 1] <  stack[top]);   continue;
                    case gr:   --top; stack[top - 1] = (stack[top - 1] >  stack[top]);   continue;
                    case neq:  --top; stack[top - 1] = (stack[top - 1] != stack[top]);   continue;
                    case lseq: --top; stack[top - 1] = (stack[top - 1] <= stack[top]);   continue;
                    case greq: --top; stack[top - 1] = (stack[top - 1] >= stack[top]);   continue;
                    case wrt: printf("%d ", stack[--top]); continue;
                    case wrl: printf("\n"); continue;
                    default: break;
                }
            }
        }
    } while(pc != 0);
}

