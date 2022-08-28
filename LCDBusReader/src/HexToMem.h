#ifndef __HEX_TO_MEM_H__
#define __HEX_TO_MEM_H__
#include <stdlib.h>
#include <stdint.h>


int read_hex_string_into_memory(uint8_t* pvDest, int iDestLength, char* szSource);


#endif // __HEX_TO_MEM_H__
