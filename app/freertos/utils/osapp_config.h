
/**
 *
 * @file osapp_config.h
 *
 * @brief Define some general structure and macro for os application
 *
 * Copyright (C) Apollo 2015-2016
 *
 */

#ifndef OSAPP_CONFIG_H_
#define OSAPP_CONFIG_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#define __RAM_CODE__
#include "bx_config.h"
#include "Freertos.h"
#include "task.h"
#include "event_groups.h"
#include "gapm_task.h"
#include "gapc_task.h"
#include "gattc_task.h"
#include "gattm_task.h"
#include "attm.h"
#include "bx_dbg.h"
#include "log.h"
#include "nvds_in_ram.h"
#include "osapp_task.h"
#include "ble_task.h"
#include "co_bt.h"
#include "co_math.h"
#include "co_list.h"
#include "prf.h"
#include "osapp_utils.h"


/*
 * DEFINES
 ****************************************************************************************
 */

///define task priority
#define OS_PRIORITY_BLE_TASK                    (configMAX_PRIORITIES  - 1)        //5
#define OS_PRIORITY_APP_TASK                    (OS_PRIORITY_BLE_TASK - 1)          // 4

#define osapp_ahi_msg_send(ptr,length,wait) os_ahi_msg_send(ptr,wait)           //deprecated

#define osapp_msg_build_send(ptr,length) (os_ahi_msg_send(ptr,portMAX_DELAY),pdTRUE) //deprecated


#endif/* OSAPP_CONFIG_H_ */

