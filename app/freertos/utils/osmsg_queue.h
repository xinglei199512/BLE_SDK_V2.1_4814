#ifndef OSMSG_QUEUE_H_
#define OSMSG_QUEUE_H_
#include "FreeRTOS.h"
#include "queue.h"
#include "ke_msg.h"

typedef struct
{
    ke_msg_id_t  id;
    ke_task_id_t dest_id;
    ke_task_id_t src_id;
    uint16_t     param_len;
    uint32_t     param[1];
}ble_rx_msg_t;

typedef struct
{
    uint8_t type;
    ble_rx_msg_t *msg;
}ble_queue_t;

typedef struct
{
    uint8_t type;
    ke_msg_id_t id;
    ke_task_id_t src_id;
    uint16_t param_len;
    uint8_t *param;
}app_ahi_rsp_queue_t;

typedef struct
{
    uint8_t type;
    void (*func)(void *);
    void *param;
}app_async_call_queue_t;

typedef union
{
    uint8_t type;
    app_ahi_rsp_queue_t ahi_rsp;
    app_async_call_queue_t async_call;
}app_queue_t;

typedef struct{
    uint16_t opcode;
    uint8_t param_len;
    uint8_t param[];
}hci_cmd_t;

typedef struct{
    uint16_t handle_flag;
    uint16_t data_len;
    uint8_t param[];
}hci_data_t;

typedef struct{
    uint8_t evt_code;
    uint8_t param_len;
    uint8_t param[];
}hci_event_t;



#endif

