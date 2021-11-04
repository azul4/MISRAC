#include "compile.h"

//====================================================================================
// 다음 토큰을 넣어둘 변수
//====================================================================================
static Token token;

//====================================================================================
// 함수명 : main
//====================================================================================
int main()
{
    char fileName[30] = "source/2.c";
    printf("program name = %s\n", fileName);
    if (!openSource(fileName)) {
        printf("open error\n");
        return 1;
    } 
    if (compile()) {
        printf("execute() 실행예정\n");
        //execute(); 
    }
    closeSource();
    
}

//====================================================================================
// 함수명 : compile
// 
//====================================================================================
int compile()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int i = 0;

    initSource();           //getSource 초기설정
    token = nextToken();    //첫 토큰
    PrintTokenInfo();       //첫 토큰 정보 출력
    
    if (token.kind == Include) {
        PreprocessorCompile();
    }

    //====================================================================================
    // main() 컴파일 위하여 int를 받고, main일 경우 block() 호출.
    //====================================================================================
    token = nextToken();
    if (strcmp(token.u.id, "main") == 0) {
        printf("strcmp(token.u.id, 'main') == 0");
        main_compile();
        token = nextToken();
        blockBegin(FIRSTADDR);  //이후 선언은 새로운 블록 
        block(0);               //0은 dummy  
    }

            
    finalSource();                                   
    i = errorN();           //오류 메세지 개수
    if (i != 0) {
        printf("Errors occured in function %s\n", __FUNCTION__);
        printf("%d errors\n", i);
    }
    listCode();             //.o 코드 출력(필요한 경우)

    return i < MINERROR;    //오류 메세지의 개수가 적은지 확인
}

void main_compile()
{
    token = checkGet(nextToken(), Main);
    token = checkGet(nextToken(), Lparen);
    token = checkGet(nextToken(), Rparen);
    token = checkGet(nextToken(), LBracket);
}

//====================================================================================
// 함수명 : block
// 인자   : pIndex(해당 블록 함수 이름의 인덱스)
//====================================================================================
void block(int pIndex)          
{
    int backP;
    
    backP = genCodeV(jmp, 0);  //내부 함수를 점프하는 명령, 이후에 백패치
    
    if(strcmp(token.u.id, "int") == 0) {
        printf("strcmp(token.u.id, 'int') == 0");
        token = nextToken();
    }

    while(1) {
        printf("token.kind = %d\n", token.kind);
        printf("16 -> Int, 8->Var, 9->Func로 분기\n");
        switch(token.kind) {
            case Int    :  token = nextToken(); constDecl(); printf("block continue\n"); continue;
            case Var    :  token = nextToken(); varDecl();   printf("block continue\n"); continue;
            case Func   :  token = nextToken(); funcDecl();  printf("block continue\n"); continue;
            default : break; //begin의 경우 블록을 벗어나야 함.
        }
        break;
    }
    backPatch(backP);             // 내부 함수를 점프하는 명령으로 패치
    #if 0 //실행 예정인 부분
    changeV(pIndex, nextCode());  // 그 함수의 시작 주소를 수정
    genCodeV(ict, frameL());      // 그 블록의 실행 때에 필요한 기억 영역을 잡는 명령
    statement();                  // 그 블록의 메인 문장
    genCodeR();                   // 리턴 명령
    #endif
    blockEnd();                   // 블록이 끝났다는 것을 table에 전달
}


int PreprocessorCompile()
{
    Token temp            = token;        // 이전 토큰 저장
    int   pp_compiled     = PREPROCESSOR_NOT_EXIST;

    token = nextToken();
    printf("token.u.id = %s\n", token.u.id);
    printf("token.kind = %d\n", token.kind);
    if (Include == temp.kind) {
        token = nextToken();

        // temp에는 #include할 라이브러리 이름이 있음.
        if(0 == strcmp(token.u.id, "signal")) MISRA_C_ERROR(21, 4);
        printf("line %d token.u.id = %s\n", __LINE__, token.u.id);

        token = checkGet(token, Id);
        
        
        // MISRA-C에 위배되는 경우 에러 검출
        token       = checkGet(token, Period); // .
        token       = checkGet(token, Id);     // h
        token       = checkGet(token, Gtr);    // >
        pp_compiled = PREPROCESSOR_COMPILED;
    } else if (strcmp(temp.u.id, "define")) {
        ;
    }

    PrintTokenInfo();
    return pp_compiled;
}
//====================================================================================
// 함수명 : constDecl
// 상수 선언 컴파일
//====================================================================================
void constDecl()
{
    Token temp; // 변수 이름을 넣는 공간

    while (1) {
        //====================================================================================
        // int를 받고 변수명이 나올 경우
        //====================================================================================
        if(token.kind == Id) {
            setIdKind(constId); // 출력을 위한 정보 설정
            temp = token;       // 이름을 넣어 둠

            //====================================================================================
            // 다음 토큰 nextToken()을 받고 그 토큰이 의미하는 것이 무엇인지 파악함.
            //====================================================================================
            token = nextToken();

            if(temp.kind == Num) {
                enterTconst(temp.u.id, token.u.value); //상수 이름과 값을 테이블에 저장
            } else if (temp.kind == Comma) {
                MISRA_C_ERROR(8, 10);
            } else if (temp.kind == Semicolon){
                break;
            } else {
                errorType("number");
            }
        }
        else if (token.kind == Comma) {
            //MISRA_C_ERROR();
            token = nextToken();
            continue;
        }
        else {
            errorMissingId();
        }

        //====================================================================================
        // 토큰의 종류가 ','가 아니라면 선언이 계속 이어짐.
        //====================================================================================
        if(token.kind != Comma) {  
            printf("토큰의 종류가 ','가 아님.\n");
            if(token.kind == Id) { // 다음이 이름이라면 쉼표를 잊었다는 의미.
                printf("토큰이 ',' 가 아니면서 Id 선언이 이어짐\n");
                //errorInsert(Comma);
                continue;
            } else {
                break;
            }
        } 
        else { // token.kind == Comma이어서 Comma 사용 금지 Rule에 어긋남
            //MISRA_C_ERROR();
            token = nextToken();
            //continue;
        }
    }
    token = checkGet(token, Semicolon); // 마지막은 ";"
}

//====================================================================================
// 함수명 : varDecl
// 변수 선언 컴파일
//====================================================================================
void varDecl()
{
    int value = 0;
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    while(1) {
        printf("%s에서 무한반복\n", __FUNCTION__);
        if(token.kind == Id) {
            setIdKind(varId);
            enterTvar(token.u.id);
            token = nextToken();
        }
        else {
            errorMissingId();
        }

        if(token.kind != Comma) {  // 다음이 쉼표라면 할당 없이 선언이 이어짐. MISRA-C 위배
            if(token.kind == Id) { // 만약 다음이 이름이라면 쉼표를 잊었단 의미
                errorInsert(Comma);
                continue;
            }
            else if (token.kind == Assign) {
                //assignValueToVar(token);
            }
            else {
                //error();
                break;
            }
        }
        token = nextToken();
    }
    printf("loop 빠져나옴\n");
    //====================================================================================
    // 토큰의 마지막은 Semicolon
    //====================================================================================
    token = checkGet(token, Semicolon);
}

//====================================================================================
// 함수명 : funcDecl
// 함수 선언 컴파일
// 유일하게 block()을 부르는 곳임
//====================================================================================
void funcDecl()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int fIndex;

    if(token.kind == Id) {
        setIdKind(funcId); // 출력을 위한 정보설정.
        fIndex = enterTfunc(token.u.id, nextCode()); //함수 이름을 테이블에 등록. 목적 주소는 일단 다음 코드의 nextCode()로
        token = checkGet(nextToken(), Lparen);
        blockBegin(FIRSTADDR);  //매개변수 이름의 레벨은 함수 블록과 같음
        
        //====================================================================================
        // 매개변수를 comma를 이용해 구분하므로 comma 이외의 관련 정보를 저장함.
        // 매개변수가 5개가 넘으면 안된다는 조항을 넣으면 좋을 것 같음.
        //====================================================================================
        while(1) {
            if(token.kind == Id) {
                setIdKind(parId);           //현재 토큰의 종류는 parId(idKind = parId)
                enterTpar(token.u.id);
                token = nextToken();
                printf("LINE %d funcDecl에서 다음에 처리할 token = %s", __LINE__, token.u.id);
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
            printf("line %d funcDecl에서 다음에 처리할 token = %s", __LINE__, token.u.id);
            int dummy;
            scanf("%d", &dummy);
        }
        token = checkGet(token, Rparen); // 매개변수 선언의 마지막은 ")"
        printf("checkGet() <- token : %s", token.u.id);
        endpar(); //매개변수부가 끝났다는 것을 테이블에 전달

        //만약 읽어들인 다음 토큰이 Semicolon(;)일 경우 오타이므로 버리고 TokenGet.
        if(token.kind == Semicolon) {
            errorDelete();
            token = nextToken();
        }
        
        block(fIndex);                      //블록 컴파일, 함수 이름의 인덱스를 전달
        token = checkGet(token, Semicolon); //마지막은 ;
    } 
    //====================================================================================
    //token.kind가 Id가 아닐 경우 함수 이름이 아님.
    //====================================================================================
    else {  
        errorMissingId(); 
    }
    printf("funcDecl()에서 return됨\n");
}



//====================================================================================
// 함수명 : statement
// 문장 컴파일
//====================================================================================
void statement()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int tIndex  = 0;
    KindT k     = 0;
    int backP   = 0;
    int backP2  = 0;

    printf("token.kind = %d\n", token.kind);
    while(1) {
        switch(token.kind) {
            case Id: {  // case 31
                tIndex = searchT(token.u.id, varId); // 우변의 변수 인덱스
                setIdKind(k = kindT(tIndex)); // 출력을 위한 정보 설정
                if (k != varId && k != parId) // 변수 이름과 매개변수 이름이 아니면
                    errorType("var/par");
                token = checkGet(nextToken(), Assign); // :=
                expression(); //식 컴파일
                genCodeT(sto, tIndex); //좌변으로의 할당 명령
                return;
            }
            //=============================================================
            // if 문 컴파일
            //=============================================================            
            case If:{  // case 2
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
            case Ret:{   //case 6
                token = nextToken();    
                expression();                   // 식 컴파일
                genCodeR();                     // ret 명령
                return;
            }
            case Begin:{ // C언어에는 begin...end 문장 없음. 참고., 
            //case 0
                token = nextToken();
                while(1) {
                    statement(); // 문장 컴파일
                    while(1) {
                        if (token.kind == Semicolon) { // 다음이 ';'이라면 문장이 이어짐
                            printf("token.kind == ';'이어서 다음 문장이 계속 이어짐.\n");
                            token = nextToken();
                            break;
                        }
                        if (token.kind == End) { // 다음이 End라면 종료
                            printf("token.kind == 'End'이어서 종료. statement 함수에서 void return\n");
                            token = nextToken();
                            return;
                        }
                        if (isStBeginKey(token)) { //다음이 문장의 시작 기호라면
                            printf("token.kind == '문장 시작 기호'이어서 세미콜론 빠트림. 추가하고 break.\n");                        
                            errorInsert(Semicolon); // ';'를 잊었으면 ';' 추가
                            break;
                        }
                        errorDelete(); // 이 이외의 경우는 오류로써 버림
                        token = nextToken();
                    }
                }
            }
            //=============================================================
            // while문 컴파일
            //=============================================================            
            case While:{ //case 4
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


            case Write:{ //case 11
                token = nextToken();
                expression(); //식 컴파일
                genCodeO(wrt); //이 값을 출력하는 wrt 명령
                return;
                // write 문장 없음
            }
            case WriteLn:{ //case 12
                token = nextToken();
                genCodeO(wrl); //줄바꿈 출력하는 wrl 명령
                return;
                // writeln 문장 없음
            }
            case End:
            case Semicolon: { //case 1, 28
                return;
            }
            case Define:
            case Include: {
                printf("need to implement\n");
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
// If, Return, While이 해당됨.
//====================================================================================
int isStBeginKey(Token t)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
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
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    KeyId k;
    k = token.kind;
    printf("Token k type = %d\n", k);

    //====================================================================================
    // 현재 토큰이 + 또는 - 이라면 다음 토큰을 받고 항 컴파일
    // 그렇지 않으면 바로 항 컴파일
    //====================================================================================
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
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
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
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int     tIndex;
    int     i;
    KeyId   k;

    if(token.kind == Id) { // 31
        tIndex = searchT(token.u.id, varId);
        printf("factor()에서 tIndex값 = %d\n", tIndex);
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
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
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

void PrintTokenInfo()
{
    printf("현재 Token의 정보 : \n");
    printf("토큰 종류(token.kind) = %d\n", token.kind);
    printf("토큰의 이름(token.u.id) = %s\n", token.u.id);
    printf("토큰의 값(token.u.value) = %d\n", token.u.value);
}