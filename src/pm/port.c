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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sys/io.h"
#include "errno.h"
#include "sys/list.h"

#include "driver/chip/chip.h"
#include "driver/chip/hal_wakeup.h"
#include "driver/chip/hal_prcm.h"
#include "driver/chip/hal_nvic.h"
#include "driver/chip/hal_mbox.h"
#include "driver/hal_board.h"
#include "driver/hal_dev.h"

#include "pm/pm.h"
#include "pm_i.h"
#include "port.h"

#ifdef CONFIG_PM

/* It is better that all interrupts are managed by itself, the interrupt should
 * be disable by it's driver if it is not used during suspend. We use this mask
 * for a sample concentrated cometrue, 1: use this irq as wakeup source */
unsigned int nvic_int_mask[] = {
	NVIC_PERIPH_IRQ_MASK0,
#if (MAX_IRQn > 32)
	NVIC_PERIPH_IRQ_MASK1,
#endif
};

ct_assert((sizeof(nvic_int_mask) + 3) / 4 >= (NVIC_PERIPH_IRQ_NUM + 31)/32);

extern void N_UART_IRQHandler(void);

int platform_prepare(enum suspend_state_t state)
{
	if (HAL_MBOX_IsPmPatchEnabled()) {
		HAL_NVIC_ConfigExtIRQ(N_UART_IRQn, N_UART_IRQHandler, NVIC_PERIPH_PRIO_DEFAULT);
	} else {
	}

	return 0;
}

void platform_wake(enum suspend_state_t state)
{
	if (HAL_MBOX_IsPmPatchEnabled()) {
		HAL_NVIC_EnableIRQ(N_UART_IRQn);
		__ISB();
		__NOP();
	} else {
	}
}

#endif
