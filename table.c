#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXLEVEL 5 //블록의 최대 깊이는 5
#define MAXTABLE 100 // 이름 테이블의 최대 길이


//====================================================================================
// TableE 구조체 멤버변수 설명:    이름 테이블 엔트리의 형태
// TableE.kind                   이름의 종류               
// TableE.name                   이름 철자               
// TableE.u.value                상수의 경우            : 값   
// TableE.u.f.raddr              함수의 경우            : 앞부분의 주소                   
// TableE.u.f.pars               함수의 경우            : 매개변수 수                   
// TableE.u.raddr                변수, 매개변수의 경우   : 주소                   
//====================================================================================
typedef struct tableE {
    KindT kind;                 
    char name[MAXNAME];
    union {
        int value;
        struct {
            RelAddr raddr;
            int pars;
        } f;
    RelAddr raddr;
    } u;
}TableE;


static TableE nameTable[MAXTABLE];      // 이름 테이블
static int tIndex = 0;                  // 이름 테이블의 인덱스
static int level = -1;                  // 현재 블록 레벨
static int table_index [MAXLEVEL] = {0};      // table_index[i]에는 블록 레벨 i의 마지막 인덱스가 들어있다.
static int addr  [MAXLEVEL] = {0};      //  addr[i]에는 블록 레벨 i의 마지막 변수의 주소가 들어있다.
static int localAddr;                   // 현재 블록 마지막 변수의 주소
static int tfIndex;

//====================================================================================
// 함수명 : blockBegin
// 이름 종류 출력을 위한 함수
//====================================================================================
void blockBegin(int firstAddr)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    //====================================================================================
    // 메인 블록일 경우 초기 설정을 합니다.
    //====================================================================================
    if (level == -1) {  
        printf("level under 0\n");
        localAddr = firstAddr;
        tIndex    = 0;
        level++;

        printf("localAddr                 = %d;\n", localAddr);
        printf("tIndex                    = %d\n" , tIndex);
        printf("level                     = %d;\n", level);
        printf("returning from blockBegin()\n");
        
        printf("함수 %s 끝\n", __FUNCTION__);
        return;
    }

    //====================================================================================
    // 현재 블록 레벨이 5 이상일 경우 에러
    //====================================================================================
    if(level == MAXLEVEL - 1) {
        errorF("too many nested blocks");
    }    


    table_index   [level]     = tIndex;
    addr          [level]     = localAddr;
    localAddr                 = firstAddr;
    level                     = level + 1;
    
    printf("table_index   [%d]     = %d\n", level - 1, table_index[level - 1]);
    printf("addr          [%d]     = %d\n", level - 1, addr[level - 1]);
    printf("localAddr              = %d\n", localAddr);
    printf("(incremented) level    = %d\n", level);

    printf("함수 %s 끝\n", __FUNCTION__);
    return;
}

//====================================================================================
// 함수명 : blockEnd
// 블록 종료할 때 호출
//====================================================================================
void blockEnd()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    level--;
    tIndex = table_index[level];   //바로 밖 블록의 정보 복구
    localAddr = addr[level];
}

//====================================================================================
// 함수명 : GetBlockLevel
// 현재 블록의 레벨을 리턴
//====================================================================================
int GetBlockLevel()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return level;
}

//====================================================================================
// 함수명 : GetParamNum
// 현재 블록 함수의 매개변수 수 리턴
//====================================================================================
int GetParamNum()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return nameTable[table_index[level-1]].u.f.pars;
}


//====================================================================================
// 함수명 : enterT
// 이름 테이블에 이름(토큰)을 등록합니다.
//====================================================================================
void enterT(char *id)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    if(tIndex++ < MAXTABLE) {
        strcpy(nameTable[tIndex].name, id);
        printf("nameTable[%d].name = %s\n", tIndex, nameTable[tIndex].name);
    } else {
        errorF("too many names");
    }
}

//====================================================================================
// 함수명 : enterTfunc
// 이름 테이블에 함수 이름과 맨 앞 주소를 등록합니다.
// 파라미터 : 함수 이름, 함수 맨 앞 부분의 주소
//====================================================================================
int  enterTfunc(char *id, int v)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    enterT(id); // nameTable[tIndex].name에 Token.u.id가 입력됨
    nameTable[tIndex].kind            = funcId;
    nameTable[tIndex].u.f.raddr.level = level;
    nameTable[tIndex].u.f.raddr.addr  = v;      //함수 맨 앞 부분의 주소
    nameTable[tIndex].u.f.pars        = 0;      //매개변수 수의 초기값

    printf("tIndex = %d\n", tIndex);
    printf("nameTable[tIndex].kind = %d\n", nameTable[tIndex].kind);
    printf("nameTable[tIndex].u.f.raddr.level (현재 블록 레벨) = %d\n", nameTable[tIndex].u.f.raddr.level);
    printf("nameTable[tIndex].u.f.raddr.addr(함수 맨 앞 부분 주소) = %d\n", nameTable[tIndex].u.f.raddr.addr);
    printf("nameTable[tIndex].u.f.pars (매개변수 수의 초기값) = %d\n", nameTable[tIndex].u.f.pars);
    tfIndex = tIndex;
    printf("return tIndex = %d\n", tIndex);
    return tIndex;    
}

//====================================================================================
// 함수명 : enterTpar
// 이름 테이블에 매개변수 이름을 등록합니다.
//====================================================================================
int  enterTpar(char *id)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    enterT(id);  // nameTable[tIndex].name에 Token.u.id가 입력됨
    nameTable[tIndex].kind            = parId;
    nameTable[tIndex].u.raddr.level   = level;
    nameTable[tfIndex].u.f.pars       += 1;          //함수의 매개변수 수 세기

    printf("nameTable[tIndex].kind = %d\n", nameTable[tIndex].kind);
    printf("nameTable[tIndex].u.f.raddr.level = %d\n", nameTable[tIndex].u.f.raddr.level);
    printf("nameTable[tfIndex].u.f.pars = %d\n", nameTable[tfIndex].u.f.pars);

    return tIndex;    
}


//====================================================================================
// 함수명 : enterTvar
// 이름 테이블에 변수 이름을 등록합니다.
//====================================================================================
int  enterTvar(char *id)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    enterT(id);
    nameTable[tIndex].kind          = varId;
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tIndex].u.raddr.addr  = localAddr++;
    return tIndex;    
}

//====================================================================================
// 함수명 : enterTconst
// 이름 테이블에 상수 이름과 값을 등록합니다.
//====================================================================================
int  enterTconst(char *id, int v)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printf("이름 테이블 nameTable에 상수 이름(temp.u.id)와 값(temp.u.value)를 저장함\n");
    printf("매개변수 id, v = %s, %d\n", id, v);
    enterT(id);
    nameTable[tIndex].kind          = constId;
    nameTable[tIndex].u.value       = v;

    return tIndex;    
}

//====================================================================================
// 함수명 : endpar
// 매개변수 선언부의 마지막에서 호출됩니다.
//====================================================================================
void endpar()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int i;
    int pars = nameTable[tfIndex].u.f.pars;

    if(pars == 0){ printf("nameTable[%d].u.f.pars = 0로 return\n", tfIndex);         return;}

    for(i = 1; i <= pars; i++) { // 각 매개변수의 주소 구하기
        nameTable[tfIndex + i].u.raddr.addr = i - 1 - pars;
        printf("nameTable[%d + %d].u.raddr.addr = %d\n", tfIndex, i, nameTable[tfIndex + i].u.raddr.addr);
    }
}

//====================================================================================
// 함수명 : changeV
// 이름 테이블 [ti]의 값을 변경.
// 이름_테이블[ti]의 값은 함수 맨 앞부분의 주소
//====================================================================================
void changeV(int ti, int newVal)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    printf("nameTable[%d].u.f.raddr.addr = %d\n", ti, nameTable[ti].u.f.raddr.addr);
    nameTable[ti].u.f.raddr.addr = newVal;
}

//====================================================================================
// 함수명 : searchT
// 이름 id의 이름 테이블 위치 리턴. 없을 경우 오류 발생.
//====================================================================================
int searchT(char *id, KindT k)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    int i;
    i = tIndex;
    strcpy(nameTable[0].name, id);  //sentinal 생성
    printf("tIndex = i = %d\n", i);
    printf("현재 id를 nameTable[0].name에 대입함 : %s\n", nameTable[0].name);
    while (strcmp(id, nameTable[i].name)) i--;
    if(i) {
        printf("이름이 있으며 searchT의 리턴값은 %d\n", i);
        return i; //이름이 있을 때
    }
    else {          //이름이 없을 때
        printf("이름이 없음\n");
        errorType("undef"); //type error 등록
        if (k == varId) return enterTvar(id); //변수라면 일단 등록
        return 0;
    }
}

//====================================================================================
// 함수명 : kindT
// 이름 테이블[i]의 종류 리턴
//====================================================================================
KindT kindT(int i)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return nameTable[i].kind;
}

//====================================================================================
// 함수명 : relAddr
// 이름 테이블[ti]의 주소 리턴
//====================================================================================
RelAddr relAddr(int ti)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return nameTable[ti].u.raddr;
}

//====================================================================================
// 함수명 : val
// 이름 테이블[ti]의 값 리턴
//====================================================================================
int val(int ti)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return nameTable[ti].u.value;
}

//====================================================================================
// 함수명 : pars
// 이름 테이블[ti]의 함수 매개변수 수 리턴
//====================================================================================
int pars(int ti)
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return nameTable[ti].u.f.pars;
}

//====================================================================================
// 함수명 : frameL
// 블록 실행 때에 필요한 메모리 용량 확인
//====================================================================================
int frameL()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    return localAddr;
}

