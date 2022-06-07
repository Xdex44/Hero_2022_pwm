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

#include "dbus.h"
#include "chassis_task.h"
#include "timer_task.h"
#include "hero_cmd.h"
#include "ahrs.h"
#include "can.h"

static float vx, vy, wz;
static uint8_t shift;

float follow_relative_angle;
struct pid pid_follow = {0}; //angle control
static void chassis_imu_update(void *argc);

void chassis_task(void const *argument)
{
  uint32_t period = osKernelSysTick();
  chassis_t pchassis = NULL;
  rc_device_t prc_dev = NULL;
  rc_info_t prc_info = NULL;
  pchassis = chassis_find("chassis");
  prc_dev = rc_device_find("uart_rc");

  if (prc_dev != NULL)
  {
    prc_info = rc_device_get_info(prc_dev);
  }
  else
  {
  }
	shift = 1;
  soft_timer_register(chassis_push_info, (void *)pchassis, 10);

  pid_struct_init(&pid_follow, MAX_CHASSIS_VW_SPEED, 50, 8.0f, 0.0f, 2.0f);//PID参数调节，更改后三位输入数据
	
	HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
	
	
  while (1)
  {	
		if (prc_info->kb.key_code==0 && prc_info->mouse.x == 0){
			chassis_set_speed(pchassis, 0, 0, 0);
			chassis_set_acc(pchassis, 0, 0, 0);
			HAL_GPIO_WritePin(LED_GPIO_Port,LD1_Pin|LD2_Pin|LD3_Pin|LD4_Pin,GPIO_PIN_SET);
			vx = 0;
			vy = 0;
			wz = 0;
		}
		else {		
			if ((prc_info->kb.key_code & KEY_SHIFT) == KEY_SHIFT) {
				shift++;
				if (shift == 4) {
					shift = 1;
				}
			}
			else {
				HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
			}
			
			
			if ((prc_info->kb.key_code & KEY_W) == KEY_W){
				HAL_GPIO_WritePin(LED_GPIO_Port,LD1_Pin,GPIO_PIN_RESET);
				vx = (float) MAX_CHASSIS_VX_SPEED / MAX_GEAR * shift;
			}
			else if ((prc_info->kb.key_code & KEY_S) == KEY_S){
				HAL_GPIO_WritePin(LED_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);
				vx = -(float) MAX_CHASSIS_VX_SPEED / MAX_GEAR * shift;
			}
			
			if ((prc_info->kb.key_code & KEY_A) == KEY_A){
				HAL_GPIO_WritePin(LED_GPIO_Port,LD3_Pin,GPIO_PIN_RESET);
				vy = (float) MAX_CHASSIS_VY_SPEED / MAX_GEAR * shift;
			}
			else if ((prc_info->kb.key_code & KEY_D) == KEY_D){
				HAL_GPIO_WritePin(LED_GPIO_Port,LD4_Pin,GPIO_PIN_RESET);
				vy = -(float) MAX_CHASSIS_VY_SPEED / MAX_GEAR * shift;
			}
			
			if (prc_info->mouse.r == 1) {
				wz = 0;
			}
			else {
				wz = -(float) prc_info->mouse.x / 500 * MAX_CHASSIS_VW_SPEED;
				wz =  (float) wz / 3 * shift;
			}
			
			chassis_set_offset(pchassis, ROTATE_X_OFFSET, ROTATE_Y_OFFSET);
			chassis_set_speed(pchassis, vx, vy, wz);
			chassis_set_acc(pchassis, 0, 0, 0);
		}	

		chassis_imu_update(pchassis);
		if (chassis_execute(pchassis)==RM_OK){
			HAL_GPIO_WritePin(LED_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
		}; //Central "execute" command to tell Chasis to run
		osDelayUntil(&period, 2);
  }
}

#define RAD_TO_DEG 57.3f
static void chassis_imu_update(void *argc)
{
  struct ahrs_sensor mpu_sensor;
  struct attitude mahony_atti;
  chassis_t pchassis = (chassis_t)argc;
  mpu_get_data(&mpu_sensor);
  mahony_ahrs_updateIMU(&mpu_sensor, &mahony_atti);
  // TODO: adapt coordinates to our own design
  chassis_gyro_update(pchassis, -mahony_atti.yaw, mpu_sensor.wz * RAD_TO_DEG);
  // TODO: adapt coordinates to our own design
  //return 0;
}

int32_t chassis_set_relative_angle(float angle)
{
  follow_relative_angle = angle;
  return 0;
}
