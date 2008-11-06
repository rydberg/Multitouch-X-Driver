#ifndef IOBUFFER_H
#define IOBUFFER_H

#include "common.h"

#define EVENT_SIZE sizeof(struct input_event)
#define DIM_EVENTS 64
#define DIM_BUFFER (DIM_EVENTS * EVENT_SIZE)

////////////////////////////////////////////////////////

struct IOBuffer {
	char begin[DIM_BUFFER], *at, *top, *end;
};

////////////////////////////////////////////////////////

void init_iobuf(struct IOBuffer *buf);
const struct input_event* get_iobuf_event(struct IOBuffer *buf, int fd);

////////////////////////////////////////////////////////

#endif
