CC = gcc
OBJ = obj
#VPATH = $(DIRS: =;)

TARGET = compiler.out

DIRS = ./main
SRCS = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
SRCS += $(foreach dir, $(DIRS), $(wildcard $(dir)/*/*.c))

OBJS = $(SRCS:.c=.o)
TEMP = $(notdir $(OBJS))

%.o: %.c
	$(CC) -c -o $(OBJ)/$(notdir $@) $< -g

all: $(TARGET) 
	
$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(TEMP) -g ; \
	mv $(TARGET) ..\



clean :
	rm -rf $(OBJ) \
	rm -rf *.out

cleanlog:
	cd log; rm -rf *.log; cd ..