#define LOG_TAG        "osapp.ancs_client"
#define LOG_LVL        LVL_DBG
#include "osapp_config.h"
#include <stdlib.h>
#include <stdbool.h>
#include "prf_types.h"
#include "gattc_task.h"
#include "co_utils.h"
#include "prf_utils.h"
#include "osapp_utils.h"
#include "ancc_task.h"
#include "bond_save.h"
#include "anc_common.h"
#include "bond_manage.h"
#include "bx_log.h"
#include "gattsc_task.h"
#include "ble_bond_errors.h"


#define APP_ADV_CHMAP 0x7
#define APP_ADV_INT 32
#define ADV_NAME "OSAPP_ANCS_CLIENT"

//slave preferred parameters
#define CONN_INTERVAL_MIN   8
#define CONN_INTERVAL_MAX   10
#define SLAVE_LATENCY       0
#define CONN_TIMEOUT        200
#define PERFORM_ACT_INCOMING_CALL    0 //0:ignore  1:answer  2:decline
extern const uint8_t anc_svc_uuid[];
const uint16_t gatt_svc_16_uuid = ATT_SVC_GENERIC_ATTRIBUTE;

static void osapp_bond_manage_init(void);
static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param);
static void osapp_ancc_get_app_atts(uint16_t length, const uint8_t* value,ke_task_id_t const src_id);
static void osapp_ancc_data_nft_src_enable(enum ancc_anc_descs cccd, uint8_t conn_id);
static void osapp_incomingcall_perform_act(uint32_t nft_uid, ke_task_id_t src, uint8_t act_id);
static void osapp_add_gattsc_task(void);
static void osapp_gattsc_svc_changed_ind_enable(uint8_t conidx);
static void osapp_ancc_enable_req(uint8_t conidx, enum prf_con_type type);


static void osapp_start_advertising(void)
{
    static uint8_t service_uuid_16[] = {0x12, 0x18};
    static uint8_t manu_specific_data[] = {0x01, 0x03, 0x01, 0x03, 0x00, 0xff, 0xcc};
    struct gapm_start_advertise_cmd *cmd = AHI_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,TASK_ID_GAPM, gapm_start_advertise_cmd);
    cmd->op.addr_src    = GAPM_STATIC_ADDR;
    cmd->channel_map    = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT;
    cmd->intv_max = APP_ADV_INT;
    cmd->op.code        = GAPM_ADV_UNDIRECT;
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    cmd->info.host.adv_data_len = ADV_DATA_PACK(cmd->info.host.adv_data,2,\
            GAP_AD_TYPE_MORE_16_BIT_UUID, service_uuid_16, sizeof(service_uuid_16),\
        GAP_AD_TYPE_MANU_SPECIFIC_DATA, manu_specific_data, sizeof(manu_specific_data));
    cmd->info.host.scan_rsp_data_len  = ADV_DATA_PACK(cmd->info.host.scan_rsp_data,1,\
            GAP_AD_TYPE_COMPLETE_NAME, ADV_NAME, sizeof(ADV_NAME));
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static int32_t osapp_disconnect(uint8_t conn_id)
{
    struct gapc_disconnect_cmd *cmd = AHI_MSG_ALLOC(GAPC_DISCONNECT_CMD, KE_BUILD_ID(TASK_ID_GAPC, conn_id),gapc_disconnect_cmd);
    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;
    return os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_gapm_profile_added_ind_handler(ke_msg_id_t const msgid, struct gapm_profile_added_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("GAPM profile added indication,id:%d,nb:%d,hdl:%d",param->prf_task_id,param->prf_task_nb,param->start_hdl);
    if(param->prf_task_id == TASK_ID_ANCC)
    {
        LOG_I("add TASK_ID_ANCC finish");
        osapp_add_gattsc_task();
    }
    if(param->prf_task_id == TASK_ID_GATTSC)
    {
        LOG_I("add TASK_ID_GATTSC finish, start adv");
        osapp_start_advertising();
    }
}

static void osapp_add_ancc_task()
{
    struct gapm_profile_task_add_cmd *req=AHI_MSG_ALLOC(GAPM_PROFILE_TASK_ADD_CMD, TASK_ID_GAPM, gapm_profile_task_add_cmd);
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH,NO_AUTH);
    req->prf_task_id = TASK_ID_ANCC;
    req->app_task = TASK_AHI;
    req->start_hdl = 0;
    os_ahi_msg_send(req,portMAX_DELAY);
}

static void osapp_add_gattsc_task(void)
{
    struct gapm_profile_task_add_cmd *req=AHI_MSG_ALLOC(GAPM_PROFILE_TASK_ADD_CMD, TASK_ID_GAPM, gapm_profile_task_add_cmd);
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH,NO_AUTH);
    req->prf_task_id = TASK_ID_GATTSC;
    req->app_task = TASK_AHI;
    req->start_hdl = 0;
    os_ahi_msg_send(req,portMAX_DELAY);
}


static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_SET_DEV_CONFIG:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_add_ancc_task();
        break;
    default:
        LOG_W("gapm_cmp_evt operation:0x%x status:0x%x",cmp_evt->operation, cmp_evt->status);
        break;
    }
}

static void osapp_gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid, struct gapc_get_dev_info_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapc_get_dev_info_cfm *cfm;
    switch(param->req)
    {
        case GAPC_DEV_NAME:
            cfm = AHI_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,src_id, gapc_get_dev_info_cfm,sizeof(ADV_NAME));
            cfm->req = GAPC_DEV_NAME;
            cfm->info.name.length = sizeof(ADV_NAME);
            memcpy(cfm->info.name.value,ADV_NAME,sizeof(ADV_NAME));
            os_ahi_msg_send(cfm,portMAX_DELAY);
            break;
        case GAPC_DEV_APPEARANCE:
            cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, src_id, gapc_get_dev_info_cfm);
            cfm->req = GAPC_DEV_APPEARANCE;
            cfm->info.appearance = 0;
            os_ahi_msg_send(cfm,portMAX_DELAY);
            break;
        case GAPC_DEV_SLV_PREF_PARAMS:
            cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, src_id, gapc_get_dev_info_cfm);
            cfm->req = GAPC_DEV_SLV_PREF_PARAMS;
            cfm->info.slv_params.con_intv_min  = CONN_INTERVAL_MIN;
            cfm->info.slv_params.con_intv_max  = CONN_INTERVAL_MAX;
            cfm->info.slv_params.slave_latency = SLAVE_LATENCY;
            cfm->info.slv_params.conn_timeout  = CONN_TIMEOUT;
            os_ahi_msg_send(cfm, portMAX_DELAY);
            break;
        default:
            LOG_W("req = 0x%x", param->req);
    }
}

static bool osapp_get_profile_content(uint8_t conidx , uint8_t* char_buff, uint8_t buff_len, const uint8_t* uuid, uint8_t uuid_len)
{
    bond_database_t bd_database;
    memset((void*)&bd_database, 0, sizeof(bond_database_t));
    bd_database.char_buff = char_buff;
    bd_database.length = buff_len;
    bond_handle_t bd_hdl;
    bd_hdl.bond_database = &bd_database;
    memcpy((void*)&bd_database.uuid, (void*)uuid, uuid_len); 
    bond_node_id_t bond_id = bond_manage_connect_index_to_bond_id(conidx);
    return BLE_BOND_SUCCESS == bond_save_param_get(bond_id, BOND_SAVE_TYPE_DATABASE, bd_hdl);
}

static void osapp_ancc_enable_rsp_handler(ke_msg_id_t const msgid, struct ancc_enable_rsp const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I("ancc enable rsp:%d",param->status);
    bond_database_t bd_database;
    bond_handle_t bond_hdl;
    bond_hdl.bond_database = &bd_database;
    struct ancc_anc_content anc;
    memcpy((void*)bd_database.uuid, (void*)anc_svc_uuid, ATT_UUID_128_LEN);
    bd_database.length = sizeof(struct ancc_anc_content);
    bd_database.char_buff = (uint8_t*)&anc;
    bond_node_id_t bond_id = bond_manage_connect_index_to_bond_id(KE_IDX_GET(src_id));

    if(param->status == GAP_ERR_NO_ERROR)
    {
        bond_save_param_get(bond_id, BOND_SAVE_TYPE_DATABASE, bond_hdl);

        LOG_I("ancc svc disc ancc.shdl = 0x%x, ancc_ehdl = 0x%x", param->anc.svc.shdl, param->anc.svc.ehdl);
        if( !memcmp((void*)&anc, (void*)&param->anc, sizeof(struct ancc_anc_content)) )//if equal
        {
            LOG_I("anc content equal");
        }
        else
        {
            LOG_I( "anc content save");
            bd_database.char_buff = (uint8_t*)(&param->anc);
            bond_save_param_set(bond_id, BOND_SAVE_TYPE_DATABASE, bond_hdl);
            bond_save_write_through();
        }
        osapp_ancc_data_nft_src_enable(ANC_DESC_DATA_SRC_CL_CFG, KE_IDX_GET(src_id));
    }
    else
    {
        LOG_W("osapp_ancc_enable_rsp_handler status = 0x%x", param->status);
    }
}

static void osapp_gattsc_enable_rsp_handler(ke_msg_id_t const msgid, struct gattsc_enable_rsp const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I("gattsc enable rsp:%d",param->status);

    bond_database_t bd_database;
    memset((void*)&bd_database, 0, sizeof(bond_database_t));
    bond_handle_t bond_hdl;
    bond_hdl.bond_database = &bd_database;
    struct gatts_client_content gatts;
    memcpy((void*)bd_database.uuid, (void*)&gatt_svc_16_uuid, ATT_UUID_16_LEN);
    bd_database.length = sizeof(struct gatts_client_content);
    bd_database.char_buff = (uint8_t*)&gatts;
    bond_node_id_t bond_id = bond_manage_connect_index_to_bond_id(KE_IDX_GET(src_id));

    if(param->status == GAP_ERR_NO_ERROR)
    {
        bond_save_param_get(bond_id, BOND_SAVE_TYPE_DATABASE, bond_hdl);
        if( !memcmp((void*)&gatts, (void*)&param->gatts, sizeof(struct gatts_client_content)) )//if equal
        {
            LOG_I("gatts content equal");
        }
        else
        {
            LOG_I( "gatts content save");
            bd_database.char_buff = (uint8_t*)(&param->gatts);
            bond_save_param_set(bond_id, BOND_SAVE_TYPE_DATABASE, bond_hdl);
            bond_save_write_through();
        }
        osapp_gattsc_svc_changed_ind_enable(KE_IDX_GET(src_id));
    }
    else
    {
        LOG_W("osapp_ancc_enable_rsp_handler status = 0x%x", param->status);
    }
}


static void osapp_gattsc_svc_chaged_handler(ke_msg_id_t const msgid, struct gattsc_svc_changed_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I("svc change ind start_hdl = 0x%x, endhdl = 0x%x", param->start_hdl, param->end_hdl);
    if(bond_manage_get_conn_sec_lvl(KE_IDX_GET(src_id)) > GAP_LK_NO_AUTH)
    {
        LOG_I("security lel  %d", bond_manage_get_conn_sec_lvl(KE_IDX_GET(src_id)));
        struct ancc_anc_content anc;
        if(osapp_get_profile_content(KE_IDX_GET(src_id), (uint8_t*)&anc, sizeof(struct ancc_anc_content), anc_svc_uuid, ATT_UUID_128_LEN))
        {
            LOG_I("anc.shdl = 0x%x, anc.ehdl = 0x%x", anc.svc.shdl, anc.svc.ehdl);
            if(!(param->end_hdl < anc.svc.shdl || param->start_hdl > anc.svc.ehdl))
            {
                LOG_I("ancc svc chagned, try disc svc again");
                osapp_ancc_enable_req(KE_IDX_GET(src_id), PRF_CON_DISCOVERY);
            }
        }
    }
}

static void osapp_ancc_data_nft_src_enable(enum ancc_anc_descs cccd, uint8_t conn_id)
{
    struct ancc_cl_cfg_ntf_en_cmd *cmd = AHI_MSG_ALLOC(ANCC_CL_CFG_NTF_EN_CMD,KE_BUILD_ID(TASK_ID_ANCC, conn_id),ancc_cl_cfg_ntf_en_cmd);
    cmd->cfg = cccd;
    cmd->enable = true;
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_ancc_enable_req(uint8_t conidx, enum prf_con_type type)
{
    struct ancc_enable_req *req = AHI_MSG_ALLOC(ANCC_ENABLE_REQ,KE_BUILD_ID(TASK_ID_ANCC,conidx),ancc_enable_req);
    req->con_type = type;
    if(type == PRF_CON_NORMAL)
    {
        osapp_get_profile_content(conidx, (uint8_t*)&req->anc, sizeof(struct ancc_anc_content), anc_svc_uuid, ATT_UUID_128_LEN);
    }
    os_ahi_msg_send(req,portMAX_DELAY);
}

static void osapp_gattsc_enable_req(uint8_t conidx, enum prf_con_type type)
{
    struct gattsc_enable_req *req = AHI_MSG_ALLOC(GATTSC_ENABLE_REQ,KE_BUILD_ID(TASK_ID_GATTSC,conidx),gattsc_enable_req);
    req->con_type = type;
    if(type == PRF_CON_NORMAL)
    {
        osapp_get_profile_content(conidx, (uint8_t*)&req->gatts, sizeof(struct gatts_client_content), (uint8_t*)&gatt_svc_16_uuid, ATT_UUID_16_LEN);
    }
    os_ahi_msg_send(req,portMAX_DELAY);
}

static void osapp_ancc_cmp_evt_handler(ke_msg_id_t const msgid, struct ancc_cmp_evt const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        case ANCC_NTF_SRC_CL_CFG_NTF_ENABLING:
            break;
        case ANCC_DATA_SRC_CL_CFG_NTF_ENABLING:
            osapp_ancc_data_nft_src_enable(ANC_DESC_NTF_SRC_CL_CFG, KE_IDX_GET(src_id));
            break;
        case ANCC_NTF_ATTS_REQUESTING:
        case ANCC_APP_ATTS_REQUESTING:
        case ANCC_NTF_ACTION_REQUESTING:
            break;
        default:
            LOG_I( "ancc cmp evt: operation = 0x%x", param->operation);
            break;
    }
}

static void osapp_gattsc_cmp_evt_handler(ke_msg_id_t const msgid, struct gattsc_cmp_evt const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I( "gattsc cmp evt: operation = 0x%x status = 0x%x", param->operation, param->status);
}

static void osapp_ancc_ntf_att_ind_handler(ke_msg_id_t const msgid, struct ancc_att_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t i = 0;
    switch(param->att_id)
    {
        case NTF_ATT_ID_APP_ID:
            LOG_I("appid = %s", param->val);
            osapp_ancc_get_app_atts(param->length, param->val, src_id);
            break;
        case NTF_ATT_ID_TITLE:
            LOG_I("att id tile:");
            for(i = 0; i < param->length; i++)
            {
                LOG_RAW("0x%02x ", param->val[i]);
            }
            LOG_RAW("\r\n");
            break;
        case NTF_ATT_ID_MSG:
            LOG_I("att id msg:");
            for(i = 0; i < param->length; i++)
            {
                LOG_RAW("0x%02x ", param->val[i]);
            }
            LOG_RAW("\r\n");
            break;
        case NTF_ATT_ID_POS_ACTION_LABEL:
            if(param->length > 0)
            {
                LOG_I("positive action label:");
                for(i = 0; i < param->length; i++)
                {
                    LOG_RAW("0x%02x ", param->val[i]);
                }
                LOG_RAW("\r\n");
            }
            break;
        case NTF_ATT_ID_NEG_ACTION_LABEL:
            if(param->length > 0)
            {
                LOG_I("negative action label:");
                for(i = 0; i < param->length; i++)
                {
                    LOG_RAW("0x%02x ", param->val[i]);
                }
                LOG_RAW("\r\n");
            }
            break;
        default:
            LOG_I("osapp_ancc_ntf_att_ind_handler defalut: 0x%x", param->att_id);
            break;
    }
}

static void osapp_gattsc_svc_changed_ind_enable(uint8_t conidx)
{
    struct gattsc_svc_changed_ind_cfg_cmd *cmd = AHI_MSG_ALLOC(GATTSC_SVC_CHANGED_IND_CFG_CMD,KE_BUILD_ID(TASK_ID_GATTSC,conidx),gattsc_svc_changed_ind_cfg_cmd);
    cmd->enable = true;
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_ancc_app_att_ind_handler(ke_msg_id_t const msgid, struct ancc_att_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t i;
    LOG_I("app att val:");
    for(i = 0; i < param->length; i++)
    {
        LOG_RAW("0x%02x ", param->val[i]);
    }
    LOG_RAW("\r\n");
}

static void osapp_ancc_get_ntf_atts(uint8_t catid,uint32_t ntf_uid,ke_task_id_t const src_id)
{
    struct ancc_get_ntf_atts_cmd *cmd = AHI_MSG_ALLOC(ANCC_GET_NTF_ATTS_CMD,src_id,ancc_get_ntf_atts_cmd);
    cmd->ntf_uid = ntf_uid;
    cmd->att_mask =  1 << NTF_ATT_ID_APP_ID| 1 << NTF_ATT_ID_TITLE | 1 << NTF_ATT_ID_MSG | 1 << NTF_ATT_ID_POS_ACTION_LABEL | 1 << NTF_ATT_ID_NEG_ACTION_LABEL;
    cmd->title_length =  20;
    cmd->subtitle_length = 0 ;
    cmd->message_length =  20;
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_ancc_get_app_atts(uint16_t length, const uint8_t* value,ke_task_id_t const src_id)
{
    BX_ASSERT(length>0 && length < ANC_APP_ID_MAX_LENGTH);
    struct ancc_get_app_atts_cmd *cmd = AHI_MSG_ALLOC(ANCC_GET_APP_ATTS_CMD,src_id,ancc_get_app_atts_cmd);
    cmd->att_mask =  1<<NTF_ATT_ID_APP_ID;
    memcpy(cmd->app_id.str,value,length);
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_incomingcall_perform_act(uint32_t nft_uid, ke_task_id_t src, uint8_t act_id)
{
    struct ancc_perform_ntf_action_cmd *cmd = AHI_MSG_ALLOC(ANCC_PERFORM_NTF_ACTION_CMD,src,ancc_perform_ntf_action_cmd);
    cmd->ntf_uid = nft_uid;
    cmd->action_id = act_id;
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_ancc_ntf_src_ind_handler(ke_msg_id_t const msgid, struct anc_ntf_src const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I("evt_id=0x%x, evt_flg=0x%x, cat_id=0x%x, cat_cnt=0x%x, ntf_uid=0x%x", 
          param->event_id, param->event_flags, param->cat_id, param->cat_cnt, param->ntf_uid
         );
    if(param->cat_id == CAT_ID_SOCIAL)
    {
        LOG_I("CAT_ID_SOCIAL");
        osapp_ancc_get_ntf_atts(param->cat_id, param->ntf_uid, src_id);
    }
    else if(param->cat_id == CAT_ID_CALL)//incoming call
    {
        
        #if (PERFORM_ACT_INCOMING_CALL == 0)
        if(param->event_id == EVT_ID_NTF_ADDED)
        {
            LOG_I("incoming call");
            osapp_ancc_get_ntf_atts(param->cat_id, param->ntf_uid, src_id);
        }
        #endif
        
        #if(PERFORM_ACT_INCOMING_CALL == 1)
        if(param->event_id == EVT_ID_NTF_ADDED)
        {
            if(param->event_flags & EVT_FLAG_POSITIVE_ACTION)
            {
                LOG_I( "answer incoming call");
                osapp_incomingcall_perform_act(param->ntf_uid , src_id, ACT_ID_POSITIVE);
            }
        }
        #endif
        
        #if(PERFORM_ACT_INCOMING_CALL == 2)
        if(param->event_id == EVT_ID_NTF_ADDED)
        {
            if(param->event_flags & EVT_FLAG_NEGATIVE_ACTION)
            {
                LOG_I("decline incoming call");
                osapp_incomingcall_perform_act(param->ntf_uid , src_id, ACT_ID_NEGATIVE);
            }
        }
        #endif
        
    }
}



static osapp_msg_handler_table_t const handler_table[]=
{
//gapm
    {GAPM_PROFILE_ADDED_IND,(osapp_msg_handler_t)osapp_gapm_profile_added_ind_handler},
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
    {GAPC_GET_DEV_INFO_REQ_IND,(osapp_msg_handler_t)osapp_gapc_get_dev_info_req_ind_handler},
//profile
    {ANCC_ENABLE_RSP,(osapp_msg_handler_t)osapp_ancc_enable_rsp_handler},
    {ANCC_CMP_EVT,(osapp_msg_handler_t)osapp_ancc_cmp_evt_handler},
    {ANCC_NTF_ATT_IND,(osapp_msg_handler_t)osapp_ancc_ntf_att_ind_handler},
    {ANCC_APP_ATT_IND,(osapp_msg_handler_t)osapp_ancc_app_att_ind_handler},
    {ANCC_NTF_SRC_IND,(osapp_msg_handler_t)osapp_ancc_ntf_src_ind_handler},
//gattsc
    {GATTSC_ENABLE_RSP,(osapp_msg_handler_t)osapp_gattsc_enable_rsp_handler},
    {GATTSC_SVC_CHANGED_IND,(osapp_msg_handler_t)osapp_gattsc_svc_chaged_handler},
    {GATTSC_CMP_EVT,(osapp_msg_handler_t)osapp_gattsc_cmp_evt_handler},

};
static osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);

void user_init()
{
    osapp_utils_set_dev_init(GAP_ROLE_PERIPHERAL,GAPM_CFG_ADDR_PUBLIC);
    osapp_bond_manage_init();
    ahi_handler_register(&handler_info);
}

static void osapp_bond_manage_init(void)
{
    bond_manage_dev_cfg_t cfg=
    {
          .evt = bond_manage_evt_cb,
          .pair_mode = BOND_MG_PAIR_MODE_INITIATE,
          .pairing_feat = {
              /// IO capabilities (@see gap_io_cap)
              .iocap = GAP_IO_CAP_DISPLAY_ONLY,//GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
              /// OOB information (@see gap_oob)
              .oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
              /// Authentication (@see gap_auth)
              /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
              /// and 'Secure Connections'.
              .auth = GAP_AUTH_REQ_MITM_BOND,
              /// Encryption key size (7 to 16)
              .key_size = 16,
              ///Initiator key distribution (@see gap_kdist)
              .ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
              ///Responder key distribution (@see gap_kdist)
              .rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY,
              /// Device security requirements (minimum security level). (@see gap_sec_req)
              .sec_req = GAP_SEC1_AUTH_PAIR_ENC,
          },
    };

    ble_bond_err_t ret = bond_manage_init(&cfg);
    BX_ASSERT(ret == BLE_BOND_SUCCESS);

}



static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param)
{
    LOG_I("bond_manage_evt_cb evt: %d",evt);

        switch(evt)
        {
            case BOND_MG_EVT_CONNECTED :
                {
                    LOG_I("bond_manage_evt_cb BOND_MG_EVT_CONNECTED");
                    if(p_param->connected.bond_id != BOND_MG_INVALID_ID)
                    {
                        LOG_I("have bonded");
                        osapp_gattsc_enable_req(p_param->connected.conn_idx, PRF_CON_NORMAL);
                    }
                }
                break;
            case BOND_MG_EVT_DISCONNECTED:
                {
                    LOG_I("user gapc_disconnect_ind: reason = 0x%x",p_param->disconnected.reason);
                    osapp_start_advertising();
                }
                break;
            case BOND_MG_EVT_DISPLAY_PW :
                {
                    bond_mg_evt_param_display_pw_t display_pw = p_param->display_pw;
                    LOG_I("BOND_MG_EVT_DISPLAY_PW key: %06d ,conn_idx: %d",display_pw.key,display_pw.conn_idx);
                }
                break;
            case BOND_MG_EVT_PAIR_RESULT :
                {
                    bond_mg_evt_param_pair_result_t pair_result = p_param->pair_result;
                    
                    
                    LOG_I("BOND_MG_EVT_PAIR_RESULT conn_idx: %d",pair_result.conn_idx);
                    LOG_I("BOND_MG_EVT_PAIR_RESULT bond_id: %d",pair_result.bond_id);

                    if(p_param->pair_result.success == BOND_MG_PAIRING_SUCCEED)
                    {
                        LOG_I("BOND_MG_PAIRING_SUCCEED ");
                        osapp_ancc_enable_req(pair_result.conn_idx, PRF_CON_DISCOVERY);
                        osapp_gattsc_enable_req(pair_result.conn_idx, PRF_CON_DISCOVERY);

                    }
                    else
                    {
                        LOG_I("BOND_MG_PAIRING_FAILED reason= %d ",pair_result.u.reason);

                        if(pair_result.u.reason == SMP_ERROR_REM_UNSPECIFIED_REASON)
                        {
                            //if you send a ecurity request in paired device , iOS7 will occur this error , should ignore.
                            LOG_I("SMP_ERROR_REM_UNSPECIFIED_REASON.(iOS7 ignore)");
                        }
                        else
                        {
                            osapp_disconnect(pair_result.conn_idx);
                        }
                    }
                }
                break;
            case BOND_MG_EVT_ENCRYPT_RESULT :
                {
                    bond_mg_evt_param_encrypt_result_t  encrypt_result = p_param->encrypt_result;

                    osapp_ancc_enable_req(encrypt_result.conn_idx, PRF_CON_NORMAL);

                    LOG_I("BOND_MG_EVT_ENCRYPT_RESULT conn_idx: %d,bond id: %d, auth_lvl: %d",
                        encrypt_result.conn_idx,encrypt_result.bond_id,encrypt_result.auth_level);
                }
                break;
            default:
                break;
        }

}



