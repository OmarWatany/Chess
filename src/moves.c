#include "chess.h"
#include "garraylist.h"

#define occupied(sq) (sq->sldr != NULL)

alist_t (*calc[])(Square *sq) = {
    [PAWN] = calcNextMovePawn, [KNIGHT] = calcNextMoveKnight, [BISHOP] = calcNextMoveBishop,
    [ROOK] = calcNextMoveRook, [QUEEN] = calcNextMoveQueen,   [KING] = calcNextMoveKing,
};

// ---------------------------------------------------------------------------
// Illegal-move detection helpers (simulate-and-validate)
// ---------------------------------------------------------------------------

// Check if the path between two positions is clear of pieces (exclusive of endpoints)
static bool isPathClear(Position from, Position to) {
    int dr = (to.row > from.row) ? 1 : (to.row < from.row) ? -1 : 0;
    int dc = (to.col > from.col) ? 1 : (to.col < from.col) ? -1 : 0;
    int r = from.row + dr, c = from.col + dc;
    while (r != to.row || c != to.col) {
        if (ctx.board.Squares[r][c].sldr != NULL) return false;
        r += dr;
        c += dc;
    }
    return true;
}

// Check if a specific piece can ATTACK a target square.
// This is about attack capability (threatened squares), not legal move destinations.
// The board is always oriented so the active player is at the bottom (rows 6-7)
// and moves upward (toward row 0). So:
//   - Active team's pawns attack toward row-1 (diagonals)
//   - Enemy team's pawns (at top, rows 0-1) attack toward row+1 (diagonals)
static bool canPieceAttack(Soldier *s, Position target) {
    int dr = target.row - s->arrPos.row;
    int dc = target.col - s->arrPos.col;
    int adr = abs(dr), adc = abs(dc);

    switch (s->type) {
    case PAWN: {
        // The active team is always at the bottom, so sets[0] (BLACK_TEAM when
        // the game starts) is at the top after the first mirror, and sets[1]
        // (WHITE_TEAM) is at the bottom.  But after mirroring, team colors swap
        // sides each turn.  The key insight: the team whose turn it is (ctx.ACTIVE)
        // sits at rows 6-7 and attacks upward (row-1).  The opponent sits at rows
        // 0-1 and attacks downward (row+1).
        //
        // Simplification: since the board is always mirrored so that the active
        // player is at the bottom, the active team's pawns attack at dr == -1 and
        // the enemy (non-active) team's pawns attack at dr == +1.
        int pawnDir = (s->team_set->teamColor == ctx.ACTIVE) ? -1 : 1;
        return (dr == pawnDir && adc == 1);
    }
    case KNIGHT:
        return (adr == 2 && adc == 1) || (adr == 1 && adc == 2);
    case BISHOP:
        return (adr == adc && adr > 0) && isPathClear(s->arrPos, target);
    case ROOK:
        if (dr == 0 && dc == 0) return false;
        return (dr == 0 || dc == 0) && isPathClear(s->arrPos, target);
    case QUEEN:
        if (dr == 0 && dc == 0) return false;
        if (dr == 0 || dc == 0) return isPathClear(s->arrPos, target);
        if (adr == adc) return isPathClear(s->arrPos, target);
        return false;
    case KING:
        return (adr <= 1 && adc <= 1 && (adr + adc) > 0);
    }
    return false;
}

// Check if a square is attacked by any LIVE piece of the given team
bool isSquareAttacked(Position target, TEAM byTeam) {
    Set_t *attackers = &ctx.board.sets[byTeam == WHITE_TEAM ? 1 : 0];
    for (int i = 0; i < 16; i++) {
        Soldier *s = &attackers->soldiers[i];
        if (s->State != LIVE) continue;
        if (canPieceAttack(s, target)) return true;
    }
    return false;
}

// Find the king's board position for the given team
static Position findKingPos(TEAM team) {
    Set_t *set = &ctx.board.sets[team == WHITE_TEAM ? 1 : 0];
    for (int i = 0; i < 16; i++) {
        if (set->soldiers[i].type == KING && set->soldiers[i].State == LIVE) return set->soldiers[i].arrPos;
    }
    // Should never happen in a valid game
    return (Position){-1, -1};
}

// Check if the king of the given team is currently in check
bool isKingInCheck(TEAM team) {
    Position kingPos = findKingPos(team);
    TEAM enemy = (team == WHITE_TEAM) ? BLACK_TEAM : WHITE_TEAM;
    return isSquareAttacked(kingPos, enemy);
}

// Check if moving a piece from->to would leave the moving side's king in check.
// Temporarily applies the move, checks, then undoes it.
static bool wouldLeaveKingInCheck(Position from, Position to, TEAM team) {
    Square *fromSq = &ctx.board.Squares[from.row][from.col];
    Square *toSq = &ctx.board.Squares[to.row][to.col];

    Soldier *movedPiece = fromSq->sldr;
    Soldier *capturedPiece = toSq->sldr;
    Position oldPos = movedPiece->arrPos;

    // Handle en passant capture (pawn moves diagonally to an empty square)
    Soldier *enPassantCaptured = NULL;
    Square *epSq = NULL;
    if (movedPiece->type == PAWN && to.col != from.col && capturedPiece == NULL) {
        epSq = &ctx.board.Squares[from.row][to.col];
        enPassantCaptured = epSq->sldr;
        if (enPassantCaptured) epSq->sldr = NULL;
    }

    // Apply move temporarily
    toSq->sldr = movedPiece;
    fromSq->sldr = NULL;
    movedPiece->arrPos = to;

    // Mark captured piece as dead so isSquareAttacked ignores it
    SOLDIER_STATE capturedOldState = DEAD;
    if (capturedPiece) {
        capturedOldState = capturedPiece->State;
        capturedPiece->State = DEAD;
    }

    bool inCheck = isKingInCheck(team);

    // Undo everything
    if (capturedPiece) capturedPiece->State = capturedOldState;
    fromSq->sldr = movedPiece;
    movedPiece->arrPos = oldPos;
    toSq->sldr = capturedPiece;
    if (enPassantCaptured) epSq->sldr = enPassantCaptured;

    return inCheck;
}

// ---------------------------------------------------------------------------
// Core move / game-flow functions
// ---------------------------------------------------------------------------

int moveSldr(Position from, Position to) {
    Square *fromSq = chooseSquare(from);
    Square *nextSq = chooseSquare(to);

    if (nextSq->sldr) killEnemey(nextSq->sldr);
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

        Soldier *enPassantSldr = ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].sldr;
        if (to.row == 2 && to.col != fromSq->sldr->arrPos.col && enPassantSldr) {
            killEnemey(ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].sldr);
            ctx.board.Squares[fromSq->sldr->arrPos.row][to.col].sldr = NULL;
        }
    }

    nextSq->sldr = fromSq->sldr;
    nextSq->sldr->arrPos = to;
    fromSq->sldr = NULL;

    changeActive();
    return 0;
}

MOVEMENT_STATUS moveFrom(Position pos) {
    ctx.fromPos = pos;
    Square *fromSq = chooseSquare(pos);
    // If out of bounds, chose empty square, or non active team's soldier
    if (!fromSq || !occupied(fromSq) || fromSq->sldr->team_set->teamColor != ctx.ACTIVE) return MOVE_INVALID;

    ctx.availableSqs = calcNextMove(fromSq);
    if (alist_empty(&ctx.availableSqs)) {
        colorBoardSquares();
        return MOVE_INVALID;
    }

    ctx.movementChange = TO;
    return MOVE_VALID;
}

MOVEMENT_STATUS moveTo(Position to) {
    Square *next = chooseSquare(to);
    if (!next) return MOVE_INVALID;
    if (occupied(next) && next->sldr->team_set->teamColor == ctx.ACTIVE) {
        resetMovement();
        moveFrom(to);
        return MOVE_MODE_CHANGE;
    }
    if (!isAvailable(next)) return MOVE_INVALID;

    moveSldr(ctx.fromPos, to);
    resetMovement();
    return MOVE_VALID;
}

void killEnemey(Soldier *sldr) {
    sldr->State = DEAD;
    sldr->team_set->count--;
}

Soldier *selectSldr(Position SqPos) {
    return ctx.board.Squares[(int)SqPos.row][(int)SqPos.col].sldr;
}

// Calculate legal moves for the piece on the given square.
// First generates candidate moves via the piece-specific function,
// then filters out any move that would leave the own king in check.
alist_t calcNextMove(Square *sq) {
    Soldier *sldr = sq->sldr;
    alist_destroy(&ctx.availableSqs);
    if (!sldr) return (alist_t){0};

    alist_t candidates = calc[sldr->type](sq);
    TEAM team = sldr->team_set->teamColor;
    Position from = sldr->arrPos;

    // Reset board colors — candidate generation colored squares green,
    // but some may be filtered out.
    colorBoardSquares();

    // Build a new list containing only legal moves
    alist_t legal = {0};
    alist_init(&legal, sizeof(Position *));

    for (size_t i = 0; i < alist_size(&candidates); i++) {
        Position pos = *(Position *)alist_at(&candidates, i);
        if (!wouldLeaveKingInCheck(from, pos, team)) {
            alist_push_pos(&legal, pos); // re-colors legal squares green
        }
    }

    alist_destroy(&candidates);
    return legal;
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
    for (int i = 1; i <= praws && inBoundaries(row - i); i++) {
        nsq = squares[row - i][colmn];
        if (occupied(nsq)) break;
        alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn});
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
            if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = nrow, .col = ncol});
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
        if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = row, .col = c});
        if (occupied(n)) break;
    }

    for (int c = colmn - 1; c >= 0; c--) {
        n = &(ctx.board.Squares[row][c]);
        if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = row, .col = c});
        if (occupied(n)) break;
    }

    for (int r = row + 1; r <= 7; r++) {
        n = &(ctx.board.Squares[r][colmn]);
        if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = r, .col = colmn});
        if (occupied(n)) break;
    }

    for (int r = row - 1; r >= 0; r--) {
        n = &(ctx.board.Squares[r][colmn]);
        if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = r, .col = colmn});
        if (occupied(n)) break;
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
        if (!occupied(next) || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row + i, .col = colmn + i});
        if (occupied(next)) break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row - i); i++) {
        next = &(ctx.board.Squares[row - i][colmn - i]);
        if (!occupied(next) || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn - i});
        if (occupied(next)) break;
    }

    for (int i = 1; inBoundaries(colmn + i) && inBoundaries(row - i); i++) {
        next = &(ctx.board.Squares[row - i][colmn + i]);
        if (!occupied(next) || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row - i, .col = colmn + i});
        if (occupied(next)) break;
    }

    for (int i = 1; inBoundaries(colmn - i) && inBoundaries(row + i); i++) {
        next = &(ctx.board.Squares[row + i][colmn - i]);
        if (!occupied(next) || isEnemy(fsq, next))
            alist_push_pos(&nextSqs, (Position){.row = row + i, .col = colmn - i});
        if (occupied(next)) break;
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
    alist_reserve(&nextSqs, 8);

    Square *n = NULL;
    for (int c = -1; c <= 1; c++) {
        for (int r = -1; r <= 1; r++) {
            if (r == 0 && c == 0) continue;
            if (!inBoundaries(row + r) || !inBoundaries(colmn + c)) continue;
            n = &(ctx.board.Squares[row + r][colmn + c]);
            if (!occupied(n) || isEnemy(fsq, n)) alist_push_pos(&nextSqs, (Position){.row = row + r, .col = colmn + c});
        }
    }
    return nextSqs;
}

bool isEnemy(Square *from, Square *to) {
    if (occupied(to) && to->sldr->team_set->teamColor != from->sldr->team_set->teamColor) return true;
    return false;
}

Square *chooseSquare(Position pos) {
    if (inBoundaries(pos.row) && inBoundaries(pos.col)) {
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

// ---------------------------------------------------------------------------
// Checkmate / stalemate detection
// ---------------------------------------------------------------------------

// Check if any piece of the given team has at least one legal move.
static bool hasAnyLegalMove(TEAM team) {
    Set_t *set = &ctx.board.sets[team == WHITE_TEAM ? 1 : 0];
    bool found = false;

    for (int i = 0; i < 16 && !found; i++) {
        Soldier *s = &set->soldiers[i];
        if (s->State != LIVE) continue;

        Square *sq = &ctx.board.Squares[s->arrPos.row][s->arrPos.col];
        alist_t moves = calc[s->type](sq);

        for (size_t j = 0; j < alist_size(&moves); j++) {
            Position pos = *(Position *)alist_at(&moves, j);
            if (!wouldLeaveKingInCheck(s->arrPos, pos, team)) {
                found = true;
                break;
            }
        }
        alist_destroy(&moves);
    }

    colorBoardSquares(); // Reset colors modified during candidate generation
    return found;
}

// Determine the game state for the team that is about to move.
// Must be called when the board is oriented for that team (their pieces at bottom).
GAME_RESULT checkGameState(TEAM team) {
    if (hasAnyLegalMove(team)) return GAME_ONGOING;
    if (isKingInCheck(team)) return GAME_CHECKMATE;
    return GAME_STALEMATE;
}
