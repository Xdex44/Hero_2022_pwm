/****************************************************************************
 *  Copyright (C) 2019 RoboMaster & UARM - EE Team.
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

#include "board.h"
#include "dbus.h"
#include "chassis.h"
#include "gimbal.h"
#include "shoot.h"

#include "init.h"
#include "chassis_task.h"
#include "gimbal_task.h"

#include "protocol.h"
#include "referee_system.h"
#include "hero_cmd.h"

#include "stm32f4xx_hal.h"

#define MANIFOLD2_CHASSIS_SIGNAL (1 << 0)
#define MANIFOLD2_GIMBAL_SIGNAL (1 << 1)
#define MANIFOLD2_SHOOT_SIGNAL (1 << 2)
#define MANIFOLD2_FRICTION_SIGNAL (1 << 3)
#define MANIFOLD2_CHASSIS_ACC_SIGNAL (1 << 4)

extern osThreadId cmd_task_t;

struct cmd_gimbal_info cmd_gimbal_info;
struct cmd_chassis_info cmd_chassis_info;
struct manifold_cmd manifold_cmd;

struct manifold_cmd *get_manifold_cmd(void)
{
  return &manifold_cmd;
}

int32_t gimbal_info_rcv(uint8_t *buff, uint16_t len); //云台信息接收函数 Gimbal information receiving function
int32_t chassis_speed_ctrl(uint8_t *buff, uint16_t len);//底盘速度控制函数
int32_t chassis_spd_acc_ctrl(uint8_t *buff, uint16_t len);//底盘加速控制函数	
int32_t shoot_firction_ctrl(uint8_t *buff, uint16_t len);//射击摩擦力函数
int32_t gimbal_angle_ctrl(uint8_t *buff, uint16_t len);//云台角度控制函数
int32_t shoot_ctrl(uint8_t *buff, uint16_t len);//射击控制函数
int32_t student_data_transmit(uint8_t *buff, uint16_t len);//额目前看来应该是裁判的事儿 might not be our business

int32_t rc_data_forword_by_can(uint8_t *buff, uint16_t len)
{
  protocol_send(GIMBAL_ADDRESS, CMD_RC_DATA_FORWORD, buff, len);//协议发送正常帧 Successfully send protocol content mark
  return 0;
}

int32_t gimbal_adjust_cmd(uint8_t *buff, uint16_t len)//云台自动调整命令，将调整位置1 
																											//Gimbal automatic adjustment command, change the adjustment bit to 1
{
  gimbal_auto_adjust_start();
  return 0;
}

void hero_cmd_task(void const *argument)
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
		//HAL_GPIO_TogglePin(LED_GPIO_Port, LD5_Pin);
  }
  else
  {
			//HAL_GPIO_TogglePin(LED_GPIO_Port, LD8_Pin);
  }

  //soft_timer_register(chassis_push_info, (void *)pchassis, 10);
//  HAL_GPIO_TogglePin(LED_GPIO_Port, LD2_Pin);
//	osDelayUntil(&period, 100);
	
  while(1){
//    if  ((prc_info->kb.key_code & 0x0001) == 0x0001) {
//      HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
//			osDelayUntil(&period, 100);
//    }
		osDelayUntil(&period, 2);
  }

}

/*以下函数均为对上述主任务中出现的函数的描述*/
int32_t student_data_transmit(uint8_t *buff, uint16_t len)
{
  uint16_t cmd_id = *(uint16_t *)buff;
  referee_protocol_tansmit(cmd_id, buff + 2, len - 2);
  return 0;
}

int32_t chassis_speed_ctrl(uint8_t *buff, uint16_t len)//buff的值为遥控传输
{
  if (len == sizeof(struct cmd_chassis_speed))
  {
    memcpy(&manifold_cmd.chassis_speed, buff, len);//将buff中的数据转给&manifold_cmd.chassis_speed
    osSignalSet(cmd_task_t, MANIFOLD2_CHASSIS_SIGNAL);
  }
  return 0;
}

int32_t chassis_spd_acc_ctrl(uint8_t *buff, uint16_t len)
{
  if (len == sizeof(struct cmd_chassis_spd_acc))
  {
    memcpy(&manifold_cmd.chassis_spd_acc, buff, len);
    osSignalSet(cmd_task_t, MANIFOLD2_CHASSIS_ACC_SIGNAL);
  }
  return 0;
}

int32_t gimbal_angle_ctrl(uint8_t *buff, uint16_t len)
{
  if (len == sizeof(struct cmd_gimbal_angle))
  {
    memcpy(&manifold_cmd.gimbal_angle, buff, len);
    osSignalSet(cmd_task_t, MANIFOLD2_GIMBAL_SIGNAL);
  }
  return 0;
}

int32_t shoot_firction_ctrl(uint8_t *buff, uint16_t len)
{
  if (len == sizeof(struct cmd_firction_speed))
  {
    memcpy(&manifold_cmd.firction_speed, buff, len);
    osSignalSet(cmd_task_t, MANIFOLD2_FRICTION_SIGNAL);
  }
  return 0;
}

int32_t shoot_ctrl(uint8_t *buff, uint16_t len)
{
  if (len == sizeof(struct cmd_shoot_num))//
  {
    memcpy(&manifold_cmd.shoot_num, buff, len);
    osSignalSet(cmd_task_t, MANIFOLD2_SHOOT_SIGNAL);
  }
  return 0;
}

int32_t gimbal_info_rcv(uint8_t *buff, uint16_t len)
{
  struct cmd_gimbal_info *info;
  info = (struct cmd_gimbal_info *)buff;
  chassis_set_relative_angle(info->yaw_ecd_angle / 10.0f);
  return 0;
}

int32_t gimbal_push_info(void *argc)
{
  struct gimbal_info info;
  gimbal_t pgimbal = (gimbal_t)argc;
  gimbal_get_info(pgimbal, &info);

  cmd_gimbal_info.mode = info.mode;
  cmd_gimbal_info.pitch_ecd_angle = info.pitch_ecd_angle * 10;
  cmd_gimbal_info.pitch_gyro_angle = info.pitch_gyro_angle * 10;
  cmd_gimbal_info.pitch_rate = info.pitch_rate * 10;
  cmd_gimbal_info.yaw_ecd_angle = info.yaw_ecd_angle * 10;
  cmd_gimbal_info.yaw_gyro_angle = info.yaw_gyro_angle * 10;
  cmd_gimbal_info.yaw_rate = info.yaw_rate * 10;

  if (get_gimbal_init_state() == 0)
  {
    cmd_gimbal_info.yaw_ecd_angle = 0;
  }

  protocol_send(PROTOCOL_BROADCAST_ADDR, CMD_PUSH_GIMBAL_INFO, &cmd_gimbal_info, sizeof(cmd_gimbal_info));

  return 0;
}

int32_t chassis_push_info(void *argc)
{
  struct chassis_info info;
  chassis_t pchassis = (chassis_t)argc;
  chassis_get_info(pchassis, &info);

  cmd_chassis_info.angle_deg = info.angle_deg * 10;
  cmd_chassis_info.gyro_angle = info.yaw_gyro_angle * 10;
  cmd_chassis_info.gyro_palstance = info.yaw_gyro_rate * 10;
  cmd_chassis_info.position_x_mm = info.position_x_mm;
  cmd_chassis_info.position_y_mm = info.position_y_mm;
  cmd_chassis_info.v_x_mm = info.v_x_mm;
  cmd_chassis_info.v_y_mm = info.v_y_mm;

  protocol_send(MANIFOLD2_ADDRESS, CMD_PUSH_CHASSIS_INFO, &cmd_chassis_info, sizeof(cmd_chassis_info));

  return 0;
}
