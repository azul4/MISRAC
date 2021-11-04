#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif

#include "codegen.h"
#include "misra_error_list.h"

#define MINERROR  10000
#define FIRSTADDR 2

#define PREPROCESSOR_COMPILED 1
#define PREPROCESSOR_NOT_EXIST 0



int compile();

//====================================================================================
// 블록 컴파일
//====================================================================================
static void block       (int pIndex);  

//====================================================================================
// 상수 선언 컴파일
//====================================================================================
static void constDecl   (void);

//====================================================================================
// 변수 선언 컴파일
//====================================================================================
static void varDecl     (void);

//====================================================================================
// 함수 선언 컴파일
//====================================================================================
static void funcDecl    (void);

//====================================================================================
// 문장 컴파일
//====================================================================================
static void statement   (void);

//====================================================================================
//식 컴파일
//====================================================================================
static void expression  (void);

//====================================================================================
// 식의 항 컴파일
//====================================================================================
static void term        (void);

//====================================================================================
// 식의 인자 컴파일
//====================================================================================
static void factor      (void);

//====================================================================================
// 조건식 컴파일
//====================================================================================
static void condition   (void);

//====================================================================================
// 인자로 들어온 토큰 t는 문장 맨 앞의 키인가?
//====================================================================================
static int  isStBeginKey(Token t);

//====================================================================================
// 전처리기 컴파일부
//====================================================================================
int PreprocessorCompile();


void main_compile();

void PrintTokenInfo();