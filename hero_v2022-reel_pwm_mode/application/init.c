/****************************************************************************
 *  Copyright (C) 2019 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/
//头文件声明 Header files
#include "main.h"
#include "can.h"
#include "board.h"
#include "motor.h"
#include "dbus.h"
#include "detect.h"
#include "test.h"
#include "chassis.h"
#include "gimbal.h"
#include "shoot.h"

#include "chassis_task.h"
#include "gimbal_task.h"
#include "timer_task.h"
#include "shoot_task.h"
#include "shoot_task_large.h"
#include "communicate.h"
#include "hero_cmd.h"
#include "init.h"

#include "protocol.h"
#include "ulog.h"
#include "param.h"
#include "offline_check.h"
#include "referee_system.h"
//struct declarance
struct chassis chassis;
struct gimbal gimbal;
struct shoot shoot;
static struct rc_device rc_dev; //Remote Control structure 

static uint8_t glb_sys_cfg; //全局系统变量确认

extern int ulog_console_backend_init(void);

void system_config(void)
{
  glb_sys_cfg = HAL_GPIO_ReadPin(SYS_CFG_Port, SYS_CFG_Pin); //获取PIN6的参数值，0或1
}

uint8_t get_sys_cfg(void)//get status of the whole system
{
  return glb_sys_cfg;
}

void hw_init(void)
{
	//Intializing process
  cali_param_init(); //Parameter caculation system initialization
  board_config();
  test_init(); // Test system initialization. However this function is void.
  system_config();
  ulog_init();
  ulog_console_backend_init();
  
  referee_param_init(); // referee system initialization
  usart3_rx_callback_register(referee_uart_rx_data_handle); //serial communication initialization
  referee_send_data_register(usart3_transmit);

  if(glb_sys_cfg == CHASSIS_APP) // CHASSIS_APP = 0. This means enter chassis system initialization status
  {
    rc_device_register(&rc_dev, "uart_rc", 0);
    dr16_forword_callback_register(rc_data_forword_by_can);
    chassis_pid_register(&chassis, "chassis", DEVICE_CAN1);
    chassis_disable(&chassis);
		
		HAL_GPIO_TogglePin(LED_GPIO_Port, LD7_Pin);
  }
  else // GIMBAL_APP = 1, enter gimbal system initialization status
  {
    rc_device_register(&rc_dev, "can_rc", 0);
    gimbal_cascade_register(&gimbal, "gimbal", DEVICE_CAN1);

    shoot_pid_register(&shoot, "shoot", DEVICE_CAN1);

    gimbal_yaw_disable(&gimbal);
    gimbal_pitch_disable(&gimbal);
    shoot_disable(&shoot);
  }

  offline_init();
}

osThreadId timer_task_t;
osThreadId chassis_task_t;
osThreadId gimbal_task_t;
osThreadId communicate_task_t;
osThreadId cmd_task_t;
osThreadId shoot_task_t;
osThreadId shoot_task_large_t;

void task_init(void)
{
  uint8_t app;
  app = get_sys_cfg(); // Read current system status

  osThreadDef(TIMER_1MS, timer_task, osPriorityHigh, 0, 512);
  timer_task_t = osThreadCreate(osThread(TIMER_1MS), NULL);

  osThreadDef(COMMUNICATE_TASK, communicate_task, osPriorityHigh, 0, 4096);
  communicate_task_t = osThreadCreate(osThread(COMMUNICATE_TASK), NULL);

  osThreadDef(CMD_TASK, hero_cmd_task, osPriorityNormal, 0, 4096);
  cmd_task_t = osThreadCreate(osThread(CMD_TASK), NULL);
	
  
  if (app == CHASSIS_APP) //Judge which processes are handling at present
  {
    osThreadDef(CHASSIS_TASK, chassis_task, osPriorityRealtime, 0, 512);
    chassis_task_t = osThreadCreate(osThread(CHASSIS_TASK), NULL);
		
		HAL_GPIO_TogglePin(LED_GPIO_Port, LD8_Pin);
  }
	 else
	 {
		 osThreadDef(GIMBAL_TASK, gimbal_task, osPriorityRealtime, 0, 512);
		 gimbal_task_t = osThreadCreate(osThread(GIMBAL_TASK), NULL);

		 //osThreadDef(SHOOT_TASK, shoot_task, osPriorityAboveNormal, 0, 512);
		 //shoot_task_t = osThreadCreate(osThread(SHOOT_TASK), NULL);
		 
	osThreadDef(SHOOT_TASK_LARGE, shoot_task_large, osPriorityRealtime, 0,4096);
	shoot_task_large_t=osThreadCreate(osThread(SHOOT_TASK_LARGE),NULL);
	 }
}
