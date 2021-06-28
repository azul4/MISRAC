#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct _CodeInfo {
    int current_line;
    char function[30];
    int start_line;
    int end_line;
} CodeInfo;

void check_MISRA_C_rules(void);
void init_info(CodeInfo *s);
void batch_analyze(FILE* fp, CodeInfo* s);
int32_t get_lines(FILE* fp);

#endif