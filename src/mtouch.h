#ifndef MTOUCH_H
#define MTOUCH_H

#include "capabilities.h"
#include "iobuffer.h"
#include "hwdata.h"
#include "state.h"

////////////////////////////////////////////////////////

struct MTouch {
	struct Capabilities caps;
	struct IOBuffer buf;
	struct HWData hw;
	struct State os, ns;
	bool grabbed;
};

////////////////////////////////////////////////////////

int configure_mtouch(struct MTouch *mt, int fd);
int open_mtouch(struct MTouch *mt, int fd);
void close_mtouch(struct MTouch *mt, int fd);

bool read_synchronized_event(struct MTouch *mt, int fd);

////////////////////////////////////////////////////////

#endif
