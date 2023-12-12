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

typedef enum { LIVE, DEAD } SOLDIER_STATE;
typedef enum { WHITE, BLACK } COLOR;
typedef enum { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING } Soldier_t;
typedef enum { FROM, TO } CHANGE;
typedef enum { ZERO, ONE, MORE_THAN_ONE } NM_OF_MOVES;
typedef struct SET_t SET_t;

typedef struct {
  int row;
  char col;
} Position;

typedef struct {
  SOLDIER_STATE State;
  Soldier_t type;
  NM_OF_MOVES NMOVES;
  char *shap;
  SET_t *TEAM;
} Soldier;

struct SET_t {
  COLOR color;
  Soldier *soldiers;
  unsigned int count;
};
// set struct

typedef struct {
  Position pos;
  Soldier *sldr;
  bool occupied;
  char *color;
} Square;

typedef struct {
  SET_t *sets[2];
  Square Squares[8][8];
} Board;

typedef struct {
  Square **stack;
  int top;
  int count;
} availableSqs;

void game();
SET_t *Set(COLOR);
Board *createBoard(SET_t *, SET_t *);
// next position functions
availableSqs *calcNextMove(Square *);
availableSqs *calcNextMovePawn(Square *);
availableSqs *calcNextMoveKnight(Square *);
availableSqs *calcNextMoveRook(Square *);
availableSqs *calcNextMoveBishop(Square *);
availableSqs *calcNextMoveQueen(Square *);
availableSqs *calcNextMoveKing(Square *);
bool vsq(Square *, Square *);
// move functions
Square *chooseSquare(CHANGE change);
int moveSldr();
bool isAvailable(Square *);
void changeActive();
// displaying functions
void drawBoard();
void displaySq(Square *);
void displaySqPosition(Square *);
void displayNextSqsList();
// av list functions
availableSqs *dlist(unsigned int);
availableSqs *mergeList(availableSqs *, availableSqs *);
void push(availableSqs *, Square *);
void stackResize(availableSqs *, int);
// closing functions
void destroy();
void erroredEnd();
void destroyAvList(availableSqs *);

// for testing
//
SET_t *onlyType(Soldier_t t, COLOR color);
