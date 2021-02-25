#define LOG_TAG        "osapp.uart_client"
#define LOG_LVL        LVL_DBG
#include "osapp_config.h"
#include <stdlib.h>
#include <stdbool.h>
#include "prf_types.h"
#include "gattc_task.h"
#include "co_utils.h"
#include "prf_utils.h"
#include "osapp_utils.h"
#include "bond_save.h"
#include "bond_manage.h"
#include "bx_log.h"
#include "ble_bond_errors.h"
#include "prf_types.h"
#include "prf_utils.h"
#include "io_ctrl.h"

#ifdef TEST_MODE_CFG
    #define SLAVE_ADDR {0xff, 0x00, 0xff, 0x00, 0xff, 0x00}     //TEST MODE MAC address , DON'T MODIFY!
#else
    #define SLAVE_ADDR {0xff, 0x01, 0xff, 0x01, 0xff, 0x01}     //NORMAL mac address
#endif

#define UART_CLIENT_MAX_MTU 185
#define UART_SVC_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e}
#define UART_SVC_RX_CHAR_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x02,0x00,0x40,0x6e}
#define UART_SVC_TX_CHAR_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x03,0x00,0x40,0x6e}
#define ATT_DESC_CLIENT_CHAR_CFG_ARRAY {0x02,0x29}

#define CONNECTED_STATE_GPIO 11
//server char
enum uart_service_chars
{
    UART_SERVER_RX,
    UART_SERVER_TX,
    UART_CHAR_MAX,    
};

enum uart_service_descs
{
    UART_TX_DESC_NTF_CFG,
    UART_DESC_MAX,
};

typedef struct 
{
    struct prf_svc svc;
    struct prf_char_inf chars[UART_CHAR_MAX];
    struct prf_char_desc_inf descs[UART_DESC_MAX];
}uart_server_content;

const struct prf_char_uuid128_def uart_service_char[UART_CHAR_MAX] =
{
    [UART_SERVER_RX] = {
        .uuid_len = ATT_UUID_128_LEN,
        .uuid = UART_SVC_RX_CHAR_UUID_128,
        .req_flag = ATT_MANDATORY,
        .prop_mand = ATT_CHAR_PROP_WR|ATT_CHAR_PROP_WR_NO_RESP,
        },
    [UART_SERVER_TX] = {
        .uuid_len = ATT_UUID_128_LEN,
        .uuid = UART_SVC_TX_CHAR_UUID_128,
        .req_flag = ATT_MANDATORY,
        .prop_mand = ATT_CHAR_PROP_NTF,
        },
};

const struct prf_char_desc_uuid128_def uart_service_char_desc[UART_DESC_MAX] = 
{
    [UART_TX_DESC_NTF_CFG] = {ATT_UUID_16_LEN,ATT_DESC_CLIENT_CHAR_CFG_ARRAY,ATT_OPTIONAL,UART_SERVER_TX},
};


const static struct gap_bdaddr slave_add = 
{
    .addr.addr = SLAVE_ADDR,
    .addr_type = 0,
};

const static uint8_t uart_svc_uuid[] = UART_SVC_UUID_128;
static uart_server_content svc_con;
static uint8_t peer_conn_id = 0;

static void osapp_start_connect(const struct gap_bdaddr* paddr)
{
    struct gapm_start_connection_cmd* cmd = AHI_MSG_ALLOC_DYN(GAPM_START_CONNECTION_CMD,
                                                                     TASK_ID_GAPM,
                                                                     gapm_start_connection_cmd, sizeof(struct gap_bdaddr));
    cmd->op.code = GAPM_CONNECTION_SELECTIVE;
    cmd->op.addr_src = GAPM_STATIC_ADDR;
    cmd->op.state = 0;
    cmd->scan_interval = 0x20;
    cmd->scan_window = 0x20;
    cmd->con_intv_min = 400;
    cmd->con_intv_max = 400;
    cmd->con_latency = 0;
    cmd->superv_to = 300;
    cmd->ce_len_min = 0;
    cmd->ce_len_max = 0;
    cmd->nb_peers = 1;
    cmd->peers[0].addr_type = paddr->addr_type;
    memcpy(cmd->peers[0].addr.addr, paddr->addr.addr, GAP_BD_ADDR_LEN);
    os_ahi_msg_send(cmd,portMAX_DELAY);  

}


static void osapp_connection_cfm(const struct gap_bdaddr* paddr)
{
    struct gapm_connection_cfm *cmd = AHI_MSG_ALLOC(GAPM_CONNECTION_CFM, TASK_ID_GAPM, gapm_connection_cfm);

    memcpy(cmd->addr.addr, paddr->addr.addr, GAP_BD_ADDR_LEN);
    cmd->addr_type = paddr->addr_type;
    cmd->con_intv_min = 0x20;
    cmd->con_intv_max = 0x20;
    cmd->con_latency = 0;
    cmd->superv_to = 300;
    cmd->ce_len_min = 0;
    cmd->ce_len_max = 0;
    os_ahi_msg_send(cmd,portMAX_DELAY);  
}

static void osapp_gapm_cancel()
{
    struct gapm_cancel_cmd *cmd = AHI_MSG_ALLOC(GAPM_CANCEL_CMD,TASK_ID_GAPM,gapm_cancel_cmd);
    cmd->operation = GAPM_CANCEL;
    osapp_ahi_msg_send(cmd,sizeof(struct gapm_cancel_cmd),portMAX_DELAY);
}


static void osapp_gapm_scan_adv_report_ind_handler(ke_msg_id_t const msgid, adv_report_t const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("scaned adv addr:");
    for(uint8_t i = 0; i < GAP_BD_ADDR_LEN; i++)
    {
        LOG_RAW("0x%02x ", param->adv_addr.addr[i]);
    }
    LOG_RAW("\r\n");
    if((!memcmp((void*)slave_add.addr.addr, (void*)param->adv_addr.addr, GAP_BD_ADDR_LEN)) && (slave_add.addr_type == param->adv_addr_type))
    {
        LOG_I("MAC match\r\n");
        uint8_t *ad_buf = (uint8_t *)param->data;
        uint8_t ad_data_length = adv_data_extract(&ad_buf , GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID);
        if(ad_data_length)
        {
            LOG_I("UUID match\r\n");
            osapp_connection_cfm(&slave_add);
        }
        else
        {
            LOG_I("UUID not match\r\n");
            osapp_gapm_cancel();
        }
    }
}

static void osapp_sdp_svc_disc_by_uuid(uint16_t conn_id)
{
    static uint16_t seq_num = 0;
    struct gattc_sdp_svc_disc_cmd* cmd  = AHI_MSG_ALLOC_DYN(GATTC_SDP_SVC_DISC_CMD, KE_BUILD_ID(TASK_ID_GATTC, conn_id), gattc_sdp_svc_disc_cmd, ATT_UUID_128_LEN);
    cmd->operation = GATTC_SDP_DISC_SVC;
    cmd->start_hdl        = 0x1;
    cmd->end_hdl          = 0xffff;
    cmd->seq_num = seq_num++;
    cmd->uuid_len = ATT_UUID_128_LEN;
    memcpy((void*)&cmd->uuid[0],&uart_svc_uuid[0],ATT_UUID_128_LEN);
    os_ahi_msg_send(cmd,portMAX_DELAY); 

}

static void osapp_exchange_mtu(uint16_t conn_id)
{
    static uint16_t seq_num = 0;
    struct gattc_exc_mtu_cmd *cmd = AHI_MSG_ALLOC(GATTC_EXC_MTU_CMD, KE_BUILD_ID(TASK_ID_GATTC, conn_id), gattc_exc_mtu_cmd);
    cmd->operation = GATTC_MTU_EXCH;
    cmd->seq_num = seq_num++;
    os_ahi_msg_send(cmd,portMAX_DELAY); 
}

static void osapp_gatt_write(uint8_t op, uint8_t hdl, uint8_t conn_id, uint8_t length, const uint8_t* pdat)// write no respose
{
    static uint16_t seq_num = 0;
    struct gattc_write_cmd* cmd  = AHI_MSG_ALLOC_DYN(GATTC_WRITE_CMD, KE_BUILD_ID(TASK_ID_GATTC, conn_id), gattc_write_cmd, length);
    memset(cmd, 0, (sizeof(struct gattc_write_cmd)+length));
    cmd->operation = op;
    cmd->auto_execute = 1;
    cmd->seq_num = seq_num++;
    cmd->handle = hdl;
    cmd->offset = 0;
    cmd->length = length;
    cmd->cursor = 0;
    memcpy(cmd->value, pdat, length);
    os_ahi_msg_send(cmd,portMAX_DELAY); 
}

static void osapp_event_ind_handler(ke_msg_id_t const msgid, struct gattc_event_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    for(uint8_t i = 0; i < param->length; i++)
    {
        LOG_RAW("%c", param->value[i]);
    }
    LOG_RAW("\r\n");
    //write back to server
    osapp_gatt_write(GATTC_WRITE_NO_RESPONSE, svc_con.chars[UART_SERVER_RX].val_hdl, peer_conn_id, param->length, param->value);
}

static void osapp_sdp_disc_ind_handler(ke_msg_id_t const msgid, struct gattc_sdp_svc_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    prf_extract_svc_uuid128_info(param, UART_CHAR_MAX, uart_service_char, svc_con.chars , UART_DESC_MAX, uart_service_char_desc,svc_con.descs);
    svc_con.svc.shdl = param->start_hdl;
    svc_con.svc.ehdl = param->end_hdl;
    uint8_t value[2];
    co_write16p((&value[0]), 1);
    osapp_gatt_write(GATTC_WRITE, svc_con.descs[UART_TX_DESC_NTF_CFG].desc_hdl, peer_conn_id, sizeof(value), &value[0]);//enable cccd
    
    LOG_I("rx: char_hdl = 0x%x, val_hdl = 0x%x, prop = 0x%x, char_ehdl_off = 0x%x,"
          "tx: char_hdl = 0x%x, val_hdl = 0x%x, prop = 0x%x, char_ehdl_off = 0x%x,"
          "desc_hdl = 0x%x",
          svc_con.chars[UART_SERVER_RX].char_hdl, svc_con.chars[UART_SERVER_RX].val_hdl, svc_con.chars[UART_SERVER_RX].prop, svc_con.chars[UART_SERVER_RX].char_ehdl_off,
          svc_con.chars[UART_SERVER_TX].char_hdl, svc_con.chars[UART_SERVER_TX].val_hdl, svc_con.chars[UART_SERVER_TX].prop, svc_con.chars[UART_SERVER_TX].char_ehdl_off,
          svc_con.descs[UART_TX_DESC_NTF_CFG].desc_hdl);

}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        case GAPM_SET_DEV_CONFIG:
            BX_ASSERT(param->status==GAP_ERR_NO_ERROR);
            osapp_start_connect(&slave_add);
            break;
        case GAPM_CONNECTION_SELECTIVE:
            LOG_I("GAPM_CONNECTION_SELECTIVE status = 0x%x", param->status);
            if(GAP_ERR_CANCELED == param->status)
            {
                osapp_start_connect(&slave_add);
            }
            else
            {
                osapp_sdp_svc_disc_by_uuid(peer_conn_id);
            }
            break;
        default:
            LOG_W("gapm_cmp_evt operation:0x%x",param->operation);
            break;
    }
}

static void osapp_gattc_cmp_evt_handler(ke_msg_id_t const msgid, struct gapc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        case GAPC_DISCONNECT:
        case GATTC_SDP_DISC_SVC:
        case GATTC_WRITE:
        case GATTC_WRITE_NO_RESPONSE:
            break;
        default:
            LOG_W("gattc_cmp_evt operation:0x%x",param->operation);
            break;
    }
}

static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param)
{
    LOG_I("bond_manage_evt_cb evt: %d",evt);

        switch(evt)
        {
            case BOND_MG_EVT_CONNECTED :
                {
                    LOG_I("BOND_MG_EVT_CONNECTED conn_hdl=%d", p_param->connected.conn_idx);
                    peer_conn_id = p_param->connected.conn_idx;
                    io_pin_set(CONNECTED_STATE_GPIO);
                    osapp_exchange_mtu(p_param->connected.conn_idx);
                }
                break;
            case BOND_MG_EVT_DISCONNECTED:
                {
                    LOG_I("disconn:0x%x",p_param->disconnected.reason);
                    io_pin_clear(CONNECTED_STATE_GPIO);
                    osapp_start_connect(&slave_add);
                }
                break;
            default:
                break;
        }
}

static void uart_client_bond_manage_init(void)
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
              .auth = GAP_AUTH_REQ_NO_MITM_BOND,
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
}


static osapp_msg_handler_table_t const handler_table[]=
{
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
    {GAPM_ADV_REPORT_IND, (osapp_msg_handler_t)osapp_gapm_scan_adv_report_ind_handler},
    {GATTC_EVENT_IND, (osapp_msg_handler_t)osapp_event_ind_handler},
    {GATTC_SDP_SVC_IND, (osapp_msg_handler_t)osapp_sdp_disc_ind_handler},
    {GATTC_CMP_EVT,(osapp_msg_handler_t)osapp_gattc_cmp_evt_handler},
};
osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);

void user_init()
{
    io_cfg_output(CONNECTED_STATE_GPIO);
    io_pin_clear(CONNECTED_STATE_GPIO);
    osapp_utils_set_dev_mtu(UART_CLIENT_MAX_MTU);
    osapp_utils_set_dev_init(GAP_ROLE_CENTRAL, GAPM_CFG_ADDR_PUBLIC);
    uart_client_bond_manage_init();
    ahi_handler_register(&handler_info);
}


