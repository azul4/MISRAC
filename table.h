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