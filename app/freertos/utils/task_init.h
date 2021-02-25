
/**
 ****************************************************************************************
 *
 * @file task_init.h
 *
 * @brief responsible for freertos task initialize 
 *
 * Copyright (C) Apollo
 *
 *
 ****************************************************************************************
 */

#ifndef TASK_INIT_H_
#define TASK_INIT_H_


/**
 ****************************************************************************************
 * @addtogroup ARCH
 * @ingroup APOLLO_00
 *
 * @brief task init of freertos
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdint.h>
#include "osmsg_queue.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "plf.h"
#include "rwip_config.h"


/*
 * DEFINES
 ****************************************************************************************
 */
#if (RC32K_USED == 1)
#define portRTC_FREQUENCY (1000000*64*32)/get_rc32k_calib_val()
#else
#if (HZ32000)
#define portRTC_FREQUENCY 32000
#else
#define portRTC_FREQUENCY 32768
#endif
#endif




/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void rtos_task_init(void);						// init of all freertos tasks.
void stk_chk_dbg( void );
void system_tick_init(void);
bool msg_send(QueueHandle_t q,void *data,uint32_t xTicksToWait);
bool msg_send_isr(QueueHandle_t q,void *data);
int32_t time_diff(uint32_t time0,uint32_t time1);


#endif // TASK_INIT_H_


