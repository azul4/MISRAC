#include "main.h"

char aHeaderIncluded[3][20] = {0};
int iHeaderStackPointer = 0;

int main()
{
    FILE *fp;
    CodeInfo source_info;
    const char* path = "totest.c";
    char aBuf[50] = {0};

    init_info(&source_info);

    if((fp = fopen(path, "r")) != NULL) {
        batch_analyze(fp, &source_info);
        analyze_structure(fp);
        while(fgets(aBuf, sizeof(aBuf), fp) != NULL) {
            analyze_sentence(fp);
        }
        fclose(fp);
    }
}

void init_info(CodeInfo* s)
{
    s->current_line = 0;
    strcpy(s->function, "");
    s->start_line = 0;
    s->end_line = 0;
}

void batch_analyze(FILE* fp, CodeInfo* s)
{
    s->current_line = get_lines(fp);
    strcpy(s->function, "main");
    s->start_line = 0;
    s->end_line = get_lines(fp);
}

void check_MISRA_C_rules(void)
{
    //check_Rule_2();
    //check_Rule_3();
    //check_Rule_4();
} 

void analyze_sentence(const char aBuf[])
{
    if(aBuf[0] == '#'){ //#define, #include ...
        strcpy(aHeaderIncluded[iHeaderStackPointer++], aBuf);
    } else if {
        ;
    }
}

int32_t get_lines(FILE* fp)
{
    int32_t lines = 0;
    char dump[100];
    while (fgets (dump , 100 , fp) != NULL ) lines++;
    printf("lines = %d\n", lines);

    return lines;
}

void analyze_structure(FILE* fp)
{
    int32_t lines = 0;
    int main_start_line = 0;
    int main_end_line = 0;
    char dump[100];

    while (fgets (dump , 100 , fp) != NULL )
    {
        while(main_flag == RAISED){

            main_start_line = get_main(fp);
        }
        
        lines++;
    } 
    printf("lines = %d\n", lines);

    return lines;
}

int get_main(FILE* fp)
{
    int main_line = 0;
    char dump[100] = {0};

    while(strcpy("int main()", fgets(dump, 100, fp)) != STRING_SAME) main_line++;
    main_line += 1;

    return main_line;
}