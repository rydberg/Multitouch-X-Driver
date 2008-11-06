#include "state.h"

/******************************************************/

void init_state(struct State *s)
{
	memset(s, 0, sizeof(struct State));
}

/******************************************************/

void modify_state(struct State *s, const struct HWData* hw)
{
	int i;
	if (s->button[0] != hw->button[0])
		xf86Msg(X_INFO, "multitouch: button changed\n");
	for (i = 0; i < DIM_BUTTON; i++)
		s->button[i] = hw->button[i];
}

/******************************************************/

const struct FingerState *find_finger(const struct State *s, int id)
{
	int i;
	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].id == id)
			return s->finger+i;
	return NULL;
}

/******************************************************/

void output_state(const struct State *s)
{
}

/******************************************************/
