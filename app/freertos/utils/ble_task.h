/**
 *
 * @file ble_task.h
 *
 * @brief The header file of ble_task.c
 *
 * Copyright (C) Apollo 2015-2016
 *
 */

#ifndef BLE_TASK_H_
#define BLE_TASK_H_
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "osmsg_queue.h"

void *ahi_msg_alloc(ke_msg_id_t const id, ke_task_id_t const dest_id, uint16_t const param_len);

/* You should call this ahi_msg_free() API only if you do not send your msg to ble stack and won't send it any more.*/
void ahi_msg_free(void *ptr);

void *hci_cmd_alloc(uint16_t ocf,uint8_t ogf,uint16_t param_len);

void *hci_data_alloc(uint16_t handle,uint8_t pb_flag,uint8_t bc_flag,uint16_t data_len);

bool os_ahi_msg_send(void *param_ptr,uint32_t xTicksToWait);

void ble_queue_create(void);

void ble_stack_task_create(void *param);

/**
 * @brief Ble task. Must exist in Apollo bluetooth system.
 * @param params: pointer to the input parameter for ble task.
 */
void ble_stack_task(void *params);

/**
 * @brief when ble exit isr, ble_schedule() must run again because something may be down when interrupt happened.
 * We Send a message from isr to ble_task in order to run ble_schedule.
 */
void msg2blestack_from_ISR(void);

__INLINE ble_rx_msg_t * ahi_param2msg(void const *param_ptr)
{
    return (ble_rx_msg_t*) (((uint8_t*)param_ptr) - offsetof(ble_rx_msg_t, param));
}

__INLINE void * ahi_msg2param(ble_rx_msg_t const *msg)
{
    return (void*) (((uint8_t*) msg) + offsetof(ble_rx_msg_t, param));
}

__INLINE hci_cmd_t *hci_param2cmd(void const *param_ptr)
{
    return (hci_cmd_t *)((uint8_t *)param_ptr - offsetof(hci_cmd_t,param));
}

__INLINE void *hci_cmd2param(hci_cmd_t const *cmd)
{
    return (void *)((uint8_t *)cmd + offsetof(hci_cmd_t,param));
}

__INLINE hci_data_t *hci_param2data(void const *param_ptr)
{
    return (hci_data_t *)((uint8_t *)param_ptr - offsetof(hci_data_t,param));
}

__INLINE void *hci_data2param(hci_data_t const *data)
{
    return (void *)((uint8_t *)data + offsetof(hci_data_t,param));
}

#define AHI_MSG_ALLOC(id, dest,param_str) \
    (struct param_str*) ahi_msg_alloc(id, dest, sizeof(struct param_str))
    
#define AHI_MSG_ALLOC_DYN(id, dest, param_str,length)  (struct param_str*)ahi_msg_alloc(id, dest, \
    (sizeof(struct param_str) + length));

#endif
