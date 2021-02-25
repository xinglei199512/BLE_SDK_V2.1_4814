#include "osapp_config.h"
#include "osapp_task.h"
#include "bx_dbg.h"
#include "log.h"
#include "co_utils.h"
#include "queued_async_framework.h"
#include "task_init.h"
#include "ll.h"
#include "app_adc_utils.h"

#define APP_QUEUE_SIZE     20
#define APP_TASK_STACK_SIZE                     500

TaskHandle_t handler_osapp_task;
StackType_t stack_osapp_task[APP_TASK_STACK_SIZE];
StaticTask_t env_osapp_task;
QueueHandle_t sys_rsp_q;
static struct co_list ahi_handler_list;

void user_init(void);

bool app_queue_ahi_rsp_send(ke_msg_id_t msg_id,ke_task_id_t src_id,uint16_t param_len,uint8_t *param,uint32_t xTicksToWait)
{
    app_ahi_rsp_queue_t ahi_rsp = 
    {
        .type = AHI_KE_MSG_TYPE,
        .id = msg_id,
        .src_id = src_id,
        .param_len = param_len,
        .param = param,
    };
    return msg_send(sys_rsp_q,&ahi_rsp,xTicksToWait);
}

static bool app_queue_async_call_send(app_async_call_queue_t *async,uint32_t xTicksToWait)
{
    return msg_send(sys_rsp_q,async,xTicksToWait);
}

static bool app_queue_async_call_send_isr(app_async_call_queue_t *async)
{
    return msg_send_isr(sys_rsp_q,async);
}

void osapp_async_call_wrapper(void (*func)(void *),void *param)
{
    app_async_call_queue_t async = 
    {
        .type = ASYNC_CALL_MSG_TYPE,
        .func =func,
        .param = param,
    };
    bool sent;
    if(in_interrupt())
    {
        sent = app_queue_async_call_send_isr(&async);
    }else
    {
        sent = app_queue_async_call_send(&async,0);
    }
    BX_ASSERT(sent);
}

static void osapp_ahi_default_msg_handler(ke_msg_id_t const msgid, void const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG(LOG_LVL_WARN,"default handler,msgid: 0x%x, dest_id: 0x%x, src_id: 0x%x\n",msgid,dest_id,src_id);
}

static void osapp_ahi_msg_rx(app_ahi_rsp_queue_t *msg)
{
    bool found = false;
    struct co_list_hdr *hdr = co_list_pick(&ahi_handler_list);
    while(hdr)
    {
        osapp_msg_handler_info_t const * ptr = CONTAINER_OF(hdr,osapp_msg_handler_info_t,hdr);
        uint8_t i;
        for(i=0;i<ptr->table_size;++i)
        {
            if(msg->id==ptr->handler_table[i].id)
            {
                found = true;
                ptr->handler_table[i].func(msg->id,msg->param,TASK_ID_AHI,msg->src_id);
                continue;
            }
        }
        hdr = co_list_next(hdr);
    }
    if(found == false)
    {
        osapp_ahi_default_msg_handler(msg->id,msg->param,TASK_ID_AHI,msg->src_id);
    }    
    vPortFree(msg->param);
}

void ahi_handler_register(osapp_msg_handler_info_t * const handler_info)
{
    co_list_push_back(&ahi_handler_list,&handler_info->hdr);
}

void app_queue_create()
{
    static StaticQueue_t rsp_q_env;
    static uint8_t rsp_q_buf[APP_QUEUE_SIZE*sizeof(app_queue_t)];
    sys_rsp_q = xQueueCreateStatic(APP_QUEUE_SIZE,sizeof(app_queue_t),rsp_q_buf,&rsp_q_env);
}

void osapp_async_call_handler(app_async_call_queue_t *ptr)
{
    ptr->func(ptr->param);
}

void osapp_task(void *params)
{
    co_list_init(&ahi_handler_list);
    app_adc_util_init();
    user_init();
    while(1)
    {
        app_queue_t data;
        if(xQueueReceive(sys_rsp_q,&data,portMAX_DELAY)!=pdPASS)
        {
            BX_ASSERT(0);
        }
        switch (data.type)
        {
        case AHI_KE_MSG_TYPE:
        {
            app_ahi_rsp_queue_t *ahi = &data.ahi_rsp;
            osapp_ahi_msg_rx(ahi);            
        }
        break;
        case HCI_EVT_MSG_TYPE:
            //osapp_hci_evt_rx(&data);
        break;
        case HCI_ACL_MSG_TYPE:
            //osapp_hci_data_rx(&data);
        break;
        case ASYNC_CALL_MSG_TYPE:
        {
            app_async_call_queue_t *async = &data.async_call;
            osapp_async_call_handler(async);
        }
        break;
        default:
            LOG(LOG_LVL_WARN,"app queue invalid type\n");
        break;
        }
//            LOG(LOG_LVL_INFO,"osapp msg received\n");
    }

}

void osapp_task_create(void *param)
{
    handler_osapp_task = xTaskCreateStatic(osapp_task,"OSAPP TASK",APP_TASK_STACK_SIZE,
            param,OS_PRIORITY_APP_TASK,stack_osapp_task,&env_osapp_task);
}
