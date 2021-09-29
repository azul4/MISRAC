#include "getSource.h"

#define MAXLINE         120             // 한 줄의 최대 문자 수
#define MAXERROR        30              // 이 이상의 오류가 있다면 종료
#define MAXNUM          14              // 상수의 최대 자리수
#define TAB             5               // 탭의 공백 수

#define INSERT_C        "#0000FF"       // 삽입 문자의 색
#define DELETE_C        "#FF0000"       // 제거 문자의 색
#define TYPE_C          "#00FF00"       // 타입 오류의 문자 색

static FILE*    fpi;                    // 소스파일
static FILE*    fptex;                  // 레이텍 출력 파일
static char     line[MAXLINE];          // 한 줄 만큼의 입력 버퍼
static int      lineIndex;              // 다음 읽어들일 문자의 위치
static char     ch;                     // 마지막으로 읽어들인 문자



static Token    cToken;                 // 마지막으로 읽어들인 토큰
static KindT    idKind;                 // 현재 토큰(ID)의 종류
static int      spaces;                 // 그 토큰 앞의 공백 개수
static int      CR;                     // 그 앞에 있는 CR의 개수
static int      printed;                // 토큰 인쇄했는지? 

static int      errorNo     = 0;        // 출력한 오류의 수
static char     nextChar    ();         // 다음 문자를 읽어 들이는 함수
static int      isKeySym    (KeyId k);  // t는 기호인가?
static int      isKeyWd     (KeyId k);  // t는 예약어인가?
static void     printSpaces ();         // 토큰 앞의 공백 출력
static void     printcToken ();         // 토큰 출력

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
    {" begin "      ,       Begin },
    {" end "        ,       End },
    {" if "         ,       If },
    {" then "       ,       Then },
    {" while "      ,       While },
    {" do "         ,       Do },
    {" return "     ,       Ret },
    {" function "   ,       Func },
    {" var "        ,       Var },
    {" const "      ,       Const },
    {" odd "        ,       Odd },
    {" write "      ,       Write },
    {" writeln "    ,       WriteLn },
    {" $dummy1 "    ,       end_of_KeyWd },
    
    // 이름과 기호(KeyId)의 테이블
    {" + "          ,       Plus },
    {" _ "          ,       Minus },
    {" * "          ,       Mult },
    {" / "          ,       Div },
    {" ( "          ,       Lparen },
    {" ) "          ,       Rparen },
    {" = "          ,       Equal },
    {" < "          ,       Lss },
    {" > "          ,       Gtr },
    {" <> "         ,       NotEq },
    {" <= "         ,       LssEq },
    {" >= "         ,       GtrEq },
    {" , "          ,       Comma },
    {" . "          ,       Period },
    {" ; "          ,       Semicolon },
    {" := "         ,       Assign },
    {" $dummy2 "    ,       end_of_KeySym }
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
    if (k < end_of_KeyWd) return 0;
    else return (k < end_of_KeySym);
}

//====================================================================================
// 함수명 : initCharClassT
// charClassT 배열의 인덱스는 ASCII 문자를 뜻합니다.
// 각각의 인덱스가 뜻하는 것을 적습니다.
// ex) digit, letter, plus, minus, ...
//====================================================================================
static void initCharClassT() //선언 안되어있음
{
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
    charClassT['='] = Equal;
    charClassT['<'] = Lss;
    charClassT['>'] = Gtr;
    charClassT[','] = Comma;
    charClassT['.'] = Period;
    charClassT[';'] = Semicolon;
    charClassT[':'] = colon;
}

//====================================================================================
// 함수명 : openSource
// 소스 파일 열기
//====================================================================================
int openSource(char fileName[]) 
{
    char fileName0[30];
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
    fclose(fpi);
}


//====================================================================================
// 함수명 : closeSource
// 소스 파일 정보 초기화
//====================================================================================
void initSource()
{
    lineIndex   = -1;
    ch          = 10;
    printed     =  1;
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
    if (cToken.kind == Period) printcToken();
    else                       errorInsert(Period);
    fprintf(fptex, "\n</PRE>\n</BODY>\n</HTML>");
}

//====================================================================================
// 함수명 : error
// 일반적인 오류 메세지 출력의 경우 (참고용)
//====================================================================================
void error(char *m)
{
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
    fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
}

//====================================================================================
// 함수명 : errorMissingOp
// 연산자가 아니라는 메세지를 .html 파일에 삽입합니다.
//====================================================================================
void errorMissingOp()
{
    fprintf(fptex, "<FONT COLOR=%s>@</FONT>", TYPE_C);
    errorNoCheck();
}


//====================================================================================
// 함수명 : errorDelete
// 읽어 들인 토큰을 버립니다.
//====================================================================================
void errorDelete()
{
    int i = 0;

    i = (int)cToken.kind;
    printSpaces();
    printed = 1;

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
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    errorNoCheck();
}

//====================================================================================
// 함수명 : errorF
// 오류 메세지를 출력하고 컴파일러를 종료합니다.
//====================================================================================
void errorF(char * m)
{
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
    return errorNo;
}

//====================================================================================
// 함수명 : nextChar
// 
//====================================================================================
char nextChar()
{
    char ch;

    if (lineIndex == -1) {
        if(fgets(line, MAXLINE, fpi) != NULL) {
            puts(line); //일반적인 오류 메세지 출력의 경우
            lineIndex = 0;
        } else {
            errorF("end of file\n"); //EOF이라면 컴파일 종료
        }
    }

    if ( (ch = line[lineIndex++]) == '\n') {
        lineIndex = -1;
        return '\n';
    }

    return ch;
}

//====================================================================================
// 함수명 : nextToken
// 다음 토큰을 읽어 들이고 그 토큰을 리턴합니다.
//====================================================================================
Token nextToken()
{
    int         i = 0;
    int         num = 0;
    KeyId       cc;
    Token       temp;
    char        ident[MAXNAME];

    printcToken(); // 앞의 토큰 출력
    spaces = 0;
    CR = 0;
    while(1) {
        if (ch == ' ') {
            spaces++;
        } else if (ch == '\t') {
            spaces += TAB;
        } else if (ch == '\n') {
            spaces = 0;
            CR++;
        } else {
            break;
        }
        ch = nextChar();
    }

    switch(cc = charClassT[ch]) {
        case letter: {
            do {
                if (i < MAXNAME)
                ident[i] = ch;
                i++;
                ch = nextChar();
            } while (charClassT[ch] == letter || charClassT[ch] == digit);

            if(i >= MAXNAME) {
                errorMessage("too long");
                i = MAXNAME - 1;
            }

            ident[i] = '\0';
            for (i = 0; i < end_of_KeyWd; i++) {
                if(strcmp(ident, KeyWdT[i].word) == 0) {
                    temp.kind = KeyWdT[i].keyId; //???
                    cToken = temp;
                    printed = 0; 
                    return temp;
                }
            }
            break;
        }

        case digit: {
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
        case colon: {
            if( (ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = Assign; /* ":=" */
            } else {
                temp.kind = nul;
            }
            break;
        }
        case Lss: {
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
            if( (ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = GtrEq; // >=
            } else {
                temp.kind = Gtr;
            }
            break;
        }
        default: {
            temp.kind = cc;
            ch = nextChar();
            break;
        }
    }
    cToken  = temp;
    printed = 0;
    return temp;
}
//====================================================================================
// 함수명 : printcToken
// compile.c :: constDecl()에서 불립니다.
// t.kind == k라면 다음 토큰을 읽어들이고 리턴
// t.kind != k라면 오류 메시지를 출력, t와 k가 같은 기호 또는 예약어라면
// t를 버리고 다음 토큰을 읽어 들이고 리턴(t를 k로 변경하게 됨)
// 이 이외의 경우, k를 삽입한 상태에서 t를 리턴
//====================================================================================

Token checkGet(Token t, KeyId k)
{
    if(t.kind == k) {
        return nextToken();
    }
    if ( (isKeyWd(k) && isKeyWd(t.kind)) ||
         (isKeySym(k) && isKeySym(t.kind))) {
        errorDelete();
        errorInsert(k);
        return nextToken();
    }
    errorInsert(k);
    return t;
    

}


//====================================================================================
// 함수명 : printSpaces
// 공백 또는 줄바꿈을 출력합니다.
//===================================================================================
static void printSpaces()
{
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
// 이전에 저장되었던 토큰을 표시합니다.
//====================================================================================
static void printcToken(void)
{
    int i = 0;

    i = (int)cToken.kind; 
    if(printed) {
        printed = 0;
        return;
    }

    printed = 1;
    printSpaces();
    if (i < end_of_KeyWd) // 만약 cToken.kind가 예약어라면
    { 
        fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
    }
    else if (i < end_of_KeySym) //연산자인지 구분 기호인지
    {
        fprintf(fptex, "$%s$", KeyWdT[i].word);
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
    else if (i == (int)Num)
    { //숫자라면
        fprintf(fptex, "%d", cToken.u.value);
    }
}
//====================================================================================
// 함수명 : setIdKind
// 현재 토큰(Id)의 종류 설정
//====================================================================================
void setIdKind(KindT k)
{
    idKind = k;
}