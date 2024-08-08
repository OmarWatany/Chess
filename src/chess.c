#define ARENA_IMPLEMENTATION
#include "arena.h"

#include "chess.h"
#include "data.h"
#include "garraylist.h"
#include "raylib.h"

Arena global_arena;
Context ctx;
Timer *blackTimer, *whiteTimer;
double globalTime = 0;

// GUI client data
Position sldrSize;
Texture2D whiteShapeText[KING + 1] = {0};
Texture2D blackShapeText[KING + 1] = {0};
Color colors[3];
uint8_t SquareColors[8][8] = {0};

int main() {
    initGameData();
    game();
    destroyData();
    return 0;
}

static inline bool isSecPassed(double previous, float seconds) {
    return (GetTime() - previous >= seconds);
}

Vector2 getSldrPos(Position arrPos) {
    return (Vector2){arrPos.col * SQUARE_WIDTH + ((SQUARE_WIDTH - sldrSize.col) / 2.f),
                     arrPos.row * SQUARE_WIDTH + INFOBAR_HEIGHT + ((SQUARE_WIDTH - sldrSize.row) / 2.f)};
}

Position getArrPos(Vector2 from) {
    return (Position){
        (from.y - INFOBAR_HEIGHT) / BOARD_WIDTH * 8,
        from.x / BOARD_WIDTH * 8,
    };
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

void game() {
    int valid = 0;
    while (!WindowShouldClose()) {
        valid = 0;
        ClearBackground(BLACK);
        BeginDrawing();
        drawBoard();
        if (isSecPassed(globalTime, 1)) {
            globalTime = GetTime();
            incTimer();
        }

        if (IsKeyReleased(KEY_Q)) {
            EndDrawing();
            break;
        } else if (IsKeyReleased(KEY_C)) {
            resetMovement();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Position tArrPos = getArrPos(GetMousePosition());
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
        EndDrawing();
    }
}

void initBoard(Board *b) {
    Set_t *black = &b->sets[0], *white = &b->sets[1];
    initSet(&b->sets[0], BLACK_TEAM);
    initSet(&b->sets[1], WHITE_TEAM);
    // onlyType(white, ROOK, WHITE_TEAM);
    // onlyType(black, ROOK, BLACK_TEAM);

    sldrSize = (Position){
        .col = whiteShapeText[PAWN].width,
        .row = whiteShapeText[PAWN].height,
    };

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
    char *sldr_types[] = {
        [PAWN] = "pawn", [KNIGHT] = "knight", [BISHOP] = "bishop", [ROOK] = "rook", [QUEEN] = "queen", [KING] = "king",
    };
    char *team_color[] = {
        [WHITE_TEAM] = "white",
        [BLACK_TEAM] = "black",
    };
    float scale_factor = 0.85f;
    Image soldierImage;
    for (int i = PAWN; i < KING + 1; i++) {
        soldierImage = LoadImage(TextFormat("./assets/128px/"
                                            "%s_%s_png_shadow.png",
                                            team_color[s->teamColor], sldr_types[i]));
        ImageResize(&soldierImage, SQUARE_WIDTH * scale_factor, SQUARE_WIDTH * scale_factor);
        if (s->teamColor == WHITE_TEAM) {
            whiteShapeText[i] = LoadTextureFromImage(soldierImage);
        } else
            blackShapeText[i] = LoadTextureFromImage(soldierImage);
        UnloadImage(soldierImage);
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

float drawTimer(int fontSize) {
    int margin = 20;
    const char *bclock = TextFormat(" _ %02.0f:%02.0f ", blackTimer->m, blackTimer->s);
    const char *wclock = TextFormat(" _ %02.0f:%02.0f ", whiteTimer->m, whiteTimer->s);
    Vector2 bTextDim = MeasureTextEx(GetFontDefault(), bclock, fontSize, 0);
    Vector2 wTextDim = MeasureTextEx(GetFontDefault(), wclock, fontSize, 0);
    float TextWidth = MAX(bTextDim.x, wTextDim.x) + margin;

    DrawText(bclock, BOARD_WIDTH - TextWidth, INFOBAR_HEIGHT * 0.f + (INFOBAR_HEIGHT / 2.f - bTextDim.y) / 2, fontSize,
             WHITE);
    DrawText(wclock, BOARD_WIDTH - TextWidth, INFOBAR_HEIGHT / 2.f + (INFOBAR_HEIGHT / 2.f - wTextDim.y) / 2, fontSize,
             WHITE);
    return TextWidth;
}

void drawInfoBar() {
    int margin = 5;
    int fontSize = 30;
    float clkWidth = drawTimer(fontSize);

    const char *black = "BLACK";
    const char *white = "WHITE";

    const char *active_c = ctx.ACTIVE == WHITE_TEAM ? white : black;
    const char *active = NULL;
    active = TextFormat("Active : %s", active_c);
    DrawText(active, margin * 2, margin * 2, fontSize, WHITE);

    const char *bText = TextFormat("%s : %d", black, ctx.board.sets[0].count);
    const char *wText = TextFormat("%s : %d", white, ctx.board.sets[1].count);

    Vector2 bTextSize = MeasureTextEx(GetFontDefault(), bText, fontSize, 0);
    Vector2 wTextSize = MeasureTextEx(GetFontDefault(), wText, fontSize, 0);
    int TextWidth = MAX(bTextSize.x, wTextSize.x) + margin * 3;

    DrawText(bText, BOARD_WIDTH - clkWidth - TextWidth * 1.1f, INFOBAR_HEIGHT * 0 + margin, fontSize, WHITE);
    DrawText(wText, BOARD_WIDTH - clkWidth - TextWidth * 1.1f, INFOBAR_HEIGHT / 2 + margin, fontSize, WHITE);
}

void drawBoard() {
    // draw ctx.board squares and soldiers
    drawInfoBar();
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            drawSq((Position){.row = y, .col = x});
        }
    }
}

void drawSq(Position arrPos) {
    Square *sq = &ctx.board.Squares[arrPos.row][arrPos.col];
    Vector2 rectPos = (Vector2){
        .x = arrPos.col * SQUARE_WIDTH,
        .y = arrPos.row * SQUARE_WIDTH + INFOBAR_HEIGHT,
    };
    DrawRectangleV(rectPos, (Vector2){.x = SQUARE_WIDTH, .y = SQUARE_WIDTH},
                   colors[SquareColors[arrPos.row][arrPos.col]]);

    if (!sq->sldr) return;
    Vector2 sldrPos = getSldrPos(sq->sldr->arrPos);
    if (sq->sldr->team_set->teamColor == WHITE_TEAM) {
        DrawTexture(whiteShapeText[sq->sldr->type], sldrPos.x, sldrPos.y, WHITE);
    } else
        DrawTexture(blackShapeText[sq->sldr->type], sldrPos.x, sldrPos.y, WHITE);
}

void colorBoardSquares() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            SquareColors[y][x] = (y + x) % 2;
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

void initColors(Color *colors) {
    colors[0] = DARKBROWN;
    colors[1] = BEIGE;
    colors[2] = GREEN;
}

void initGameData() {
    arena_init(&global_arena, 1024 * 2);
    SetTraceLogLevel(LOG_NONE);
    InitWindow(800, BOARD_HEIGHT + INFOBAR_HEIGHT, "Chess");
    SetTargetFPS(60);
    initColors(colors);
    initBoard(&ctx.board);
    blackTimer = &ctx.board.sets[0].timer;
    whiteTimer = &ctx.board.sets[1].timer;
    ctx.ACTIVE = WHITE_TEAM;
    ctx.movementChange = FROM;
    ctx.fromSquare = NULL;
    colorBoardSquares();
}

// i need an arena
void destroyData() {
    for (int i = PAWN; i < KING + 1; i++) {
        UnloadTexture(whiteShapeText[i]);
        UnloadTexture(blackShapeText[i]);
    }
    CloseWindow();
    arena_destroy(&global_arena);
    alist_destroy(&ctx.availableSqs);
}

int16_t alist_push_pos(alist_t *list, Position arrPos) {
    Square *sq = chooseSquare(arrPos);
    if (!list || !sq) return EXIT_FAILURE;
    SquareColors[arrPos.row][arrPos.col] = 2;
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
