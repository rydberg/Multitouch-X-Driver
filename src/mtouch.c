#include "mtouch.h"

/******************************************************/

int configure_mtouch(struct MTouch *mt, int fd)
{
	int rc = read_capabilities(&mt->caps, fd);
	if (rc < 0)
		return rc;
	output_capabilities(&mt->caps);
	return 0;
}

/******************************************************/

int open_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	init_iobuf(&mt->buf);
	init_hwdata(&mt->hw);
	init_state(&mt->os);
	init_state(&mt->ns);
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)1));
	return rc;
}

/******************************************************/

int close_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	return rc;
}

/******************************************************/

bool read_synchronized_event(struct MTouch *mt, int fd)
{
	const struct input_event* ev;
	while(ev = get_iobuf_event(&mt->buf, fd))
		if (read_hwdata(&mt->hw, ev))
		    return 1;
	return 0;
}

/******************************************************/

void parse_event(struct MTouch *mt)
{
	modify_state(&mt->ns, &mt->hw, &mt->caps);
}

/******************************************************/
