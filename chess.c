#include "./chess.h"
#include "data.h"
#include "raylib/include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Context *context;

int main() {
    initgdata();

    game();

    destroydata();

    return 0;
}

void cancelMovment() {
    context->movementChange = FROM;
    colorBoardSquares();
}

void game() {
    int valid = 0;
    while (!WindowShouldClose()) {
        valid = 0;
        ClearBackground(BROWN);
        BeginDrawing();
        drawBoard();

        if (IsKeyReleased(KEY_Q)) {
            break;
        } else if (IsKeyReleased(KEY_F)) {
            ToggleFullscreen();
        } else if (IsKeyReleased(KEY_C)) {
            // cancel movment
            cancelMovment();
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Position pos = getPos();
            valid = moveSldr(pos);
            if (valid == 1) mirrorBoard();
        }
        EndDrawing();
    }
}

int moveSldr(Position pos) {
    Square **from = &context->fromSquare, *next;
    switch (context->movementChange) {
    case FROM:
        (*from) = chooseSquare(pos);
        // if choosed empty square or non active team's soldier
        if (!(*from)->occupied || (*from)->sldr->TEAM->color != context->ACTIVE) {
            colorBoardSquares();
            return 0;
        }
        context->available = calcNextMove(*from);
        if (context->available == NULL || context->available->top <= 0) {
            colorBoardSquares();
            return 0;
        }
        context->movementChange = TO;
        return 2;
    case TO:
        next = chooseSquare(pos);
        if (next->occupied && next->sldr->TEAM->color == context->ACTIVE) {
            cancelMovment();
            moveSldr(pos);
            return 2;
        }
        if (isAvailable(next)) {
            // if he is going to kill an enemy
            if (next->occupied) {
                killEnemey(next->sldr);
            }

            if ((*from)->sldr->type == PAWN) {
                for (int i = 0; i < 8; i++) {
                    if (context->board->sets[1]->soldiers[i].otherdt->enpassant)
                        context->board->sets[1]->soldiers[i].otherdt->enpassant = false;
                    if (context->board->sets[0]->soldiers[i + 8].otherdt->enpassant)
                        context->board->sets[0]->soldiers[i + 8].otherdt->enpassant = false;
                }

                if (pos.row == 4 && (*from)->sldr->otherdt->NMOVES == ZERO) {
                    (*from)->sldr->otherdt->enpassant = true;
                }

                if ((*from)->sldr->otherdt->NMOVES < MORE_THAN_ONE) ((*from)->sldr->otherdt->NMOVES)++;

                if (pos.row == 2 && pos.col != (*from)->sldr->pos.col) {
                    killEnemey(context->board->Squares[(*from)->sldr->pos.row][pos.col].sldr);
                    context->board->Squares[(*from)->sldr->pos.row][pos.col].occupied = false;
                }
            }

            (*from)->occupied = false;
            next->sldr = (*from)->sldr;
            next->occupied = true;
            (*from)->sldr = NULL;
            next->sldr->pos = pos;
            context->movementChange = FROM;
            colorBoardSquares();
            changeActive();
            return 1;
        }
    }
    return 0;
}

void killEnemey(Soldier *sldr) {
    sldr->State = DEAD;
    sldr->TEAM->count--;
}

Position getPos() {
    Vector2 mouse = GetMousePosition();
    Position pos = {10, 10};
    pos.col = mouse.x / BOARD_WIDTH * 8;
    pos.row = mouse.y / BOARD_WIDTH * 8;
    return pos;
}

Soldier *selectSldr(Position SqPos) {
    return context->board->Squares[SqPos.row][SqPos.col].sldr;
}

Board *createBoard(Set_t *white, Set_t *black) {
    Board *b = malloc(sizeof(Board));
    b->sets[0] = black;
    b->sets[1] = white;

    // initialize the square & set positions
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->Squares[i][j].rectangle.x = j * SQUARE_WIDTH;
            b->Squares[i][j].rectangle.y = i * SQUARE_WIDTH;
            b->Squares[i][j].rectangle.width = SQUARE_WIDTH;
            b->Squares[i][j].rectangle.height = SQUARE_WIDTH;
            b->Squares[i][j].occupied = false;
            b->Squares[i][j].sldr = NULL;
            b->Squares[i][j].color = context->colors[!isEven(i + j)];
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

Set_t *createSet(TEAM_COLOR color) {
    Set_t *s = malloc(sizeof(Set_t));
    s->count = 16;
    s->color = color;
    s->soldiers = malloc(16 * sizeof(Soldier));

    SOLDIER_TYPE types[4] = {ROOK, KNIGHT, BISHOP, PAWN};
    switch (color) {
    case BLACK_TEAM:
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
    case WHITE_TEAM:
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
    char *sldr_types[] = {"pawn", "knight", "bishop", "rook", "queen", "king"};
    char *team_color[] = {"white", "black"};
    float scale_factor = 0.85f;
    for (int i = 0; i < 16; i++) {
        Image soldierImage = {0};
        soldierImage = LoadImage(TextFormat("./assets/"
                                            "128px/"
                                            "%s_%s_png_shadow.png",
                                            team_color[s->color], sldr_types[s->soldiers[i].type - 1]));
        ImageResize(&soldierImage, SQUARE_WIDTH * scale_factor, SQUARE_WIDTH * scale_factor);
        s->soldiers[i].shapText = LoadTextureFromImage(soldierImage);
        UnloadImage(soldierImage);
    }

    return s;
}

Set_t *onlyType(SOLDIER_TYPE t, TEAM_COLOR color) {
    Set_t *s = createSet(color);
    for (int i = 0; i < 16; i++) {
        if (s->soldiers[i].type != t) {
            s->soldiers[i].State = DEAD;
            s->count--;
        }
    }
    return s;
}

void mirrorBoard() {
    Square temp, *sq1, *sq2;
    Position *tpos;

    // mirror each soldier position
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
            tpos = &context->board->sets[j]->soldiers[i].pos;
            tpos->row = 7 - tpos->row;
            tpos->col = 7 - tpos->col;
        }
    }

    // mirror rectangles
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &context->board->Squares[7 - i][j];
            sq2 = &context->board->Squares[i][j];
            temp = *sq1;
            sq1->rectangle = sq2->rectangle;
            sq2->rectangle = temp.rectangle;
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &context->board->Squares[i][7 - j];
            sq2 = &context->board->Squares[i][j];
            temp = *sq1;
            sq1->rectangle = sq2->rectangle;
            sq2->rectangle = temp.rectangle;
        }
    }

    // mirror rows
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &context->board->Squares[7 - i][j];
            sq2 = &context->board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }

    // mirror columns
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &context->board->Squares[i][7 - j];
            sq2 = &context->board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }
}

availableSqs *calcNextMove(Square *sq) {
    Soldier *sldr = sq->sldr;
    destroyAvList(context->available);
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
        nsq = &context->board->Squares[row - i][colmn];
        if (!nsq->occupied) push(nextSqs, nsq);
    }
    // check the next colmns
    for (int i = -1; i <= 1; i++) {
        if (i != 0) {
            nsq = &(context->board->Squares[row - 1][colmn + i]);
            if (isEnemy(fsq, nsq)) push(nextSqs, nsq);
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
            if (i != 0 && inBoundaries(colmn + i)) {
                enmsq = &(context->board->Squares[row][colmn + i]);
                nsq = &(context->board->Squares[row - 1][colmn + i]);
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
        if (col == 0 || !inBoundaries(ncol)) continue;
        for (int r = -1; r <= 1; r++) {
            if (r == 0) continue;
            if (!(col % 2)) {
                nrow = row + r;
            } else {
                nrow = row + (2 * r);
            }
            if (!inBoundaries(nrow)) continue;
            n = &(context->board->Squares[nrow][ncol]);
            if (!n->occupied || isEnemy(fsq, n)) push(nextSqs, n);
        }
    }

    return nextSqs;
}

availableSqs *calcNextMoveRook(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    // available squares before and after.
    //

    availableSqs *nextSqs = NULL;
    nextSqs = dlist(15);
    Square *n = NULL;
    for (int c = colmn + 1; c <= 7; c++) {
        n = &(context->board->Squares[row][c]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int c = colmn - 1; c >= 0; c--) {
        n = &(context->board->Squares[row][c]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row + 1; r <= 7; r++) {
        n = &(context->board->Squares[r][colmn]);
        if (!n->occupied) {
            push(nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            push(nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row - 1; r >= 0; r--) {
        n = &(context->board->Squares[r][colmn]);
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

    Square *next = NULL;

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row + i); i++) {
        next = &(context->board->Squares[row + i][colmn + i]);
        if (!next->occupied) {
            push(nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            push(nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row - i); i++) {
        next = &(context->board->Squares[row - i][colmn - i]);
        if (!next->occupied) {
            push(nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            push(nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row - i); i++) {
        next = &(context->board->Squares[row - i][colmn + i]);
        if (!next->occupied) {
            push(nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            push(nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row + i); i++) {
        next = &(context->board->Squares[row + i][colmn - i]);
        if (!next->occupied) {
            push(nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            push(nextSqs, next);
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
            if (inBoundaries(row + r) && inBoundaries(colmn + c)) {
                n = &(context->board->Squares[row + r][colmn + c]);
                if (isEnemy(fsq, n) || !n->occupied) push(nextSqs, n);
            }
        }
    }
    return nextSqs;
}

bool isEnemy(Square *from, Square *to) {
    if (to->occupied && to->sldr->TEAM->color != from->sldr->TEAM->color) return true;
    return false;
}

Square *chooseSquare(Position pos) {
    if (pos.row != 10) {
        return &(context->board->Squares[pos.row][pos.col]);
    } else
        return 0;
}

bool isAvailable(Square *sq) {
    if (context->available != NULL) {
        for (int i = 0; i < context->available->count; i++) {
            if (sq == context->available->stack[i]) {
                return true;
            }
        }
    }
    return false;
}

// I think it's a bad practice but it's cool
void changeActive() {
    context->ACTIVE = !context->ACTIVE;
}

void drawBoard() {
    // draw context->board squares and soldiers
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            drawSq(&context->board->Squares[i][j]);
        }
    }
}

void drawSq(Square *sq) {
    DrawRectangleRec(sq->rectangle, sq->color);
    if (sq->occupied)
        DrawTexture(sq->sldr->shapText, sq->rectangle.x + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f),
                    sq->rectangle.y + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f), WHITE);
}

void colorBoardSquares() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            context->board->Squares[i][j].color = context->colors[(i + j) % 2];
        }
    }
}

bool inBoundaries(int pos) {
    if (0 <= pos && pos <= 7) return true;
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
    if (st->top == st->count - 1) stackResize(st, 5);
    sq->color = context->colors[2];
    st->stack[st->top] = sq;
    st->top += 1;
}

void stackResize(availableSqs *st, int size) {
    if (!st) return;
    st->count += size;
    Square **nst = realloc(st->stack, sizeof(Square *) * st->count);
    if (nst) st->stack = nst;
}

void destroyAvList(availableSqs *st) {
    if (st != NULL) {
        free(st->stack);
        free(st);
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

Color *initColor() {
    Color *colors = malloc(3 * sizeof(char *));
    colors[0] = DARKBROWN;
    colors[1] = BEIGE;
    colors[2] = GREEN;
    return colors;
}

void initgdata() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "Chess");
    SetTargetFPS(60);
    context = malloc(sizeof(Context));
    Set_t *white = createSet(WHITE_TEAM);
    Set_t *black = createSet(BLACK_TEAM);
    context->colors = initColor();
    context->board = createBoard(white, black);
    context->ACTIVE = WHITE_TEAM;
    context->available = NULL;
    context->movementChange = FROM;
    context->fromSquare = NULL;
    colorBoardSquares();
}

void destroydata() {
    for (int i = 0; i < 16; i++) {
        UnloadTexture(context->board->sets[0]->soldiers[i].shapText);
        UnloadTexture(context->board->sets[1]->soldiers[i].shapText);
    }
    CloseWindow();
    for (int i = 0; i < 8; i++) {
        free(context->board->sets[0]->soldiers[i + 8].otherdt);
        free(context->board->sets[1]->soldiers[i].otherdt);
    }
    free(context->board->sets[0]->soldiers[4].otherdt);
    free(context->board->sets[1]->soldiers[12].otherdt);

    free(context->board->sets[0]->soldiers);
    free(context->board->sets[1]->soldiers);

    for (int i = 0; i < 2; i++)
        free(context->board->sets[i]);

    destroyAvList(context->available);
    free(context->colors);
    free(context->board);
    free(context);
}

void debug() {
    printf("%ld\n", sizeof(Context));
}
