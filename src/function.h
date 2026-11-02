#ifndef __FUNCTION_H_
#define __FUNCTION_H_
#include <zf_common_headfile.h>



uint8 Limit_uint8( uint8 a, uint8 b, uint8 c );
int Limit_int( int a, int b, int c );
uint8 find_extreme_value(uint8 *array_value, uint8 num0, uint8 num1, uint8 model);
void connect_point(uint8 *array_value, uint8 num0, uint8 num1);
uint8 find_extreme_index(uint8 *array_value, uint8 num0, uint8 num1, uint8 model);

#endif