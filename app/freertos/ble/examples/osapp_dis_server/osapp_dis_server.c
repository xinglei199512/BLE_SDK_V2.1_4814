#define LOG_TAG        "osapp.dis_server"
#define LOG_LVL        LVL_DBG
#include "osapp_config.h"
#include "diss_task.h"
#include "bxotas_task.h"
#include "app_uart.h"
#include "log.h"
#include "osapp_utils.h"
#include "bond_manage.h"


#define APP_ADV_CHMAP 0x7
//#define APP_ADV_INT_MIN 32 //20 ms
//#define APP_ADV_INT_MAX 32 //20 ms
#define APP_ADV_INT_MIN 160 //100 ms
#define APP_ADV_INT_MAX 160 //100 ms
//#define APP_ADV_INT_MIN 2048 //1.28 s
//#define APP_ADV_INT_MAX 2048 //1.28 s
#define CONN_INTV 40 //50ms

/// Manufacturer Name Value
#define APP_DIS_MANUFACTURER_NAME       ("Apollo")
#define APP_DIS_MANUFACTURER_NAME_LEN   (sizeof(APP_DIS_MANUFACTURER_NAME))

/// ADV Data
#define APP_DIS_ADV_DATA                ("Apollo Dis Server")

/// Model Number String Value
#define APP_DIS_MODEL_NB_STR            ("RW-BLE-1.0")
#define APP_DIS_MODEL_NB_STR_LEN        (10)

/// Serial Number
#define APP_DIS_SERIAL_NB_STR           ("1.0.0.0-LE")
#define APP_DIS_SERIAL_NB_STR_LEN       (10)

/// Firmware Revision
#define APP_DIS_FIRM_REV_STR            ("6.1.2")
#define APP_DIS_FIRM_REV_STR_LEN        (5)

/// System ID Value - LSB -> MSB
#define APP_DIS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define APP_DIS_SYSTEM_ID_LEN           (8)

/// Hardware Revision String
#define APP_DIS_HARD_REV_STR           ("1.0.0")
#define APP_DIS_HARD_REV_STR_LEN       (5)

/// Software Revision String
#define APP_DIS_SW_REV_STR              ("6.3.0")
#define APP_DIS_SW_REV_STR_LEN          (5)

/// IEEE
#define APP_DIS_IEEE                    ("\xFF\xEE\xDD\xCC\xBB\xAA")
#define APP_DIS_IEEE_LEN                (6)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementer’s Forum assigned Vendor ID value)
 *      Vendor ID : 0x045E      (Microsoft Corp)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 */
#define APP_DIS_PNP_ID               ("\x02\x5E\x04\x40\x00\x00\x03")
#define APP_DIS_PNP_ID_LEN           (7)

static int32_t osapp_add_dis_server_task()
{
    struct diss_db_cfg* db_cfg;
    // Allocate the DISS_CREATE_DB_REQ
    struct gapm_profile_task_add_cmd *req = AHI_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,TASK_ID_GAPM, gapm_profile_task_add_cmd, sizeof(struct diss_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH, NO_AUTH);
    req->prf_task_id = TASK_ID_DISS;
    req->app_task = TASK_AHI;
    req->start_hdl = 0;

    // Set parameters 
    db_cfg = (struct diss_db_cfg* ) req->param;
    db_cfg->features = DIS_ALL_FEAT_SUP;
    return os_ahi_msg_send(req,portMAX_DELAY);

}

static int32_t osapp_add_bxotas_task()
{
    struct gapm_profile_task_add_cmd *req=AHI_MSG_ALLOC(GAPM_PROFILE_TASK_ADD_CMD, TASK_ID_GAPM, gapm_profile_task_add_cmd);
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH,NO_AUTH);
    req->prf_task_id = TASK_ID_BXOTAS;
    req->app_task = TASK_AHI;
    req->start_hdl = 0;
    return os_ahi_msg_send(req,portMAX_DELAY);
}

static int32_t osapp_start_advertising()
{
        // Prepare the GAPM_START_ADVERTISE_CMD message
    struct gapm_start_advertise_cmd *cmd = AHI_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,TASK_ID_GAPM, gapm_start_advertise_cmd);

    cmd->op.addr_src    = GAPM_STATIC_ADDR;
    cmd->channel_map    = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT_MIN;
    cmd->intv_max = APP_ADV_INT_MAX;
    cmd->op.code        = GAPM_ADV_UNDIRECT;
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    cmd->info.host.adv_data_len = ADV_DATA_PACK(cmd->info.host.adv_data,1,GAP_AD_TYPE_COMPLETE_NAME,\
            APP_DIS_ADV_DATA,sizeof(APP_DIS_ADV_DATA));
            // Flag value is set by the GAP
    cmd->info.host.adv_data_len       = ADV_DATA_LEN - 3;
    cmd->info.host.scan_rsp_data_len  = SCAN_RSP_DATA_LEN;
//    nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,                    &cmd->info.host.adv_data[0]) ;
//    nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,                      &cmd->info.host.scan_rsp_data[0]);
    return os_ahi_msg_send(cmd,portMAX_DELAY);
        
}

static int32_t osapp_get_dev_name(ke_task_id_t const dest_id)
{
    nvds_tag_len_t device_name_length = NVDS_LEN_DEVICE_NAME;
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm, device_name_length);
    cfm->req = GAPC_DEV_NAME;
    nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, cfm->info.name.value);
    cfm->info.name.length = device_name_length;
    return os_ahi_msg_send(cfm, portMAX_DELAY);
}

static int32_t osapp_get_dev_appearance(ke_task_id_t const dest_id)
{
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm);
    cfm->req = GAPC_DEV_APPEARANCE;
    cfm->info.appearance = 0;
    return os_ahi_msg_send(cfm, portMAX_DELAY);
}

static int32_t osapp_get_dev_slv_pref_params(ke_task_id_t const dest_id)
{
    struct gapc_get_dev_info_cfm *cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, dest_id,gapc_get_dev_info_cfm);
    cfm->req = GAPC_DEV_SLV_PREF_PARAMS;
    cfm->info.slv_params.con_intv_min = 8;
    cfm->info.slv_params.con_intv_max = 10;
    cfm->info.slv_params.slave_latency = 0;
    cfm->info.slv_params.conn_timeout = 200;
    return os_ahi_msg_send(cfm, portMAX_DELAY);
}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{    
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_SET_DEV_CONFIG:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_add_dis_server_task();
        break;
    case GAPM_PROFILE_TASK_ADD:
        LOG_I("gapm cmp evt profile task add,status:0x%x",cmp_evt->status);
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        break;
    case GAPM_ADV_UNDIRECT:
        LOG_I("gapm_cmp_evt adv_ind,status:0x%x",cmp_evt->status);
        break;
    default:
        LOG_W("gapm_cmp_evt operation:0x%x",cmp_evt->operation);
        break;
    }
}

static void osapp_bxotas_config()
{
    struct bxotas_firmware_dest_cmd *cmd =AHI_MSG_ALLOC(BXOTAS_FIRMWARE_DEST_CMD,TASK_ID_BXOTAS,bxotas_firmware_dest_cmd);
    cmd->firmware_dest = LOCAL_FLASH;
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_gapm_profile_added_ind_handler(ke_msg_id_t const msgid, struct gapm_profile_added_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("GAPM profile added indication,id:%d,nb:%d,hdl:%d",param->prf_task_id,param->prf_task_nb,param->start_hdl);
    if(param->prf_task_id == TASK_ID_DISS)
    {
        osapp_add_bxotas_task();

    }
    if(param->prf_task_id==TASK_ID_BXOTAS)
    {
        osapp_bxotas_config();
        osapp_start_advertising();
    }
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

static void osapp_diss_value_req_ind_handler(ke_msg_id_t const msgid,struct diss_value_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    // Initialize length
    uint8_t len = 0;
    // Pointer to the data
    uint8_t *data = NULL;

    // Check requested value
    switch (param->value)
    {
        case DIS_MANUFACTURER_NAME_CHAR:
        {
            // Set information
            len = APP_DIS_MANUFACTURER_NAME_LEN;
            data = (uint8_t *)APP_DIS_MANUFACTURER_NAME;
        } break;

        case DIS_MODEL_NB_STR_CHAR:
        {
            // Set information
            len = APP_DIS_MODEL_NB_STR_LEN;
            data = (uint8_t *)APP_DIS_MODEL_NB_STR;
        } break;

        case DIS_SYSTEM_ID_CHAR:
        {
            // Set information
            len = APP_DIS_SYSTEM_ID_LEN;
            data = (uint8_t *)APP_DIS_SYSTEM_ID;
        } break;

        case DIS_PNP_ID_CHAR:
        {
            // Set information
            len = APP_DIS_PNP_ID_LEN;
            data = (uint8_t *)APP_DIS_PNP_ID;
        } break;

        case DIS_SERIAL_NB_STR_CHAR:
        {
            // Set information
            len = APP_DIS_SERIAL_NB_STR_LEN;
            data = (uint8_t *)APP_DIS_SERIAL_NB_STR;
        } break;

        case DIS_HARD_REV_STR_CHAR:
        {
            // Set information
            len = APP_DIS_HARD_REV_STR_LEN;
            data = (uint8_t *)APP_DIS_HARD_REV_STR;
        } break;

        case DIS_FIRM_REV_STR_CHAR:
        {
            // Set information
            len = APP_DIS_FIRM_REV_STR_LEN;
            data = (uint8_t *)APP_DIS_FIRM_REV_STR;
        } break;

        case DIS_SW_REV_STR_CHAR:
        {
            // Set information
            len = APP_DIS_SW_REV_STR_LEN;
            data = (uint8_t *)APP_DIS_SW_REV_STR;
        } break;

        case DIS_IEEE_CHAR:
        {
            // Set information
            len = APP_DIS_IEEE_LEN;
            data = (uint8_t *)APP_DIS_IEEE;
        } break;

        default:
            BX_ASSERT(0);
            break;
    }

    // Allocate confirmation to send the value
    struct diss_value_cfm *cfm_value = AHI_MSG_ALLOC_DYN(DISS_VALUE_CFM,
            src_id,
            diss_value_cfm,
            len);

    // Set parameters
    cfm_value->value = param->value;
    cfm_value->length = len;
    if (len)
    {
        // Copy data
        memcpy(&cfm_value->data[0], data, len);
    }
    // Send message   
    os_ahi_msg_send(cfm_value, portMAX_DELAY);

}

static void osapp_bxotas_start_cfm(uint8_t status)
{
    struct bxotas_start_cfm *cfm = AHI_MSG_ALLOC(BXOTAS_START_CFM, TASK_ID_BXOTAS, bxotas_start_cfm);
    cfm->status = status;
    os_ahi_msg_send(cfm, portMAX_DELAY);
}

static void osapp_bxotas_start_req_ind_handler(ke_msg_id_t const msgid,struct bxotas_start_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("OTA START:%d,%d",param->conidx,param->segment_data_max_length);
    osapp_bxotas_start_cfm(OTA_REQ_ACCEPTED);
}

static void osapp_bxotas_finish_ind_handler(ke_msg_id_t const msgid,void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("OTA_DONE");
    /* do not reset chip immediately */
    BX_DELAY_US(1000000);
    platform_reset(0);
}

static const osapp_msg_handler_table_t handler_table[]=
{
                {DISS_VALUE_REQ_IND,     (osapp_msg_handler_t)osapp_diss_value_req_ind_handler},
                {GAPM_PROFILE_ADDED_IND,(osapp_msg_handler_t)osapp_gapm_profile_added_ind_handler},
                {GAPC_GET_DEV_INFO_REQ_IND,(osapp_msg_handler_t)osapp_gapc_get_dev_info_req_ind_handler},
                {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
                {BXOTAS_START_REQ_IND,(osapp_msg_handler_t)osapp_bxotas_start_req_ind_handler},
                {BXOTAS_FINISH_IND,(osapp_msg_handler_t)osapp_bxotas_finish_ind_handler},
};
osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);


static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param)
{
    LOG_I("bond_manage_evt_cb evt: %d",evt);
    switch(evt)
    {
        case BOND_MG_EVT_CONNECTED :
            {
                LOG_I("BOND_MG_EVT_CONNECTED");
            }
            break;
        case BOND_MG_EVT_DISCONNECTED :
            {
                LOG_I("disconn:%d",p_param->disconnected.reason);
                osapp_start_advertising();
            }
            break;
        default:
            break;
    }

}

static void dis_server_bond_manage_init(void)
{
    bond_manage_dev_cfg_t cfg=
    {
          .evt = bond_manage_evt_cb,
          .pair_mode = BOND_MG_PAIR_MODE_WAIT_FOR_REQ,
          .pairing_feat = {
              /// IO capabilities (@see gap_io_cap)
              .iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
              /// OOB information (@see gap_oob)
              .oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
              /// Authentication (@see gap_auth)
              /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
              /// and 'Secure Connections'.
              .auth = GAP_AUTH_REQ_NO_MITM_NO_BOND,
              /// Encryption key size (7 to 16)
              .key_size = 16,
              ///Initiator key distribution (@see gap_kdist)
              .ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY ,
              ///Responder key distribution (@see gap_kdist)
              .rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY ,
              /// Device security requirements (minimum security level). (@see gap_sec_req)
              .sec_req = GAP_NO_SEC,
          },
    };
    
    ble_bond_err_t err = bond_manage_init(&cfg);
    BX_ASSERT(err==BLE_BOND_SUCCESS);

//    bond_manage_action_param_t   action;
//
//    action.static_password = 123456;
//    BX_ASSERT(bond_manage_action_set(BOND_MG_ACTION_STATIC_PW,&action) == BLE_BOND_SUCCESS);
}

void user_init()
{

    osapp_utils_set_dev_init(GAP_ROLE_PERIPHERAL,GAPM_CFG_ADDR_PUBLIC);
    dis_server_bond_manage_init();
    ahi_handler_register(&handler_info);

}


