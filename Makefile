CFLAGS = -Wall -Wextra -g
RAYLIB = -I./raylib/include/ -L./raylib/lib/ -lraylib -lGL -lm 
GDSLIB = -I./gdslib/include/ -L./gdslib/lib/ -lgdslib 
LIBS = $(RAYLIB) $(GDSLIB)


install: chess.c data.c
	gcc $(CFLAGS) chess.c data.c -o chess $(LIBS)

win: chess.c data.c
	x86_64-w64-mingw32-gcc chess.c data.c -o wnchess


# chess.o: chess.c
# 	gcc -c chess.c

clean: 
	rm chess.o
