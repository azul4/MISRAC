//====================================================================================
// 구조체명 : Opcode
// 목적 코드(명령어)를 수행합니다.
//====================================================================================
typedef enum codes {    // 명령어 코드
    lit, opr, lod, sto, cal, ret, ict, jmp, jpc
} OpCode;


typedef enum ops {      // 연산 명령 코드
    neg, add, sub, mul, my_div, odd, eq, ls, gr,
    neq, lseq, greq, wrt, wrl
} Operator;


//====================================================================================
// 함수명 : genCodeV, genCodeT, genCodeO, genCodeR
// genCodeV:    명령어 생성, 주소부에 V
// genCodeT:    명령어 생성, 주소는 이름 테이블에서
// genCodeO:    명령어 생성, 주소부에 연산 명령
// genCodeR:    ret 명령어 생성
//====================================================================================
int     genCodeV    (OpCode op, int v);
int     genCodeT    (OpCode op, int ti);
int     genCodeO    (Operator p);
int     genCodeR    ();

//====================================================================================
// 함수명 : backPatch
// 명령어 백패치
//====================================================================================
void    backPatch   (int i);

//====================================================================================
// 함수명 : nextCode
// 다음 명령어의 주소를 리턴합니다.
//====================================================================================
int     nextCode    ();

//====================================================================================
// 함수명 : listCode
// 목적 코드를 출력합니다
//====================================================================================
void    listCode    ();

//====================================================================================
// 함수명 : execute
// 목적 코드(명령어)를 수행합니다.
//====================================================================================
void    execute     ();