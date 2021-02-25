#include "osapp_config.h"
#include <stddef.h>
#include <stdbool.h>
#include "bx_config.h"
#include "osapp_mesh.h"
#include "osapp_svc_manager.h"
#include "bx_ring_queue.h"
#include "security.h"
#include "co_utils.h"
#include "co_math.h"
#include "sdk_mesh_definitions.h"
#include "mesh_env.h"
#include "sdk_mesh_config.h"
#include "osapp_utils.h"
#include "beacon.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_client.h"
#include "node_save.h"
#include "mesh_stack_init.h"
#include "set_adv_payload_31Byte_patch.h"
#include "set_adv_payload_31Byte_patch.h"
#include "mesh_gatt.h"
#include "osapp_utils.h"
#include "mesh_core_api.h"
#include "mesh_queued_msg.h"
#include "aes_128.h"
#include "network_pdu_decrypt.h"
#include "adv_bearer_rx.h"
#include "adv_bearer_tx.h"
#include "osapp_task.h"
#include "mesh_sched.h"
#include "mesh_reset_database.h"
#include "true_random.h"
#include "bxotas_task.h"
/*
 * DEFINES
 ****************************************************************************************
 */
//#define OSAPP_MESH_AES_TEST


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum{
    MESH_INIT,
    MESH_TX,
    MESH_STOP,
    MESH_ON_AIR_OP_CANCELED,
    MESH_CALLBACK,
};
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
//DATA STRUCT

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

TaskHandle_t mesh_handler_osapp_task;
//StackType_t mesh_stack_osapp_task[[MESH_TASK_STACK_SIZE];
StaticTask_t mesh_env_osapp_task;


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void osapp_bxotas_config(void);
static int32_t osapp_add_bxotas_task(void);

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void mesh_stack_init_process_next_stage(void)
{
    mesh_queued_msg_send((void (*)(void *))mesh_stack_init_process,NULL);
}

static int32_t osapp_reset()
{
    struct gapm_reset_cmd *cmd = AHI_MSG_ALLOC(GAPM_RESET_CMD,TASK_ID_GAPM,gapm_reset_cmd);
    cmd->operation = GAPM_RESET;
    return osapp_msg_build_send(cmd, sizeof(struct gapm_reset_cmd));
}
static void unresolvable_addr_generate(uint8_t *dst)
{
    uint32_t addr_low = co_rand_word();
    uint16_t addr_high = co_rand_hword();
    memcpy(dst,&addr_low,sizeof(addr_low));
    memcpy(dst+sizeof(addr_low),&addr_high,sizeof(addr_high));
    dst[5]&=0x3f; //set two MSB to 0
}
static int32_t osapp_set_dev_config()
{
    uint32_t seed = get_random_seed();
    co_random_init(seed);
    struct gapm_set_dev_config_cmd* cmd = AHI_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,TASK_ID_GAPM,gapm_set_dev_config_cmd);
    cmd->operation = GAPM_SET_DEV_CONFIG;
    cmd->role      = GAP_ROLE_ALL;
    cmd->renew_dur = 0;
    unresolvable_addr_generate(cmd->addr.addr);
    //cmd->irk
    cmd->addr_type = GAPM_CFG_ADDR_PUBLIC;
    cmd->pairing_mode = GAPM_PAIRING_LEGACY;
    cmd->gap_start_hdl = 0;
    cmd->gatt_start_hdl = 0;
    //cmd->att_cfg
    cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;
    cmd->sugg_max_tx_time   = BLE_MIN_TIME;
    cmd->max_mtu = GAP_MAX_LE_MTU;
    cmd->max_mps = GAP_MAX_LE_MTU;
    //cmd->max_nb_lecb
    //cmd->audio_cfg
    cmd->tx_pref_rates = GAP_RATE_LE_1MBPS;
    cmd->rx_pref_rates = GAP_RATE_LE_1MBPS;

    cmd->att_cfg = GAPM_MASK_ATT_SVC_CHG_EN | GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN;

    return osapp_msg_build_send(cmd, sizeof(struct gapm_set_dev_config_cmd) );
    
}


void mesh_send_init_msg()
{
    BX_DELAY_US((rand() % 200)*1000+2);
    mesh_queued_msg_send((void (*)(void *))mesh_stack_init_process,NULL);
}

//must call once !
void mesh_init_start_scan(void)
{
   mesh_sched_start_scan();
}


static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_RESET:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_set_dev_config();
        break;
    case GAPM_SET_DEV_CONFIG:
        LOG(LOG_LVL_INFO,"set_dev_config_cmp\n");
        mesh_send_init_msg();
        osapp_add_bxotas_task();
        break;
    case GAPM_ADV_NON_CONN:
    case GAPM_SCAN_PASSIVE:
    case GAPM_ADV_UNDIRECT:
         //TODO
        break;
    case GAPM_USE_ENC_BLOCK:
        //LOG(LOG_LVL_INFO,"gapm_cmp_evt: use_enc_block\n");
        break;
    default:
        LOG(LOG_LVL_WARN,"gapm_cmp_evt operation:0x%x,status=0x%x\n",cmp_evt->operation,cmp_evt->status);
        break;
    }
}

static uint8_t *get_mesh_adv_data_ptr(uint8_t  *buf,uint8_t length)
{
    uint8_t *ptr;
    uint8_t ad_length;
    for(ptr=buf,ad_length=ptr[0]; ptr-buf<length; ptr += ad_length+1,ad_length=ptr[0])
    {
        uint8_t ad_type = ptr[1];
        if(ad_type==MESH_PROVISIONING_AD_TYPE||ad_type==MESH_MESSAGE_AD_TYPE||ad_type==MESH_BEACON_AD_TYPE)
        {
            return ptr;
        }
    }
    return NULL;
}


static void osapp_gapm_adv_report_ind_handler(ke_msg_id_t const msgid, struct gapm_adv_report_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    ble_txrx_time_t rx_time = *adv_rx_time_dequeue();
    if(param->report.evt_type!=ADV_NONCONN_UNDIR)
    {
        //LOG(LOG_LVL_INFO,"not non-conn adv\n");
        return;
    }
    if(provision_is_database_pending())
    {
        LOG(3,"database_pending rx !\n");
        return;
    }
    uint8_t * mesh_data_ptr = get_mesh_adv_data_ptr((uint8_t  *)param->report.data,param->report.data_len);
    if(mesh_data_ptr)
    {
        uint8_t mesh_data_length = param->report.data_len + param->report.data - mesh_data_ptr;
        uint8_t pkt_type = mesh_data_ptr[1];
        mesh_adv_rx(pkt_type, mesh_data_ptr + 2 ,mesh_data_length - 2,rx_time,param->report.rssi);
     }
    else
    {
        //LOG(LOG_LVL_INFO,".");
    }
}

static void osapp_device_ready_ind_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    //reset cmd
    LOG(LOG_LVL_INFO,"Device Ready\n");
    osapp_reset();
}

static void osapp_gattc_cmp_evt_handler(ke_msg_id_t const msgid,struct gattc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t src_id)
{
    LOG(LOG_LVL_INFO,"osapp_gattc_cmp_evt_handler \n");
}

static int32_t osapp_gapc_conn_confirm(ke_task_id_t dest_id)
{
    struct gapc_connection_cfm *cfm = AHI_MSG_ALLOC(GAPC_CONNECTION_CFM, dest_id, gapc_connection_cfm);

    cfm->auth = GAP_AUTH_REQ_NO_MITM_NO_BOND;
    LOG(3,"osapp_gapc_conn_confirm\n");

    return osapp_msg_build_send(cfm,sizeof(struct gapc_connection_cfm));
}
static void osapp_gapc_param_update_ind_handler(ke_msg_id_t const msgid, struct gapc_param_updated_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
   LOG(LOG_LVL_INFO,"param update ind_%x,%x,%x\n",param->con_interval,param->con_latency,param->sup_to);
}
static void osapp_gapc_conn_req_ind_handler(ke_msg_id_t const msgid, struct gapc_connection_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    osapp_gapc_conn_confirm(src_id);

    LOG(3,"osapp_gapc_conn_req_ind_handler sup_to:%ds\n", param->sup_to/100);
    ble_mesh_gatt_connect_evt_callback(MESH_CONNECT_SRCID_TO_HANDLE(src_id));
}
/**
 ****************************************************************************************
 * @brief callback when receive GAPC_DISCONNECT_IND, indicate the connect has broken off. And normally, we should restart advertising when connection break off.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAPM).
 * @param[in] src_id    ID of the sending task instance.
 *
 ****************************************************************************************
 */
static void osapp_gapc_disconnect_ind_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapc_disconnect_ind const *ind = param;

    LOG(LOG_LVL_INFO,"Connect lost 0x%X \n",ind->conhdl);
    LOG(LOG_LVL_INFO,"Disconnect Reason 0x%X \n",ind->reason);

    ble_mesh_gatt_disconnect_evt_callback(MESH_CONNECT_SRCID_TO_HANDLE(ind->conhdl));
}
static void osapp_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,struct gattc_mtu_changed_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"Exchanged MTU value:%d, seq_num:%d\n",param->mtu,param->seq_num);
    ble_mesh_gatt_exchange_mtu_req_handle(MESH_CONNECT_SRCID_TO_HANDLE(src_id),param->mtu);
}
static int32_t osapp_gapc_param_update_cfm(ke_task_id_t const src_id)
{
    struct gapc_param_update_cfm *cfm = AHI_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM, src_id, gapc_param_update_cfm);

    cfm->accept = 0x01;
    cfm->ce_len_max = 0xffff;
    cfm->ce_len_min = 0xffff;
    LOG(3,"osapp_gapc_param_update_cfm\n");

    return osapp_msg_build_send(cfm, sizeof(struct gapc_param_update_cfm));
}

static int32_t osapp_get_dev_name(ke_task_id_t const dest_id)
{
    nvds_tag_len_t device_name_length = NVDS_LEN_DEVICE_NAME;
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm, device_name_length);
    cfm->req = GAPC_DEV_NAME;
    nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, cfm->info.name.value);
    cfm->info.name.length = device_name_length;
    return osapp_msg_build_send(cfm, sizeof(struct gapc_get_dev_info_cfm)+device_name_length);
}
static int32_t osapp_get_dev_appearance(ke_task_id_t const dest_id)
{
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm);
    cfm->req = GAPC_DEV_APPEARANCE;
    cfm->info.appearance = 0;
    return osapp_msg_build_send(cfm, sizeof(struct gapc_get_dev_info_cfm));
}
static int32_t osapp_get_dev_slv_pref_params(ke_task_id_t const dest_id)
{
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm);
    cfm->req = GAPC_DEV_SLV_PREF_PARAMS;
    cfm->info.slv_params.con_intv_min = 20;
    cfm->info.slv_params.con_intv_max = 40;
    cfm->info.slv_params.slave_latency = 0;
    cfm->info.slv_params.conn_timeout = 200;
    return osapp_msg_build_send(cfm, sizeof(struct gapc_get_dev_info_cfm));
}

static void osapp_gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid, struct gapc_get_dev_info_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapc_get_dev_info_req_ind const *req_ind =param;
    switch(req_ind->req)
    {
    case GAPC_DEV_NAME:
        osapp_get_dev_name(src_id);
        break;
    case GAPC_DEV_APPEARANCE:
        osapp_get_dev_appearance(src_id);
        break;
    case GAPC_DEV_SLV_PREF_PARAMS:
        osapp_get_dev_slv_pref_params(src_id);
        break;
    default :
        BX_ASSERT(0);
        break;
    }
}

static void osapp_gapc_param_update_req_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"param update request\n");
    osapp_gapc_param_update_cfm(src_id);
}
static void osapp_gapc_le_pkt_size_ind_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapc_le_pkt_size_ind *ind = (struct gapc_le_pkt_size_ind *)param;
    LOG(LOG_LVL_INFO,"tx_oct=%d, rx_oct=%d\n",ind->max_tx_octets,ind->max_rx_octets);
}

static void osapp_bxotas_config(void)
{
    struct bxotas_firmware_dest_cmd *cmd =AHI_MSG_ALLOC(BXOTAS_FIRMWARE_DEST_CMD,TASK_ID_BXOTAS,bxotas_firmware_dest_cmd);
    cmd->firmware_dest = LOCAL_FLASH;
    osapp_ahi_msg_send(cmd,sizeof(struct bxotas_firmware_dest_cmd),portMAX_DELAY);

}

static void osapp_bxotas_start_cfm(uint8_t status)
{
    struct bxotas_start_cfm *cfm = AHI_MSG_ALLOC(BXOTAS_START_CFM, TASK_ID_BXOTAS, bxotas_start_cfm);
    cfm->status = status;
    osapp_ahi_msg_send(cfm, sizeof(struct bxotas_start_cfm), portMAX_DELAY);
}

static int32_t osapp_add_bxotas_task(void)
{
    struct gapm_profile_task_add_cmd *req=AHI_MSG_ALLOC(GAPM_PROFILE_TASK_ADD_CMD, TASK_ID_GAPM, gapm_profile_task_add_cmd);
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH,NO_AUTH);
    req->prf_task_id = TASK_ID_BXOTAS;
    req->app_task = TASK_AHI;
    req->start_hdl = 0;
    return osapp_msg_build_send(req,sizeof(struct gapm_profile_task_add_cmd));
}


static void osapp_gapm_profile_added_ind_handler(ke_msg_id_t const msgid, struct gapm_profile_added_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"GAPM profile added indication,id:%d,nb:%d,hdl:%d\n",param->prf_task_id,param->prf_task_nb,param->start_hdl);
    if(param->prf_task_id==TASK_ID_BXOTAS)
    {
        osapp_bxotas_config();
    }
}

static void osapp_bxotas_start_req_ind_handler(ke_msg_id_t const msgid,struct bxotas_start_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"OTA START:%d,%d\n",param->conidx,param->segment_data_max_length);
    osapp_bxotas_start_cfm(OTA_REQ_ACCEPTED);
}

static void osapp_bxotas_finish_ind_handler(ke_msg_id_t const msgid,void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"OTA_DONE\n");
    /* do not reset chip immediately */
    mesh_sched_stop_scan(NULL);
    BX_DELAY_US(1000000);
    platform_reset(0);
}


static const osapp_msg_handler_table_t handler_table[]=
{
        [0] =   
                {GAPM_ADV_REPORT_IND,(osapp_msg_handler_t)osapp_gapm_adv_report_ind_handler},
                {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
                {GAPC_DISCONNECT_IND,(osapp_msg_handler_t)osapp_gapc_disconnect_ind_handler},
                {GAPM_USE_ENC_BLOCK_IND,(osapp_msg_handler_t)osapp_gapm_use_enc_block_ind},
                {GAPM_DEVICE_READY_IND,(osapp_msg_handler_t)osapp_device_ready_ind_handler},
                {GATTC_CMP_EVT,(osapp_msg_handler_t)osapp_gattc_cmp_evt_handler},
                {GAPC_PARAM_UPDATED_IND,(osapp_msg_handler_t)osapp_gapc_param_update_ind_handler},
                {GAPC_CONNECTION_REQ_IND,(osapp_msg_handler_t)osapp_gapc_conn_req_ind_handler},
                {GATTC_MTU_CHANGED_IND,(osapp_msg_handler_t)osapp_gattc_mtu_changed_ind_handler},
                {GAPC_PARAM_UPDATE_REQ_IND,(osapp_msg_handler_t)osapp_gapc_param_update_req_handler},
                {GAPC_LE_PKT_SIZE_IND,    (osapp_msg_handler_t)osapp_gapc_le_pkt_size_ind_handler},              
                {GAPC_GET_DEV_INFO_REQ_IND,(osapp_msg_handler_t)osapp_gapc_get_dev_info_req_ind_handler},
                {GAPM_PROFILE_ADDED_IND,(osapp_msg_handler_t)osapp_gapm_profile_added_ind_handler},
                {BXOTAS_START_REQ_IND,(osapp_msg_handler_t)osapp_bxotas_start_req_ind_handler},
                {BXOTAS_FINISH_IND,(osapp_msg_handler_t)osapp_bxotas_finish_ind_handler},


};
 osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);

void user_init()
{
    osapp_svc_manager_init();
    ahi_handler_register(&handler_info);
    mesh_sched_init();
};

