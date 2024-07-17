CFLAGS = -Wall -Wextra -g
RAYLIB = -I./raylib/include/ -L./raylib/lib/ -lraylib -lGL -lm 
GDSLIB = -I./gdslib/include/ -L./gdslib/lib/ -lgdslib 
LIBS = $(RAYLIB) $(GDSLIB)
CC = gcc


linux : chess.c data.c
	$(CC) $(CFLAGS) chess.c moves.c data.c -o chess $(LIBS)

run : linux 
	@./chess

win: chess.c data.c
	x86_64-w64-mingw32-gcc chess.c data.c -o wnchess


# chess.o: chess.c
# 	gcc -c chess.c

clean: 
	rm chess.o
