#include "data.h"
#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void load(Context *context) {
    char *file = "./.data";
    FILE *fd = NULL;
    fd = fopen(file, "rb");
    TEAM_COLOR *act = malloc(sizeof(TEAM_COLOR));
    if (fd) {
        Set_t *black = context->board->sets[0];
        Set_t *white = context->board->sets[1];

        fread(act, sizeof(TEAM_COLOR), 1, fd);
        if (*act != context->ACTIVE) {
            mirrorBoard();
        }
        context->ACTIVE = *act;

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                // printf("%d %d ", i, j);
                readSq(fd, &context->board->Squares[i][j]);
            }
        }
        // sleep(240);
        readSet(fd, white, context);
        readSet(fd, black, context);

    } else {
        perror("coudn't open file or not found");
    }
    fclose(fd);
    free(act);
}
// for each square in the board saev
// sldr pos , sq occupation  , set count,
//

void readSet(FILE *file, Set_t *s, Context *context) {
    fread(&s->count, sizeof(int), 1, file);
    int col = 0, row = 0;
    for (int i = 0; i < 16; i++) {
        readSldr(file, &s->soldiers[i]);
        col = s->soldiers[i].pos.col;
        row = s->soldiers[i].pos.row;
        context->board->Squares[row][col].sldr = &s->soldiers[i];
    }
}

void readSldr(FILE *file, Soldier *sldr) {
    SOLDIER_STATE *state = &sldr->State;
    Position *pos = &sldr->pos;
    fread(state, sizeof(SOLDIER_STATE), 1, file);
    fread(pos, sizeof(Position), 1, file);
    if (sldr->type == PAWN || sldr->type == KING) {
        fread(sldr->otherdt, sizeof(OtherData), 1, file);
    }
}

void readSq(FILE *file, Square *sq) {
    fread(&sq->occupied, sizeof(bool), 1, file);
    if (!sq->occupied) {
        // printf("occupation: %d\n", sq->occupied);
        sq->sldr = NULL;
    }
    /*
    if (sq->sldr != NULL) {
      printf("occupation: %d", sq->occupied);
      printf(" %s\n", sq->sldr->shap);
    }
    */
}

void save(Context *context) {
    char *file = "./.data";
    FILE *fd = NULL;
    fd = fopen(file, "wb");
    if (fd) {
        Set_t *white = context->board->sets[1];
        Set_t *black = context->board->sets[0];

        fflush(fd);
        fwrite(&context->ACTIVE, sizeof(TEAM_COLOR), 1, fd);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                writeSq(fd, &context->board->Squares[i][j]);
            }
        }
        writeSet(fd, white);
        writeSet(fd, black);
    } else {
        perror("coudn't open file or not found");
    }
    fclose(fd);
}

void writeSet(FILE *file, Set_t *s) {
    fwrite(&s->count, sizeof(int), 1, file);
    for (int i = 0; i < 16; i++) {
        writeSldr(file, &s->soldiers[i]);
    }
}

void writeSldr(FILE *file, Soldier *sldr) {
    SOLDIER_STATE *state = &sldr->State;
    Position *pos = &sldr->pos;
    fwrite(state, sizeof(SOLDIER_STATE), 1, file);
    fwrite(pos, sizeof(Position), 1, file);
    if (sldr->type == PAWN || sldr->type == KING) {
        fwrite(sldr->otherdt, sizeof(OtherData), 1, file);
    }
}

void writeSq(FILE *file, Square *sq) {
    bool *occ = &sq->occupied;
    fwrite(occ, sizeof(bool), 1, file);
}
