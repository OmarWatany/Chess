CFLAGS = -Wall -Wextra -Werror 
RAYLIB = -I./raylib/include/ -L./raylib/lib/ -lraylib -lGL -lm 
LIBS = $(RAYLIB)


install: chess.c data.c
	gcc $(CFLAGS)  chess.c data.c -o chess $(LIBS)

win: chess.c data.c
	x86_64-w64-mingw32-gcc chess.c data.c -o wnchess


# chess.o: chess.c
# 	gcc -c chess.c

clean: 
	rm chess.o
