#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3
#define FIRSTADDR 2

static Token token;
static void block(int pIndex);
static void constDecl   (void);
static void varDecl     (void);
static void funcDecl    (void);
static void statement   (void);
static void expression  (void);
static void term        (void);
static void factor      (void);
static void condition   (void);
static int  isStBeginKey(Token t);

int compile()
{
    int i = 0;

    printf("start compilation\n");
    initSource();           //getSource 초기설정
    token = nextToken();    //첫 토큰
    blockBegin(FIRSTADDR);  //이후 선언은 새로운 블록
    block(0);               //0은 dummy
    finalSource();          
    i = errorN();           //오류 메세지 개수
    if (i != 0) {
        printf("%d errors\n", i);
    }
    //listCode();           //.o 코드 출력(필요한 경우)

    return i < MINERROR;    //오류 메세지의 개수가 적은지 확인
}


#endif