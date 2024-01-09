#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux
#define CLEAR system("clear")
#endif

#ifdef __WIN32__
#define CLEAR system("cls")
#endif

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef enum { DEAD, LIVE, CANT_MOVE } SOLDIER_STATE;
typedef enum { WHITE, BLACK } COLOR;
typedef enum { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING } Soldier_t;
typedef enum { FROM, TO } CHANGE;
typedef enum { ZERO, ONE, MORE_THAN_ONE } NM_OF_MOVES;
typedef struct Set_t Set_t;

typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    NM_OF_MOVES NMOVES;
    union {
        bool enpassant;
        bool check;
    };
} OtherData;

typedef struct {
    Set_t *TEAM;
    OtherData *otherdt;
    char *shap;
    Position pos;
    Soldier_t type;
    SOLDIER_STATE State;
} Soldier;

struct Set_t {
    Soldier *soldiers;
    unsigned int count;
    COLOR color;
};
// set struct

typedef struct {
    Soldier *sldr;
    char *color;
    bool occupied;
} Square;

typedef struct {
    Set_t *sets[2];
    Square Squares[8][8];
    COLOR ACTIVE;
} Board;

typedef struct {
    Square **stack;
    int top;
    int count;
} availableSqs;

typedef struct {
    Board *board;
    availableSqs *available;
    char **colors;
    COLOR ACTIVE;
} Data;

void game();
Set_t *createSet(COLOR color);
Board *createBoard(Set_t *white, Set_t *black);
// next position functions
availableSqs *calcNextMove(Square *sq);
availableSqs *calcNextMovePawn(Square *fsq);
availableSqs *calcNextMoveKnight(Square *fsq);
availableSqs *calcNextMoveRook(Square *fsq);
availableSqs *calcNextMoveBishop(Square *fsq);
availableSqs *calcNextMoveQueen(Square *fsq);
availableSqs *calcNextMoveKing(Square *fsq);
bool isEnemy(Square *from, Square *to);
// move functions
Square *chooseSquare(Position pos);
Position choosePos(CHANGE change);
int moveSldr();
bool isAvailable(Square *sq);
void changeActive();
// displaying functions
void drawBoard();
void displaySq(Square *sq);
void displayNextSqsList();
// av list functions
availableSqs *dlist(unsigned int);
availableSqs *mergeList(availableSqs *, availableSqs *);
void push(availableSqs *, Square *);
void stackResize(availableSqs *, int);
// closing functions
void destroydata();
void erroredEnd();
void destroyAvList(availableSqs *);

// for testing
//
Set_t *onlyType(Soldier_t t, COLOR color);
void mirrorBoard(Board *);
void colorBoardSquares();
bool inBound(int);
char **initColor();
void drawWhileWhite();
void drawWhileBlack();
void initgdata();
