#include "chess.h"
#include "data.h"
#include "garraylist.h"
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

void initColors(char *colors[3]);
void initShapes();
void debug();

Context *gdata = &ctx;

char *colors[3];
char *shapes[KING + 1];

void erroredEnd() {
    destroyData();
    printf("there is an error\n");
    exit(1);
}

int printMenu() {
    char data[64];
    CLEAR;
    printf("1. Start New Game\n");
    printf("2. Resume\n");
    printf("3. Quit\n");
    printf("Enter option : ");
    if (!scanf(" %s", data)) erroredEnd();
    return data[0];
}

void menu() {
    int in = printMenu();

    switch (in) {
    case '1':
        game();
        save(gdata);
        break;
    case '2':
        load(gdata);
        game();
        save(gdata);
        break;
    case '3':
        break;
    case 'd':
        debug();
        break;
    default:
        break;
    }
}

int main() {
    initColors(colors);
    initGameData();
    initShapes();
    // initgdata();

    menu();

    destroyData();
    return 0;
}

Position choosePos(CHANGE change) {
    Position pos = {10, 10};
    /* on  cancel return  {10,x}
     * on  errors rerturn {20,x} */

    int valid, raw;
    char buffer[64], col;

    switch (change) {
    case FROM:
        printf("From : ");
        break;
    case TO:
        printf("To : ");
        break;
    }

    valid = scanf("%s", buffer);
    if (!valid || strlen(buffer) > 2) {
        if (strcmp(buffer, "cancel") == 0) {
            colorBoardSquares();
            return pos;
        }
        choosePos(change);
    }

    col = buffer[0];
    raw = buffer[1] - '0';
    // if it's from a to h capitalize it
    if ('a' <= col && col <= 'h') {
        // to upper case;
        col = col - 32;
    }

    if (!(72 >= col && col >= 65 && 1 <= raw && raw <= 8)) {
        choosePos(change);
    }

    switch (gdata->ACTIVE) {
    case WHITE_TEAM:
        pos.row = 8 - raw;
        pos.col = col - 65;
        return pos;
        break;
    case BLACK_TEAM:
        pos.row = raw - 1;
        pos.col = 'H' - col;
        return pos;
        break;
    }
    return pos;
}
void printCommands() {
    printf("1. choose soldier");
    printf("\t2. quit\n");
    printf("Command : ");
}

void game() {
    int valid;
    char data[64];
    bool RUNING_STAT = true;
    // CLEAR;
    while (RUNING_STAT) {
        valid = 0;
        drawBoard();
        printCommands();
        if (!scanf(" %s", data)) erroredEnd();
        Position tArrPos;
        switch (data[0]) {
        case '1':
            while (valid != 1) {
                drawBoard();
                tArrPos = choosePos(ctx.movementChange);
                if (ctx.movementChange == FROM) {
                    valid = moveFrom(tArrPos);
                } else {
                    valid = moveTo(tArrPos);
                    if (valid == 1) {
                        Square *nextSq = chooseSquare(tArrPos);
                        Soldier *sldr = nextSq->sldr;
                        sldr->arrPos = tArrPos;
                        mirrorBoard();
                    }
                }
            }
            break;
        case '2':
            RUNING_STAT = false;
            break;
        default:
            break;
        }
        CLEAR;
    }
}

void initShapes() {
    shapes[PAWN] = strdup("PW");
    shapes[KNIGHT] = strdup("KN");
    shapes[BISHOP] = strdup("BI");
    shapes[ROOK] = strdup("RK");
    shapes[QUEEN] = strdup("QN");
    shapes[KING] = strdup("KG");
}

void drawBoard() {
    // draw gdata->board squares and soldiers
    CLEAR;
    switch (gdata->ACTIVE) {
    case BLACK_TEAM:
        drawWhileBlack();
        break;
    case WHITE_TEAM:
        drawWhileWhite();
        break;
    }
}

void drawWhileWhite() {
    printf("    \033[;31mBLACK  soldiers count : %d\033[;0m\n", ctx.board.sets[0].count);
    for (int y = 0; y < 8; y++) {
        printf("%d  ", 8 - y);
        for (int x = 0; x < 8; x++) {
            drawSq((Position){.col = x, .row = y});
        }
        printf("\n");
    }
    printf("  ");
    for (int i = 0; i < 8; i++) {
        printf(" %2c ", 'A' + i);
        // printf(" %2c ", 'A' + i);
    }
    printf("\n");
    printf("    WHITE  soldiers count : %d\n", ctx.board.sets[1].count);
}

void drawWhileBlack() {
    printf("    WHITE  soldiers count : %d\n", ctx.board.sets[1].count);
    for (int y = 0; y < 8; y++) {
        printf("%d  ", y + 1);
        for (int x = 0; x < 8; x++) {
            drawSq((Position){.col = x, .row = y});
        }
        printf("\n");
    }
    printf("  ");
    for (int i = 0; i < 8; i++) {
        printf(" %2c ", 'H' - i);
        // printf(" %2c ", 'A' + i);
    }
    printf("\n");
    printf("    \033[;31m%s  soldiers count : %d\033[;0m\n", "BLACK", ctx.board.sets[0].count);
}

void drawSq(Position pos) {
    Square *sq = chooseSquare(pos);
    char *sqColor = colors[SquareColors[pos.row][pos.col]];
    if (sq->occupied) {
        if (sq->sldr->team_set->teamColor == BLACK_TEAM) {
            printf(" \033[;31m%s\033[;0m ", shapes[sq->sldr->type]);
        } else {
            printf(" %s ", shapes[sq->sldr->type]);
        }
    } else {
        printf("%s%s", sqColor, sqColor);
        printf("%s%s", sqColor, sqColor);
    }
}

void initColors(char *colors[3]) {
    colors[0] = strdup("█");                  // white
    colors[1] = strdup("\033[;31m█\033[;0m"); // red
    colors[2] = strdup("\033[;32m█\033[;0m"); // green
}

void destroyData() {
    for (int i = PAWN; i < KING + 1; i++)
        free(shapes[i]);

    for (int i = 0; i < 4; i++)
        free(colors[i]);

    arena_destroy(&global_arena);
    alist_destroy(&ctx.availableSqs);
}
