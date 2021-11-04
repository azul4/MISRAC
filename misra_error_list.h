#include <stdio.h>
#include <stdlib.h>
void MISRA_C_ERROR();
void initErrorListupFile();
void PrintMisraFormat(int iErrNumMajor, int iErrNumMinor, char aErrMsg[]);

#define ERRMSG_7_1  "Octal Constants Shall Not Be Used"
#define ERRMSG_21_4 "The Standard Header File <signal.h> Shall Not Be Used"
#define ERRMSG_8_10 "Comma Operation Shall Not Be Used"