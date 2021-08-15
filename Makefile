CC = gcc
OBJ = obj
#VPATH = $(DIRS: =;)

TARGET = sender.out

DIRS = ./main
SRCS = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
SRCS += $(foreach dir, $(DIRS), $(wildcard $(dir)/*/*.c))

OBJS = $(SRCS:.c=.o)
TEMP = $(notdir $(OBJS))

all: $(TARGET) 
	
$(TARGET) : $(OBJS)
	cd $(OBJ); \
	$(CC) -o $(TARGET) $(TEMP) -g ; \
	mv $(TARGET) ..\

%.o: %.c
	$(CC) -c -o $(OBJ)/$(notdir $@) $< -g

clean :
	rm -rf $(OBJ) \
	rm -rf *.out

cleanlog:
	cd log; rm -rf *.log; cd ..