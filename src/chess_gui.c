#include "chess.h"
#include "arena.h"
#include "garraylist.h"
#include "raylib.h"

void initTextures();
void initColors(Color *colors);
static inline Position getArrPos(Vector2 from);
static inline Vector2 getSldrPos(Position arrPos);
static inline bool isSecPassed(double previous, float seconds);

// GUI client's data
Position sldrSize;
Texture2D whiteShapeText[KING + 1] = {0};
Texture2D blackShapeText[KING + 1] = {0};
Color colors[3];

int main() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(800, BOARD_HEIGHT + INFOBAR_HEIGHT, "Chess");
    SetTargetFPS(60);

    initColors(colors);
    initGameData();
    initTextures();

    game();

    destroyData();
    return 0;
}

void game() {
    int valid;
    while (!WindowShouldClose()) {
        valid = 0;
        if (isSecPassed(globalTime, 1)) {
            globalTime = GetTime();
            incTimer();
        }

        if (IsKeyReleased(KEY_C)) {
            resetMovement();
        } else if (IsKeyReleased(KEY_Q)) {
            break;
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

        ClearBackground(BLACK);
        BeginDrawing();
        drawBoard();
        EndDrawing();
    }
}

bool isSecPassed(double previous, float seconds) {
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

void initTextures() {
    Image soldierImage;
    Set_t *s;
    float scale_factor = 0.85f;
    char *sldr_types[] = {
        [PAWN] = "pawn", [KNIGHT] = "knight", [BISHOP] = "bishop", [ROOK] = "rook", [QUEEN] = "queen", [KING] = "king",
    };
    char *team_color[] = {
        [WHITE_TEAM] = "white",
        [BLACK_TEAM] = "black",
    };
    for (int j = 0; j < 2 + 1; j++) {
        s = &ctx.board.sets[j];
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

    sldrSize = (Position){
        .col = whiteShapeText[PAWN].width,
        .row = whiteShapeText[PAWN].height,
    };
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

void destroyData() {
    for (int i = PAWN; i < KING + 1; i++) {
        UnloadTexture(whiteShapeText[i]);
        UnloadTexture(blackShapeText[i]);
    }
    CloseWindow();
    arena_destroy(&global_arena);
    alist_destroy(&ctx.availableSqs);
}

void initColors(Color *colors) {
    colors[0] = DARKBROWN;
    colors[1] = BEIGE;
    colors[2] = GREEN;
}
