CC = gcc
OBJ = obj
#VPATH = $(DIRS: =;)

TARGET = compiler.out

DIRS = ./
SRCS = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))

OBJS = $(SRCS:.c=.o)
TEMP = $(notdir $(OBJS))

%.o: %.c
	$(CC) -c -o $(OBJ)/$(notdir $@) $< -g

all: 
	gcc -c -o $(OBJ)/codegen.o codegen.c -g
	gcc -c -o $(OBJ)/compile.o compile.c -g
	gcc -c -o $(OBJ)/getSource.o getSource.c -g
	gcc -c -o $(OBJ)/table.o table.c -g
	gcc -o $(TARGET) $(OBJ)/codegen.o $(OBJ)/compile.o $(OBJ)/getSource.o $(OBJ)/table.o -g
	



clean :
	rm -rf ./$(OBJ)/*.o
	rm -rf *.out

cleanlog:
	cd log; rm -rf *.log; cd ..