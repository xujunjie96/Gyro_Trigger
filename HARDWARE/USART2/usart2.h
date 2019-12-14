#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h" 

struct SGyro
{
	short w[3];
	short T;
};
struct SAngle
{
	short Angle[3];
	short T;
};

void My_USART2_Init(u32 bound);
void CopeSerial2Data(unsigned char ucData);
#endif