

#ifndef __brushed_commh__
#define __brushed_commh__

#include "dsp56f807.h"

typedef struct sDutyControl_tag
{
	byte    Dir;	
	Int16   Duty;
} sDutyControlBL;


// the PWM limit are MIN_DUTY= 2%   and MAX_DUTY=98%
#define MIN_DUTY 58  // 2% of  800 +2*deadtime
#define DEAD_TIME 0x16
#define MAX_DUTY 756// 98% of 1333
#define  STEP 6// step every cycle  
#define  PWMFREQ 800 // 200 = 200KHz  800= 50KHz 1333= 30KHz



void TD0_interrupt(void);

void TD0_Enable(void);


void TD0_Disable(void);

void Init_Brushed_Comm(void);
void TD0_init (void);
void PWM_generate_DC(byte i, Int16 pwm_value, byte dir);

#endif 