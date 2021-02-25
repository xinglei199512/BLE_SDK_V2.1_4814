#include <stdbool.h>
#include "mesh_sched.h"
#include "log.h"
#include "co_hci.h"
#include "adv_bearer_tx.h"
#include "queued_async_framework.h"
#include "mesh_queued_msg.h"
#include "gapm_task.h"
#include "osapp_task.h"
#include "set_adv_payload_31Byte_patch.h"
#include "stack_mem_cfg.h"
#include "gapm_task.h"
#include "ble_task.h"
#include "osapp_config.h"
#include "mesh_core_api.h"

#define MESH_ADV_INTERVAL               160  //unit: 0.625ms
#define MESH_SCAN_INTERVAL              48   //30ms
#define MESH_SCHED_LIST_LENGTH 32
#define MESH_SCHED_HIGH_PRIORITY_ADV_LIST_LENGTH 3


enum SCAN_STATE{
    SCAN_STOP,
    SCAN_ONGOING,
    SCAN_CANCELLING
};

enum mesh_sched_async_op
{
    MESH_SCAN_CANCEL,
    MESH_ADV_TX_START,
};

typedef struct
{
    void *param;
    enum mesh_sched_async_op op;
}mesh_sched_t;

static void mesh_sched_process(mesh_sched_t *ptr);
static void mesh_sched_complete_callback(mesh_sched_t *ptr,void * param,uint8_t status);
PRIORITY_QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(mesh_sched_list, MESH_SCHED_LIST_LENGTH,
    MESH_SCHED_HIGH_PRIORITY_ADV_LIST_LENGTH,mesh_sched_t, mesh_sched_process, mesh_sched_complete_callback);

static bool scan_enabled;
static enum SCAN_STATE scan_ongoing;

static bool mesh_sched_queue_empty()
{
    if(priority_queued_async_process_get_amount(&mesh_sched_list,false)!=0)
    {
        return false;
    }
    if(priority_queued_async_process_get_amount(&mesh_sched_list,true)!=0)
    {
        return false;
    }
    return true;
}

static void scan_cancel(void (*cb)(),bool high_priority)
{
    mesh_sched_t buf = {
        .op = MESH_SCAN_CANCEL,
        .param = cb,
    };
    bool flag = priority_queued_async_process_start(&mesh_sched_list,high_priority,&buf,NULL);
    BX_ASSERT(flag);
}

static void stop_scan_cmd_send()
{
    scan_ongoing = SCAN_CANCELLING;
    struct gapm_cancel_cmd *cmd = AHI_MSG_ALLOC(GAPM_CANCEL_CMD,TASK_ID_GAPM,gapm_cancel_cmd);
    cmd->operation = GAPM_CANCEL;
    osapp_ahi_msg_send(cmd,sizeof(struct gapm_cancel_cmd),portMAX_DELAY);
}

static void start_scan_cmd_send()
{
    BX_ASSERT(scan_ongoing == SCAN_STOP);
    scan_ongoing = SCAN_ONGOING;
    struct gapm_start_scan_cmd *cmd = AHI_MSG_ALLOC(GAPM_START_SCAN_CMD,TASK_ID_GAPM,gapm_start_scan_cmd);
    cmd->op.code = GAPM_SCAN_PASSIVE;
    cmd->op.addr_src = GAPM_GEN_NON_RSLV_ADDR;
    cmd->interval = MESH_SCAN_INTERVAL;
    cmd->window = MESH_SCAN_INTERVAL;
    cmd->mode = GAP_OBSERVER_MODE;
    cmd->filt_policy = SCAN_ALLOW_ADV_ALL;
    cmd->filter_duplic = SCAN_FILT_DUPLIC_DIS;
    osapp_ahi_msg_send(cmd,sizeof(struct gapm_start_scan_cmd),portMAX_DELAY);

}

static void mesh_sched_complete_callback(mesh_sched_t *ptr,void * param,uint8_t status)
{
    if(ptr->op == MESH_SCAN_CANCEL)
    {
        void (*cb)(void) = (void (*)(void))ptr->param;
        if(cb)
        {
            cb();
        }
    }
    if(mesh_sched_queue_empty())
    {
        if(scan_enabled&&scan_ongoing==SCAN_STOP)
        {
            start_scan_cmd_send();
//            LOG(LOG_LVL_WARN,"scan_start\n");

        }else if(scan_enabled==false&&scan_ongoing==SCAN_ONGOING)
        {
            scan_cancel(NULL,false);
//            LOG(LOG_LVL_WARN,"scan_cancel\n");
        }
    }
}

static void adv_cmd_send_and_notify(mesh_adv_tx_t *ptr)
{
    struct gapm_start_advertise_cmd_patch *cmd = AHI_MSG_ALLOC(GAPM_START_ADVERTISE_CMD, TASK_ID_GAPM, gapm_start_advertise_cmd_patch);
    //fill cmd
    if(ptr->pkt_type < GATT_SERVICE_ADV_PKT)
    {
        cmd->op.code = GAPM_ADV_NON_CONN;
        cmd->info.host.mode = GAP_BROADCASTER_MODE;
        //no sacn rsp data
        cmd->info.host.scan_rsp_data_len = 0;
    }
    else
    {
        mesh_core_params_t param;
        mesh_core_params_get(MESH_CORE_PARAM_NAME , &param);
        cmd->op.code = GAPM_ADV_UNDIRECT;
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
        //sacn rsp data
        cmd->info.host.scan_rsp_data_len = 0;
        cmd->info.host.scan_rsp_data_len = param.name.len + 2;
        cmd->info.host.scan_rsp_data[0] = param.name.len + 1;
        cmd->info.host.scan_rsp_data[1] = GAP_AD_TYPE_SHORTENED_NAME;
        memcpy((cmd->info.host.scan_rsp_data+2),param.name.name,param.name.len);

        //add company_id
        uint8_t mfu_start_offset = param.name.len + 2;
        uint16_t company_id = 0xffff;
        cmd->info.host.scan_rsp_data_len += sizeof(company_id) + 2;
        cmd->info.host.scan_rsp_data[mfu_start_offset] = sizeof(company_id) + 1;
        cmd->info.host.scan_rsp_data[mfu_start_offset + 1] = GAP_AD_TYPE_MANU_SPECIFIC_DATA;
        memcpy((cmd->info.host.scan_rsp_data+mfu_start_offset+2), &company_id, sizeof(company_id));
    }
    cmd->op.addr_src = GAPM_STATIC_ADDR;
    cmd->op.state = 0;
    cmd->intv_min = MESH_ADV_INTERVAL;
    cmd->intv_max = MESH_ADV_INTERVAL;
    cmd->channel_map = ADV_ALL_CHNLS_EN;
    cmd->info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    cmd->info.host.adv_data_len = ptr->data_length;
    memcpy(cmd->info.host.adv_data,ptr->adv_data ,ADV_DATA_BUF_SIZE);
    osapp_ahi_msg_send(cmd, sizeof(struct gapm_start_advertise_cmd_patch),portMAX_DELAY);
    mesh_queued_msg_send((void(*)(void *))bearer_on_adv_start_callback,ptr);
}

static void scan_cancel_end(void *param)
{
    priority_queued_async_process_end(&mesh_sched_list,NULL,0);
}

static void mesh_sched_process(mesh_sched_t *ptr)
{
    BX_ASSERT(scan_ongoing!=SCAN_CANCELLING);
    switch(ptr->op)
    {
    case MESH_SCAN_CANCEL:
//        LOG(LOG_LVL_WARN,"mesh_sched2 %d\n",scan_ongoing);

        if(scan_ongoing == SCAN_ONGOING)
        {
            stop_scan_cmd_send();
        }else
        {
            mesh_queued_msg_send(scan_cancel_end,NULL);
        }
    break;
    case MESH_ADV_TX_START:
        BX_ASSERT(scan_ongoing == SCAN_STOP);
        adv_cmd_send_and_notify(ptr->param);
    break;
    default:
        BX_ASSERT(0);
    break;
    }
}

void mesh_sched_stop_scan(void (*cb)())
{
    scan_enabled = false;
    scan_cancel(cb,false);
//    LOG(LOG_LVL_WARN,"stop_scan2\n");

}

void mesh_sched_start_scan()
{
    if(scan_enabled == false)
    {
        scan_enabled = true;
        if(mesh_sched_queue_empty())
        {
            start_scan_cmd_send();
            LOG(LOG_LVL_WARN,"start-api\n");
        }
    }
}

void mesh_sched_adv_tx(mesh_adv_tx_t *ptr,bool high_priority)
{
    if(scan_ongoing == SCAN_ONGOING)
    {
        scan_cancel(NULL,high_priority);
//        LOG(LOG_LVL_WARN,"scan_cancel3\n");
    }
    mesh_sched_t buf = {
     .op = MESH_ADV_TX_START,
     .param  = ptr,
    };
    bool flag = priority_queued_async_process_start(&mesh_sched_list,high_priority,&buf,NULL);
    BX_ASSERT(flag);
}

static void mesh_sched_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
//    LOG(3, "operation:%x status:%x\n", param->operation, param->status);
    switch(param->operation)
    {
    case GAPM_SCAN_PASSIVE:
        BX_ASSERT(scan_ongoing == SCAN_CANCELLING);
        scan_ongoing = SCAN_STOP;
        scan_cancel_end(NULL);
    break; 
    case GAPM_ADV_NON_CONN: 
    case GAPM_ADV_UNDIRECT:
        if(param->status == GAP_ERR_CANCELED || param->status == GAP_ERR_NO_ERROR ) 
        {
            priority_queued_async_process_end(&mesh_sched_list,NULL,0);
        }else {
//           LOG(2, "adv cmp status:%x\n", param->status);
        }
    break;
    default:

    break;
    }
}

static const osapp_msg_handler_table_t mesh_sched_handler_table[] = {
   [0] = {GAPM_CMP_EVT,(osapp_msg_handler_t)mesh_sched_gapm_cmp_evt_handler},
};

osapp_msg_handler_info_t mesh_sched_handler_info = HANDLER_ARRAY_INFO(mesh_sched_handler_table);

void mesh_sched_init()
{
    ahi_handler_register(&mesh_sched_handler_info);
}
