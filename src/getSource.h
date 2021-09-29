#ifndef TBL
#define TBL
#include "table.h"
#endif

#define MAXNAME 31 //이름의 최대 길이

#include <stdio.h>

typedef enum keys {
    Begin, End, If, Then, While, Do, Ret, Func, Var, Const, Odd, Write, WriteLn,
    end_of_KeyWd, 
    Plus, Minus, Mult, Div, Lparen, Rparen, Equal, Lss, Gtr, NotEq, LssEq, GtrEq, Comma, Period, Semicolon, Assign, 
    end_of_KeySym,
    Id, Num, nul,
    end_of_Token,
    letter, digit, colon, others
} KeyId;

typedef struct token {
    KeyId kind;
    union {
        char id[MAXNAME];
        int value;
    } u;
}Token;

Token       nextToken       ();
Token       checkGet        (Token t, KeyId k);


int         openSource      (char fileName[]); // 소스 파일 열기
void        closeSource     (void);
void        initSource      ();
void        finalSource     ();

void        errorType       (char *m);
void        errorInsert     (KeyId k);
void        errorMissingId  (); //이름이 없다는 메시지를 html 파일에 삽입
void        errorMissingOp  ();
void        errorDelete     ();
void        errorMessage    (char * m);
void        errorF          (char * m);
int         errorN          ();

void        setIdKind       (KindT k);

void        errorNoCheck    ();
