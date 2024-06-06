#OBJS specifies which files to compile as part of the project
OBJS = src/main.c

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = bin/program

#This is the target that compiles our executable
all : $(OBJS)
	gcc $(OBJS) -w -lSDL2 -lSDL2_image -lm -o $(OBJ_NAME)
	./$(OBJ_NAME)

program :
	./$(OBJ_NAME)

clean:
	rm bin/*

viewport:
	gcc src/viewport_demo.c -w -lSDL2 -lSDL2_image -lm -o bin/viewport

wolf:
	gcc src/main_wolf.c -w -lSDL2 -lSDL2_image -lm -o bin/wolf

object_renderer:
	gcc src/object_renderer.c -w -lSDL2 -lSDL2_image -lm -o bin/object_renderer

testcode:
	gcc src/test_code.c -w -lSDL2 -lSDL2_image -lm -o bin/test_code
