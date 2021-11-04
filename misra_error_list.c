#include "misra_error_list.h"

static FILE*    fpErr;                    // MISRA-C 오류 정리파일

void initErrorListupFile()
{
    printf("\nfunction = %s::%s\n", __FILE__, __FUNCTION__);
    char fileName0[30] = "MISRA_C_Defects.txt";

    if((fpErr = fopen(fileName0, "w")) == NULL) {
        printf("can't open %s\n", fileName0);
        exit(1);
    }
}

void MISRA_C_ERROR(int iErrNumMajor, int iErrNumMinor)
{
    switch(iErrNumMajor) {
        // Standard Libraries
        case 7 : {
            switch(iErrNumMinor) {
                case 1  : PrintMisraFormat(iErrNumMajor, iErrNumMinor, ERRMSG_7_1);
                default : break;
            }

        }

        case 8 : {
            switch(iErrNumMinor) {
                case 10 : PrintMisraFormat(iErrNumMajor, iErrNumMinor, ERRMSG_8_10);
            }
        }
        case 21: {
            switch(iErrNumMinor) {
                case 4: PrintMisraFormat(iErrNumMajor, iErrNumMinor, ERRMSG_21_4);
                default : break;
            }
        }
        
        default : break;
    }
}

void PrintMisraFormat(int iErrNumMajor, int iErrNumMinor, char aErrMsg[])
{
    printf("\n\n\n\n\nErr No.%d_%d: %s\n", iErrNumMajor, iErrNumMinor, aErrMsg);
}