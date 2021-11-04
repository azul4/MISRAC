#include "getSource.h"

#define MAXLINE         120             // 한 줄의 최대 문자 수
#define MAXERROR        30              // 이 이상의 오류가 있다면 종료
#define MAXNUM          14              // 상수의 최대 자리수
#define TAB             4               // 탭의 공백 수

#define INSERT_C        "#0000FF"       // 삽입 문자의 색, blue
#define DELETE_C        "#FF0000"       // 제거 문자의 색
#define TYPE_C          "#00FF00"       // 타입 오류의 문자 색, green

static FILE*    fpi;                    // 소스파일
static FILE*    fptex;                  // 레이텍 출력 파일
static char     line[MAXLINE];          // 한 줄 만큼의 입력 버퍼
static int      lineIndex;              // 다음 읽어들일 문자의 위치
static char     ch;                     // 마지막으로 읽어들인 문자

static Token    cToken;                 // 마지막으로 읽어들인 토큰
static KindT    idKind;                     // 현재 토큰(ID)의 종류
static int      spaces;                     // 그 토큰 앞의 공백 개수
static int      CR;                          // 그 앞에 있는 CR의 개수
static int      printed;                    // 토큰 인쇄했는지? 

static int      errorNo         = 0;        // 출력한 오류의 수
static char     nextChar        ();         // 다음 문자를 읽어 들이는 함수
static int      isKeySym        (KeyId k);  // t가 기호일 때 True return
static int      isKeyWd         (KeyId k);  // t는 예약일 때 True Return
static void     printSpaces     ();         // 토큰 앞의 공백 출력
static void     printcToken     ();         // 토큰 출력
static void     initCharClassT  ();

//====================================================================================
// 구조체명 : keyWd
// 예약어, 기호, 이름(KeyId)
//====================================================================================
struct keyWd {
    char *word;
    KeyId keyId;
};

//====================================================================================
// 구조체 배열명 : KeyWdT
// 구조체 타입   : keyWd
// 예약어, 기호, 이름(KeyID)의 테이블
//====================================================================================
static struct keyWd KeyWdT[] = {    
    {"begin"      ,       Begin },
    {"end"        ,       End },
    {"if"         ,       If },
    {"then"       ,       Then },
    {"while"      ,       While },
    {"do"         ,       Do },
    {"return"     ,       Ret },
    {"function"   ,       Func },
    {"var"        ,       Var },
    {"const"      ,       Const },
    {"odd"        ,       Odd },
    {"write"      ,       Write },
    {"writeln"    ,       WriteLn },
    {"#define"    ,       Define},
    {"#include"   ,       Include},
    {"main()"   ,         Main},
    {"$dummy1"    ,       end_of_KeyWd },

    // 자료형
    {"int"        ,       Int},
    {"char"       ,       CHAR},
    {"$dummy2"    ,       end_of_Type},
    
    // 이름과 기호(KeyId)의 테이블
    {"+"          ,       Plus },
    {"_"          ,       Minus },
    {"*"          ,       Mult },
    {"/"          ,       Div },
    {"("          ,       Lparen },
    {")"          ,       Rparen },
    {"=="         ,       Equal },
    {"&lt;"       ,       Lss },
    {"&gt;"       ,       Gtr },
    {"<>"         ,       NotEq },
    {"<="         ,       LssEq },
    {">="         ,       GtrEq },
    {","          ,       Comma },
    {"."          ,       Period },
    {";"          ,       Semicolon },
    {"="          ,       Assign },
    {"#"          ,       Sharp},
    {"{"          ,       LBracket},
    {"}"          ,       RBracket},
    {"$dummy2"    ,       end_of_KeySym }
};
static KeyId charClassT[256];       // 문자의 종류를 나타내는 테이블

//====================================================================================
// 함수명 : isKeyWd
// 키 k가 예약어인지 판단합니다.
// True  리턴 : k가 begin, if, then, while... 일때 
// False 리턴 : k가 +, -, *, / ... 일때
//====================================================================================
int isKeyWd(KeyId k)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return (k < end_of_KeyWd);
}

//====================================================================================
// 함수명 : isKeySym
// 키 k가 기호인지 판단합니다.
// 0     리턴 : k가 begin, if, then, while... 일때 
// True  리턴 : k가 +, -, *, / ... 일때
// False 리턴 : 이외의 값
//====================================================================================
int isKeySym(KeyId k)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if (k < end_of_KeyWd) return 0;
    else return (k < end_of_KeySym);
}

//====================================================================================
// 함수명 : initCharClassT
// charClassT 배열의 인덱스는 ASCII 문자를 뜻합니다.
// 각각의 인덱스가 뜻하는 것을 적습니다.
// ex) digit, letter, plus, minus, ...
//====================================================================================
static void initCharClassT()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int i = 0;
    for(i = 0  ; i < 256 ; i++) charClassT[i] = others;
    for(i = '0'; i <= '9'; i++) charClassT[i] = digit;
    for(i = 'A'; i <= 'Z'; i++) charClassT[i] = letter;
    for(i = 'a'; i <= 'z'; i++) charClassT[i] = letter;

    charClassT['+'] = Plus;
    charClassT['-'] = Minus;
    charClassT['*'] = Mult;
    charClassT['/'] = Div;
    charClassT['('] = Lparen;
    charClassT[')'] = Rparen;
    charClassT['='] = Assign;
    charClassT['<'] = Lss;
    charClassT['>'] = Gtr;
    charClassT[','] = Comma;
    charClassT['.'] = Period;
    charClassT[';'] = Semicolon;
    charClassT[':'] = colon;

    charClassT['#'] = Sharp;
}

//====================================================================================
// 함수명 : openSource
// 소스 파일 열기
//====================================================================================
int openSource(char fileName[]) 
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    char fileName0[30] = {0};
    fpi = fopen(fileName, "r");
    if(fpi == NULL) {
        printf("can't open %s\n", fileName);
        return 0;
    }

    strcpy(fileName0, fileName);
    strcat(fileName0, ".html");
    if( (fptex = fopen(fileName0, "w")) == NULL) {
        printf("can't open %s\n", fileName0);
        return 0;
    }
    return 1;
}

//====================================================================================
// 함수명 : closeSource
// 소스 파일과 .html 파일 닫기
//====================================================================================
void closeSource(void)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    fclose(fpi);
}


//====================================================================================
// 함수명 : closeSource
// 소스 파일 정보 초기화
//====================================================================================
void initSource()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    lineIndex   = -1;
    ch          = 10;
    printed     =  PRINTED;
    initCharClassT();

    //html 출력
    fprintf(fptex, "<HTML>\n");
    fprintf(fptex, "<HEAD>\n<TITLE>COMPILED SOURCE PROGRAM</TITLE>\n</HEAD>\n");
    fprintf(fptex, "<BODY>\n<PRE>\n");

}

//====================================================================================
// 함수명 : finaSource
// HTML파일 종결
//====================================================================================
void finalSource()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    // if (cToken.kind == Semicolon) printcToken();
    // else                          errorInsert(Semicolon);
    fprintf(fptex, "\n</PRE>\n</BODY>\n</HTML>");
}

//====================================================================================
// 함수명 : error
// 일반적인 오류 메세지 출력의 경우 (참고용)
//====================================================================================
void error(char *m)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if (lineIndex > 0) printf("%*s\n", lineIndex, "***^");
    else               printf("^\n");

    printf("*** error *** %s\n", m);
    errorNo++;
    if(errorNo > MAXERROR) {
        printf("too many errors\n");
        printf("abort compilation\n");
        exit(1);
    }
}

//====================================================================================
// 함수명 : errorNoCheck
// 오류 개수를 체크하고 MAXERROR 값보다 더 크면 종료합니다.
//====================================================================================
void errorNoCheck()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if(errorNo++ > MAXERROR)
    {
        fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>");
        exit(1);
    }
}

//====================================================================================
// 함수명 : errorType
// 자료형 오류를 .html 파일에 삽입합니다.
//====================================================================================
void errorType(char *m)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printSpaces();
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    printcToken();
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorInsert
// keyString(k)를 .html 파일에 삽입합니다.
//====================================================================================
void errorInsert(KeyId k)   
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if(k < end_of_KeyWd) {
        fprintf(fptex, "\\ \\insert{{\\bf %s}}", KeyWdT[k].word);
    }
    else {
        fprintf(fptex, "<FONT COLOR=%s><b><%s></b></FONT>", INSERT_C, KeyWdT[k].word);
    }
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorMissingId
// 이름이 아니라는 메시지를 html 파일에 삽입
//====================================================================================
void errorMissingId()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
}

//====================================================================================
// 함수명 : errorMissingOp
// 연산자가 아니라는 메세지를 .html 파일에 삽입합니다.
//====================================================================================
void errorMissingOp()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    fprintf(fptex, "<FONT COLOR=%s>@</FONT>", TYPE_C);
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorDelete
// 읽어 들인 토큰을 버립니다.
//====================================================================================
void errorDelete()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int i = 0;

    i = (int)cToken.kind;
    printSpaces();
    printed = PRINTED;

    //====================================================================================
    // i가 예약어라면 
    //====================================================================================
    if(i < end_of_KeyWd) { 
        fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
    }
    //====================================================================================
    // i가 연산자 혹은 구분자이면
    //====================================================================================    
    else if (i < end_of_KeySym) {
        fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
    }
    //====================================================================================
    // i가 식별자라면 
    //====================================================================================    
    else if (i == (int)Id) {
        fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, cToken.u.id);
    }   
    //====================================================================================
    // i가 숫자라면 
    //====================================================================================    
    else if (i == (int)Num) {
        fprintf(fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, cToken.u.value);
    }
}

//====================================================================================
// 함수명 : errorMessage
// 오류 메세지를 html 파일에 출력합니다.
//====================================================================================
void errorMessage(char *m)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorF
// 오류 메세지를 출력하고 컴파일러를 종료합니다.
//====================================================================================
void errorF(char * m)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    errorMessage(m);
    fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</BODY>\n</HTML>\n");
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorN
// 오류 개수를 리턴합니다.
//====================================================================================
int  errorN()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printf("errorNo = %d\n", errorNo);
    return errorNo;
}

//====================================================================================
// 함수명 : nextChar
// 분석 대상 파일에서 다음 글자를 리턴합니다.
//====================================================================================
char nextChar()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    char ch;

    //====================================================================================
    // 라인의 시작일때 로직
    //====================================================================================
    if (lineIndex == -1) {
        printf("lineIndex == -1 \n");
        if(fgets(line, MAXLINE, fpi) != NULL) {
            printf("*********************************************************\n");
            puts(line); //일반적인 오류 메세지 출력의 경우
            printf("*********************************************************\n");
            lineIndex = 0;
        } else {
            errorF("end of file\n"); //EOF이라면 컴파일 종료
        }
    }
    printf("lineIndex = %d\n", lineIndex);

    //====================================================================================
    // 해당 줄의 시작을 ch에 저장함과 동시에, 
    // 줄의 끝에서 enter를 만났다면 lineIndex를 -1로 초기화시키고 
    // 다음 글자로 enter(10)을 리턴
    //====================================================================================
    if ( (ch = line[lineIndex++]) == '\n') {
        printf("line[%d] = ascii 10 = ch\n", lineIndex - 1);
        lineIndex = -1;
        return '\n';
    }
    
    printf("함수 %s return value = '%c' %d\n", __FUNCTION__, ch, ch);
    return ch;
}

//====================================================================================
// 함수명 : nextToken
// 다음 토큰을 읽어 들이고 그 토큰을 리턴합니다.
//====================================================================================
Token nextToken()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int         i = 0;
    int         num = 0;
    KeyId       cc;
    Token       temp;
    char        ident[MAXNAME];

    printcToken(); // 앞의 토큰 HTML에 기록
    spaces    = 0;
    CR        = 0;

    //====================================================================================
    // 다음 글자가 enter, tab, white space일 경우
    // 각각의 경우를 기록하고 올바른 char가 나올 때 까지 char를 찾는다.
    //====================================================================================
    while(1) {
        if (ch == ' ') {
            spaces++;
        } else if (ch == '\t') {
            spaces += TAB;
        } else if (ch == '\n') {
            spaces = 0;
            CR++;
        } else {
            printf("모든 enter, tab, white space가 기록되었음\n");
            printf("spaces = %d\tCR = %d\n", spaces, CR);
            break;
        }
        ch = nextChar();
    }

    printf("cc = charClassT[%d(%c)] = %d\n", ch, ch, charClassT[ch]);
    switch(cc = charClassT[ch]) {
        case letter: { // 식별자
            printf("cc는 letter\n");
            //===================================================================
            // 읽어들이는 문자가 letter, digit type이 아닐 때 까지 문자를 읽어들인다.
            //===================================================================            
            do {
                if (i < MAXNAME) ident[i] = ch; //식별자의 이름은 32자 제한
                i++;
                ch = nextChar();
            } while (charClassT[ch] == letter || charClassT[ch] == digit);

            //===================================================================
            // 식별자의 길이가 32를 초과할 경우 에러를 띄운다.
            //===================================================================            
            if(i >= MAXNAME) {
                errorMessage("too long");
                i = MAXNAME - 1;
            }

            ident[i] = '\0';

            printf("토큰 이름 = %s\t길이 = %ld\n\n", ident, strlen(ident));
            
            //===================================================================
            // 예약어일 경우 cToken에 해당 토큰 정보를 입력하고 그것을 return한다.
            //===================================================================                        
            for (i = 0; i < end_of_KeyWd; i++) {
                if(strcmp(ident, KeyWdT[i].word) == 0) {
                    printf("읽어들인 Token이 예약어로 반환됨\n");
                    printf("ident = %s\tKeyWdT[%d].word = %s\n\n", ident, i, KeyWdT[i].word);
                    temp.kind   = KeyWdT[i].keyId;
                    cToken      = temp;
                    printed     = NOT_PRINTED;
                    return temp;
                }
            }

            //===================================================================
            // 자료형일 경우 cToken에 해당 토큰 정보를 입력하고 그것을 return한다.
            //===================================================================                        
            for (i = end_of_KeyWd; i < end_of_Type; i++) {
                if(strcmp(ident, KeyWdT[i].word) == 0) {
                    printf("읽어들인 Token이 자료형으로 반환됨\n");
                    printf("ident = %s\tKeyWdT[%d].word = %s\n\n", ident, i, KeyWdT[i].word);
                    temp.kind   = KeyWdT[i].keyId;
                    cToken      = temp;
                    printed     = NOT_PRINTED;
                    return temp;
                }
            }            
            //===================================================================
            // 사용자가 선언한 이름(변수명)일 경우 temp.kind에 id를, temp.u.id에 그 이름을 복사한다.
            //===================================================================      
            printf("사용자가 선언한 이름(변수명)일 경우 temp.kind에 id를, temp.u.id에 그 이름을 복사한다.\n");                              
            temp.kind = Id;
            strcpy(temp.u.id, ident);
            break;
        }

        case digit: {
            printf("cc는 digit\n");
            num = 0;
            do {
                num = 10 * num + (ch - '0');
                i++;
                ch = nextChar();
            } while(charClassT[ch] == digit);
            
            if(i > MAXNUM) {
                errorMessage("too large");
            }

            temp.kind = Num;
            temp.u.value = num;

            break;
        }

        //Switch-Case 말고는 colon 안쓰임
        case colon: {
            printf("cc는 colon\n");
            if( (ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = Assign; /* "=" */
            } else {
                temp.kind = nul;
            }
            break;
        }

        case Lss: {
            printf("cc는 Lesser (<)\n");
            if( (ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = LssEq; // <=
            } else if (ch == '>') {
                ch = nextChar();
                temp.kind = NotEq; // <>
            } else {
                temp.kind = Lss;
            }
            break;
        }

        case Gtr: {
            printf("cc는 Greater(>)\n");
            ch = nextChar();
            if(ch == '=') {
                ch = nextChar();
                temp.kind = GtrEq; // >=
            } else if (8) {
                temp.kind = Gtr;
            }
            break;
        }

        case Sharp: { //#define 또는 #include
            printf("cc는 #으로 시작함\n");
            //===================================================================
            // 읽어들이는 문자가 define 또는 include 일 때 까지 문자를 읽어들인다.
            //===================================================================            
            do {
                if (i < MAXNAME) ident[i] = ch; //식별자의 이름은 32자 제한
                i++;
                ch = nextChar();
            } while (charClassT[ch] == letter);
            
            ident[i] = '\0';
            printf("토큰 이름 = %s\t길이 = %ld\n\n", ident, strlen(ident));

            //===================================================================
            // 예약어일 경우 cToken에 define 또는 include를 입력하고 그것을 return한다.
            //===================================================================               
                     
            for (i = 0; i < end_of_KeyWd; i++) {
                if(strcmp(ident, KeyWdT[i].word) == 0) {
                    printf("읽어들인 Token이 예약어로 반환됨\n");
                    printf("ident = %s\tKeyWdT[%d].word = %s\n\n", ident, i, KeyWdT[i].word);
                    temp.kind   = KeyWdT[i].keyId;
                    cToken      = temp;
                    printed     = NOT_PRINTED; 
                    return temp;
                }
            }            
            break;
        }      

        case Assign: {
            printf("cc는 Assign(=)\n");
            ch = nextChar();
            temp.kind = Assign; /* "=" */
            break;
        }          

        case Semicolon: {
            printf("cc는 Semicolon(;)\n");
            temp.kind = Semicolon;
            ch = nextChar();
            break;
        }

        case Comma: {
            printf("cc는 Comma(,))\n");
            temp.kind = Comma;
            ch = nextChar();
            break;            
        }

        case Period: {
            printf("cc는 Period(.)\n");
            temp.kind = Period;
            ch = nextChar();
            break;            
        } 

        case LBracket: {
            printf("cc는 LBracket({})\n");
            temp.kind = LBracket;
            ch = nextChar();
            break;            
        }                

        default: {
            printf("cc는 >, <, =, 숫자, 식별자, #가 아님.\n");
            printf("cc ID = %d\n", cc);
            temp.kind = cc;
            break;
        }
    }
    cToken  = temp;
    printed = NOT_PRINTED;

    printf("마지막에서 nextToken() return됨\n");
    printf("return value : \n");
    printf("temp.kind = %d\n", temp.kind);
    printf("temp.u.id = %s\n", temp.u.id);
    printf("temp.u.value = %d\n", temp.u.value);
    return temp;
}

//====================================================================================
// 함수명 : checkGet
// compile.c :: constDecl()에서 불립니다.
// t.kind == k(파라미터의 Key Id)라면 다음 토큰을 읽어들이고 리턴
// t.kind != k(파라미터의 Key Id)라면 오류 메시지를 출력하면서, k로 덮어씀. + 다음 토큰 읽어들임.
// 이 이외의 경우, k를 삽입한 상태에서 t를 리턴
//====================================================================================

Token checkGet(Token t, KeyId k)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if(t.kind == k) {
        printf("이미 지정된 Token은 t.kind == %d\n", k);
        printf("지정된 토큰 k(%d) 와 t.kind(%d)가 동일함\n", k, t.kind);
        return nextToken();
    }
    if ( (isKeyWd(k) && isKeyWd(t.kind)) ||
         (isKeySym(k) && isKeySym(t.kind))) {
        printf("KeyId k는 예약어이거나 기호\n");
        errorDelete();
        errorInsert(k);
        return nextToken();
    }
    errorInsert(k);
    printf("checkGet()의 마지막에서 return, retval Token Kind = %d\n", t.kind);
    return t;
}


//====================================================================================
// 함수명 : printSpaces
// 공백 또는 줄바꿈을 HTML 파일에 출력합니다.
//===================================================================================
static void printSpaces()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printf("공백 또는 줄바꿈을 HTML 파일에 출력합니다.\n");
    // printf("CR = %d\t spaces = %d\n", CR, spaces);
    while (CR-- > 0) {
        fprintf(fptex, "\n");
    }
    while (spaces-- > 0) {
        fprintf(fptex, " ");
    }
    
    CR = 0;
    spaces = 0;
}   


//====================================================================================
// 함수명 : printcToken
// 이전에 저장되었던 토큰을 HTML에 표시합니다.
//====================================================================================
static void printcToken(void)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printf("전에 저장되었던 토큰을 HTML에 표시합니다.\n");
    int i = 0;

    i = (int)cToken.kind; 
    if (printed == PRINTED) {
        printed = NOT_PRINTED;
        return;
    }

    printed = PRINTED;
    printSpaces();
    if (i < end_of_KeyWd) // 만약 cToken.kind가 예약어라면
    { 
        fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
    }
    else if (i < end_of_Type) // 만약 cToken.kind가 자료형이라면
    { 
        fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
    }    
    else if (i < end_of_KeySym) //연산자인지 구분 기호인지
    {
        fprintf(fptex, "%s", KeyWdT[i].word);
    }
    else if (i == (int)Id)
    { //식별자라면
        switch(idKind) {
            case varId : {
                fprintf(fptex, "%s", cToken.u.id); return;
            }
            case parId: {
                fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
            }
            case funcId: {
                fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
            }
            case constId: {
                fprintf(fptex, "<tt>%s</tt>", cToken.u.id); return;
            }
            default : break;
        }
    }
    else if (i == (int)Num) //숫자라면
    { 
        fprintf(fptex, "%d", cToken.u.value);
    }
}
//====================================================================================
// 함수명 : setIdKind
// 현재 토큰(Id)의 종류 설정
//====================================================================================
void setIdKind(KindT k)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    idKind = k;
}