#include "arena.h"
#include "chess.h"
#include "garraylist.h"
#include "gringbuffer.h"
#include "networking.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void initTextures();
void initColors(Color *colors);
void onlineGame();
int hostListen();
void clientInit();
static inline Position getArrPos(Vector2 from);
static inline Vector2 getSldrPos(Position arrPos);

// GUI client's data
Position sldrSize;
Texture2D whiteShapeText[KING + 1] = {0};
Texture2D blackShapeText[KING + 1] = {0};
Color colors[3];
TEAM PLAYER = WHITE_TEAM;

SOCKET host = -1, client = -1;
bool online = true;

fd_set default_set;

void mainMenu() {
    bool start = true;
    char *menuOptions[] = {
        "START GAME", "RESUME", "HOST", "CLIENT", "QUIT GAME",
    };

    switch (menu(menuOptions, ARRAY_LEN(menuOptions))) {
    case 0: // Start Game button
        online = false;
        game();
        break;
    case 2: // Host
        if (hostListen()) return;
        break;
    case 3: // Client
        clientInit();
        break;
    default:
        start = false;
        break;
    };
    if (start && online) onlineGame();
}

// NOTE: I can take main as common factor
int main() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(BOARD_WIDTH, BOARD_HEIGHT + INFOBAR_HEIGHT, "Chess");
    SetTargetFPS(60);

    initColors(colors);
    initGameData();
    initTextures();

    mainMenu();
    destroyData();
    return 0;
}

int hostListen() {
    host = serverSocket(0, PORT);
    listen(host, 2);
    char *waiting = "Waiting for connection...";
    int fontSize = 40;
    Vector2 loadingDim = MeasureTextEx(GetFontDefault(), waiting, fontSize, 0);
    bool loading = true;
    while (!ISVALIDSOCKET(client)) {
        if (loading) {
            ClearBackground(WHITE);
            BeginDrawing();
            DrawText(waiting, (BOARD_WIDTH - loadingDim.x) / 2.f, (BOARD_WIDTH - loadingDim.y) / 2.f, fontSize, BLACK);
            EndDrawing();
        }
        if (IsKeyReleased(KEY_Q)) {
            close(host);
            return EXIT_FAILURE;
        }
        struct sockaddr_storage clientAddrress;
        socklen_t clientLen = sizeof(clientAddrress);
        client = accept(host, (struct sockaddr *)&clientAddrress, &clientLen);
        if (!ISVALIDSOCKET(client)) continue;

        nonblock(client);
        loading = false;
        char addressBuffer[100];
        char portNumber[8];
        getnameinfo((struct sockaddr *)&clientAddrress, clientLen, addressBuffer, sizeof(addressBuffer), portNumber,
                    sizeof(portNumber), NI_NUMERICHOST);
        printf("Client %s:%s connected\n", addressBuffer, portNumber);
    }
    FD_ZERO(&default_set);
    FD_SET(client, &default_set);
    return EXIT_SUCCESS;
}

void clientInit() {
    PLAYER = BLACK_TEAM;
    mirrorBoard();
    host = connectHost(0, PORT);
    FD_ZERO(&default_set);
    FD_SET(host, &default_set);
}

void onlineGame() {
    int valid;
    globalTime = time(0);
    Message moveMsg = {0};

    while (!WindowShouldClose()) {
        ClearBackground(BLACK);
        BeginDrawing();
        drawBoard();
        EndDrawing();
        valid = 0;

        if (isSecPassed(globalTime, 1)) incTimer();
        if (IsKeyReleased(KEY_C)) resetMovement();
        if (IsKeyReleased(KEY_Q) || host < 0) goto FUNC_END;

        moveMsg = getMessage(PLAYER == WHITE_TEAM ? client : host);
        if (msgVerify(&moveMsg)) moveMsg.kind = MSG_BOGUS;
        if (moveMsg.kind == PEER_CLOSED) goto FUNC_END;

        if (ctx.ACTIVE != PLAYER) {
            switch (moveMsg.kind) {
            case MSG_BOGUS:
                printf("MSG_BOGUS\n");
                // TODO: resend Message
                break;
            case MSG_MOVE:
                // temporary solution
                mirrorBoard();
                valid = moveFrom(moveMsg.from);
                valid = moveTo(moveMsg.to);
                if (valid == 1) {
                    Square *nextSq = chooseSquare(moveMsg.to);
                    Soldier *sldr = nextSq->sldr;
                    sldr->arrPos = moveMsg.to;
                }
                mirrorBoard();
            default:
                break;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ctx.ACTIVE == PLAYER) {
            Position tArrPos = getArrPos(GetMousePosition());
            if (ctx.movementChange == FROM) {
                valid = moveFrom(tArrPos);
            } else if (ctx.movementChange == TO && (valid = moveTo(tArrPos)) == 1) {
                moveMsg.to = tArrPos;
                Square *nextSq = chooseSquare(tArrPos);
                Soldier *sldr = nextSq->sldr;
                sldr->arrPos = tArrPos;
                msgSetup(&moveMsg);
                sendMessage(PLAYER == WHITE_TEAM ? client : host, &moveMsg);
            }
            if ((ctx.movementChange == TO && valid == 2) || ctx.movementChange == FROM) moveMsg.from = tArrPos;
        }
    }
FUNC_END:
    if (host >= 0) close(host);
    if (client >= 0) close(client);
}

void game() {
    globalTime = time(0);

    int valid;
    while (!WindowShouldClose()) {
        valid = 0;
        if (isSecPassed(globalTime, 1)) incTimer();

        if (IsKeyReleased(KEY_C)) {
            resetMovement();
        } else if (IsKeyReleased(KEY_Q)) {
            break;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            Position tArrPos = getArrPos(GetMousePosition());
            if (ctx.movementChange == FROM) {
                valid = moveFrom(tArrPos);
            } else if (ctx.movementChange == TO && (valid = moveTo(tArrPos))) {
                Square *nextSq = chooseSquare(tArrPos);
                Soldier *sldr = nextSq->sldr;
                sldr->arrPos = tArrPos;
                mirrorBoard();
            }
        }

        ClearBackground(BLACK);
        BeginDrawing();
        drawBoard();
        EndDrawing();
    }
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
    Square *sq = &(ctx.board.Squares[arrPos.row][arrPos.col]);
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
    ring_destroy(&rbuffer);
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
