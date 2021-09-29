#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif

#include "codegen.h"

#define MINERROR  3
#define FIRSTADDR 2

//====================================================================================
// 다음 토큰을 넣어둘 변수
//====================================================================================
static Token token;


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
// 함수명 : main
// 
//====================================================================================
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

//====================================================================================
// 함수명 : compile
// 
//====================================================================================
int compile()
{
    int i = 0;

    printf("start compilation\n");
    initSource();           //getSource 초기설정
    token = nextToken();    //첫 토큰
    blockBegin(FIRSTADDR);  //이후 선언은 새로운 블록 
    block(0);               //0은 dummy              //정의 안되어있음
    finalSource();                                   //정의 안되어있음
    i = errorN();           //오류 메세지 개수
    if (i != 0) {
        printf("%d errors\n", i);
    }
    //listCode();           //.o 코드 출력(필요한 경우)

    return i < MINERROR;    //오류 메세지의 개수가 적은지 확인
}


//====================================================================================
// 함수명 : block
// 인자   : pIndex , 해당 블록 함수 이름의 인덱스
//====================================================================================
void block(int pIndex)          
{
    int backP;
    
    backP = genCodeV(jmp, 0);  //내부 함수를 점프하는 명령, 이후에 백패치

    while(1) {
        switch(token.kind) {
            case Const  :  token = nextToken(); constDecl(); continue;
            case Var    :  token = nextToken(); varDecl();   continue;
            case Func   :  token = nextToken(); funcDecl();  continue;
            default : break;
        }
        break;
    }
    backPatch(backP);             // 내부 함수를 점프하는 명령으로 패치
    changeV(pIndex, nextCode());  // 그 함수의 시작 주소를 수정
    genCodeV(ict, frameL());      // 그 블록의 실행 때에 필요한 기억 영역을 잡는 명령
    statement();                  // 그 블록의 메인 문장
    genCodeR();                   // 리턴 명령
    blockEnd();                   // 블록이 끝났다는 것을 table에 전달
}

//====================================================================================
// 함수명 : constDecl
// 상수 선언 컴파일
//====================================================================================
void constDecl()
{
    Token temp;

    while (1) {
        if(token.kind == Id) {
            setIdKind(constId); // 출력을 위한 정보 설정
            temp = token;       // 이름을 넣어 둠
            token = checkGet(nextToken(), Equal); //이름 다음은 무조건 '='
            if(token.kind == Num) {
                enterTconst(temp.u.id, token.u.value); //상수 이름과 값을 테이블에 저장
            } else {
                errorType("number"); 
            }
            token = nextToken();
        } else {
            errorMissingId();
        }

        if(token.kind != Comma) { // 토큰의 종류가 ','라면 상수 선언이 이어짐.
            if(token.kind == Id) { // 다음이 이름이라면 쉼표를 잊었다는 의미.
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }
        token = nextToken();
    }

    token = checkGet(token, Semicolon); // 마지막은 ";"
}

//====================================================================================
// 함수명 : varDecl
// 변수 선언 컴파일
//====================================================================================
void varDecl()
{
    while(1) {
        if(token.kind == Id) {
            setIdKind(varId);
            enterTvar(token.u.id);
            token = nextToken();
        }
        else {
            errorMissingId();
        }

        if(token.kind != Comma) {
            if(token.kind == Id) {
                errorInsert(Comma);
                continue;
            }
            else {
                break;
            }
        }
        token = nextToken();
    }
    token = checkGet(token, Semicolon);
}

//====================================================================================
// 함수명 : funcDecl
// 함수 선언 컴파일
//====================================================================================
void funcDecl()
{
    int fIndex;

    if(token.kind == Id) {
        setIdKind(funcId); // 출력을 위한 정보설정.
        fIndex = enterTfunc(token.u.id, nextCode()); //함수 이름을 테이블에 등록. 목적 주소는 일단 다음 코드의 nextCode()로
        token = checkGet(nextToken(), Lparen);
        blockBegin(FIRSTADDR);  //매개변수 이름의 레벨은 함수 블록과 같음

        while(1) {
            if(token.kind == Id) {
                setIdKind(parId);
                enterTpar(token.u.id);
                token = nextToken();
            } else {
                break;
            }

            if(token.kind != Comma) {
                if(token.kind == Id) {
                    errorInsert(Comma);
                    continue;
                } else {
                    break;
                }
            }
            token = nextToken();
        }
        token = checkGet(token, Rparen); // 마지막은 ")"
        endpar(); //매개변수부가 끝났다는 것을 테이블에 전달
        if(token.kind == Semicolon) {
            errorDelete();
            token = nextToken();
        }
        block(fIndex);
        token = checkGet(token, Semicolon);
    } else {
        errorMissingId(); //함수 이름이 아님.
    }
}


//====================================================================================
// 함수명 : statement
// 문장 컴파일
//====================================================================================
void statement()
{
    int tIndex  = 0;
    KindT k     = 0;
    int backP   = 0;
    int backP2  = 0;

    while(1) {
        switch(token.kind) {
            case Id: {
                // C언어에는 할당문이 없음
            }
            //=============================================================
            // if 문 컴파일
            //=============================================================            
            case If:{ 
                token = nextToken();
                condition();
                token = checkGet(token, Then); // then 그런데 필요 없을수도
                backP = genCodeV(jpc, 0);      // jpc 명령
                statement();                   // 문장 컴파일
                backPatch(backP);              // 위의 jpc 명령에 백패치
                return;
            }

            //=============================================================
            // return문 컴파일
            //=============================================================
            case Ret:{   
                token = nextToken();    
                expression();                   // 식 컴파일
                genCodeR();                     // ret 명령
                return;
            }
            case Begin:{
                // begin .. end 문장 없음
            }
            //=============================================================
            // while문 컴파일
            //=============================================================            
            case While:{
                token = nextToken();
                backP2 = nextCode();            // while 문장의 가장 뒤로 jmp하는 명령의 이동 목적지
                condition();                    // 조건문 컴파일
                token = checkGet(token, Do);    // do가 아닐때
                backP = genCodeV(jpc, 0);       // 조건식이 거짓일 때 점프하는 jpc 명령
                statement();                    // 문장 컴파일
                genCodeV(jmp, backP2);          // while 문장의 맨 앞으로 점프하는 명령
                backPatch(backP);               // jpc 명령의 jump 위치를 백패치
                return;
            }


            case Write:{
                // write 문장 없음
            }
            case WriteLn:{
                // writeln 문장 없음
            }
            case End:
            case Semicolon: {
                return;
            }
            default:{
                errorDelete();                  //문장 맨 앞의 키까지 버림. 읽어들인 토큰을 버림.
                token = nextToken();
                continue;
                
            }
        }
    }
}

//====================================================================================
// 함수명 : isStBeginKey
// 토큰 t가 문장 맨 앞의 키워드일때
//====================================================================================
int isStBeginKey(Token t)
{
    switch(t.kind) {
        case If:
        case Begin:
        case Ret:
        case While:
        case Write:
        case WriteLn: {
            return 1;
        }
        default: {
            return 0;
        }
    }
}

//====================================================================================
// 함수명 : expression
// 식 컴파일
//====================================================================================
void expression()
{
    KeyId k;
    k = token.kind;

    if(k == Plus || k == Minus) {
        token = nextToken();
        term();
        if(k == Minus) {
            genCodeO(neg);
        } 
    }
    else term();

    k = token.kind;
    while (k == Plus || k == Minus) {
        token = nextToken();
        term();
        if(k == Minus) {
            genCodeO(sub);
        } else {
            genCodeO(add);
        }
        k = token.kind;
    }
}

//====================================================================================
// 함수명 : term
// 식의 항 컴파일
//====================================================================================
void term()
{
    KeyId k;
    factor();
    k = token.kind;
    while (k == Mult || k == Div) {
        token = nextToken();
        factor();
        if (k == Mult)  genCodeO(mul);
        else            genCodeO(my_div);
        k = token.kind;
    }
}

//====================================================================================
// 함수명 : factor
// 식의 인자 컴파일
//====================================================================================
void factor()
{
    int     tIndex;
    int     i;
    KeyId   k;

    if(token.kind == Id) { //
        tIndex = searchT(token.u.id, varId);
        setIdKind(k = kindT(tIndex)); // 출력을 위한 정보 설정
        switch(k) {
            case varId:
            case parId: {
                genCodeT(lod, tIndex);
                token = nextToken();
                break;
            }
            case constId: {
                genCodeV(lit, val(tIndex));
                token = nextToken();
                break;
            }
            case funcId: {
                token = nextToken();
                if(token.kind == Lparen) {
                    i = 0;
                    token = nextToken();
                    if(token.kind != Rparen) {
                        for( ;; ) {
                            expression(); i++; //실제 매개변수 컴파일
                            if(token.kind == Comma) {  // 다음이 쉼표라면 실제 매개변수가 이어짐
                                token = nextToken();
                                continue;
                            }
                            token = checkGet(token, Rparen);
                            break;
                        }
                    } else {
                        token = nextToken();
                    }
                    if (pars(tIndex) != i) errorMessage("\\#par"); //pars(tIndex)는 임시 매개변수의 개수)
                } else {
                    errorInsert(Lparen);
                    errorInsert(Rparen);
                }
                genCodeT(cal, tIndex); //call 명령
                break;
            }
        }
    }
    else if (token.kind == Num) { //상수
        genCodeV(lit, token.u.value);
        token = nextToken();
    }
    else if (token.kind == Lparen) { // 인자
        token = nextToken();
        expression();
        token = checkGet(token, Rparen);
    }
    
    //==========================================================================
    // 인자 뒤가 또 인자이면 오류
    //==========================================================================
    switch(token.kind) { 
        case Id: case Num: case Lparen: {
            errorMissingOp();
            factor();
        }
        default: return;
    }
}

//====================================================================================
// 함수명 : condition
// 조건식 컴파일
//====================================================================================
void condition()
{
    KeyId k;
    if(token.kind == Odd) {
        token = nextToken();
        expression();
        genCodeO(odd);
    } else {
        expression();
        k = token.kind;
        switch(k) {
            case Equal:
            case Lss:
            case Gtr:
            case NotEq:
            case LssEq:
            case GtrEq:
            break;

            default:
                errorType("rel-op");
                break;
        }

        token = nextToken();
        expression();
        switch(k) {
            case Equal: genCodeO(eq); break;
            case Lss:   genCodeO(ls); break;
            case Gtr:   genCodeO(gr); break;
            case NotEq: genCodeO(neq); break;
            case LssEq: genCodeO(lseq); break;
            case GtrEq: genCodeO(greq); break;

        }
    }
}
