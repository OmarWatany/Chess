#define ARENA_IMPLEMENTATION
#include "arena.h"
#include "chess.h"
#include "data.h"
#include "garraylist.h"
#include <stdio.h>

Arena global_arena;
Context ctx;
Timer *blackTimer, *whiteTimer;
double globalTime = 0;
uint8_t SquareColors[8][8] = {0};

void initGameData() {
    arena_init(&global_arena, 1024 * 2);
    initBoard(&ctx.board);
    blackTimer = &ctx.board.sets[0].timer;
    whiteTimer = &ctx.board.sets[1].timer;
    ctx.ACTIVE = WHITE_TEAM;
    ctx.movementChange = FROM;
    ctx.fromSquare = NULL;
    colorBoardSquares();
}

void initBoard(Board *b) {
    Set_t *black = &b->sets[0], *white = &b->sets[1];
    initSet(&b->sets[0], BLACK_TEAM);
    initSet(&b->sets[1], WHITE_TEAM);
    // onlyType(white, ROOK, WHITE_TEAM);
    // onlyType(black, ROOK, BLACK_TEAM);

    // Initialize the square & set positions
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->Squares[y][x].occupied = false;
            b->Squares[y][x].sldr = NULL;
        }
    }

    // Black Set
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 8; x++) {
            if (black->soldiers[(y * 8) + x].State == DEAD) continue;
            b->Squares[y][x].occupied = true;
            b->Squares[y][x].sldr = &(black->soldiers[(y * 8) + x]);
            b->Squares[y][x].sldr->arrPos = (Position){.row = y, .col = x};
        }
    }

    // White Set
    for (int y = 6; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (white->soldiers[((y - 6) * 8) + x].State == DEAD) continue;
            b->Squares[y][x].occupied = true;
            b->Squares[y][x].sldr = &(white->soldiers[((y - 6) * 8) + x]);
            b->Squares[y][x].sldr->arrPos = (Position){.row = y, .col = x};
        }
    }
}

void initSet(Set_t *s, TEAM teamColor) {
    if (!s) {
        perror("no set");
        exit(1);
    }
    s->count = 16;
    s->teamColor = teamColor;
    memset(&s->timer, 0, sizeof(Timer));

    SOLDIER_TYPE types[4] = {ROOK, KNIGHT, BISHOP, PAWN};
    switch (teamColor) {
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
    SOLDIER_TYPE type;
    for (int i = 0; i < 16; i++) {
        type = s->soldiers[i].type;
        s->soldiers[i].team_set = s;
        s->soldiers[i].State = LIVE;
        if (type == PAWN || type == KING) {
            s->soldiers[i].otherdt = arena_alloc(&global_arena, sizeof(OtherData));
            s->soldiers[i].otherdt->NMOVES = ZERO;
            s->soldiers[i].otherdt->enpassant = false;
        }
    }
}

void onlyType(Set_t *s, SOLDIER_TYPE t, TEAM color) {
    initSet(s, color);
    for (int i = 0; i < 16; i++) {
        if (s->soldiers[i].type != t) {
            s->soldiers[i].State = DEAD;
            s->count--;
        }
    }
}

void incTimer() {
    Timer *temp = NULL;
    if (ctx.ACTIVE == WHITE_TEAM)
        temp = whiteTimer;
    else
        temp = blackTimer;
    temp->s++;

    if (60 == temp->s) {
        temp->m++;
        temp->s = 0;
    }
}

void mirrorBoard() {
    Square temp, *sq1, *sq2;

    // mirror rows
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 8; x++) {
            sq1 = &ctx.board.Squares[7 - y][x];
            sq2 = &ctx.board.Squares[y][x];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }

    // mirror columns
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &ctx.board.Squares[i][7 - j];
            sq2 = &ctx.board.Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }

    // mirror each soldier position
    Soldier *sldr;
    for (int s = 0; s < 2; s++) {
        for (int j = 0; j < 16; j++) {
            sldr = &ctx.board.sets[s].soldiers[j];
            sldr->arrPos = (Position){
                .row = 7 - sldr->arrPos.row,
                .col = 7 - sldr->arrPos.col,
            };
        }
    }
}

void colorBoardSquares() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            SquareColors[y][x] = isOdd(y + x); // Set to color's position in Colors list
        }
    }
}

bool inBoundaries(int pos) {
    if (0 <= pos && pos <= 7) return true;
    return false;
}

void errorExit() {
    destroyData();
    printf("there is an error\n");
    exit(1);
}

alist_t mergeList(alist_t *first, alist_t *second) {
    for (size_t i = 0; i < alist_size(second); i++) {
        alist_push(first, alist_at(second, i));
    }
    alist_destroy(second);
    return *first;
}

// i need an arena
int16_t alist_push_pos(alist_t *list, Position arrPos) {
    Square *sq = chooseSquare(arrPos);
    if (!list || !sq) return EXIT_FAILURE;
    SquareColors[arrPos.row][arrPos.col] = 2; // GREEN's position in Colors list
    Position pos = arrPos;
    int16_t result = alist_push(list, &pos);
    return result;
}

Square *alist_sq_at(alist_t *list, size_t at) {
    Position pos = *(Position *)alist_at(list, at);
    return chooseSquare(pos);
}

void debug() {
    printf("%ld\n", sizeof(ctx));
}
