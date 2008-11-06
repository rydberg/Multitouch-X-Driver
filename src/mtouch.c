#include "mtouch.h"
#include <errno.h>

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

int init_mtouch(struct MTouch *mt)
{
	return 0;
}

/******************************************************/
