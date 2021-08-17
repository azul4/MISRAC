#define MAXLEVEL 5 //블록의 최대 깊이는 5

static int tIndex = 0;                  // 이름 테이블의 인덱스
static int level = -1;                  // 현재 블록 레벨
static int index [MAXLEVEL] = {0};      // index[i]에는 블록 레벨 i의 마지막 인덱스가 들어있다.
static int addr  [MAXLEVEL] = {0};      //  addr[i]에는 블록 레벨 i의 마지막 변수의 주소가 들어있다.
static int localAddr;
static int tfIndex;

void blockBegin(int firstAddr)
{
    //====================================================================================
    // 메인 블록일 경우 초기 설정을 합니다.
    //====================================================================================
    if (level == -1) {  
        localAddr = firstAddr;
        tIndex = 0;
        level++;
        return;
    }

    //====================================================================================
    // 메인 블록일 경우 초기 설정을 합니다.
    //====================================================================================
    if(level == MAXLEVEL - 1) {
        errorF("too many nested blocks");
    }    

    index   [level]     = tIndex;
    addr    [level]     = localAddr;
    localAddr           = firstAddr;
    level               = level + 1;
    return;
}
void blockEnd()
{

}