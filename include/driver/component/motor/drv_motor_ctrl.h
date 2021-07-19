/**
  * @file  drv_motor_ctrl.h
  * @author  XRADIO IOT WLAN Team
  */

/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MOTOR_CTRL_H_
#define _MOTOR_CTRL_H_

#include "driver/component/component_def.h"
#include "driver/chip/hal_gpio.h"
#include "driver/chip/hal_def.h"
#include "driver/chip/hal_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  motor_max_speed uint32_t

/**
  * @brief The motor type. The MOTOR_HIGH_LEVEL stand for outup high level
  *    the motor is run.
  */
typedef enum {
	MOTOR_HIGH_LEVEL,
	MOTOR_LOW_LEVEL,
} MOTOR_CTRL_TYPE;

/**
  * @brief Config the pwm channel.
  */
typedef struct {
	uint32_t hz;
	PWM_CH_ID pwm_Ch;
	MOTOR_CTRL_TYPE type;
} Motor_Ctrl;

motor_max_speed DRV_Motor_Ctrl_Init(Motor_Ctrl *param);
void DRV_Motor_Ctrl_DeInit();
void DRV_Motor_Enable();
void DRV_Motor_Disable();
Component_Status DRV_Motor_Speed_Ctrl(uint32_t speed);
void Motor_test();

#ifdef __cplusplus
}
#endif

#endif /* _MOTOR_CTRL_H_ */
