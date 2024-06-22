#include "./chess.h"
#include "data.h"
#include "gdslib/include/garraylist.h"
#include "raylib/include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Context ctx;
int main() {
    initgdata();

    game();

    destroydata();

    return 0;
}

Clock *blackClock;
Clock *whiteClock;
double globalTime = 0;

bool isSecPassed(float seconds) {
    if (GetTime() - globalTime >= seconds) {
        globalTime = GetTime();
        return true;
    }
    return false;
}

void incClock() {
    Clock *temp = NULL;
    if (ctx.ACTIVE == WHITE_TEAM)
        temp = whiteClock;
    else
        temp = blackClock;
    temp->s++;

    if (60 == temp->s) {
        temp->m++;
        temp->s = 0;
    }
}

void game() {
    int valid = 0;
    while (!WindowShouldClose()) {
        valid = 0;
        ClearBackground(BLACK);
        BeginDrawing();
        drawBoard();
        if (isSecPassed(1)) {
            incClock();
        }

        if (IsKeyReleased(KEY_Q)) {
            EndDrawing();
            break;
        } else if (IsKeyReleased(KEY_C)) {
            cancelMovment();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Position pos = getArrPos(GetMousePosition());
            valid = moveSldr(pos);
            if (valid == 1) mirrorBoard();
        }
        EndDrawing();
    }
}

Board *createBoard(Set_t *white, Set_t *black) {
    Board *b = malloc(sizeof(Board));
    b->sets[0] = black;
    b->sets[1] = white;

    // Initialize the square & set positions
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            b->Squares[i][j].rectangle.x = j * SQUARE_WIDTH;
            b->Squares[i][j].rectangle.y = i * SQUARE_WIDTH + INFOBAR_HEIGHT;
            b->Squares[i][j].rectangle.width = SQUARE_WIDTH;
            b->Squares[i][j].rectangle.height = SQUARE_WIDTH;
            b->Squares[i][j].occupied = false;
            b->Squares[i][j].sldr = NULL;
            b->Squares[i][j].color = ctx.colors[!isEven(i + j)];
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
    memset(&s->clk, 0, sizeof(Clock));
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
            tpos = &ctx.board->sets[j]->soldiers[i].pos;
            tpos->row = 7 - tpos->row;
            tpos->col = 7 - tpos->col;
        }
    }

    // mirror rectangles
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &ctx.board->Squares[7 - i][j];
            sq2 = &ctx.board->Squares[i][j];
            temp = *sq1;
            sq1->rectangle = sq2->rectangle;
            sq2->rectangle = temp.rectangle;
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &ctx.board->Squares[i][7 - j];
            sq2 = &ctx.board->Squares[i][j];
            temp = *sq1;
            sq1->rectangle = sq2->rectangle;
            sq2->rectangle = temp.rectangle;
        }
    }

    // mirror rows
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &ctx.board->Squares[7 - i][j];
            sq2 = &ctx.board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }

    // mirror columns
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &ctx.board->Squares[i][7 - j];
            sq2 = &ctx.board->Squares[i][j];
            temp = *sq1;
            *sq1 = *sq2;
            *sq2 = temp;
        }
    }
}

void drawClock(int fontSize) {
    char *bclock = strdup(TextFormat("%02.0f:%02.0f", blackClock->m, blackClock->s));
    char *wclock = strdup(TextFormat("%02.0f:%02.0f", whiteClock->m, whiteClock->s));
    Vector2 bTextDim = MeasureTextEx(GetFontDefault(), bclock, fontSize, 0);
    Vector2 wTextDim = MeasureTextEx(GetFontDefault(), wclock, fontSize, 0);

    DrawText(bclock, (BOARD_WIDTH - bTextDim.x) / 2, INFOBAR_HEIGHT * 0.f + (INFOBAR_HEIGHT / 2.f - bTextDim.y) / 2,
             fontSize, WHITE);
    DrawText(wclock, (BOARD_WIDTH - wTextDim.x) / 2, INFOBAR_HEIGHT / 2.f + (INFOBAR_HEIGHT / 2.f - wTextDim.y) / 2,
             fontSize, WHITE);
    free(bclock);
    free(wclock);
}

void drawInfoBar() {
    int padding = 5;
    int fontSize = 30;
    drawClock(fontSize);
    char *bText = strdup(TextFormat("BLACK : %d", ctx.board->sets[0]->count));
    char *wText = strdup(TextFormat("WHITE : %d", ctx.board->sets[1]->count));

    char *active = NULL;
    if (ctx.ACTIVE == WHITE_TEAM)
        active = strdup(TextFormat("Active : %s", "WHITE"));
    else
        active = strdup(TextFormat("Active : %s", "BLACK"));

    int TextWidth = MAX(MeasureText(bText, fontSize), MeasureText(wText, fontSize));
    DrawText(bText, BOARD_WIDTH - TextWidth * 1.1f, INFOBAR_HEIGHT * 0 + padding, fontSize, WHITE);
    DrawText(wText, BOARD_WIDTH - TextWidth * 1.1f, INFOBAR_HEIGHT / 2 + padding, fontSize, WHITE);
    DrawText(active, padding * 2, padding * 2, fontSize, WHITE);

    free(bText);
    free(wText);
    free(active);
}

void drawBoard() {
    // draw ctx.board squares and soldiers
    drawInfoBar();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            drawSq(&ctx.board->Squares[i][j]);
        }
    }
}

void drawSq(Square *sq) {
    DrawRectangleRec(sq->rectangle, sq->color);
    if (sq->occupied) {
        Vector2 sldrPos = {sq->rectangle.x + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f),
                           sq->rectangle.y + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f)};
        DrawTexture(sq->sldr->shapText, sldrPos.x, sldrPos.y, WHITE);
    }
}

void colorBoardSquares() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            ctx.board->Squares[i][j].color = ctx.colors[(i + j) % 2];
        }
    }
}

bool inBoundaries(int pos) {
    if (0 <= pos && pos <= 7) return true;
    return false;
}

void erroredEnd() {
    destroydata();
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

Color *initColor() {
    Color *colors = malloc(3 * sizeof(char *));
    colors[0] = DARKBROWN;
    colors[1] = BEIGE;
    colors[2] = GREEN;
    return colors;
}

void initgdata() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(800, BOARD_HEIGHT + INFOBAR_HEIGHT, "Chess");
    SetTargetFPS(60);
    Set_t *white = createSet(WHITE_TEAM);
    Set_t *black = createSet(BLACK_TEAM);
    blackClock = &black->clk;
    whiteClock = &white->clk;
    // Set_t *white = onlyType(ROOK, WHITE_TEAM);
    // Set_t *black = onlyType(ROOK, BLACK_TEAM);
    ctx.colors = initColor();
    ctx.board = createBoard(white, black);
    ctx.ACTIVE = WHITE_TEAM;
    ctx.movementChange = FROM;
    ctx.fromSquare = NULL;
    colorBoardSquares();
}

void destroydata() {
    for (int i = 0; i < 16; i++) {
        UnloadTexture(ctx.board->sets[0]->soldiers[i].shapText);
        UnloadTexture(ctx.board->sets[1]->soldiers[i].shapText);
    }
    CloseWindow();
    for (int i = 0; i < 8; i++) {
        free(ctx.board->sets[0]->soldiers[i + 8].otherdt);
        free(ctx.board->sets[1]->soldiers[i].otherdt);
    }
    free(ctx.board->sets[0]->soldiers[4].otherdt);
    free(ctx.board->sets[1]->soldiers[12].otherdt);

    free(ctx.board->sets[0]->soldiers);
    free(ctx.board->sets[1]->soldiers);

    for (int i = 0; i < 2; i++)
        free(ctx.board->sets[i]);

    alist_destroy(&ctx.availableSqs);
    free(ctx.colors);
    free(ctx.board);
}

int16_t alist_push_sq(alist_t *list, Square *sq) {
    if (!list || !sq) return EXIT_FAILURE;
    sq->color = GREEN;
    Position pos = getArrPos((Vector2){sq->rectangle.x, sq->rectangle.y});
    int16_t result = alist_push(list, &pos);
    Position pos2 = *(Position *)alist_at(list, list->size - 1);
    chooseSquare(pos2)->color = GREEN;
    return result;
}

Square *alist_sq_at(alist_t *list, size_t at) {
    Position pos = *(Position *)alist_at(list, at);
    return chooseSquare(pos);
}

void debug() {
    printf("%ld\n", sizeof(ctx));
}
