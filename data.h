#include "./chess.h"
#include <unistd.h>

void load(Data *gData);
void readSq(FILE *, Square *);
void readSldr(FILE *, Soldier *);
void readSet(FILE *, Set_t *, Data *);

void save(Data *gData);
void writeSet(FILE *, Set_t *);
void writeSldr(FILE *, Soldier *);
void writeSq(FILE *, Square *);
