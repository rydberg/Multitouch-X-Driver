#ifndef MTEVENT_H
#define MTEVENT_H

#include "capabilities.h"
#include "hwdata.h"

////////////////////////////////////////////////////////

struct FingerState {
	struct FingerData hw;
	int id;
};

////////////////////////////////////////////////////////

struct State {
	struct FingerState finger[DIM_FINGER];
	button_t button;
	int nfinger;
	int nextid;
};

////////////////////////////////////////////////////////

void init_state(struct State *s);
void modify_state(struct State *s,
		  const struct HWData* hw,
		  const struct Capabilities* caps);
void output_state(const struct State *s);

const struct FingerState *find_finger(const struct State *s, int id);

////////////////////////////////////////////////////////

#endif
