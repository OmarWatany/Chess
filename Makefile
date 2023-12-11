CFLAGS = -Wall -Wextra -Werror
# LIBS = -lncurses

install: chess.c 
	gcc chess.c -o chess

win: chess.c
	x86_64-w64-mingw32-gcc chess.c -o wnchess

# chess.o: chess.c
# 	gcc -c chess.c

clean: 
	rm chess.o
