#include "./chess.h"
#include <unistd.h>

void load(Context *context);
void readSq(FILE *, Square *);
void readSldr(FILE *, Soldier *);
void readSet(FILE *, Set_t *, Context *);

void save(Context *context);
void writeSet(FILE *, Set_t *);
void writeSldr(FILE *, Soldier *);
void writeSq(FILE *, Square *);
