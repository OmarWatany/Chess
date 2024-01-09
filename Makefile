CFLAGS = -Wall -Wextra -Werror -g
# LIBS = -lncurses

install: chess.c data.c
	gcc $(CFLAGS) chess.c data.c -o chess

win: chess.c data.c
	x86_64-w64-mingw32-gcc chess.c data.c -o wnchess


# chess.o: chess.c
# 	gcc -c chess.c

clean: 
	rm chess.o
