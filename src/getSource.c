#include "getSource.h"

#define MAXLINE 120 //한 줄의 최대 문자 수
#define MAXERROR 30 // 이 이상의 오류가 있다면 종료
#define MAXNUM  14   //상수의 최대 자리수
#define TAB 4 //탭의 공백 수

#define TYPE_C "#00FF00" //타입 오류의 문자 색

static FILE* fpi; //소스파일
static FILE* fptex; //레이텍 출력 파일
static char line[MAXLINE]; //한 줄 만큼의 입력 버퍼
static int lineIndex;    // 다음 읽어들일 문자의 위치
static char ch; //마지막으로 읽어들인 문자


static int lineIndex; // 다음 읽어들일 문자의 위치

static char ch;     //마지막으로 읽어들인 문자

static Token cToken; // 마지막으로 읽어들인 토큰
static KindT idKind; //현재 토큰(ID)의 종류
static int spaces;  //그 토큰 앞의 공백 개수
static int CR;      //그 앞에 있는 CR의 개수
static int printed; //토큰 인쇄했는지? 

static int errorNo = 0; //출력한 오류의 수

static void printcToken(void);  //토큰 출력

struct keyWd {
    char *word;
    KeyId keyId;
};
static KeyId charClassT[256]; //문자의 종류를 나타내는 테이블
static struct keyWd KeyWdT[] = { /*예약어, 기호, 이름(KeyID)의 테이블 */
    {" begin ", Begin },
    {" end ", End },
    {" if ", If },
    {" then ", Then },
    {" while ", While },
    {" do ", Do },
    {" return ", Ret },
    {" function ", Func },
    {" var ", Var },
    {" const ", Const },
    {" odd ", Odd },
    {" write ", Write },
    {" writeln ", WriteLn },
    {" $dummy1 ", end_of_KeyWd },
    
    // 이름과 기호(KeyId)의 테이블
    {" + ", Plus },
    {" _ ", Minus },
    {" * ", Mult },
    {" / ", Div },
    {" ( ", Lparen },
    {" ) ", Rparen },
    {" = ", Equal },
    {" < ", Lss },
    {" > ", Gtr },
    {" <> ", NotEq },
    {" <= ", LssEq },
    {" >= ", GtrEq },
    {" , ", Comma },
    {" . ", Period },
    {" ; ", Semicolon },
    {" := ", Assign },
    {" $dummy2 ", end_of_KeySym }
};

int openSource(char fileName[]) // 소스 파일 열기
{
    char fileName0[30];
    fpi = fopen(fileName, "r");
    if(fpi == NULL) {
        printf("can't open %s\n", fileName);
        return 0;
    }

    strcpy(fileName0, fileName);

    return 1;
}

void closeSource(void)
{
    fclose(fpi);
}


//====================================================================================
//
//====================================================================================
void initSource()
{
    lineIndex = -1;
    ch = 10;
    printed = 1;
    initCharClassT();


    //LaTeX 출력
    //아직 안할것.

    //html 출력
    fprintf(fptex, "<HTML>\n");
    fprintf(fptex, "<HEAD>\n<TITLE>COMPILED SOURCE PROGRAM</TITLE>\n<HEAD>");
    fprintf(fptex, "<BODY>\b<PRE>\n");

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
// 함수명 : errorMessage
// 오류 메세지를 html 파일에 출력합니다.
//====================================================================================
void errorMessage(char *m)
{
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    errorNoCheck();
}

