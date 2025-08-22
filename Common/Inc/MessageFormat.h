#include <stdint.h>
using namespace std;
#ifndef __MessageFormat_H
#define __MessageFormat_H

#define MESSAGE_FORMAT_MAX_DATA_LEN 1024

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

PACK(typedef struct
{	
	char h1;
	uint16_t length;
    uint32_t checksum;
	char data[MESSAGE_FORMAT_MAX_DATA_LEN];
}MessageFormat);

/**
*checksum calculater for error detection
* @param data string data to calculate checksum
* @return 1 byte checksum value
*/
uint32_t calculateChecksum(const string& data); 
#endif // __MessageFormat_H