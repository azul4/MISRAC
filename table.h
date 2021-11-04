typedef enum kindT {
    varId, funcId, parId, constId
} KindT;

//====================================================================================
// 구조체명 : RelAddr
// 변수, 매개변수, 함수 주소의 형태를 기록합니다.
//====================================================================================
typedef struct relAddr {
    int level;
    int addr;
} RelAddr;

/*
#if 0
//====================================================================================
// 함수명 : blockBegin
// 블록 시작 때 호출(맨 앞부분의)
//====================================================================================
void blockBegin(int firstAddr);

//====================================================================================
// 함수명 : blockEnd
// 블록 종료할 때 호출
//====================================================================================
void blockEnd();

//====================================================================================
// 함수명 : GetBlockLevel
// 현재 블록 레벨 리턴
//====================================================================================
int GetBlockLevel();

//====================================================================================
// 함수명 : GetParamNum
// 현재 블록 함수의 매개변수 수 리턴
//====================================================================================
int GetParamNum();

//====================================================================================
// 함수명 : enterTfunc
// 이름 테이블에 함수 이름과 맨 앞 주소 등록
//====================================================================================
int enterTfunc(char *id, int v);

//====================================================================================
// 함수명 : enterTvar
// 이름 테이블에 매개변수 이름 등록
//====================================================================================
int  enterTvar(char *id);

//====================================================================================
// 함수명 : enterT
// 이름 테이블에 이름을 등록합니다.
//====================================================================================
void enterT(char *id);
int  enterTfunc(char *id, int v);
int  enterTpar(char *id);
int  enterTvar(char *id);
int  enterTvar(char *id);
void endpar();

#else*/

void blockBegin(int firstAddr);
void blockEnd();
int  GetBlockLevel();
int  GetParamNum();
void enterT(char *id);
int  enterTfunc(char *id, int v);
int  enterTpar(char *id);
int  enterTvar(char *id);
int  enterTconst(char *id, int v);
void endpar();
void changeV(int ti, int newVal);
int searchT(char *id, KindT k);
KindT kindT(int i);
RelAddr relAddr(int ti);
int val(int ti);
int pars(int ti);
int frameL();