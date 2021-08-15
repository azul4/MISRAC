#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct _date {

	int year;

	int month;

	int day;

}date;



typedef struct _student {

	char student_num[30];

	char student_name[20];

	date* student_birthday;

} student;



typedef struct _year {

	int member_year;

}year;





int main()

{



	printf("포인터를 구조체 멤버로 선언하는 프로그램");

	student sStudent;

	date* sStudent_birthday = malloc(sizeof(date));



    printf("여기\n");

	sStudent_birthday->day = 2002;

	sStudent_birthday->month = 5;

	sStudent_birthday->day = 5;



	strcpy(sStudent.student_num, "20210712");

	strcpy(sStudent.student_name, "홍길동");

	sStudent.student_birthday = sStudent_birthday;







	printf("학번 : %s", sStudent.student_num);

	printf("성명 : %s", sStudent.student_name);

	printf("생년월일 : %d %d %d", sStudent.student_birthday->year, sStudent.student_birthday->month, sStudent.student_birthday->day);

}