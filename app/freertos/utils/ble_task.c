/**
 *
 * @file ble_task.c
 *
 * @brief This file mainly packed ble system into a freertos task.
 *
 * Copyright (C) Apollo 2015-2016
 *
 */
#include "osapp_config.h"
#include "ble_task.h"
#include "sys_sleep.h"
#include "os_bridge.h"
#include "log.h"
#include "task_init.h"
#include "compiler_flag.h"

#define BLE_QUEUE_SIZE     3
#define BLE_TASK_STACK_SIZE                     300  // actual size = (XXX_TASK_STACK_SIZE * 4) Byte
extern uint32_t error;

/// the handler of ble task which can be use when create ble task.
TaskHandle_t handler_ble_stack_task;
StackType_t stack_ble_task[BLE_TASK_STACK_SIZE];
StaticTask_t env_ble_task;
QueueHandle_t sys_cmd_q;

N_XIP_SECTION void msg2blestack_from_ISR()
{

    /// We send msg only to prevent there are no messages in the queue.
    /// When there are no message in the queue ble_schedule will not be called.
    if(xQueueIsQueueEmptyFromISR(sys_cmd_q))
    {
        ble_queue_t data = 
        {
            .type = 0,
            .msg = NULL,
        };
        bool sent = msg_send_isr(sys_cmd_q,&data);
        BX_ASSERT(sent);
    }
}

void *ahi_msg_alloc(ke_msg_id_t const id, ke_task_id_t const dest_id, uint16_t const param_len)
{
    ble_rx_msg_t *msg = (ble_rx_msg_t*) pvPortMalloc(sizeof(ble_rx_msg_t) +
                                                    param_len - sizeof (uint32_t));
    void *param_ptr = NULL;

    BX_ASSERT(msg != NULL);

    msg->id        = id;
    msg->dest_id   = dest_id;
    msg->src_id    = TASK_ID_AHI;                   //ahi task
    msg->param_len = param_len;

    param_ptr = ahi_msg2param(msg);

    memset(param_ptr, 0, param_len);

    return param_ptr;
}

void ahi_msg_free(void *ptr)
{
    vPortFree(ahi_param2msg(ptr));
}

void *hci_cmd_alloc(uint16_t ocf,uint8_t ogf,uint16_t param_len)
{
    hci_cmd_t *cmd = pvPortMalloc(param_len+offsetof(hci_cmd_t,param));
    if(NULL == cmd)
    {
        return NULL;
    }
    cmd->opcode = HCI_OPCODE(ocf,ogf);
    cmd->param_len = param_len;
    return cmd->param;
}

void *hci_data_alloc(uint16_t handle,uint8_t pb_flag,uint8_t bc_flag,uint16_t data_len)
{
    hci_data_t *data = pvPortMalloc(data_len + offsetof(hci_data_t,param));
    if(NULL == data)
    {
        return NULL;
    }
    data->handle_flag = bc_flag<<14 | pb_flag<<12 | handle;
    data->data_len = data_len;
    return data->param;
}

bool os_ahi_msg_send(void *param_ptr,uint32_t xTicksToWait)
{
    ble_queue_t data = 
    {
        .type = AHI_KE_MSG_TYPE,
        .msg = ahi_param2msg(param_ptr),
    };
    return msg_send(sys_cmd_q,&data,xTicksToWait);
}

void ble_queue_create()
{
    static StaticQueue_t cmd_q_env;
    static uint8_t cmd_q_buf[BLE_QUEUE_SIZE*sizeof(ble_queue_t)];
    sys_cmd_q = xQueueCreateStatic(BLE_QUEUE_SIZE,sizeof(ble_queue_t),cmd_q_buf,&cmd_q_env);
}


void ble_stack_task_create(void *param)
{
    handler_ble_stack_task = xTaskCreateStatic(ble_stack_task,"BLE STACK TASK",BLE_TASK_STACK_SIZE,
        param,OS_PRIORITY_BLE_TASK,stack_ble_task,&env_ble_task);
}

/**
 * @brief Ble task. Must exist in Apollo bluetooth system.
 * @param params: pointer to the input parameter for ble task.
 */
void ble_stack_task(void *params)
{
    rwip_init(error);
    while(1)
    {
        ble_schedule();
        ble_queue_t data;
        if(xQueueReceive(sys_cmd_q, &data, portMAX_DELAY)==pdTRUE)
        {
            if(data.msg)                                    // msg from AHI
            {
                ble_msg_start_recv(data.type,data.msg);
            }
        }    
    }
}

