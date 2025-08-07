#include <stdint.h>

#ifndef __MessageFormat_H
#define __MessageFormat_H

#define MESSAGE_FORMAT_MAX_DATA_LEN 1024

typedef struct
{
	uint16_t length;
	char data[MESSAGE_FORMAT_MAX_DATA_LEN];
}MessageFormat;

#endif // __MessageFormat_H