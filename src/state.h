#ifndef MTEVENT_H
#define MTEVENT_H

#include "hwdata.h"

////////////////////////////////////////////////////////

struct FingerState {
	struct FingerData hw;
	int id;
};

////////////////////////////////////////////////////////

struct State {
	struct FingerState finger[DIM_FINGER];
	int button[DIM_BUTTON];
	int nfinger, lastid;
};

////////////////////////////////////////////////////////

void init_state(struct State *s);
void modify_state(struct State *s, const struct HWData* hw);
void output_state(const struct State *s);

const struct FingerState *find_finger(const struct State *s, int id);

////////////////////////////////////////////////////////

#endif
