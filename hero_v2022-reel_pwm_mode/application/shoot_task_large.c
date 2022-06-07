#include "board.h"
#include "dbus.h"
#include "chassis.h"
#include "gimbal.h"
#include "shoot.h"
#include "sys.h"

#include "init.h"
#include "chassis_task.h"
#include "gimbal_task.h"

#include "protocol.h"
#include "referee_system.h"
#include "hero_cmd.h"
#include "shoot_task_large.h"
//Timer necessary
#include "tim.h"
#include "stm32f4xx_hal.h"


void set_large_fric_speed(double input_speed){
	//Input between 0-100, 0 means 0% speed, 100% means 100% speed
	//Convert input percentage to number between 1000-2000, because the motor can only take 1000us-2000us pulse widths
	//Set speed for left wheel
	__HAL_TIM_SET_COMPARE(FRIC_LARGE_L_TIMER,FRIC_LARGE_L_CHANNEL,input_speed);
	__HAL_TIM_SET_COMPARE(FRIC_LARGE_R_TIMER,FRIC_LARGE_R_CHANNEL,input_speed);
	osDelay(1);
	//Set speed for right wheel
}

void init_large_frics(){
	  uint32_t period = osKernelSysTick();
		//pwm init set the maximum pulse width and the minimum speed val 1450
	  //HAL_TIM_PWM_Start(FRIC_LARGE_L_TIMER,FRIC_LARGE_L_CHANNEL);
	  //HAL_TIM_PWM_Start(FRIC_LARGE_R_TIMER,FRIC_LARGE_R_CHANNEL);
	
		__HAL_TIM_SET_COMPARE(FRIC_LARGE_L_TIMER,FRIC_LARGE_L_CHANNEL,STOP_SPEED);
		__HAL_TIM_SET_COMPARE(FRIC_LARGE_R_TIMER,FRIC_LARGE_R_CHANNEL,STOP_SPEED);
		__HAL_TIM_SET_COMPARE(REEL_TIMER,REEL_CHANNEL,STOP_SPEED);
		osDelay(2000);
}


void shoot_task_large(void const *argument){
	///System variables
	 uint32_t period = osKernelSysTick();
	// controller parameter
  rc_device_t prc_dev = NULL;
  rc_info_t prc_info = NULL;
  prc_dev = rc_device_find("can_rc");
	uint16_t flag = 1450;

  if (prc_dev != NULL)
  {
    prc_info = rc_device_get_info(prc_dev);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LD3_Pin);
  }
  else
  {
  }
	//Initialize the PWM ports (start PWM and give initialization signal)
	 init_large_frics();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(prc_info->mouse.l==1) {
			HAL_GPIO_WritePin(LED_GPIO_Port, LD4_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_GPIO_Port, LD5_Pin,GPIO_PIN_SET);
			__HAL_TIM_SET_COMPARE(FRIC_LARGE_L_TIMER,FRIC_LARGE_L_CHANNEL,FRICTION_SPEED);
  		__HAL_TIM_SET_COMPARE(FRIC_LARGE_R_TIMER,FRIC_LARGE_R_CHANNEL,FRICTION_SPEED);
			__HAL_TIM_SET_COMPARE(REEL_TIMER,REEL_CHANNEL,REEL_SPEED);
		}
		else {
			HAL_GPIO_WritePin(LED_GPIO_Port, LD5_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_GPIO_Port, LD4_Pin,GPIO_PIN_SET);			
			__HAL_TIM_SET_COMPARE(FRIC_LARGE_L_TIMER,FRIC_LARGE_L_CHANNEL,STOP_SPEED);
  		__HAL_TIM_SET_COMPARE(FRIC_LARGE_R_TIMER,FRIC_LARGE_R_CHANNEL,STOP_SPEED);
			__HAL_TIM_SET_COMPARE(REEL_TIMER,REEL_CHANNEL,STOP_SPEED);
		}
		osDelay(1);
	}
}

// the function is not used
void shoot_task_large_can(void const *argument){
	///System variables
	 uint32_t period = osKernelSysTick();
	 
	//Initialize the PWM ports (start PWM and give initialization signal)
	 init_large_frics();

	
	//Finish initilization of ports
	  //double speed=0;
    while(1){
			//If condition is okay, then shoot large task should set the friction wheels at the same input speed, will only worry about left wheel for now
			set_large_fric_speed(5);
			osDelayUntil(&period,5);
    }
}
