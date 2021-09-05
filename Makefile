OBJS = src/*.c 
CC = gcc

COMPILER_FLAGS = -Werror -Wfloat-conversion -ggdb -g 

LINKER_FLAGS = 0

ifeq ($(OS), Windows_NT)
	LINKER_FLAGS = -lmingw32
else
	LINKER_FLAGS = -lm
endif

OBJ_NAME = VirtualMachine

all : $(OBJS) 
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME) 