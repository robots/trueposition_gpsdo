#ifndef EXTI_h_
#define EXTI_h_

#include "platform.h"

typedef void(*exti_fnc_t)(void);

#ifndef EXTI_Trigger_Rising
typedef enum
{
  EXTI_Trigger_Rising = 0x08,
  EXTI_Trigger_Falling = 0x0C,  
  EXTI_Trigger_Rising_Falling = 0x10
} EXTITrigger_TypeDef;
#endif

#ifndef EXTI_PortSourceGPIOA
enum {
	EXTI_PortSourceGPIOA=0,
	EXTI_PortSourceGPIOB,
	EXTI_PortSourceGPIOC,
	EXTI_PortSourceGPIOD,
	EXTI_PortSourceGPIOE,
	EXTI_PortSourceGPIOF,
	EXTI_PortSourceGPIOG,
	EXTI_PortSourceGPIOH,
	EXTI_PortSourceGPIOI,
	EXTI_PortSourceGPIOJ,
	EXTI_PortSourceGPIOK,
};
#endif

enum exti_interrupt_t {
	EXTI_0,
	EXTI_1,
	EXTI_2,
	EXTI_3,
	EXTI_4,
	EXTI_5,
	EXTI_6,
	EXTI_7,
	EXTI_8,
	EXTI_9,
	EXTI_10,
	EXTI_11,
	EXTI_12,
	EXTI_13,
	EXTI_14,
	EXTI_15,
	EXTI_16,
	EXTI_17,
	EXTI_18,
	EXTI_19,
};

void exti_set_handler(uint8_t exti, exti_fnc_t fnc);
void exti_enable(uint8_t exti, EXTITrigger_TypeDef trig, uint8_t gpio);
void exti_reenable(uint8_t exti);
void exti_disable(uint8_t exti);
void exti_trigger(uint8_t exti);
int exti_status(uint8_t exti);


#endif
