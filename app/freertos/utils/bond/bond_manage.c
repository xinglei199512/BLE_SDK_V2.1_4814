/**
 ****************************************************************************************
 *
 * @file   bond_manage.c
 *
 * @brief  ble bond manage apis.
 *
 * @author  Hui Chen
 * @date    2018-12-24 10:09
 * @version <0.0.0.1>
 *
 * @license
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "bond_manage.h"
#include <stdlib.h>
#include <stdbool.h>
#include "prf_types.h"
#include "gattc_task.h"
#include "co_utils.h"
#include "log.h"
#include "prf_utils.h"
#include "osapp_utils.h"
#include "osapp_task.h"
//bond file
#include "bond_save.h"
#include "bond_save_security.h"
#include "bond_save_flash.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/*
 * ENUMERATIONS
 ****************************************************************************************
 */
enum
{
    //0x01 to accept request, 0x00 to reject request
    BOND_MG_req_reject = 0x00,
    BOND_MG_req_accept = 0x01,
};
/// bond manage iocap dispaly or input
//enum
//{
//    BOND_MG_iocop_display = 0x00,
//    BOND_MG_iocop_input = 0x01,
//};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    struct co_list_hdr hdr;
    sign_counter_t      sign_counter;
    struct gap_bdaddr bdaddr;
    enum gap_role role;
    enum gap_lk_sec_lvl sec_lvl;
    bond_node_id_t      id;
}bond_manage_dev_info_t;


//the friend ship about [connect index] with  [bond save id]
// beacuse the conn_index is [0~max],so array index can use the conn_index to search bond_id
typedef struct
{
    bond_manage_dev_cfg_t       cfg;
    struct co_list bonding_list;
    bond_manage_dev_info_t      dev_info[BOND_MG_MAX_CONNECT_NUM];
    reslove_result_cb_t         reslove_addr_cb;
    uint32_t                    static_password;//user static password ,//set static password
    bond_security_info_t        cur_bond;
    uint8_t                     cur_conn_index;//current active connect
}bond_manage_database_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
 //CCCD
static void osapp_gattc_svc_chg_cfg_ind_handler(ke_msg_id_t const msgid,struct gattc_svc_changed_cfg const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);


static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_gapc_conn_confirm(uint8_t conn_index,bond_node_id_t id);
//=============bond own handler
static void osapp_gapc_cmp_evt_handler(ke_msg_id_t const msgid, struct gapc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_conn_req_ind_handler(ke_msg_id_t const msgid, struct gapc_connection_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_gapc_disconnect_ind_handler(ke_msg_id_t const msgid, struct gapc_disconnect_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
//static void osapp_gattc_cmp_evt_handler(ke_msg_id_t const msgid,struct gattc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t src_id);
static void osapp_gapm_addr_solved_ind_handler(ke_msg_id_t const msgid, struct gapm_addr_solved_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_gapc_security_ind_handler(ke_msg_id_t const msgid, struct gapc_security_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_bond_req_ind_handler(ke_msg_id_t const msgid, struct gapc_bond_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_bond_ind_handler(ke_msg_id_t const msgid, struct gapc_bond_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,struct gapc_encrypt_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_encrypt_ind_handler(ke_msg_id_t const msgid,struct gapc_encrypt_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
//=============bond  function
static void bond_manage_gapc_bond_cmd(uint8_t conn_idx);
static void bond_manage_bond_process_connect(uint8_t conn_idx);
static void bond_manage_pair_result_to_save(uint8_t result,uint8_t conn_idx);
static void bond_save_recover_complete(bool success , bond_node_id_t id);
static bool bond_manage_gapc_pairing_params_verify( struct gapc_pairing *p_pairing_feat);
static void bond_manage_generate_pw_num(uint8_t *p_num);
static void bond_manage_generate_ltk(struct gapc_ltk *p_ltk_local,uint8_t keysize);
static void bond_manage_generate_csrk(struct gap_sec_key *p_csrk_local);
static void bond_manage_generate_legacy_oob(uint8_t *p_num);
static void bond_manage_bond_init_start(uint8_t conn_idx);

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

static bond_manage_database_t l_bond=
{
    .static_password = BOND_MG_INVALID_PASSWORD,
    .reslove_addr_cb = NULL,
};


static osapp_msg_handler_table_t const handler_table[]=
{
    //gapm
            ///GAPM event complete
            {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
            //gapm addr solved ind
            {GAPM_ADDR_SOLVED_IND,(osapp_msg_handler_t)osapp_gapm_addr_solved_ind_handler},
    //gapc
            ///This is the generic complete event for GAP operations. All operation triggers this event when operation is finished.
            {GAPC_CMP_EVT,(osapp_msg_handler_t)osapp_gapc_cmp_evt_handler},
            ///connection indicate: receive connect request from master
            {GAPC_CONNECTION_REQ_IND,(osapp_msg_handler_t)osapp_gapc_conn_req_ind_handler},
            ///connection lost indicate handler
            {GAPC_DISCONNECT_IND,(osapp_msg_handler_t)osapp_gapc_disconnect_ind_handler},
            ///Event Triggered on master side when slave request to have a certain level of authentication.
            {GAPC_SECURITY_IND,(osapp_msg_handler_t)osapp_gapc_security_ind_handler},
            ///Event Triggered during a bonding procedure in order to get:
                ///Slave pairing information
                ///Pairing temporary key (TK)
                ///Key to provide to peer device during key exchange.
                ///This event shall be followed by a GAPC_BOND_CFM message with same request code value.
            {GAPC_BOND_REQ_IND,(osapp_msg_handler_t)osapp_gapc_bond_req_ind_handler},
            ///Event triggered when bonding information is available such as:
                ///Status of the pairing (succeed or failed)
                ///Key exchanged by peer device.
            {GAPC_BOND_IND,(osapp_msg_handler_t)osapp_gapc_bond_ind_handler},
            ///Event Triggered during encryption procedure on slave device in order to retrieve LTK according to random
                ///number and encryption diversifier value.
                ///This event shall be followed by a GAPC_ENCRYPT_CFM message.
            {GAPC_ENCRYPT_REQ_IND,(osapp_msg_handler_t)osapp_gapc_encrypt_req_ind_handler},
            ///Event triggered when encryption procedure succeed, it contains the link authentication level provided
                ///during connection confirmation (see GAPC_CONNECTION_CFM)
            {GAPC_ENCRYPT_IND,(osapp_msg_handler_t)osapp_gapc_encrypt_ind_handler},

            //Event triggered when connection established in IOS
            {GATTC_SVC_CHANGED_CFG_IND,(osapp_msg_handler_t)osapp_gattc_svc_chg_cfg_ind_handler},
};
static osapp_msg_handler_info_t  handler_info = HANDLER_ARRAY_INFO(handler_table);

//declare
/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

static void osapp_gattc_svc_chg_cfg_ind_handler(ke_msg_id_t const msgid,
                                        struct gattc_svc_changed_cfg const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"gattc svc chg ind_cfg = 0x%x\r\n", param->ind_cfg);
}

static void show_csrk(struct gap_sec_key const *csrk)
{
    LOG(LOG_LVL_INFO,"CSRK->\n");
    osapp_utils_log_hex_data(csrk->key,GAP_KEY_LEN);
}


static void show_irk(struct gap_sec_key const *irk)
{
    LOG(LOG_LVL_INFO,"IRK->\n");
    osapp_utils_log_hex_data(irk->key,GAP_KEY_LEN);
}

static void show_bdaddr(struct gap_bdaddr const *addr)
{
    LOG(LOG_LVL_INFO,"BDADDR->\n");
    osapp_utils_log_hex_data(addr->addr.addr,BD_ADDR_LEN);
}

static void show_ltk(struct gapc_ltk const *ltk)
{
    LOG(LOG_LVL_INFO,"LTK->\n");
    osapp_utils_log_hex_data(ltk->ltk.key,GAP_KEY_LEN);
    LOG(LOG_LVL_INFO,"EDIV->\n");
    osapp_utils_log_hex_data((uint8_t *)&ltk->ediv,2);
    LOG(LOG_LVL_INFO,"RANDNB->\n");
    osapp_utils_log_hex_data(ltk->randnb.nb,8);
    LOG(LOG_LVL_INFO,"KEYSIZE-> %d \n",ltk->key_size);
}

static void dev_info_reset(bond_manage_dev_info_t *dev)
{
    memset(dev,0,sizeof(bond_manage_dev_info_t));
    dev->id = BOND_MG_INVALID_ID;
}

/**
 ****************************************************************************************
 * @brief   Func bond_manage_init
 *
 * @param[in] dev_addr   The pointer of bond manage init device config data.
 *
 ****************************************************************************************
 */
ble_bond_err_t bond_manage_init(bond_manage_dev_cfg_t *p_cfg)
{
    LOG(LOG_LVL_INFO,"bond_manage_init\n");

    ble_bond_err_t err = BLE_BOND_SUCCESS;

    if(p_cfg !=NULL)
    {
        if(bond_manage_gapc_pairing_params_verify(&p_cfg->pairing_feat))
        {
            //1.save config data
            l_bond.cfg = *p_cfg;
            //2. bond save init
            bond_save_init();
            //4.init bond id ->connect index
            for(uint8_t i=0;i<BOND_MG_MAX_CONNECT_NUM;i++)
            {
                dev_info_reset(&l_bond.dev_info[i]);
            }
            //5.register handle table
            ahi_handler_register(&handler_info);
        }
        else
        {
            err = BLE_BOND_ERROR_INVALID_PARAM;
        }
    }
    else
    {
        err = BLE_BOND_ERROR_NULL;
    }

    return err;
}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_RESOLV_ADDR:
        if(cmp_evt->status == GAP_ERR_NOT_FOUND)
        {
            LOG(LOG_LVL_WARN,"bond manage  IRK matched fail!!!\n");
            if(l_bond.reslove_addr_cb)  l_bond.reslove_addr_cb(BOND_MG_PAIRING_FAILED,NULL);
        }
        break;
    case GAPM_ADV_UNDIRECT:
    case GAPM_ADV_DIRECT:
    case GAPM_ADV_DIRECT_LDC:
        if(cmp_evt->status == GAP_ERR_NO_ERROR)
        {
            LOG(LOG_LVL_INFO,"slave\n");
            l_bond.dev_info[l_bond.cur_conn_index].role = GAP_ROLE_PERIPHERAL;
            bond_manage_bond_process_connect(l_bond.cur_conn_index);
        }
    break;
    case GAPM_CONNECTION_DIRECT:
    case GAPM_CONNECTION_AUTO:
    case GAPM_CONNECTION_SELECTIVE:
        if(cmp_evt->status == GAP_ERR_NO_ERROR)
        {
            LOG(LOG_LVL_INFO,"master\n");
            l_bond.dev_info[l_bond.cur_conn_index].role = GAP_ROLE_CENTRAL;
            bond_manage_bond_process_connect(l_bond.cur_conn_index);
        }
    break;
    default: break;
    }
}

static void osapp_gapc_conn_req_ind_handler(ke_msg_id_t const msgid, struct gapc_connection_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conn_idx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_INFO,"gapc_conn_req_ind -> conn_idx = %d,param->conhdl = %d \n",conn_idx,param->conhdl);

    if(conn_idx<BOND_MG_MAX_CONNECT_NUM)
    {
        l_bond.dev_info[conn_idx].bdaddr.addr_type   = param->peer_addr_type;
        l_bond.dev_info[conn_idx].bdaddr.addr = param->peer_addr;
        l_bond.dev_info[conn_idx].id = BOND_MG_INVALID_ID;
        l_bond.cur_conn_index = conn_idx;
    }
    else
    {
        LOG(LOG_LVL_ERROR," conn_req conn_idx %d!!!\n",conn_idx);
    }
}

static bond_manage_dev_info_t *secuirity_req_list_head()
{
    struct co_list_hdr *hdr = co_list_pick(&l_bond.bonding_list);
    bond_manage_dev_info_t *dev = NULL;
    if(hdr)
    {
        dev = CONTAINER_OF(hdr, bond_manage_dev_info_t, hdr);
    }
    return dev;
}

static void osapp_gapc_disconnect_ind_handler(ke_msg_id_t const msgid, struct gapc_disconnect_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"bond manage disconnect,reason:%d\n",param->reason);
    uint8_t conn_idx = KE_IDX_GET(src_id);
    BX_ASSERT(conn_idx<BOND_MG_MAX_CONNECT_NUM);
    bond_manage_dev_info_t *head = secuirity_req_list_head();
    co_list_extract(&l_bond.bonding_list, &l_bond.dev_info[conn_idx].hdr,0);
    if(head && (head-l_bond.dev_info == conn_idx))
    {
        bond_manage_dev_info_t *next_head = secuirity_req_list_head();
        if(next_head)
        {
            bond_manage_bond_init_start(next_head - l_bond.dev_info);
        }
    }
    bond_manage_evt_param_t evt_param;
    evt_param.disconnected.reason = param->reason;
    evt_param.disconnected.bond_id = l_bond.dev_info[conn_idx].id;
    evt_param.disconnected.conn_idx = conn_idx;
    if(l_bond.cfg.evt)
    {
        l_bond.cfg.evt(BOND_MG_EVT_DISCONNECTED,&evt_param);
    }
    dev_info_reset(&l_bond.dev_info[conn_idx]);
}

static void osapp_gapm_addr_solved_ind_handler(ke_msg_id_t const msgid, struct gapm_addr_solved_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO , "bond manage IRK matched success!\n");

    if(l_bond.reslove_addr_cb)
    {
        l_bond.reslove_addr_cb(BOND_MG_PAIRING_SUCCEED,param);
    }
}

static void bonding_list_pop(uint8_t conn_idx)
{
    struct co_list_hdr *hdr = co_list_pop_front(&l_bond.bonding_list);
    BX_ASSERT(hdr);
    bond_manage_dev_info_t *dev = CONTAINER_OF(hdr, bond_manage_dev_info_t, hdr);
    BX_ASSERT(dev-l_bond.dev_info == conn_idx);
}

static void bond_procedure_complete(uint8_t conn_idx)
{
    bonding_list_pop(conn_idx);
    bond_manage_dev_info_t *dev = secuirity_req_list_head();
    if(dev)
    {
        bond_manage_bond_init_start(dev-l_bond.dev_info);
    }
}

static void osapp_gapc_cmp_evt_handler(ke_msg_id_t const msgid, struct gapc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"bond mamage gapc cmp evt op=%d,status=%d\n", param->operation,param->status);
    switch(param->operation)
    {
        case GAPC_SECURITY_REQ:
            if(param->status == GAP_ERR_NO_ERROR)
            {
                LOG(LOG_LVL_INFO,"bond mamage GAPC security request completed\n");
            }
            else
            {
                LOG(LOG_LVL_INFO,"bond mamage GAPC security request error=%d\n", param->status);
            }
        break;
        case GAPC_BOND:
        case GAPC_ENCRYPT:
        break;
        default:break;
    }
}

static void bonding_list_add_and_start(uint8_t conn_idx)
{
    bool empty = co_list_is_empty(&l_bond.bonding_list);
    co_list_push_back(&l_bond.bonding_list,&l_bond.dev_info[conn_idx].hdr);
    if(empty)
    {
        bond_manage_bond_init_start(conn_idx);
    }
}

static void osapp_gapc_security_ind_handler(ke_msg_id_t const msgid, struct gapc_security_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"bond mamage gapc_security_ind auth %d\n",param->auth);
    uint8_t conn_idx = KE_IDX_GET(src_id);
    BX_ASSERT(conn_idx<BOND_MG_MAX_CONNECT_NUM);
    bond_manage_master_req_security(conn_idx);
}

static void osapp_gapc_bond_req_ind_handler(ke_msg_id_t const msgid, struct gapc_bond_req_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"gapc_bond_req_ind %d\n",param->request);

    uint8_t conn_idx = KE_IDX_GET(src_id);

    if(conn_idx>=BOND_MG_MAX_CONNECT_NUM)
    {
        LOG(LOG_LVL_ERROR," gapc_bond_req_ind_ conn_idx %d!!!\n",conn_idx);
        return;
    }

    switch(param->request)
    {
        case GAPC_PAIRING_REQ:
        {
            LOG(LOG_LVL_INFO,"GAPC_PAIRING_REQ auth_req:%d \n",param->data.auth_req);
            bonding_list_add_and_start(conn_idx);
        }
        break;
        case GAPC_IRK_EXCH:
        {
            struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM,src_id, gapc_bond_cfm);

            cfm->request = GAPC_IRK_EXCH;
            cfm->accept = BOND_MG_req_accept;

            osapp_utils_param_t param;

            osapp_utils_get_params(OSAPP_UTILS_TYPE_GET_BD_ADDR,&param);
            cfm->data.irk.addr = param.addr;
            osapp_utils_get_params(OSAPP_UTILS_TYPE_GET_IRK,&param);
            cfm->data.irk.irk = param.irk;

            osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
        }
        break;
        case GAPC_LTK_EXCH:
        {
            LOG(LOG_LVL_INFO,"GAPC_LTK_EXCH key_size: %d\n",param->data.key_size);

            struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, src_id, gapc_bond_cfm);

            cfm->request = GAPC_LTK_EXCH;
            cfm->accept = BOND_MG_req_accept;

            struct gapc_ltk *p_ltk_local = (struct gapc_ltk *)&l_bond.cur_bond.ltk_local;

            bond_manage_generate_ltk(p_ltk_local,param->data.key_size);//generate new data
            cfm->data.ltk = *p_ltk_local;
            
            show_ltk(&cfm->data.ltk);

            osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
        }
        break;
        case GAPC_TK_EXCH:
        {
            LOG(LOG_LVL_INFO,"GAPC_TK_EXCH tk_type:%d \n",param->data.tk_type);
            if(l_bond.cfg.evt == NULL)
            {
                struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, src_id, gapc_bond_cfm);
                cfm->request = GAPC_TK_EXCH;
                cfm->accept = BOND_MG_req_reject;
                osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
            }
            else
            {
                switch(param->data.tk_type)
                {
                    ///  - GAP_TK_OOB:       TK get from out of band method
                    case GAP_TK_OOB:
                        {
                            struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, src_id, gapc_bond_cfm);
                            cfm->request = GAPC_TK_EXCH;
                             //evt send

                            bond_manage_evt_param_t evt;

                            evt.legacy_oob.conn_idx = conn_idx;
                            cfm->accept = BOND_MG_req_accept;

                            bond_manage_generate_legacy_oob(evt.legacy_oob.key);
                            memcpy(cfm->data.tk.key,evt.legacy_oob.key,GAP_KEY_LEN);
                            osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);

                             l_bond.cfg.evt(BOND_MG_EVT_LEGACY_OOB,&evt);
                        }
                        break;
                    ///  - GAP_TK_DISPLAY:   TK generated and shall be displayed by local device
                    case GAP_TK_DISPLAY:
                        {
                           struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, src_id, gapc_bond_cfm);

                           cfm->request = GAPC_TK_EXCH;

                            //evt send
                           bond_manage_evt_param_t evt;

                           evt.display_pw.conn_idx =  conn_idx;
                           cfm->accept = BOND_MG_req_accept;

                           if(l_bond.static_password != BOND_MG_INVALID_PASSWORD)
                           {
                               evt.display_pw.key = l_bond.static_password;
                           }
                           else
                           {
                               bond_manage_generate_pw_num((uint8_t *)&evt.display_pw.key);
                           }

                           cfm->data.tk.key[0] = evt.display_pw.key&0xff;
                           cfm->data.tk.key[1] = (evt.display_pw.key>>8)&0xff;
                           cfm->data.tk.key[2] = (evt.display_pw.key>>16)&0xff;

                           osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);

                           l_bond.cfg.evt(BOND_MG_EVT_DISPLAY_PW,&evt);
                        }
                        break;
                    ///  - GAP_TK_KEY_ENTRY: TK shall be entered by user using device keyboard
                    case GAP_TK_KEY_ENTRY:
                        {
                           bond_manage_evt_param_t evt;

                           evt.input_pw.conn_idx =  conn_idx;
                           l_bond.cfg.evt(BOND_MG_EVT_INPUT_PW,&evt);

                        }
                        break;
                    default:break;
                }
            }
        }
        break;
        case GAPC_CSRK_EXCH:
        {
            LOG(LOG_LVL_INFO,"GAPC_CSRK_EXCH\n");
            struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM,src_id, gapc_bond_cfm);

            cfm->request = GAPC_CSRK_EXCH;
            cfm->accept = BOND_MG_req_accept;

            struct gap_sec_key *p_csrk_local = &l_bond.cur_bond.csrk_local;

            bond_manage_generate_csrk(p_csrk_local);//generate new data
            cfm->data.csrk = *p_csrk_local;

            osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
        }
        break;
        case GAPC_NC_EXCH :
        {
            //evt send
            if(l_bond.cfg.evt)
            {
               bond_manage_evt_param_t evt;

               evt.nc_pw.conn_idx = conn_idx;
               memcpy((uint8_t *)&evt.nc_pw.key,param->data.nc_data.value,4);

               l_bond.cfg.evt(BOND_MG_EVT_NC_PW,&evt);
            }
            else
            {
               struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, src_id, gapc_bond_cfm);

               cfm->request = GAPC_NC_EXCH;
               cfm->accept = BOND_MG_req_reject;
               osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
            }
        }
        break;
        case GAPC_OOB_EXCH :
        {
            //TODO:
            //param->data.oob_data.rand
        }
        break;
        default:
            LOG(LOG_LVL_INFO,"No Handler!osapp_gapc_bond_req_ind_handler %d\n",param->request);
            break;
    }
}

static enum gap_lk_sec_lvl bond_manage_gap_auth_to_sec_lvl(uint8_t auth,bool encrypted)
{
    // secure connection bit set, we are on a secure connected link
    if(auth & GAP_AUTH_SEC_CON)
    {
        return GAP_LK_SEC_CON;
    }
    // MITM flag set, authenticated link
    else if (auth & GAP_AUTH_MITM)
    {
        return GAP_LK_AUTH;
    }
    // only bond flag set, or in encrypted link, we are on a unauthenticated link
    else if ((auth & GAP_AUTH_BOND)||encrypted)
    {
        return GAP_LK_UNAUTH;
    }else
    {
        return GAP_LK_NO_AUTH;
    }

}

static void osapp_gapc_bond_ind_handler(ke_msg_id_t const msgid, struct gapc_bond_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"gapc_bond_ind %d\n",param->info);

    uint8_t conn_idx = KE_IDX_GET(src_id);

    if(conn_idx>=BOND_MG_MAX_CONNECT_NUM)
    {
        LOG(LOG_LVL_ERROR," osapp_gapc_bond_ind_handler conn_idx %d!!!\n",conn_idx);
        return;
    }
    switch(param->info)
    {
         case GAPC_CSRK_EXCH:
            show_csrk(&param->data.csrk);
            l_bond.cur_bond.csrk_peer = param->data.csrk;
            l_bond.cur_bond.csrk_present = 1;
            break;
         case GAPC_IRK_EXCH:
            show_irk(&param->data.irk.irk);
            show_bdaddr(&param->data.irk.addr);
            l_bond.cur_bond.irk = param->data.irk.irk;
            l_bond.dev_info[conn_idx].bdaddr = param->data.irk.addr;
            l_bond.cur_bond.irk_present = 1;
            break;
         case GAPC_LTK_EXCH:
            show_ltk(&param->data.ltk);
            l_bond.cur_bond.ltk_peer = param->data.ltk;
            l_bond.cur_bond.ltk_present = 1;
            break;
        case GAPC_PAIRING_SUCCEED:
        case GAPC_PAIRING_FAILED:
            {
                bond_manage_evt_param_t evt;
                evt.pair_result.conn_idx = conn_idx;
                if(param->info == GAPC_PAIRING_SUCCEED)
                {
                    evt.pair_result.success = BOND_MG_PAIRING_SUCCEED;
                    l_bond.cur_bond.auth = param->data.auth.info;
                    l_bond.dev_info[conn_idx].sec_lvl = bond_manage_gap_auth_to_sec_lvl(param->data.auth.info,true);
                    if((l_bond.cur_bond.auth & GAP_AUTH_SEC_CON) == GAP_AUTH_SEC_CON)
                    {
                        memcpy(&l_bond.cur_bond.ltk_local,&l_bond.cur_bond.ltk_peer,sizeof(struct gapc_ltk));
                    }
                    evt.pair_result.u.auth = param->data.auth.info;
                }else
                {
                    evt.pair_result.success = BOND_MG_PAIRING_FAILED;
                    evt.pair_result.u.reason = param->data.reason;
                }
                bond_manage_pair_result_to_save(evt.pair_result.success,conn_idx);
                bond_save_write_through();
                evt.pair_result.bond_id = l_bond.dev_info[conn_idx].id;
                if(l_bond.cfg.evt)
                {
                    l_bond.cfg.evt(BOND_MG_EVT_PAIR_RESULT,&evt);
                }
                bond_procedure_complete(conn_idx);
            }
           break;
        default:
            LOG(LOG_LVL_ERROR,"No Handler!osapp_gapc_bond_ind_handler->ind->info= %d\n",param->info);
            break;
    }
}

static void osapp_gapc_encrypt_req_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_encrypt_req_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"encrypt request ediv=0x%x\n",param->ediv);

    uint8_t conn_idx = KE_IDX_GET(src_id);

    if(conn_idx>=BOND_MG_MAX_CONNECT_NUM)
    {
        LOG(LOG_LVL_ERROR," osapp_gapc_encrypt_req_ind_handler conn_idx %d!!!\n",conn_idx);
        return;
    }

    struct gapc_encrypt_cfm *cfm = AHI_MSG_ALLOC(GAPC_ENCRYPT_CFM, src_id, gapc_encrypt_cfm);
    if(l_bond.dev_info[conn_idx].id !=BOND_MG_INVALID_ID)
    {
        LOG(LOG_LVL_INFO," encrypt found\n");
        struct gapc_ltk ltk;
        if(bond_save_security_get(l_bond.dev_info[conn_idx].id,BONDSAVE_SECURITY_LTK_LOCAL,&ltk) == BLE_BOND_SUCCESS)
        {
            if( (param->ediv == ltk.ediv)
                &&(!memcmp(param->rand_nb.nb,ltk.randnb.nb,GAP_RAND_NB_LEN))
              )
            {
                cfm->found    = true;
                cfm->ltk = ltk.ltk;
                cfm->key_size = ltk.key_size;
            }
            else
            {
                cfm->found = false;
            }
        }else
        {
            cfm->found = false;
        }
    }else
    {
        cfm->found = false;
    }
    osapp_ahi_msg_send(cfm, sizeof(struct gapc_encrypt_cfm),portMAX_DELAY);
}

static void osapp_gapc_encrypt_ind_handler(ke_msg_id_t const msgid,
                                        struct gapc_encrypt_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"gapc_encrypt_ind Auth_level = %d\n",param->auth);

    uint8_t conn_idx = KE_IDX_GET(src_id);

    if(conn_idx>=BOND_MG_MAX_CONNECT_NUM)
    {
        LOG(LOG_LVL_ERROR," osapp_gapc_encrypt_req_ind_handler conn_idx %d!!!\n",conn_idx);
        return;
    }
    l_bond.dev_info[conn_idx].sec_lvl = bond_manage_gap_auth_to_sec_lvl(param->auth,true);

    bond_manage_evt_param_t evt;
    
    evt.encrypt_result.conn_idx = conn_idx;
    evt.encrypt_result.bond_id = l_bond.dev_info[conn_idx].id;
    evt.encrypt_result.auth_level = param->auth;

    //send evt
    if(l_bond.cfg.evt)
    {
        l_bond.cfg.evt(BOND_MG_EVT_ENCRYPT_RESULT,&evt);
    }
}

void bond_manage_initiate_security_procedure(uint8_t conn_idx)
{
    if(l_bond.dev_info[conn_idx].role == GAP_ROLE_PERIPHERAL)
    {
        bond_manage_slave_req_security(conn_idx);
    }else
    {
        bond_manage_master_req_security(conn_idx);
    }
}

static void bond_save_recover_complete(bool success , bond_node_id_t id)
{
    uint8_t conn_index = l_bond.cur_conn_index;

    LOG(LOG_LVL_INFO , "bond_save_recover_complete,success=%d id=%d conn_idx= %d \n",success,id,conn_index);

    if(conn_index<BOND_MG_MAX_CONNECT_NUM)
    {
        //1.save connect index to bond id
        l_bond.dev_info[conn_index].id = id;
        //2.send cfm msg
        osapp_gapc_conn_confirm(conn_index,id);
        //3.send secure req
        if(l_bond.cfg.pair_mode == BOND_MG_PAIR_MODE_INITIATE)
        {
            bond_manage_initiate_security_procedure(conn_index);
        }
        //4.evt send
        if(l_bond.cfg.evt)
        {
            bond_manage_evt_param_t evt;

            evt.connected.result = success;
            evt.connected.conn_idx = conn_index;
            evt.connected.bond_id = id;

            l_bond.cfg.evt(BOND_MG_EVT_CONNECTED,&evt);
        }
    }
//    else
//    {
//        //connect too much, don't cfm.
//    }
}

static void osapp_gapc_conn_confirm(uint8_t conn_index,bond_node_id_t id)
{
    struct gapc_connection_cfm *cfm = AHI_MSG_ALLOC(GAPC_CONNECTION_CFM, KE_BUILD_ID(TASK_ID_GAPC,conn_index), gapc_connection_cfm);

    //1.reset cfm
    //memset(cfm,0,sizeof(struct gapc_connection_cfm));
    //2. set new value
    if(id != BOND_MG_INVALID_ID)
    {
        sign_counter_t sign_count={0};
        bond_handle_t p_param;

        p_param.sign_counter = &sign_count;
        bond_save_param_get(id,BOND_SAVE_TYPE_SIGN_COUNTER,p_param);
        
        cfm->lsign_counter = sign_count.local;
        cfm->rsign_counter = sign_count.peer;
        bond_save_security_get(id,BONDSAVE_SECURITY_CSRK_LOCAL,&cfm->lcsrk);
        bond_save_security_get(id, BONDSAVE_SECURITY_CSRK_PEER, &cfm->rcsrk);
        bond_save_security_get(id, BONDSAVE_SECURITY_AUTH,&cfm->auth);

        //TODO: add svc change,add other key
//        cfm->svc_changed_ind_enable
        struct gapc_ltk ltk;
        if(bond_save_security_get(id,BONDSAVE_SECURITY_LTK_LOCAL,&ltk)!=BLE_BOND_SUCCESS)
        {
            cfm->ltk_present=false;
        }
        else
        {
            cfm->ltk_present=true;
        }

        LOG(LOG_LVL_INFO,"################## gapc_conn_confirm ################\n");
        LOG(LOG_LVL_INFO,"AUTH-> %d \n",cfm->auth);
        LOG(LOG_LVL_INFO,"ltk_present-> %d \n",cfm->ltk_present);
        LOG(LOG_LVL_INFO,"################## gapc_conn_confirm ################\n");
    }

    //3. send
    os_ahi_msg_send(cfm,portMAX_DELAY);
}

/**
 ****************************************************************************************
 * @brief   Func bond_manage [gap slave role] send req msg to[gap master role]
 *
 * @param[in] conn_index  The connect handle index .
 *
 ****************************************************************************************
 */
void bond_manage_slave_req_security(uint8_t conn_index)
{
    BX_ASSERT(l_bond.dev_info[conn_index].role == GAP_ROLE_PERIPHERAL);

    //Normal operation is first connect bond and reconnect don't bond.
    //But if the iPhone forget the bonded device,the mcu will never bond again,this will cause you cannot bond.
    //To avoid this issue,we bond after connect at any time . So it will cause iPhone5C(iOS7) bond fail , but iPhone7(iOS12) is OK.
    struct gapc_security_cmd *cmd = AHI_MSG_ALLOC(GAPC_SECURITY_CMD,KE_BUILD_ID(TASK_ID_GAPC,conn_index),gapc_security_cmd);

    cmd->operation = GAPC_SECURITY_REQ;
    cmd->auth = l_bond.cfg.pairing_feat.auth;

    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void osapp_gapc_encrypt_cmd(uint8_t conn_idx)
{
    struct gapc_encrypt_cmd *cmd = AHI_MSG_ALLOC(GAPC_ENCRYPT_CMD, KE_BUILD_ID(TASK_ID_GAPC,conn_idx), gapc_encrypt_cmd);
    cmd->operation = GAPC_ENCRYPT;
    ble_bond_err_t err = bond_save_security_get(l_bond.dev_info[conn_idx].id,BONDSAVE_SECURITY_LTK_PEER,&cmd->ltk);
    BX_ASSERT(err == BLE_BOND_SUCCESS);
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

void bond_manage_master_req_security(uint8_t conn_index)
{
    BX_ASSERT(conn_index<BOND_MG_MAX_CONNECT_NUM);
    if(l_bond.dev_info[conn_index].id == BOND_MG_INVALID_ID)
    {
        bonding_list_add_and_start(conn_index);
    }else
    {
        osapp_gapc_encrypt_cmd(conn_index);
    }
}

/**
 ****************************************************************************************
 * @brief   Func bond_manage_random_resolve_private_addr
 *
 * @param[in] dev_addr   bd address to resolve irk key.
 * @param[in] key_cnt    irk key count.
 * @param[in] fill_cb    Callback function to fill random private addr irk.
 * @param[in] result_cb  Callback function to send the rusult of resolving random private addr
 *
 * @return    None.
 ****************************************************************************************
 */
void bond_manage_random_resolve_private_addr(bd_addr_t dev_addr,uint8_t key_cnt,fill_rpa_irk_list_cb_t fill_cb,reslove_result_cb_t result_cb)
{
    BX_ASSERT(key_cnt && fill_cb && result_cb);

    uint16_t key_len;
    struct gapm_resolv_addr_cmd *cmd;

    //save callback
    l_bond.reslove_addr_cb = result_cb;
    //resolve
    key_len = key_cnt * sizeof(struct gap_sec_key);
    cmd = AHI_MSG_ALLOC_DYN(GAPM_RESOLV_ADDR_CMD,TASK_ID_GAPM,gapm_resolv_addr_cmd , key_len);
    cmd->operation = GAPM_RESOLV_ADDR;
    cmd->nb_key = key_cnt;
    cmd->addr = dev_addr;
    //fill irks
    fill_cb(cmd->irk);
    //send msg
    osapp_ahi_msg_send(cmd , sizeof(struct gapm_resolv_addr_cmd) + key_len , portMAX_DELAY);
}
/**
 ****************************************************************************************
 * @brief   Func bond_manage_connect_index_to_bond_id
 *
 * @param[in] conn_index  The connect handle index .
 *
 * @return    bond_node_id The bond id,may be BOND_MG_INVALID_ID.
 ****************************************************************************************
 */
bond_node_id_t bond_manage_connect_index_to_bond_id(uint8_t conn_index)
{
    bond_node_id_t id = BOND_MG_INVALID_ID;

    if(conn_index<BOND_MG_MAX_CONNECT_NUM)
    {
        id = l_bond.dev_info[conn_index].id;
    }

    return id;
}

enum gap_lk_sec_lvl bond_manage_get_conn_sec_lvl(uint8_t conn_index)
{
    BX_ASSERT(conn_index<BOND_MG_MAX_CONNECT_NUM);
    return l_bond.dev_info[conn_index].sec_lvl;
}

/**
 ****************************************************************************************
 * @brief   Func bond_manage_action_set
 *
 * @param[in] action    The application send action to bond manage.
 * @param[in] p_param   The pointer of bond manage action  param.
 *
 * @return    The result of bond manage action set.
 ****************************************************************************************
 */
ble_bond_err_t bond_manage_action_set(bond_manage_action_t action,bond_manage_action_param_t *p_param)
{
    ble_bond_err_t err = BLE_BOND_SUCCESS;

    if(p_param != NULL)
    {
        switch(action)
        {
            //input password num
            case BOND_MG_ACTION_KEY_INPUT_PW :
                {
                    struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, KE_BUILD_ID(TASK_ID_GAPC, p_param->input_key.conn_idx), gapc_bond_cfm);

                    //memset((uint8_t *)&cfm,0,sizeof(struct gapc_bond_cfm));

                    cfm->request = GAPC_TK_EXCH;
                    cfm->accept = BOND_MG_req_accept;
                    cfm->data.tk.key[0] = p_param->input_key.key&0xff;
                    cfm->data.tk.key[1] = (p_param->input_key.key>>8)&0xff;
                    cfm->data.tk.key[2] = (p_param->input_key.key>>16)&0xff;

                    osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
                }
                break;
            case BOND_MG_ACTION_NC_PW :
                {
                   struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM, KE_BUILD_ID(TASK_ID_GAPC, p_param->nc_right.conn_idx), gapc_bond_cfm);

                   cfm->request = GAPC_NC_EXCH;
                   cfm->accept = p_param->nc_right.result;

                   osapp_ahi_msg_send(cfm, sizeof(struct gapc_bond_cfm),portMAX_DELAY);
                }
                break;
            //oob info
            case BOND_MG_ACTION_OOB_PW :
                {
                    //todo:
                }
                break;
            //set static password
            case BOND_MG_ACTION_STATIC_PW :
                if(p_param->static_password<1000000)
                {
                    l_bond.static_password = p_param->static_password;
                }
                else
                {
                    err = BLE_BOND_ERROR_INVALID_PARAM;
                }
                break;
            default:
                err = BLE_BOND_ERROR_INVALID_PARAM;
                break;
        }
    }
    else
    {
        err = BLE_BOND_ERROR_NULL;
    }
    return err;
}

//===========================================================================================

static void bond_manage_pair_response(uint8_t conn_idx)
{
    struct gapc_bond_cfm *cfm = AHI_MSG_ALLOC(GAPC_BOND_CFM,
        KE_BUILD_ID(TASK_ID_GAPC,conn_idx), gapc_bond_cfm);
    cfm->request = GAPC_PAIRING_RSP;
    if(l_bond.cfg.pair_mode == BOND_MG_PAIR_MODE_NO_PAIR)
    {
        cfm->accept = BOND_MG_req_reject;
    }
    else
    {
        cfm->accept = BOND_MG_req_accept;
    }
    cfm->data.pairing_feat = l_bond.cfg.pairing_feat;
    bond_manage_evt_param_t evt;
    evt.security_req.p_pairing_feat = &cfm->data.pairing_feat;
    evt.security_req.p_accept = &cfm->accept;
    evt.security_req.conn_idx = conn_idx;
    evt.security_req.role = l_bond.dev_info[conn_idx].role;
    l_bond.cfg.evt(BOND_MG_EVT_SECURITY_REQ,&evt);
    os_ahi_msg_send(cfm, portMAX_DELAY);
}

static void bond_manage_bond_init_start(uint8_t conn_idx)
{
    if(l_bond.dev_info[conn_idx].role == GAP_ROLE_PERIPHERAL)
    {
        bond_manage_pair_response(conn_idx);
    }else
    {
        BX_ASSERT(l_bond.dev_info[conn_idx].id== BOND_MG_INVALID_ID);
        bond_manage_gapc_bond_cmd(conn_idx);
    }
}

/**
 ****************************************************************************************
 * @brief   Func checking whether security gapc pairing parameters are valid.
 *
 * @param[in] p_pairing_feat  The point of security gapc pairing parameters to verify.
 *
 * @return    [true/false] .Whether the security gapc pairing parameters are valid.
 ****************************************************************************************
 */
static bool bond_manage_gapc_pairing_params_verify( struct gapc_pairing *p_pairing_feat)
{
    // NULL check.
    if (p_pairing_feat == NULL)
    {
        return false;
    }

    // IO Capabilities must be one of the valid values from @ref enum gap_io_cap.
    if (p_pairing_feat->iocap >= GAP_IO_CAP_LAST)
    {
        return false;
    }

    // Authentication Requirements of the valid values from @ref enum gap_auth.
    if (p_pairing_feat->auth > GAP_AUTH_REQ_MASK)
    {
        return false;
    }

    /// Device security requirements (minimum security level). (@see gap_sec_req)
    if(p_pairing_feat->sec_req > GAP_SEC1_SEC_CON_PAIR_ENC)
    {
        return false;
    }

    // Key size cannot be below 7 bytes,and key size cannot be above 16 bytes.
    if((p_pairing_feat->key_size<7) ||p_pairing_feat->key_size>16)
    {
        return false;
    }

    return true;
}

/**
 ****************************************************************************************
 * @brief   Func bond_manage_generate_pw_num.
 *
 * @param[out] p_num    The ponit of bond_manage_generate_pw_num.
 *
 ****************************************************************************************
 */
static void bond_manage_generate_pw_num(uint8_t *p_num)
{
    if(p_num)
    {
        uint32_t pw=0;
        osapp_utils_random_generate((uint8_t *)&pw,3);//0~999999(int)
        pw %= 1000000;
        memcpy(p_num,(uint8_t *)&pw,3);
    }
}

static void bond_manage_generate_ltk(struct gapc_ltk *p_ltk_local,uint8_t keysize)
{
    if(p_ltk_local)
    {
        memset((uint8_t *)p_ltk_local,0,sizeof(struct gapc_ltk));

        osapp_utils_random_generate((uint8_t *)&p_ltk_local->ediv,2);
        osapp_utils_random_generate(p_ltk_local->randnb.nb,GAP_RAND_NB_LEN);
        p_ltk_local->key_size = keysize;
        osapp_utils_random_generate(p_ltk_local->ltk.key,keysize);
    }
}

static void bond_manage_generate_csrk(struct gap_sec_key *p_csrk_local)
{
    if(p_csrk_local)
    {
        memset((uint8_t *)p_csrk_local,0,sizeof(struct gap_sec_key));
        osapp_utils_random_generate(p_csrk_local->key,GAP_KEY_LEN);
    }
}

static void bond_manage_generate_legacy_oob(uint8_t *p_num)
{
    if(p_num)
    {
        osapp_utils_random_generate(p_num,GAP_KEY_LEN);
    }
}

static void bond_manage_gapc_bond_cmd(uint8_t conn_idx)
{
    struct gapc_bond_cmd *cmd = AHI_MSG_ALLOC(GAPC_BOND_CMD, KE_BUILD_ID(TASK_ID_GAPC,conn_idx), gapc_bond_cmd);
    cmd->operation = GAPC_BOND;
    cmd->pairing = l_bond.cfg.pairing_feat;
    uint8_t accept = BOND_MG_req_accept;
    bond_manage_evt_param_t evt;
    evt.security_req.p_pairing_feat = &cmd->pairing;
    evt.security_req.p_accept = &accept;
    evt.security_req.conn_idx = conn_idx;
    evt.security_req.role = l_bond.dev_info[conn_idx].role;
    l_bond.cfg.evt(BOND_MG_EVT_SECURITY_REQ,&evt);
    os_ahi_msg_send(cmd,portMAX_DELAY);
}

static void bond_manage_bond_process_connect(uint8_t conn_idx)
{
    bond_save_recover_security(&l_bond.dev_info[conn_idx].bdaddr, bond_save_recover_complete);
}

static void bond_manage_pair_result_to_save(uint8_t result,uint8_t conn_idx)
{
    LOG(LOG_LVL_INFO,"bond_manage_pair_result_to_save result = %d conn_idx = %d\n",result,conn_idx);
    BX_ASSERT(conn_idx<BOND_MG_MAX_CONNECT_NUM);


    if(result == BOND_MG_PAIRING_FAILED)
    {
        if(l_bond.dev_info[conn_idx].id != BOND_MG_INVALID_ID)
        {
            ble_bond_err_t err = bond_save_delete_node(l_bond.dev_info[conn_idx].id);
            LOG(LOG_LVL_INFO," bond_manage_pair_fail delete id= %d!!!\n",l_bond.dev_info[conn_idx].id);
        }
    }
    else
    {
        ble_bond_err_t err = BLE_BOND_SUCCESS;
        if(l_bond.dev_info[conn_idx].id == BOND_MG_INVALID_ID)//new
        {
             err = bond_save_allocate_new_id(&l_bond.dev_info[conn_idx].id);
        }
        if(err == BLE_BOND_SUCCESS)
        {//save
            bond_security_t bond_security = {
                .info = l_bond.cur_bond,
                .bdaddr = l_bond.dev_info[conn_idx].bdaddr,
            };
            //show_bdaddr(&bond_security.bdaddr);
                //show_irk(&bond_security.info.irk);
                //show_ltk(&bond_security.info.ltk_local);
                //show_ltk(&bond_security.info.ltk_peer);
            bond_handle_t save_hdl = {.bond_security = &bond_security};
            err = bond_save_param_set(l_bond.dev_info[conn_idx].id,BOND_SAVE_TYPE_SECURITY,save_hdl);
            //LOG(LOG_LVL_INFO,"ret=0x%x\n",err);
            //save_hdl.sign_counter = &l_bond.dev_info[conn_idx].sign_counter;
            //bond_save_param_set(l_bond.dev_info[conn_idx].id,BOND_SAVE_TYPE_SIGN_COUNTER,save_hdl);
        }else
        {
            LOG(LOG_LVL_INFO," bond_manage_pair_success new id alloc err = %d!!!\n",err);
        }
    }
}

/**
 ****************************************************************************************
 * @brief     If this index is connected
 * @param[in] conn_idx    connection index
 * @return    If this index is connected
 ****************************************************************************************
 */
bool bond_manage_is_connected(uint8_t conn_idx)
{
    bool valid = false;
    for(uint8_t i=0 ; i<GAP_BD_ADDR_LEN ; i++)
    {
        if(l_bond.dev_info[conn_idx].bdaddr.addr.addr[i] != 0)
        {
            valid = true;
            break;
        }
    }
    return valid;
}

enum gap_role bond_manage_get_connection_role(uint8_t conn_idx)
{
    BX_ASSERT(conn_idx<BOND_MG_MAX_CONNECT_NUM);
    return l_bond.dev_info[conn_idx].role;
}

struct gap_bdaddr *bond_manage_get_peer_addr(uint8_t conn_idx)
{
    BX_ASSERT(conn_idx<BOND_MG_MAX_CONNECT_NUM);
    return &l_bond.dev_info[conn_idx].bdaddr;
}

