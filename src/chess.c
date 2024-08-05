#define ARENA_IMPLEMENTATION

#include "chess.h"
#include "arena.h"
#include "data.h"
#include "garraylist.h"
#include "raylib.h"

Arena global_arena;
Context ctx;
Timer *blackTimer, *whiteTimer;
double globalTime = 0;

int main() {
    initGameData();
    game();
    destroyData();
    return 0;
}

static inline bool isSecPassed(double previous, float seconds) {
    return (GetTime() - previous >= seconds);
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

void initBoard(Board *b) {
    Set_t *black = &b->sets[0], *white = &b->sets[1];
    initSet(&b->sets[0], BLACK_TEAM);
    initSet(&b->sets[1], WHITE_TEAM);
    // onlyType(white, ROOK, WHITE_TEAM);
    // onlyType(black, ROOK, BLACK_TEAM);

    // Initialize the square & set positions
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            b->Squares[y][x].pos.col = x * SQUARE_WIDTH;
            b->Squares[y][x].pos.row = y * SQUARE_WIDTH + INFOBAR_HEIGHT;
            b->Squares[y][x].occupied = false;
            b->Squares[y][x].sldr = NULL;
            b->Squares[y][x].color = ctx.colors[isOdd(y + x)];
        }
    }

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 8; x++) {
            if (black->soldiers[(y * 8) + x].State == LIVE) {
                b->Squares[y][x].occupied = true;
                b->Squares[y][x].sldr = &(black->soldiers[(y * 8) + x]);
                b->Squares[y][x].sldr->pos.col = x;
                b->Squares[y][x].sldr->pos.row = y;
            }
        }
    }

    for (int y = 6; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (white->soldiers[((y - 6) * 8) + x].State == LIVE) {
                b->Squares[y][x].occupied = true;
                b->Squares[y][x].sldr = &(white->soldiers[((y - 6) * 8) + x]);
                b->Squares[y][x].sldr->pos.col = x;
                b->Squares[y][x].sldr->pos.row = y;
            }
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
    // s->soldiers = arena_alloc(&global_arena, 16 * sizeof(Soldier));

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
    for (int i = 0; i < 16; i++) {
        s->soldiers[i].team = s;
        s->soldiers[i].State = LIVE;
        s->soldiers[i].otherdt = NULL;
        if (s->soldiers[i].type == PAWN || s->soldiers[i].type == KING)
            s->soldiers[i].otherdt = arena_alloc(&global_arena, sizeof(OtherData));
        if (s->soldiers[i].type == PAWN) {
            s->soldiers[i].otherdt->NMOVES = ZERO;
            s->soldiers[i].otherdt->enpassant = false;
        } else if (s->soldiers[i].type == KING) {
            s->soldiers[i].otherdt->NMOVES = ZERO;
            s->soldiers[i].otherdt->check = false;
        }
    }
    char *sldr_types[] = {
        [PAWN] = "pawn", [KNIGHT] = "knight", [BISHOP] = "bishop", [ROOK] = "rook", [QUEEN] = "queen", [KING] = "king"};
    char *team_color[] = {[WHITE_TEAM] = "white", [BLACK_TEAM] = "black"};
    float scale_factor = 0.85f;
    for (int i = 0; i < 16; i++) {
        Image soldierImage = {0};
        soldierImage = LoadImage(TextFormat("./assets/"
                                            "128px/"
                                            "%s_%s_png_shadow.png",
                                            team_color[s->teamColor], sldr_types[s->soldiers[i].type]));
        ImageResize(&soldierImage, SQUARE_WIDTH * scale_factor, SQUARE_WIDTH * scale_factor);
        s->soldiers[i].shapText = LoadTextureFromImage(soldierImage);
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
    Position *tpos;

    // mirror each soldier position
    for (int s = 0; s < 2; s++) {
        for (int j = 0; j < 16; j++) {
            tpos = &ctx.board.sets[s].soldiers[j].pos;
            tpos->row = 7 - tpos->row;
            tpos->col = 7 - tpos->col;
        }
    }

    // mirror rectangles
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &ctx.board.Squares[7 - i][j];
            sq2 = &ctx.board.Squares[i][j];
            temp = *sq1;
            sq1->pos = sq2->pos;
            sq2->pos = temp.pos;
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            sq1 = &ctx.board.Squares[i][7 - j];
            sq2 = &ctx.board.Squares[i][j];
            temp = *sq1;
            sq1->pos = sq2->pos;
            sq2->pos = temp.pos;
        }
    }

    // mirror rows
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sq1 = &ctx.board.Squares[7 - i][j];
            sq2 = &ctx.board.Squares[i][j];
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
            drawSq(&ctx.board.Squares[y][x]);
            // drawSqEx(&ctx.board.Squares[y][x], (Vector2){
            //                                        .x = x * SQUARE_WIDTH,
            //                                        .y = y * SQUARE_WIDTH + INFOBAR_HEIGHT,
            //                                    });
        }
    }
}

void drawSq(Square *sq) {
    DrawRectangle(sq->pos.col, sq->pos.row, SQUARE_WIDTH, SQUARE_WIDTH, sq->color);
    if (sq->occupied) {
        Vector2 sldrPos = {sq->pos.col + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f),
                           sq->pos.row + ((SQUARE_WIDTH - sq->sldr->shapText.width) / 2.f)};
        DrawTexture(sq->sldr->shapText, sldrPos.x, sldrPos.y, WHITE);
    }
}

void colorBoardSquares() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            ctx.board.Squares[i][j].color = ctx.colors[(i + j) % 2];
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
    initColors(ctx.colors);
    initBoard(&ctx.board);
    // ctx.board = initBoard();
    blackTimer = &ctx.board.sets[0].timer;
    whiteTimer = &ctx.board.sets[1].timer;
    ctx.ACTIVE = WHITE_TEAM;
    ctx.movementChange = FROM;
    ctx.fromSquare = NULL;
    colorBoardSquares();
}

// i need an arena
void destroyData() {
    for (int i = 0; i < 16; i++) {
        UnloadTexture(ctx.board.sets[0].soldiers[i].shapText);
        UnloadTexture(ctx.board.sets[1].soldiers[i].shapText);
    }
    CloseWindow();
    arena_destroy(&global_arena);

    alist_destroy(&ctx.availableSqs);
}

int16_t alist_push_sq(alist_t *list, Square *sq) {
    if (!list || !sq) return EXIT_FAILURE;
    sq->color = GREEN;
    Position pos = getArrPos((Vector2){sq->pos.col, sq->pos.row});
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
