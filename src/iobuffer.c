#include "iobuffer.h"

////////////////////////////////////////////////////////

void init_iobuf(struct IOBuffer *buf)
{
	memset(buf, 0, sizeof(struct IOBuffer));
	buf->at = buf->begin;
	buf->top = buf->at;
	buf->end = buf->begin + DIM_BUFFER;
}

////////////////////////////////////////////////////////

const struct input_event* get_iobuf_event(struct IOBuffer *buf, int fd)
{
	const struct input_event *ev;
	int n = buf->top - buf->at;
	if (n < EVENT_SIZE) {
		/* partial event is available: save it */
		if (buf->at != buf->begin && n > 0)
			memmove(buf->begin, buf->at, n);
		/* start from the beginning */
		buf->at = buf->begin;
		buf->top = buf->at + n;
		/* read more data */
		SYSCALL(n = read(fd, buf->top, buf->end - buf->top));
		if (n <= 0)
			return NULL;
		buf->top += n;
	}
	if (buf->top - buf->at < EVENT_SIZE)
		return NULL;
	ev = (const struct input_event *)buf->at;
	buf->at += EVENT_SIZE;
	return ev;
}

////////////////////////////////////////////////////////
