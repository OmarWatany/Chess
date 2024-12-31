#ifndef _CHESS_HEADER
#define _CHESS_HEADER

#include "arena.h"
#include "gds_types.h"
#include <stdbool.h>

#ifdef __linux
#define CLEAR system("clear")
#elif defined(__WIN32__)
#define CLEAR system("cls")
#endif

#define BOARD_WIDTH 800.0f
#define BOARD_HEIGHT 800.0f
#define BOARD_START 0.0f
#define SQUARE_WIDTH (BOARD_WIDTH / 8.0f)
#define INFOBAR_HEIGHT 80
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define isOdd(x) ((x) % 2)
#define isEven(x) (!isOdd(x))

typedef enum { DEAD, LIVE, CANT_MOVE } SOLDIER_STATE;
typedef enum { WHITE_TEAM, BLACK_TEAM } TEAM;
typedef enum { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING } SOLDIER_TYPE;
typedef enum { FROM, TO } CHANGE;
typedef enum { ZERO, ONE, MORE_THAN_ONE } NM_OF_MOVES;
typedef enum { MSG_BOGUS = 1, MSG_MOVE, PEER_CLOSED } MESSAGE_KIND;

typedef struct Set_t Set_t;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    double m, s;
} Timer;

typedef struct {
    NM_OF_MOVES NMOVES;
    union {
        bool enpassant;
        bool check;
    };
} OtherData;

typedef struct {
    Position arrPos;
    Set_t *team_set;
    OtherData *otherdt;
    SOLDIER_TYPE type;
    SOLDIER_STATE State;
} Soldier;

struct Set_t {
    Soldier soldiers[16];
    unsigned int count;
    TEAM teamColor;
    Timer timer;
};

typedef struct {
    Soldier *sldr;
    bool occupied;
} Square;

typedef struct {
    Set_t sets[2];
    Square Squares[8][8];
} Board;

typedef struct {
    alist_t availableSqs;
    TEAM ACTIVE;
    CHANGE movementChange;
    Board board;
    Position fromPos;
} Context;

typedef struct {
    char start[2];
    MESSAGE_KIND kind;
    Position from, to;
    char end[2];
} Message;

void game();
void killEnemey(Soldier *sldr);
void initSet(Set_t *s, TEAM color);
void initBoard(Board *b);
// next position functions
alist_t calcNextMove(Square *sq);
alist_t calcNextMovePawn(Square *fsq);
alist_t calcNextMoveKnight(Square *fsq);
alist_t calcNextMoveRook(Square *fsq);
alist_t calcNextMoveBishop(Square *fsq);
alist_t calcNextMoveQueen(Square *fsq);
alist_t calcNextMoveKing(Square *fsq);
bool isEnemy(Square *from, Square *to);
// move functions
Square *chooseSquare(Position pos);
Position choosePos(CHANGE change);
Soldier *selectSldr(Position SqPos);
int moveSldr(Position from, Position to);
int moveFrom(Position pos);
int moveTo(Position to);
bool isAvailable(Square *sq);
bool isSecPassed(time_t previous, time_t seconds);
void changeActive();
void resetMovement();
void incTimer();
// displaying functions
void drawBoard();
void drawSq(Position arrPos);
void drawSqp(Square *sq);
void displayNextSqsList();
// av list functions
alist_t mergeList(alist_t *, alist_t *);
// closing functions
void destroyData();
void errorExit();
void mirrorBoard();
void mirrorMassage(Message *msg);
void msgSetup(Message *msg);
int msgVerify(Message *msg);
void colorBoardSquares();
bool inBoundaries(int);
void drawWhileWhite();
void drawWhileBlack();
void initGameData();

int16_t alist_push_pos(alist_t *list, Position arrPos);
Square *alist_sq_at(alist_t *list, size_t at);

int gui_menu();

// for testing
void onlyType(Set_t *s, SOLDIER_TYPE t, TEAM color);

// global variables
extern ringbuffer rbuffer;
extern Context ctx;
extern Arena global_arena;
extern Timer *blackTimer, *whiteTimer;
extern time_t globalTime;
extern uint8_t SquareColors[8][8];
extern bool online;

#endif // !_CHESS_HEADER
