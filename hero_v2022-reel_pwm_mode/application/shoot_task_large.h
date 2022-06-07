#ifndef __SHOOT_TASK_LARGE_H__
#define __SHOOT_TASK_LARGE_H__
#include "tim.h"
#include "stm32f4xx_hal.h"

//Defined values
//NOTE: IF CHANNEL AND TIMER IS CHANGED, MAKE SURE TO CHANGE SETTINGS IN tim.c (see MAIN.c), and RUN PROPER INITILIZATION
//FUNCTION FROM MAIN.C
//Things to change when channel/timer is changed:
//Timer name and channel name function in main.c, create new one MX_TIMX_Init if necessary
//Make sure to set timer to the proper frequency, the source frequency is 90MHz or 180 MHz, not sure which one, adjust scaling factor 
//Timer clock activation in tim.c, function HAL_TIM_PWM_MspInit, make sure to add the corresponding timer to the elseif chain
//Make sure to call HAL_TIM_PWM_Init with the correct variable (&htimx) in the timer initialziation function
//Make sure to initialize the channels by using the sConfigOC structure, see other timer initialization for template
//Make sure to set correct output pin that corresponds to the channel (can be found in circuit document), set the pin by adding to the elseif chain in HAL_TIM_MspPostInit function, see TIM1/TIM4 for template

//Friction wheel channels
//Right now, the left wheel is connected to PWM pin E, right wheel is connected to PWM pin F
#define FRIC_LARGE_L_CHANNEL TIM_CHANNEL_1
#define FRIC_LARGE_R_CHANNEL TIM_CHANNEL_4
#define REEL_CHANNEL TIM_CHANNEL_3
//Friction wheel timers
#define FRIC_LARGE_L_TIMER &htim1
#define FRIC_LARGE_R_TIMER &htim1
#define REEL_TIMER &htim1

#define STOP_SPEED 1450
#define FRICTION_SPEED 1100
#define REEL_SPEED 1400

//Function Prototypes
void shoot_task_large(void const *argument);
void set_large_fric_speed(double input_speed);
void init_large_frics();
	

	#endif // __SHOOT_TASK_LARGE_H__