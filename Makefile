OBJ2 = commander2.cpp

CC = g++

COMPILER_FLAGS = -w

LINKER_FLAGS = -lSDL2 -lSDL2_image

OBJ_NAME2 = commander2

all: 
	$(CC) $(OBJ2) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME2)
	gcc -o Control_PC Control_PC.c