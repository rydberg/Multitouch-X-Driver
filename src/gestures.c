#include "gestures.h"

/******************************************************/

void extract_gestures(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *b = mt->ns.finger;
	const struct FingerState *e = b + mt->ns.nfinger;
	const struct FingerState *p, *fs;
	int nof = count_fingers(&mt->os);
	int nsf = count_fingers(&mt->ns);
	int dn = 0, i;
	memset(gs, 0, sizeof(struct Gestures));
	if (nof == nsf) {
		for (p = b; p != e; p++) {
			if (fs = find_finger(&mt->os, p->id)) {
				gs->dx += p->hw.position_x - fs->hw.position_x;
				gs->dy += p->hw.position_y - fs->hw.position_y;
				dn++;
			}
		}
	}
	if (gs->dx || gs->dy) {
		gs->dx /= dn;
		gs->dy /= dn;
		if (nsf == 1)
			SETBIT(gs->type, GS_MOVE);
		if (nsf == 2)
			SETBIT(gs->type, GS_VSCROLL);
		if (nsf == 3)
			SETBIT(gs->type, GS_HSCROLL);
	}
	if (mt->ns.button == (1U << MT_BUTTON_LEFT)) {
		if (nsf == 2)
			mt->ns.button = (1U << MT_BUTTON_RIGHT);
		if (nsf == 3)
			mt->ns.button = (1U << MT_BUTTON_MIDDLE);
	}
	for (i = 0; i < DIM_BUTTON; i++) {
		if (GETBIT(mt->ns.button, i) != GETBIT(mt->os.button, i)) {
			SETBIT(gs->type, GS_BUTTON);
			gs->btix[gs->nbt] = i + 1;
			gs->btval[gs->nbt] = GETBIT(mt->ns.button, i);
			gs->nbt++;
		}
	}
	mt->os = mt->ns;
}

