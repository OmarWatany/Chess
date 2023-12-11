#include "chess.h"
#include <stdio.h>
#include <stdlib.h>

Board *gboard;
availableSqs *available;
COLOR ACTIVE;

int main() {
  // SET_t *white = onlyType(QUEEN, WHITE);
  // SET_t *black = onlyType(QUEEN, BLACK);
  SET_t *white = Set(WHITE);
  SET_t *black = Set(BLACK);
  gboard = createBoard(white, black);
  ACTIVE = WHITE;

  game();
  destroy();
  return 0;
}

void game() {
  int data, valid;
  Square *choosen;
  bool RUNING_STAT = true;
  CLEAR;
  while (RUNING_STAT) {
    drawBoard();
    printf("1. choose soldier");
    printf("\t2. quit\n");
    printf("Enter option : ");
    valid = scanf("%d", &data);
    if (!valid)
      erroredEnd();
    switch (data) {
    case 1:
      choosen = chooseSquare(FROM);

      destroyAvList(available);
      available = calcNextMove(choosen);
      if (available == NULL)
        erroredEnd();

      displayNextSqsList();
      Square *next;
      next = chooseSquare(TO);
      moveSldr(choosen, next);
      break;
    case 2:
      RUNING_STAT = false;
      break;
    }
    CLEAR;
  }
}

SET_t *Set(COLOR color) {
  SET_t *s = malloc(sizeof(SET_t));
  s->count = 16;
  // s->color = color;
  s->soldiers = malloc(16 * sizeof(Soldier));

  // Soldier soldiers[16];
  for (int i = 0; i < 16; i++) {
    s->soldiers[i].NMOVES = ZERO;
    s->soldiers[i].TEAM = s;
  }

  for (int i = 0; i < 16; i++) {
    s->soldiers[i].State = LIVE;
  }

  Soldier_t types[4] = {ROOK, KNIGHT, BISHOP, PAWN};
  // set positions
  switch (color) {
  case BLACK:
    s->color = BLACK;
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
  case WHITE:
    s->color = WHITE;
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

  for (int i = 0; i < 16; i++) {
    switch (s->soldiers[i].type) {
    case PAWN:
      s->soldiers[i].shap = strdup("PW");
      break;
    case KNIGHT:
      s->soldiers[i].shap = strdup("KN");
      break;
    case BISHOP:
      s->soldiers[i].shap = strdup("BI");
      break;
    case ROOK:
      s->soldiers[i].shap = strdup("RK");
      break;
    case QUEEN:
      s->soldiers[i].shap = strdup("QN");
      break;
    case KING:
      s->soldiers[i].shap = strdup("KG");
      break;
    }
  }

  return s;
}

SET_t *onlyType(Soldier_t t, COLOR color) {
  SET_t *s;
  s = Set(color);
  for (int i = 0; i < 16; i++) {
    if (s->soldiers[i].type != t) {
      s->soldiers[i].State = DEAD;
      s->count--;
    }
  }
  return s;
}

Board *createBoard(SET_t *white, SET_t *black) {
  Board *b = malloc(sizeof(Board));
  b->sets[0] = black;
  b->sets[1] = white;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      b->Squares[i][j].occupied = false;
      if ((i + j) % 2 == 0) {
        b->Squares[i][j].color = strdup("+");
      } else {
        b->Squares[i][j].color = strdup("\033[;31m#\033[;0m");
      }
    }
  }

  // set each square position
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      b->Squares[i][j].pos.col = j + 'A';
      b->Squares[i][j].pos.row = 8 - i;
    }
  }

  // set sets positions
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
      if (black->soldiers[(i * 8) + j].State == LIVE) {
        b->Squares[i][j].occupied = true;
        b->Squares[i][j].sldr = &(black->soldiers[(i * 8) + j]);
      }
    }
  }

  for (int i = 6; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (white->soldiers[((i - 6) * 8) + j].State == LIVE) {
        b->Squares[i][j].occupied = true;
        b->Squares[i][j].sldr = &(white->soldiers[((i - 6) * 8) + j]);
      }
    }
  }

  return b;
}

availableSqs *calcNextMove(Square *sq) {
  Soldier *sldr = sq->sldr;
  switch (sldr->type) {
  case PAWN:
    return calcNextMovePawn(sq);
    break;
  case KNIGHT:
    return calcNextMoveKnight(sq);
    break;
  case BISHOP:
    return calcNextMoveBishop(sq);
    break;
  case ROOK:
    return calcNextMoveRook(sq);
    break;
  case QUEEN:
    return calcNextMoveQueen(sq);
    break;
  case KING:
    return calcNextMoveKing(sq);
    break;
  }
}

availableSqs *calcNextMovePawn(Square *f) {
  // TODO : calculate es passant
  availableSqs *nextSqs;
  int colmn = f->pos.col - 'A', row = 8 - f->pos.row;
  // number possible squares
  int praws;
  // check if there is victims in corners or beside him

  if (f->sldr->NMOVES == ZERO) {
    nextSqs = dlist(4);
    praws = 2;
  } else {
    nextSqs = dlist(3);
    praws = 1;
  }

  Square *n;

  // check the next rows
  for (int i = 1; i <= praws; i++) {
    switch (f->sldr->TEAM->color) {
    case BLACK:
      n = &(gboard->Squares[row + i][colmn]);
      break;
    case WHITE:
      n = &(gboard->Squares[row - i][colmn]);
      break;
    }
    if (vsq(f, n))
      push(nextSqs, n);
  }
  // check the next colmns
  for (int i = -1; i <= 1; i++) {
    if (i == 0)
      continue;
    if (f->sldr->TEAM->color == BLACK) {
      n = &(gboard->Squares[row + 1][colmn + i]);
    } else {
      n = &(gboard->Squares[row - 1][colmn + i]);
    }
    if (n->occupied && vsq(f, n))
      push(nextSqs, n);
  }

  return nextSqs;
}

availableSqs *calcNextMoveKnight(Square *f) {
  availableSqs *nextSqs;
  int colmn = f->pos.col - 'A', row = 8 - f->pos.row;
  nextSqs = dlist(8);

  // set boundries
  int cb1 = -2, cb2 = 2;
  int rb1 = -1, rb2 = 1;

  if (colmn < 2) {
    cb1 = -1 * colmn;
  }
  if (colmn > 5) {
    cb2 = 7 - colmn;
  }
  if (row < 2) {
    rb1 = row;
  }
  if (row >= 6) {
    rb2 = 7 - row;
  }

  Square *n;
  for (int col = cb1; col <= cb2; col++) {
    if (col == 0)
      continue;
    for (int r = rb1; r <= rb2; r++) {
      if (r == 0)
        continue;
      if ((col % 2) == 0) {
        n = &(gboard->Squares[row + r][colmn + col]);
      } else {
        n = &(gboard->Squares[row + (2 * r)][colmn + col]);
      }
      if (vsq(f, n))
        push(nextSqs, n);
    }
  }

  return nextSqs;
}

availableSqs *calcNextMoveRook(Square *f) {
  int colmn = f->pos.col - 'A', row = 8 - f->pos.row;
  int b1 = 0, b2 = 7;

  availableSqs *nextSqs;
  nextSqs = dlist(15);

  Square *n;

  for (int c = colmn + 1; c <= 7; c++) {
    n = &(gboard->Squares[row][c]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int c = colmn - 1; c >= b1; c--) {
    n = &(gboard->Squares[row][c]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int r = row + 1; r <= b2; r++) {
    n = &(gboard->Squares[r][colmn]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int r = row - 1; r >= b1; r--) {
    n = &(gboard->Squares[r][colmn]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  return nextSqs;
}

availableSqs *calcNextMoveBishop(Square *f) {
  int colmn = f->pos.col - 'A', row = 8 - f->pos.row;
  int b1 = 0, b2 = 7;

  availableSqs *nextSqs;
  nextSqs = dlist(15);

  Square *n;

  for (int i = 1; colmn + i <= 7 && row - i >= 0; i++) {
    n = &(gboard->Squares[row - i][colmn + i]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int i = 1; colmn + i <= 7 && row + i <= 7; i++) {
    n = &(gboard->Squares[row + i][colmn + i]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int i = 1; colmn - i >= 0 && row + i <= 7; i++) {
    n = &(gboard->Squares[row + i][colmn - i]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  for (int i = 1; colmn - i >= 0 && row - i >= 0; i++) {
    n = &(gboard->Squares[row - i][colmn - i]);
    if (!n->occupied && vsq(f, n)) {
      push(nextSqs, n);
    } else if (vsq(f, n) && n->occupied) {
      push(nextSqs, n);
      break;
    } else
      break;
  }

  return nextSqs;
}

availableSqs *calcNextMoveQueen(Square *f) {

  availableSqs *first = calcNextMoveBishop(f);
  availableSqs *second = calcNextMoveRook(f);
  availableSqs *nextSqs = mergeList(first, second);
  // number possible squares
  return nextSqs;
}

availableSqs *calcNextMoveKing(Square *f) {
  availableSqs *nextSqs;
  Square *n;
  int colmn = f->pos.col - 'A', row = 8 - f->pos.row;
  for (int c = -1; c <= 1; c++) {
    for (int r = -1; r <= 1; r++) {
      n = &(gboard->Squares[row + r][colmn + c]);
      if (vsq(f, n))
        push(nextSqs, n);
    }
  }
  return nextSqs;
}

bool vsq(Square *from, Square *to) {
  if (!to->occupied) {
    return true;
  } else {
    if (to->sldr->TEAM->color != from->sldr->TEAM->color)
      // if is Enemy return is valid
      return true;
  }
  return false;
}

Square *chooseSquare(CHANGE change) {
  int valid, raw;
  char sq[2], col;
  Square *s;
  switch (change) {
  case FROM:
    printf("From : ");
    break;
  case TO:
    printf("To : ");
    break;
  }

  valid = scanf("%s", sq);
  if (!valid)
    erroredEnd();

  col = sq[0];
  raw = sq[1] - '0';
  if (col > 72 || col < 65) {
    printf("Column : ");
    valid = scanf(" %c", &col);
  }
  if (raw > 8 || col < 1) {
    printf("Raw : ");
    valid = scanf("%d", &raw);
  }

  s = &(gboard->Squares[8 - raw][(col - 65)]);

  switch (change) {
  case FROM:
    if (s->sldr->TEAM->color == ACTIVE) {
      return s;
    } else {
      return chooseSquare(FROM);
    }
    break;
  case TO:
    return s;
    break;
  }
}

void moveSldr(Square *from, Square *next) {
  if (isAvailable(next)) {
    switch (from->sldr->NMOVES) {
    case ZERO:
      from->sldr->NMOVES = ONE;
      break;
    case ONE:
      from->sldr->NMOVES = MORE_THAN_ONE;
      break;
    default:
      break;
    }

    from->occupied = false;
    // if he going to kill enemy
    if (next->occupied) {
      next->sldr->State = DEAD;
      next->sldr->TEAM->count--;
    }
    next->sldr = from->sldr;
    next->occupied = true;
    from->sldr = NULL;
    changeActive();
  }
}

bool isAvailable(Square *sq) {
  for (int i = 0; i < available->count; i++) {
    if (sq == available->stack[i]) {
      return true;
    }
  }
  return false;
}

// I think it's a bad practice but it's cool
void changeActive() { ACTIVE = !ACTIVE; }

void drawBoard() {
  // draw gboard squares and soldiers
  printf("   \033[;31m%s  soldiers count : %d\033[;0m\n", "BLACK",
         gboard->sets[0]->count);
  for (int i = 0; i < 8; i++) {
    printf("%d  ", 8 - i);
    for (int j = 0; j < 8; j++) {
      displaySq(&gboard->Squares[i][j]);
    }
    printf("\n");
  }

  printf(" ");
  for (int i = 0; i < 8; i++) {
    printf(" %2c ", gboard->Squares[0][i].pos.col);
  }
  printf("\n");
  printf("   WHITE  soldiers count : %d\n", gboard->sets[1]->count);
}

void displaySq(Square *sq) {
  if (sq->occupied) {
    // printf("%s\n", sq->sldr->shap);
    if (sq->sldr->TEAM->color == BLACK) {
      printf("\033[;31m%s\033[;0m  ", sq->sldr->shap);
    } else
      printf("%s  ", sq->sldr->shap);
  } else
    printf("%s   ", sq->color);
}

void displaySqPosition(Square *sq) {
  printf("%c%d ", sq->pos.col, sq->pos.row);
}

void displayNextSqsList() {
  for (int i = 0; i < available->top; i++) {
    displaySqPosition(available->stack[i]);
  }
  printf("\n");
}

availableSqs *dlist(unsigned int size) {
  availableSqs *temp = malloc(sizeof(availableSqs));
  temp->top = 0;
  temp->count = size;
  temp->stack = malloc(temp->count * sizeof(Square *));
  return temp;
}

void push(availableSqs *st, Square *sq) {

  if (st->top == st->count) {
    stackResize(st, 5);
  } else {
    st->stack[st->top] = sq;
    st->top += 1;
  }
}

void stackResize(availableSqs *st, int size) {
  if (st) {
    st->count += size;
    Square **nst = realloc(st->stack, sizeof(Square *) * st->count);
    if (nst)
      st->stack = nst;
  }
}

void destroy() {
  for (int i = 0; i < 16; i++) {
    free(gboard->sets[0]->soldiers[i].shap);
    free(gboard->sets[1]->soldiers[i].shap);
  }
  free(gboard->sets[0]->soldiers);
  free(gboard->sets[1]->soldiers);

  for (int i = 0; i < 2; i++)
    free(gboard->sets[i]);

  destroyAvList(available);

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      free(gboard->Squares[i][j].color);
    }
  }

  free(gboard);
}

void destroyAvList(availableSqs *st) {
  if (st != NULL) {
    free(st->stack);
    free(st);
    st = NULL;
  }
}

void erroredEnd() {
  destroy();
  exit(1);
}

availableSqs *mergeList(availableSqs *first, availableSqs *second) {
  availableSqs *nextSqs;
  nextSqs = first;
  for (int i = 0; i < second->top; i++) {
    push(nextSqs, second->stack[i]);
  }
  destroyAvList(second);
  return nextSqs;
}
