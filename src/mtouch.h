#ifndef MTOUCH_H
#define MTOUCH_H

#include "capabilities.h"
#include "hwdata.h"

////////////////////////////////////////////////////////

struct MTouch {
	struct Capabilities caps;
	struct HWData hw;
	bool grabbed;
};

////////////////////////////////////////////////////////

int configure_mtouch(struct MTouch *mt, int fd);
int open_mtouch(struct MTouch *mt, int fd);
void close_mtouch(struct MTouch *mt, int fd);

////////////////////////////////////////////////////////

#endif
