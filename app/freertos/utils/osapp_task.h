#ifndef OSAPP_TASK_H_
#define OSAPP_TASK_H_
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ke_task.h"
#include "ahi.h"
#include "co_bt.h"
#include "osmsg_queue.h"
#define ASYNC_CALL_MSG_TYPE 0x06

typedef void (*osapp_msg_handler_t)(ke_msg_id_t const msgid,
                            void const *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);

typedef const struct
{
    ke_msg_id_t         id;
    osapp_msg_handler_t func;
}osapp_msg_handler_table_t;

typedef struct
{
    struct co_list_hdr hdr;
    osapp_msg_handler_table_t * const handler_table;
    const uint8_t table_size;
}osapp_msg_handler_info_t;

#define HANDLER_ARRAY_INFO(hdl)                         {{0},hdl,sizeof(hdl)/sizeof(hdl[0])}

void app_queue_create(void);

bool app_queue_ahi_rsp_send(ke_msg_id_t msg_id,ke_task_id_t src_id,uint16_t param_len,uint8_t *param,uint32_t xTicksToWait);

void ahi_handler_register(osapp_msg_handler_info_t * const handler_info);

void osapp_task(void *params);

void osapp_task_create(void *param);

void osapp_async_call_wrapper(void (*func)(void *),void *param);


#endif

