#include "./data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Data *gdata;
void debug() {
    printf("%ld\n", sizeof(NM_OF_MOVES));
    printf("%ld\n", sizeof(*gdata->board->sets[0]->soldiers[8].otherdt));
    printf("%ld\n", sizeof(gdata->board->sets[0]->soldiers[8].otherdt->enpassant));
    printf("%ld\n", sizeof(bool));
}

int main() {
    char data[64];
    initgdata();

    CLEAR;
    printf("1. Start New Game\n");
    printf("2. Resume\n");
    printf("3. Cancel\n");
    printf("Enter option : ");
    if (!scanf(" %s", data))
        erroredEnd();
    switch (data[0]) {
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

    destroydata();
    return 0;
}

void game() {
    int valid;
    char data[64];
    bool RUNING_STAT = true;
    // CLEAR;
    while (RUNING_STAT) {
        drawBoard();
        printf("1. choose soldier");
        printf("\t2. quit\n");
        printf("Enter option : ");
        if (!scanf(" %s", data))
            erroredEnd();

        switch (data[0]) {
        case '1':
            valid = 0;
            while (valid != 1) {
                valid = moveSldr();
                drawBoard();
                if (valid == 2)
                    break; // if canceled don't mirror
                mirrorBoard(gdata->board);
            }
            break;
        case '2':
            RUNING_STAT = false;
            break;
        default:
            continue;
            break;
        }
        CLEAR;
    }
}

Board *createBoard(Set_t *white, Set_t *black) {
    Board *b = malloc(sizeof(Board));
    b->sets[0] = black;
    b->sets[1] = white;

    // initialize the square & set positions
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->Squares[i][j].occupied = false;
            b->Squares[i][j].sldr = NULL;
            if ((i + j) % 2 == 0) {
                b->Squares[i][j].color = gdata->colors[0];
            } else {
                b->Squares[i][j].color = gdata->colors[1];
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            if (black->soldiers[(i * 8) + j].State == LIVE) {
                b->Squares[i][j].occupied = true;
                b->Squares[i][j].sldr = &(black->soldiers[(i * 8) + j]);
                b->Squares[i][j].sldr->pos.col = j;
                b->Squares[i][j].sldr->pos.row = i;
            }
        }
    }

    for (int i = 6; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (white->soldiers[((i - 6) * 8) + j].State == LIVE) {
                b->Squares[i][j].occupied = true;
                b->Squares[i][j].sldr = &(white->soldiers[((i - 6) * 8) + j]);
                b->Squares[i][j].sldr->pos.col = j;
                b->Squares[i][j].sldr->pos.row = i;
            }
        }
    }

    return b;
}

Set_t *createSet(COLOR color) {
    Set_t *s = malloc(sizeof(Set_t));
    s->count = 16;
    s->color = color;
    s->soldiers = malloc(16 * sizeof(Soldier));

    Soldier_t types[4] = {ROOK, KNIGHT, BISHOP, PAWN};
    switch (color) {
    case BLACK:
        for (int i = 0; i < 3; i++) {
            s->soldiers[i].type = types[i];
            s->soldiers[7 - i].type = types[i];
        }
        s->soldiers[3].type = QUEEN;
        s->soldiers[4].type = KING;
        for (int i = 8; i < 16; i++) {
            s->soldiers[i].type = PAWN;
        }
        break;
    case WHITE:
        for (int i = 0; i < 3; i++) {
            s->soldiers[8 + i].type = types[i];
            s->soldiers[15 - i].type = types[i];
        }
        s->soldiers[11].type = QUEEN;
        s->soldiers[12].type = KING;
        for (int i = 0; i < 8; i++) {
            s->soldiers[i].type = PAWN;
        }
        break;
    }

    // Soldier soldiers[16];
    for (int i = 0; i < 16; i++) {
        s->soldiers[i].TEAM = s;
        s->soldiers[i].State = LIVE;
        s->soldiers[i].otherdt = NULL;
        if (s->soldiers[i].type == PAWN) {
            s->soldiers[i].otherdt = malloc(sizeof(OtherData));
            s->soldiers[i].otherdt->NMOVES = ZERO;
            s->soldiers[i].otherdt->enpassant = false;
        } else if (s->soldiers[i].type == KING) {
            s->soldiers[i].otherdt = malloc(sizeof(OtherData));
            s->soldiers[i].otherdt->NMOVES = ZERO;
            s->soldiers[i].otherdt->check = false;
        }
    }

    for (int i = 0; i < 16; i++) {
        switch (s->soldiers[i].type) {
        case PAWN:
            s->soldiers[i].shap = strdup("PW");
            break;
        case KNIGHT:
            s->soldiers[i].shap = strdup("KN");
            break;
        case BISHOP:
            s->soldiers[i].shap = strdup("BI");
            break;
        case ROOK:
            s->soldiers[i].shap = strdup("RK");
            break;
        case QUEEN:
            s->soldiers[i].shap = strdup("QN");
            break;
        case KING:
            s->soldiers[i].shap = strdup("KG");
            break;
        }
    }

    return s;
}

Set_t *onlyType(Soldier_t t, COLOR color) {
    Set_t *s = createSet(color);
    for (int i = 0; i < 16; i++) {
        if (s->soldiers[i].type != t) {
            s->soldiers[i].State = DEAD;
            s->count--;
        }
    }
    return s;
}

void mirrorBoard(Board *board) {
    Square temp, *sq1, *sq2;
    Position *tpos;

    // mirror each soldier position
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
            tpos = &board->sets[j]->soldiers[i].pos;
            tpos->row = 7 - tpos->row;
            tpos->col = 7 - tpos->col;
        }
    }

    // mirror the rows
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &gdata->board->Squares[7 - i][j];
            sq2 = &board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }

    // mirror the columns
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &gdata->board->Squares[i][7 - j];
            sq2 = &board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }
}

availableSqs *calcNextMove(Square *sq) {
    Soldier *sldr = sq->sldr;
    destroyAvList(gdata->available);
    switch (sldr->type) {
    case PAWN:
        return calcNextMovePawn(sq);
        break;
    case KNIGHT:
        return calcNextMoveKnight(sq);
        break;
    case BISHOP:
        return calcNextMoveBishop(sq);
        break;
    case ROOK:
        return calcNextMoveRook(sq);
        break;
    case QUEEN:
        return calcNextMoveQueen(sq);
        break;
    case KING:
        return calcNextMoveKing(sq);
        break;
    }
    return 0;
}

availableSqs *calcNextMovePawn(Square *fsq) {
    availableSqs *nextSqs = NULL;
    Square *nsq = NULL, *enmsq = NULL;
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row, praws = 0;
    // number possible squares

    nextSqs = dlist(5);
    if (fsq->sldr->otherdt->NMOVES == ZERO) {
        praws = 2;
    } else {
        praws = 1;
    }

    // check if there is victims in corners or beside him
    // check the next rows
    for (int i = 1; i <= praws; i++) {
        nsq = &gdata->board->Squares[row - i][colmn];
        if (!nsq->occupied)
            push(nextSqs, nsq);
    }
    // check the next colmns
    for (int i = -1; i <= 1; i++) {
        if (i != 0) {
            nsq = &(gdata->board->Squares[row - 1][colmn + i]);
            if (isEnemy(fsq, nsq))
                push(nextSqs, nsq);
        }
    }
    // to calc en passant
    // if my row is 3 and one of my adjacents were empty
    // then it became occupied
    // i can perform en passant & can't do it in next move
    //
    // check adjacent enemies
    if (fsq->sldr->pos.row == 3) {
        for (int i = -1; i <= 1; i++) {
            if (i != 0 && inBound(colmn + i)) {
                enmsq = &(gdata->board->Squares[row][colmn + i]);
                nsq = &(gdata->board->Squares[row - 1][colmn + i]);
                if (isEnemy(fsq, enmsq) && enmsq->sldr->otherdt->enpassant) {
                    push(nextSqs, nsq);
                }
            }
        }
    }
    return nextSqs;
}

availableSqs *calcNextMoveKnight(Square *fsq) {
    availableSqs *nextSqs;
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    nextSqs = dlist(8);
    int nrow, ncol;

    // set boundries

    Square *n = NULL;
    for (int col = -2; col <= 2; col++) {
        ncol = colmn + col;
        if (col == 0 || !inBound(ncol))
            continue;
        for (int r = -1; r <= 1; r++) {
            if (r == 0)
                continue;
            if (!(col % 2)) {
                nrow = row + r;
            } else {
                nrow = row + (2 * r);
            }
            if (!inBound(nrow))
                continue;
            n = &(gdata->board->Squares[nrow][ncol]);
            if (!n->occupied || isEnemy(fsq, n))
                push(nextSqs, n);
        }
    }

    return nextSqs;
}

availableSqs *calcNextMoveRook(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    // available squares before and after.
    //

    availableSqs *nextSqs;
    nextSqs = dlist(15);

    Square *n = NULL;
    for (int c = colmn + 1; c <= 7; c++) {
        n = &(gdata->board->Squares[row][c]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int c = colmn - 1; c >= 0; c--) {
        n = &(gdata->board->Squares[row][c]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row + 1; r <= 7; r++) {
        n = &(gdata->board->Squares[r][colmn]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row - 1; r >= 0; r--) {
        n = &(gdata->board->Squares[r][colmn]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    return nextSqs;
}

availableSqs *calcNextMoveBishop(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    availableSqs *nextSqs;
    nextSqs = dlist(15);

    Square *n = NULL;

    for (int i = 1; inBound(colmn + i) && inBound(row + i); i++) {
        n = &(gdata->board->Squares[row + i][colmn + i]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int i = 1; inBound(colmn - i) && inBound(row - i); i++) {
        n = &(gdata->board->Squares[row - i][colmn - i]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int i = 1; inBound(colmn + i) && inBound(row - i); i++) {
        n = &(gdata->board->Squares[row - i][colmn + i]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int i = 1; inBound(colmn - i) && inBound(row + i); i++) {
        n = &(gdata->board->Squares[row + i][colmn - i]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    return nextSqs;
}

availableSqs *calcNextMoveQueen(Square *f) {
    availableSqs *second = calcNextMoveBishop(f);
    availableSqs *first = calcNextMoveRook(f);
    availableSqs *nextSqs = mergeList(first, second);
    // number possible squares
    return nextSqs;
}

availableSqs *calcNextMoveKing(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;
    availableSqs *nextSqs;
    nextSqs = dlist(8);

    Square *n = NULL;
    for (int c = -1; c <= 1; c++) {
        for (int r = -1; r <= 1; r++) {
            if (inBound(row + r) && inBound(colmn + c)) {
                n = &(gdata->board->Squares[row + r][colmn + c]);
                if (isEnemy(fsq, n) || !n->occupied)
                    push(nextSqs, n);
            }
        }
    }
    return nextSqs;
}

bool isEnemy(Square *from, Square *to) {
    if (to->occupied && to->sldr->TEAM->color != from->sldr->TEAM->color)
        return true;
    return false;
}

Square *chooseSquare(Position pos) {
    if (pos.row != 10) {
        return &(gdata->board->Squares[pos.row][pos.col]);
    } else {
        return 0;
    }
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
    case WHITE:
        pos.row = 8 - raw;
        pos.col = col - 65;
        return pos;
        break;
    case BLACK:
        pos.row = raw - 1;
        pos.col = 'H' - col;
        return pos;
        break;
    }
    return pos;
}

int moveSldr() {
    Position pos = {0};
    pos = choosePos(FROM);

    Square *from = chooseSquare(pos);
    if (from == 0)
        // if it canceled return 2
        return 2;

    if (!from->occupied)
        return moveSldr();

    if (from->sldr->TEAM->color != gdata->ACTIVE) {
        switch (gdata->ACTIVE) {
        case WHITE:
            printf("White change\n");
            break;
        case BLACK:
            printf("Black change\n");
            break;
        }
        return moveSldr();
    }

    gdata->available = calcNextMove(from);

    if (gdata->available == NULL || gdata->available->top <= 0) {
        // if it can't move try again
        printf("Can't move\n");
        return moveSldr();
    }

    drawBoard();

    pos = choosePos(TO);
    Square *next = chooseSquare(pos);
    if (next == 0)
        return 2;

    if (isAvailable(next)) {
        from->occupied = false;
        // if he going to kill enemy
        if (next->occupied) {
            next->sldr->State = DEAD;
            next->sldr->TEAM->count--;
        }

        if (from->sldr->type == PAWN) {
            if (from->sldr->otherdt->NMOVES < MORE_THAN_ONE)
                (from->sldr->otherdt->NMOVES)++;

            for (int i = 0; i < 8; i++) {
                if (gdata->board->sets[1]->soldiers[i].otherdt->enpassant)
                    gdata->board->sets[1]->soldiers[i].otherdt->enpassant = false;
                if (gdata->board->sets[0]->soldiers[i + 8].otherdt->enpassant)
                    gdata->board->sets[0]->soldiers[i + 8].otherdt->enpassant =
                        false;
            }

            if (pos.row == 3) {
                if (from->sldr->otherdt->NMOVES == ZERO) {
                    from->sldr->otherdt->enpassant = true;
                } /* else
                     from->sldr->otherdt->enpassant = COULD_BE;
                 */
            }

            if (pos.row == 2 && pos.col != from->sldr->pos.col) {
                gdata->board->Squares[from->sldr->pos.row][pos.col].sldr->State =
                    DEAD;
                gdata->board->Squares[from->sldr->pos.row][pos.col]
                    .sldr->TEAM->count--;
                gdata->board->Squares[from->sldr->pos.row][pos.col].occupied = false;
            }
        }
        next->sldr = from->sldr;
        next->occupied = true;
        from->sldr = NULL;
        next->sldr->pos = pos;
        colorBoardSquares();
        changeActive();
        return 1;
    }
    return 2;
}

bool isAvailable(Square *sq) {
    if (gdata->available != NULL) {
        for (int i = 0; i < gdata->available->count; i++) {
            if (sq == gdata->available->stack[i]) {
                return true;
            }
        }
    }
    return false;
}

// I think it's a bad practice but it's cool
void changeActive() { gdata->ACTIVE = !gdata->ACTIVE; }

void drawBoard() {
    // draw gdata->board squares and soldiers
    //
    CLEAR;
    switch (gdata->ACTIVE) {
    case BLACK:
        drawWhileBlack();
        break;
    case WHITE:
        drawWhileWhite();
        break;
    }
}

void drawWhileWhite() {
    printf("    \033[;31mBLACK  soldiers count : %d\033[;0m\n",
           gdata->board->sets[0]->count);
    for (int i = 0; i < 8; i++) {
        printf("%d  ", 8 - i);
        for (int j = 0; j < 8; j++) {
            displaySq(&gdata->board->Squares[i][j]);
            // displaySqPosition(&gdata->board->Squares[i][j]);
        }
        printf("\n");
    }
    printf("  ");
    for (int i = 0; i < 8; i++) {
        printf(" %2c ", 'A' + i);
        // printf(" %2c ", 'A' + i);
    }
    printf("\n");
    printf("    WHITE  soldiers count : %d\n", gdata->board->sets[1]->count);
}

void drawWhileBlack() {
    printf("    WHITE  soldiers count : %d\n", gdata->board->sets[1]->count);
    for (int i = 0; i < 8; i++) {
        printf("%d  ", i + 1);
        for (int j = 0; j < 8; j++) {
            displaySq(&gdata->board->Squares[i][j]);
            // displaySqPosition(&gdata->board->Squares[i][j]);
        }
        printf("\n");
    }
    printf("  ");
    for (int i = 0; i < 8; i++) {
        printf(" %2c ", 'H' - i);
        // printf(" %2c ", 'A' + i);
    }
    printf("\n");
    printf("    \033[;31m%s  soldiers count : %d\033[;0m\n", "BLACK",
           gdata->board->sets[0]->count);
}

void displaySq(Square *sq) {
    if (sq->occupied) {
        if (sq->sldr->TEAM->color == BLACK) {
            printf(" \033[;31m%s\033[;0m ", sq->sldr->shap);
        } else {
            printf(" %s ", sq->sldr->shap);
        }
    } else {
        printf("%s%s", sq->color, sq->color);
        printf("%s%s", sq->color, sq->color);
    }
}

void displayNextSqsList() {
    for (int i = 1; i <= gdata->available->top; i++) {
        if (i % 7 == 0)
            printf("\n");
    }
    printf("\n");
}

void colorBoardSquares() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 2 == 0) {
                gdata->board->Squares[i][j].color = gdata->colors[0];
            } else {
                gdata->board->Squares[i][j].color = gdata->colors[1];
            }
        }
    }
}

bool inBound(int pos) {
    if (0 <= pos && pos <= 7)
        return true;
    return false;
}

availableSqs *dlist(unsigned int size) {
    availableSqs *temp = malloc(sizeof(availableSqs));
    temp->top = 0;
    temp->count = size;
    temp->stack = malloc(temp->count * sizeof(Square *));
    return temp;
}

void push(availableSqs *st, Square *sq) {
    if (st->top == st->count - 1) {
        stackResize(st, 5);
    }
    sq->color = gdata->colors[2];
    st->stack[st->top] = sq;
    st->top += 1;
}

void stackResize(availableSqs *st, int size) {
    if (st) {
        st->count += size;
        Square **nst = realloc(st->stack, sizeof(Square *) * st->count);
        if (nst)
            st->stack = nst;
    }
}

void destroydata() {
    for (int i = 0; i < 16; i++) {
        free(gdata->board->sets[0]->soldiers[i].shap);
        free(gdata->board->sets[1]->soldiers[i].shap);
    }
    for (int i = 0; i < 8; i++) {
        free(gdata->board->sets[0]->soldiers[i + 8].otherdt);
        free(gdata->board->sets[1]->soldiers[i].otherdt);
    }
    free(gdata->board->sets[0]->soldiers[4].otherdt);
    free(gdata->board->sets[1]->soldiers[12].otherdt);

    free(gdata->board->sets[0]->soldiers);
    free(gdata->board->sets[1]->soldiers);

    for (int i = 0; i < 2; i++)
        free(gdata->board->sets[i]);

    destroyAvList(gdata->available);
    free(gdata->colors);
    free(gdata->board);
    free(gdata);
}

void destroyAvList(availableSqs *st) {
    if (st != NULL) {
        free(st->stack);
        free(st);
        // st = NULL;
    }
}

void erroredEnd() {
    destroydata();
    printf("there is an error\n");
    exit(1);
}

availableSqs *mergeList(availableSqs *first, availableSqs *second) {
    availableSqs *nextSqs;
    nextSqs = first;
    for (int i = 0; i < second->top; i++) {
        push(first, second->stack[i]);
    }
    destroyAvList(second);
    return nextSqs;
}

char **initColor() {
    char **colors = malloc(3 * sizeof(char *));
    colors[0] = "█";                  // white
    colors[1] = "\033[;31m█\033[;0m"; // red
    colors[2] = "\033[;32m█\033[;0m"; // green
    return colors;
}

void initgdata() {
    gdata = malloc(sizeof(Data));
    Set_t *white = createSet(WHITE);
    Set_t *black = createSet(BLACK);
    gdata->colors = initColor();
    gdata->board = createBoard(white, black);
    gdata->ACTIVE = WHITE;
    gdata->available = NULL;
    colorBoardSquares();
}
