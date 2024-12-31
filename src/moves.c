#include "chess.h"
#include "garraylist.h"
#include <unistd.h>

alist_t (*calc[])(Square *sq) = {
    [PAWN] = calcNextMovePawn, [KNIGHT] = calcNextMoveKnight, [BISHOP] = calcNextMoveBishop,
    [ROOK] = calcNextMoveRook, [QUEEN] = calcNextMoveQueen,   [KING] = calcNextMoveKing,
};

int moveSldr(Position from, Position to) {
    Square *fromSq = chooseSquare(from);
    Square *nextSq = chooseSquare(to);

    if (nextSq->sldr) {
        killEnemey(nextSq->sldr);
    }
    if (fromSq->sldr->type == PAWN) {
        // reset soldiers' state
        for (int i = 0; i < 8; i++) {
            ctx.board.sets[1].soldiers[i].otherdt->enpassant = false;
            ctx.board.sets[0].soldiers[i + 8].otherdt->enpassant = false;
        }

        // works with mirrored boards
        if (to.row == 4 && fromSq->sldr->otherdt->NMOVES == ZERO) {
            fromSq->sldr->otherdt->enpassant = true;
        }

        if (fromSq->sldr->otherdt->NMOVES < MORE_THAN_ONE) (fromSq->sldr->otherdt->NMOVES)++;

        if (to.row == 2 && to.col != fromSq->sldr->arrPos.col) {
            killEnemey(ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].sldr);
            ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].occupied = false;
            ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].sldr = NULL;
        }
    }

    nextSq->occupied = true;
    nextSq->sldr = fromSq->sldr;
    fromSq->sldr = NULL;
    fromSq->occupied = false;

    changeActive();
    return 0;
}

int moveFrom(Position pos) {
    ctx.fromPos = pos;
    Square *fromSq = chooseSquare(pos);
    // If chose empty square or non active team's soldier
    if (!fromSq->occupied || fromSq->sldr->team_set->teamColor != ctx.ACTIVE) {
        colorBoardSquares();
        return 0;
    }
    ctx.availableSqs = calcNextMove(fromSq);
    if (alist_empty(&ctx.availableSqs)) {
        colorBoardSquares();
        return 0;
    }
    ctx.movementChange = TO;
    return 2;
}

int moveTo(Position to) {
    Square *next = chooseSquare(to);
    if (next->occupied && next->sldr->team_set->teamColor == ctx.ACTIVE) {
        resetMovement();
        moveFrom(to);
        return 2;
    }
    if (!isAvailable(next)) return 0;

    moveSldr(ctx.fromPos, to);

    resetMovement();
    return 1;
}

void killEnemey(Soldier *sldr) {
    sldr->State = DEAD;
    sldr->team_set->count--;
}

Soldier *selectSldr(Position SqPos) {
    return ctx.board.Squares[(int)SqPos.row][(int)SqPos.col].sldr;
}

alist_t calcNextMove(Square *sq) {
    Soldier *sldr = sq->sldr;
    alist_destroy(&ctx.availableSqs);
    if (!sldr) return (alist_t){0};
    return calc[sldr->type](sq);
}

alist_t calcNextMovePawn(Square *fsq) {
    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position *));
    alist_reserve(&nextSqs, 5);
    Square *nsq = NULL, *enmsq = NULL;
    int colmn = fsq->sldr->arrPos.col, row = fsq->sldr->arrPos.row, praws = 0;

    // Number of possible squares
    praws = (fsq->sldr->otherdt->NMOVES == ZERO) + 1;

    Square *squares[8][8] = {0};
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            squares[y][x] = &ctx.board.Squares[y][x];

    // check if there is victims in corners or beside him
    // check the next rows
    for (int i = 1; i <= praws && inBoundaries(row - 1) && !squares[row - 1][colmn]->occupied; i++) {
        nsq = squares[row - i][colmn];
        if (!nsq->occupied) alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn});
    }
    // check the next colmns
    for (int c = -1; c <= 1; c++) {
        if (!inBoundaries(colmn + c)) continue;
        if (c != 0 && inBoundaries(row - 1)) {
            nsq = squares[row - 1][colmn + c];
            if (isEnemy(fsq, nsq)) alist_push_pos(&nextSqs, (Position){.row = row - 1, .col = colmn + c});
        }
    }
    // to calc en passant
    // if my row is 3 and one of my adjacents were empty
    // then it became occupied
    // i can perform en passant & can't do it in next move
    //
    // check adjacent enemies
    if (fsq->sldr->arrPos.row == 3) {
        for (int i = -1; i <= 1; i++) {
            if (i != 0 && inBoundaries(colmn + i)) {
                enmsq = squares[row][colmn + i];
                nsq = squares[row - 1][colmn + i];
                if (isEnemy(fsq, enmsq) && enmsq->sldr->type == PAWN && enmsq->sldr->otherdt->enpassant) {
                    alist_push_pos(&nextSqs, (Position){.row = row - 1, .col = colmn + i});
                }
            }
        }
    }
    return nextSqs;
}

alist_t calcNextMoveKnight(Square *fsq) {
    int colmn = fsq->sldr->arrPos.col, row = fsq->sldr->arrPos.row;

    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position *));
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
            n = &(ctx.board.Squares[nrow][ncol]);
            if (!n->occupied || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = nrow, .col = ncol});
        }
    }

    return nextSqs;
}

alist_t calcNextMoveRook(Square *fsq) {
    int colmn = fsq->sldr->arrPos.col, row = fsq->sldr->arrPos.row;
    // available squares before and after.
    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position *));
    Square *n = NULL;

    for (int c = colmn + 1; c <= 7; c++) {
        n = &(ctx.board.Squares[row][c]);
        if (!n->occupied || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = row, .col = c});
        if (n->occupied) break;
    }

    for (int c = colmn - 1; c >= 0; c--) {
        n = &(ctx.board.Squares[row][c]);
        if (!n->occupied || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = row, .col = c});
        if (n->occupied) break;
    }

    for (int r = row + 1; r <= 7; r++) {
        n = &(ctx.board.Squares[r][colmn]);
        if (!n->occupied || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = r, .col = colmn});
        if (n->occupied) break;
    }

    for (int r = row - 1; r >= 0; r--) {
        n = &(ctx.board.Squares[r][colmn]);
        if (!n->occupied || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = r, .col = colmn});
        if (n->occupied) break;
    }

    return nextSqs;
}

alist_t calcNextMoveBishop(Square *fsq) {
    int colmn = fsq->sldr->arrPos.col, row = fsq->sldr->arrPos.row;

    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position *));
    alist_reserve(&nextSqs, 15);

    Square *next = NULL;

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row + i); i++) {
        next = &(ctx.board.Squares[row + i][colmn + i]);
        if (!next->occupied || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row + i, .col = colmn + i});
        if (next->occupied) break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row - i); i++) {
        next = &(ctx.board.Squares[row - i][colmn - i]);
        if (!next->occupied || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn - i});
        if (next->occupied) break;
    }

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row - i); i++) {
        next = &(ctx.board.Squares[row - i][colmn + i]);
        if (!next->occupied || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn + i});
        if (next->occupied) break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row + i); i++) {
        next = &(ctx.board.Squares[row + i][colmn - i]);
        if (!next->occupied || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row + i, .col = colmn - i});
        if (next->occupied) break;
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
    int colmn = fsq->sldr->arrPos.col, row = fsq->sldr->arrPos.row;
    alist_t nextSqs = {0};
    alist_init(&nextSqs, sizeof(Position *));
    alist_reserve(&nextSqs, 5);

    Square *n = NULL;
    for (int c = -1; c <= 1; c++) {
        for (int r = -1; r <= 1; r++) {
            if (inBoundaries(row + r) && inBoundaries(colmn + c)) {
                n = &(ctx.board.Squares[row + r][colmn + c]);
                if (!n->occupied || isEnemy(fsq, n))
                    alist_push_pos(&nextSqs, (Position){.row = row + r, .col = colmn + c});
            }
        }
    }
    return nextSqs;
}

bool isEnemy(Square *from, Square *to) {
    if (to->occupied && to->sldr->team_set->teamColor != from->sldr->team_set->teamColor) return true;
    return false;
}

Square *chooseSquare(Position pos) {
    if (pos.row != 10) {
        return &(ctx.board.Squares[(int)pos.row][(int)pos.col]);
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

void resetMovement() {
    ctx.movementChange = FROM;
    colorBoardSquares();
}
