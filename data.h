#include "./chess.h"
#include <unistd.h>

void load(globalData *gData);
void readSq(FILE *, Square *);
void readSldr(FILE *, Soldier *);
void readSet(FILE *, Set_t *, globalData *);

void save(globalData *gData);
void writeSet(FILE *, Set_t *);
void writeSldr(FILE *, Soldier *);
void writeSq(FILE *, Square *);
