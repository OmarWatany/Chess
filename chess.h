#ifndef _CHESS_HEِِِADER
#define _CHESS_HEِِِADER

#include "raylib/include/raylib.h"
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

#define WIN_WIDTH 800.0f
#define WIN_HEIGHT 800.0f
#define BOARD_WIDTH 800.0f
#define BOARD_HEIGHT 800.0f
#define BOARD_START 0.0f
#define SQUARE_WIDTH (WIN_WIDTH / 8.0f)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define isOdd(x) ((x) % 2)
#define isEven(x) (!isOdd(x))

typedef enum { DEAD, LIVE, CANT_MOVE } SOLDIER_STATE;
typedef enum { WHITE_TEAM, BLACK_TEAM } TEAM_COLOR;
typedef enum { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING } SOLDIER_TYPE;
typedef enum { FROM, TO } CHANGE;
typedef enum { ZERO, ONE, MORE_THAN_ONE } NM_OF_MOVES;

typedef struct Set_t Set_t;
typedef struct Position Position;
typedef struct OtherData OtherData;
typedef struct Soldier Soldier;
typedef struct Square Square;
typedef struct Board Board;
typedef struct availableSqs availableSqs;
typedef struct Context Context;

struct Position {
    int row;
    int col;
};

struct OtherData {
    NM_OF_MOVES NMOVES;
    union {
        bool enpassant;
        bool check;
    };
};

struct Soldier {
    Set_t *TEAM;
    OtherData *otherdt;
    Texture2D shapText;
    Position pos;
    SOLDIER_TYPE type;
    SOLDIER_STATE State;
};

struct Set_t {
    Soldier *soldiers;
    unsigned int count;
    TEAM_COLOR color;
};

struct Square {
    Soldier *sldr;
    Color color;
    Rectangle rectangle;
    bool occupied;
};

struct Board {
    Set_t *sets[2];
    Square Squares[8][8];
};

struct availableSqs {
    Square **stack;
    int top;
    int count;
};

struct Context {
    Board *board;
    availableSqs *available;
    Color *colors;
    TEAM_COLOR ACTIVE;
    CHANGE movementChange;
    Square *fromSquare;
};

void game();
void killEnemey(Soldier *sldr);
Set_t *createSet(TEAM_COLOR color);
Board *createBoard(Set_t *white, Set_t *black);
// next position functions
availableSqs *calcNextMove(Square *sq);
availableSqs *calcNextMovePawn(Square *fsq);
availableSqs *calcNextMoveKnight(Square *fsq);
availableSqs *calcNextMoveRook(Square *fsq);
availableSqs *calcNextMoveBishop(Square *fsq);
availableSqs *calcNextMoveQueen(Square *fsq);
availableSqs *calcNextMoveKing(Square *fsq);
// astack_t *calcNextMoveKing(Square *fsq);
bool isEnemy(Square *from, Square *to);
// move functions
Square *chooseSquare(Position pos);
Position choosePos(CHANGE change);
Position getPos();
Soldier *selectSldr(Position SqPos);
int moveSldr(Position pos);
bool isAvailable(Square *sq);
void changeActive();
// displaying functions
void drawBoard();
void drawSq(Square *sq);
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
void mirrorBoard();
void colorBoardSquares();
bool inBoundaries(int);
Color *initColor();
void drawWhileWhite();
void drawWhileBlack();
void initgdata();

// for testing
//
Set_t *onlyType(SOLDIER_TYPE t, TEAM_COLOR color);

#endif // !_CHESS_HEِِِADER
