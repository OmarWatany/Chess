#include "chess.h"
#include "garraylist.h"

float cLerp(float start, float end, float amount) {
    float result = start + amount * (end - start);
    return result;
}

int moveSldr(Position pos) {
    Square **from = &ctx.fromSquare, *next;
    switch (ctx.movementChange) {
    case FROM:
        (*from) = chooseSquare(pos);
        // If chose empty square or non active team's soldier
        if (!(*from)->occupied || (*from)->sldr->TEAM->color != ctx.ACTIVE) {
            colorBoardSquares();
            return 0;
        }
        ctx.availableSqs = calcNextMove(*from);
        if (alist_empty(&ctx.availableSqs)) {
            colorBoardSquares();
            return 0;
        }
        ctx.movementChange = TO;
        return 2;
    case TO:
        next = chooseSquare(pos);
        if (next->occupied && next->sldr->TEAM->color == ctx.ACTIVE) {
            cancelMovment();
            moveSldr(pos);
            return 2;
        }
        if (isAvailable(next)) {
            // If he is going to kill an enemy
            if (next->occupied) {
                killEnemey(next->sldr);
            }

            if ((*from)->sldr->type == PAWN) {
                for (int i = 0; i < 8; i++) {
                    if (ctx.board->sets[1]->soldiers[i].otherdt->enpassant)
                        ctx.board->sets[1]->soldiers[i].otherdt->enpassant = false;
                    if (ctx.board->sets[0]->soldiers[i + 8].otherdt->enpassant)
                        ctx.board->sets[0]->soldiers[i + 8].otherdt->enpassant = false;
                }

                if (pos.row == 4 && (*from)->sldr->otherdt->NMOVES == ZERO) {
                    (*from)->sldr->otherdt->enpassant = true;
                }

                if ((*from)->sldr->otherdt->NMOVES < MORE_THAN_ONE) ((*from)->sldr->otherdt->NMOVES)++;

                if (pos.row == 2 && pos.col != (*from)->sldr->pos.col) {
                    killEnemey(ctx.board->Squares[(int)(*from)->sldr->pos.row][(int)pos.col].sldr);
                    ctx.board->Squares[(int)(*from)->sldr->pos.row][(int)pos.col].occupied = false;
                }
            }

            (*from)->occupied = false;
            next->sldr = (*from)->sldr;
            next->occupied = true;
            (*from)->sldr = NULL;

            // while (next->sldr->pos.col != pos.col)
            //     next->sldr->pos.col++;
            // while (next->sldr->pos.row != pos.row) {
            //     next->sldr->pos.row += 2;
            //     DrawTexture(next->sldr->shapText, sldrPos.x, sldrPos.y, WHITE);
            // }
            // next->sldr->pos.col = cLerp(next->sldr->pos.col, pos.col, 0.1f);
            // next->sldr->pos.row = cLerp(next->sldr->pos.row, pos.row, 0.1f);
            next->sldr->pos = pos;
            ctx.movementChange = FROM;
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

Position getArrPos(Vector2 from) {
    return (Position){
        (from.y - INFOBAR_HEIGHT) / BOARD_WIDTH * 8,
        from.x / BOARD_WIDTH * 8,
    };
}

Soldier *selectSldr(Position SqPos) {
    return ctx.board->Squares[(int)SqPos.row][(int)SqPos.col].sldr;
}

alist_t calcNextMove(Square *sq) {
    Soldier *sldr = sq->sldr;
    alist_destroy(&ctx.availableSqs);
    if (!sldr) return (alist_t){0};
    alist_t (*calc[])(Square *sq) = {
        calcNextMovePawn, calcNextMoveKnight, calcNextMoveBishop, calcNextMoveRook, calcNextMoveQueen, calcNextMoveKing,
    };
    return calc[sldr->type - 1](sq);
}

alist_t calcNextMovePawn(Square *fsq) {
    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Square *));
    alist_reserve(&nextSqs, 5);
    Square *nsq = NULL, *enmsq = NULL;
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row, praws = 0;

    // Number of possible squares
    praws = (fsq->sldr->otherdt->NMOVES == ZERO) + 1;

    Square *squares[8][8] = {0};
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            squares[i][j] = &ctx.board->Squares[i][j];

    // check if there is victims in corners or beside him
    // check the next rows
    for (int i = 1; i <= praws && inBoundaries(row - 1) && !squares[row - 1][colmn]->occupied; i++) {
        nsq = squares[row - i][colmn];
        if (!nsq->occupied) alist_push_sq(&nextSqs, nsq);
    }
    // check the next colmns
    for (int c = -1; c <= 1; c++) {
        if (!inBoundaries(colmn + c)) continue;
        if (c != 0 && inBoundaries(row - 1)) {
            nsq = squares[row - 1][colmn + c];
            if (isEnemy(fsq, nsq)) alist_push_sq(&nextSqs, nsq);
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
                enmsq = squares[row][colmn + i];
                nsq = squares[row - 1][colmn + i];
                if (isEnemy(fsq, enmsq) && enmsq->sldr->type == PAWN && enmsq->sldr->otherdt->enpassant) {
                    alist_push_sq(&nextSqs, nsq);
                }
            }
        }
    }
    return nextSqs;
}

alist_t calcNextMoveKnight(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Square *));
    alist_reserve(&nextSqs, 8);
    int nrow, ncol;

    // set boundries

    Square *n = NULL;
    for (int c = -2; c <= 2; c++) {
        ncol = colmn + c;
        if (c == 0 || !inBoundaries(ncol)) continue;
        for (int r = -1; r <= 1; r++) {
            if (r == 0) continue;
            if (!(c % 2)) {
                nrow = row + r;
            } else {
                nrow = row + (2 * r);
            }
            if (!inBoundaries(nrow)) continue;
            n = &(ctx.board->Squares[nrow][ncol]);
            if (!n->occupied || isEnemy(fsq, n)) alist_push_sq(&nextSqs, n);
        }
    }

    return nextSqs;
}

alist_t calcNextMoveRook(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    // available squares before and after.
    //

    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position));
    Square *n = NULL;

    for (int c = colmn + 1; c <= 7; c++) {
        n = &(ctx.board->Squares[row][c]);
        if (!n->occupied) {
            alist_push_sq(&nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            alist_push_sq(&nextSqs, n);
            break;
        } else
            break;
    }

    for (int c = colmn - 1; c >= 0; c--) {
        n = &(ctx.board->Squares[row][c]);
        if (!n->occupied) {
            alist_push_sq(&nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            alist_push_sq(&nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row + 1; r <= 7; r++) {
        n = &(ctx.board->Squares[r][colmn]);
        if (!n->occupied) {
            alist_push_sq(&nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            alist_push_sq(&nextSqs, n);
            break;
        } else
            break;
    }

    for (int r = row - 1; r >= 0; r--) {
        n = &(ctx.board->Squares[r][colmn]);
        if (!n->occupied) {
            alist_push_sq(&nextSqs, n);
        } else if (isEnemy(fsq, n)) {
            alist_push_sq(&nextSqs, n);
            break;
        } else
            break;
    }

    return nextSqs;
}

alist_t calcNextMoveBishop(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;

    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Square *));
    alist_reserve(&nextSqs, 15);

    Square *next = NULL;

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row + i); i++) {
        next = &(ctx.board->Squares[row + i][colmn + i]);
        if (!next->occupied) {
            alist_push_sq(&nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            alist_push_sq(&nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row - i); i++) {
        next = &(ctx.board->Squares[row - i][colmn - i]);
        if (!next->occupied) {
            alist_push_sq(&nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            alist_push_sq(&nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row - i); i++) {
        next = &(ctx.board->Squares[row - i][colmn + i]);
        if (!next->occupied) {
            alist_push_sq(&nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            alist_push_sq(&nextSqs, next);
            break;
        } else
            break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row + i); i++) {
        next = &(ctx.board->Squares[row + i][colmn - i]);
        if (!next->occupied) {
            alist_push_sq(&nextSqs, next);
        } else if (isEnemy(fsq, next)) {
            alist_push_sq(&nextSqs, next);
            break;
        } else
            break;
    }

    return nextSqs;
}

alist_t calcNextMoveQueen(Square *f) {
    alist_t second = calcNextMoveBishop(f);
    alist_t first = calcNextMoveRook(f);
    alist_t nextSqs = mergeList(&first, &second);
    // number possible squares
    return nextSqs;
}

alist_t calcNextMoveKing(Square *fsq) {
    int colmn = fsq->sldr->pos.col, row = fsq->sldr->pos.row;
    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Square *));
    alist_reserve(&nextSqs, 5);

    Square *n = NULL;
    for (int c = -1; c <= 1; c++) {
        for (int r = -1; r <= 1; r++) {
            if (inBoundaries(row + r) && inBoundaries(colmn + c)) {
                n = &(ctx.board->Squares[row + r][colmn + c]);
                if (isEnemy(fsq, n) || !n->occupied) alist_push_sq(&nextSqs, n);
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
        return &(ctx.board->Squares[(int)pos.row][(int)pos.col]);
    } else
        return 0;
}

bool isAvailable(Square *sq) {
    if (alist_at(&ctx.availableSqs, 0) == NULL) return false;
    for (size_t i = 0; i < alist_size(&ctx.availableSqs); i++) {
        if (sq == alist_sq_at(&ctx.availableSqs, i)) return true;
    }
    return false;
}

// I think it's a bad practice but it's cool
inline void changeActive() {
    ctx.ACTIVE = !ctx.ACTIVE;
}

void cancelMovment() {
    ctx.movementChange = FROM;
    colorBoardSquares();
}
