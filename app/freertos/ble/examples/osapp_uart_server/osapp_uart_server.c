#define LOG_TAG        "osapp.uart_server"
#define LOG_LVL        LVL_DBG
#include "osapp_config.h"
#include "osapp_svc_manager.h"
#include <stdlib.h>
#include "osapp_utils.h"
#include "bond_manage.h"
#include "app_uart.h"
#include "bx_log.h"

#define APP_ADV_CHMAP 0x7
#define APP_ADV_INT 32
#define UART_SVC_ADV_NAME "Apollo UART Server"
#define UART_SVC_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e}
#define UART_SVC_RX_CHAR_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x02,0x00,0x40,0x6e}
#define UART_SVC_TX_CHAR_UUID_128 {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,\
    0xe0,0x93,0xf3,0xa3,0xb5,0x03,0x00,0x40,0x6e}
#define ATT_DECL_CHAR_ARRAY {0x03,0x28}
#define ATT_DESC_CLIENT_CHAR_CFG_ARRAY {0x02,0x29}
#define UART_SERVER_MAX_MTU  247
#define UART_SVC_RX_BUF_SIZE (UART_SERVER_MAX_MTU - 3)
#define UART_SVC_TX_BUF_SIZE (UART_SERVER_MAX_MTU - 3)
//slave preferred parameters
#define CONN_INTERVAL_MIN   8
#define CONN_INTERVAL_MAX   10
#define SLAVE_LATENCY       0
#define CONN_TIMEOUT        200

#define UART_SERVER_TIME_PERIOD     2//(unitï¼portTICK_PERIOD_MS)
TimerHandle_t xTimerUser;
StaticTimer_t xTimerBuffers;
static uint32_t conn_mtu = ATT_DEFAULT_MTU;

enum uart_svc_att_db_handles
{
    UART_SVC_IDX_RX_CHAR,
    UART_SVC_IDX_RX_VAL,
    UART_SVC_IDX_TX_CHAR,
    UART_SVC_IDX_TX_VAL,
    UART_SVC_IDX_TX_NTF_CFG,
    UART_SVC_ATT_NUM
};
struct gattm_svc_desc const uart_svc_desc ={
        .start_hdl = 0,
        .task_id = TASK_ID_AHI,
        .perm = PERM(SVC_MI,DISABLE)|PERM(SVC_EKS,DISABLE)|\
            PERM(SVC_AUTH,NO_AUTH)|PERM(SVC_UUID_LEN,UUID_128),PERM_VAL(SVC_SECONDARY,0),
        .nb_att = UART_SVC_ATT_NUM,
        .uuid = UART_SVC_UUID_128,
};
struct gattm_att_desc const uart_svc_att_db[UART_SVC_ATT_NUM] = {
            [UART_SVC_IDX_RX_CHAR] = {
                .uuid = ATT_DECL_CHAR_ARRAY,
                .perm = PERM(RD,ENABLE),
                .max_len = 0,
                .ext_perm= PERM(UUID_LEN,UUID_16),
            },
            [UART_SVC_IDX_RX_VAL] = {
                .uuid = UART_SVC_RX_CHAR_UUID_128,
                .perm = PERM(WRITE_REQ,ENABLE)|PERM(WRITE_COMMAND,ENABLE)|PERM(WP,NO_AUTH),
                .max_len = UART_SVC_RX_BUF_SIZE,
                .ext_perm = PERM(UUID_LEN,UUID_128)|PERM(RI,ENABLE),
            },
            [UART_SVC_IDX_TX_CHAR] = {
                .uuid = ATT_DECL_CHAR_ARRAY,
                .perm = PERM(RD,ENABLE),
                .max_len = 0,
                .ext_perm = PERM(UUID_LEN,UUID_16),
            },
            [UART_SVC_IDX_TX_VAL] = {

                .uuid = UART_SVC_TX_CHAR_UUID_128,
                .perm = PERM(NTF,ENABLE),
                .max_len = UART_SVC_TX_BUF_SIZE,
                .ext_perm = PERM(UUID_LEN,UUID_128)|PERM(RI,ENABLE),
            },
            [UART_SVC_IDX_TX_NTF_CFG] = {
              .uuid = ATT_DESC_CLIENT_CHAR_CFG_ARRAY,
              .perm = PERM(RD,ENABLE)|PERM(WRITE_REQ,ENABLE),
               .max_len = 0,
               .ext_perm = PERM(UUID_LEN,UUID_16),
            },
};
app_uart_inst_t uart0_inst = UART_INSTANCE(0);
static uint8_t uart0_buf[UART_SERVER_MAX_MTU - 3];
static uint8_t conn_hdl;
static bool uart0_tx_busy;
static void uart_server_read_req_ind(osapp_svc_helper_t const *,ke_task_id_t const,uint16_t);
static void uart_server_write_req_ind(osapp_svc_helper_t const *,ke_task_id_t const,uint16_t,uint16_t,uint16_t,uint8_t const*);
osapp_svc_helper_t uart_server_svc_helper = 
{
    .svc_desc = &uart_svc_desc,
    .att_desc = uart_svc_att_db,
    .att_num = UART_SVC_ATT_NUM,
    .read = uart_server_read_req_ind,
    .write = uart_server_write_req_ind,
};

static void uart_server_read_req_ind(osapp_svc_helper_t const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx)
{

    LOG_I("read att_idx:%d",att_idx);
    if(att_idx == UART_SVC_IDX_TX_NTF_CFG)
    {
        struct gattc_read_cfm *cfm = AHI_MSG_ALLOC(GATTC_READ_CFM,src_id, gattc_read_cfm);
        cfm->handle = osapp_get_att_handle_helper(svc_helper,att_idx);
        cfm->length=0;
        cfm->status = 0;
        os_ahi_msg_send(cfm,portMAX_DELAY);
    }
}

static void uart_write_callback(void *dummy,uint8_t status)
{
    uart0_tx_busy = false;
    LOG_W("uart tx done");
}

static void uart_server_write_req_ind(osapp_svc_helper_t const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx,uint16_t offset,uint16_t length,uint8_t const*value)
{
    LOG_I("write att_idx:%d",att_idx);
    struct gattc_write_cfm *cfm = AHI_MSG_ALLOC(GATTC_WRITE_CFM,src_id,gattc_write_cfm);
    cfm->status = ATT_ERR_NO_ERROR;
    cfm->handle = osapp_get_att_handle_helper(svc_helper,att_idx);
    osapp_ahi_msg_send(cfm, sizeof(struct gattc_write_cfm),portMAX_DELAY);  
    if(att_idx != UART_SVC_IDX_RX_VAL)
    {
        return;
    }
    if(uart0_tx_busy)
    {
        LOG_I("tx busy,data discard");
    }else
    {
        uart0_tx_busy = true;
        app_uart_write(&uart0_inst.inst, (uint8_t *)value, length, uart_write_callback,NULL);
    }
}

static void osapp_start_advertising()
{
    static uint8_t svc_uuid_ptr[] = ATT_DECL_CHAR_ARRAY;
        // Prepare the GAPM_START_ADVERTISE_CMD message
    struct gapm_start_advertise_cmd *cmd = AHI_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,TASK_ID_GAPM, gapm_start_advertise_cmd);
    cmd->op.addr_src    = GAPM_STATIC_ADDR;
    cmd->channel_map    = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT;
    cmd->intv_max = APP_ADV_INT;
    cmd->op.code        = GAPM_ADV_UNDIRECT;
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
            // Flag value is set by the GAP
    cmd->info.host.adv_data_len = ADV_DATA_PACK(cmd->info.host.adv_data,2,\
            GAP_AD_TYPE_COMPLETE_NAME,UART_SVC_ADV_NAME,sizeof(UART_SVC_ADV_NAME),\
            GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID,svc_uuid_ptr,sizeof(svc_uuid_ptr));
    cmd->info.host.scan_rsp_data_len  = 0;
    os_ahi_msg_send(cmd,portMAX_DELAY);   
}

static void add_svc_callback(uint8_t status,osapp_svc_helper_t *svc_helper)
{
    osapp_start_advertising();
}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_SET_DEV_CONFIG:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_add_svc_req_helper(&uart_server_svc_helper,1,add_svc_callback);
        break;
    case GAPM_ADV_UNDIRECT:
        LOG_I("adv status:%d",cmp_evt->status);
        break;
    default:
        LOG_W("gapm_cmp_evt operation:0x%x",cmp_evt->operation);
        break;
            
    }
}


//static void osapp_send_notification_isr(ke_task_id_t const dest_id,uint8_t const *data,uint16_t length)
static void osapp_send_notification(void * param)
{
    ke_task_id_t dest_id = KE_BUILD_ID(TASK_ID_GATTC, conn_hdl);
    static uint16_t notify_seq_num = 0;
    struct gattc_send_evt_cmd *cmd= AHI_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,dest_id, gattc_send_evt_cmd, sizeof(uart0_buf));
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = notify_seq_num++;
    cmd->handle = osapp_get_att_handle_helper(&uart_server_svc_helper,UART_SVC_IDX_TX_VAL);
    cmd->length = *((uint16_t *)param);//sizeof(uart0_buf);
    memcpy(cmd->value,uart0_buf,cmd->length);
    os_ahi_msg_send(cmd, portMAX_DELAY);
}

static void uart0_rx_callback(void *dummy,uint8_t status);
static void osapp_notify_and_read_uart(void *dummy)
{
    uint32_t notify_length = (uint32_t)dummy;
        
    if(notify_length)
    {
        osapp_send_notification(&notify_length);
    }
    app_uart_read(&uart0_inst.inst,uart0_buf,conn_mtu,uart0_rx_callback,(void *)conn_mtu);
}

static void vTimer_callback( TimerHandle_t xTimer )
{
    uint32_t remain_lenght = 0;
    uint32_t uart_read_size = 0;

    periph_err_t uart_err = app_uart_read_cancel(&uart0_inst.inst, &remain_lenght, (void **)&uart_read_size);
    
    if (PERIPH_NO_ERROR == uart_err)
    {
        uint32_t notify_length = uart_read_size - (uint16_t)remain_lenght;
        osapp_async_call_wrapper(osapp_notify_and_read_uart, (void *)notify_length);
    }
}

static void uart0_rx_callback(void *dummy,uint8_t status)
{
    LOG_I("uart0_rx_callback");
    osapp_async_call_wrapper(osapp_notify_and_read_uart,dummy);
}

static void uart_init()
{
    uart0_inst.param.baud_rate = UART_BAUDRATE_115200;
    uart0_inst.param.tx_pin_no = 12;
    uart0_inst.param.rx_pin_no = 13;
    app_uart_init(&uart0_inst.inst);
    app_uart_read(&uart0_inst.inst,uart0_buf,conn_mtu,uart0_rx_callback,(void *)conn_mtu);
}

static void xTimer_creat_and_start()
{

    xTimerUser = xTimerCreateStatic( "UartTimeoutTimer",
                                     UART_SERVER_TIME_PERIOD,
                                     pdTRUE,
                                     ( void * )0,
                                     vTimer_callback,
                                     &xTimerBuffers
                                     );
    xTimerStart( xTimerUser, pdFALSE);
}



static void osapp_gattc_cmp_evt_handler(ke_msg_id_t const msgid,struct gattc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t src_id)
{
    switch(param->operation)
    {
        case GATTC_NOTIFY:
            if(param->status == ATT_ERR_NO_ERROR)
            {
                LOG_I("notification done");
            }else
            {
                LOG_I("noti status:%d",param->status);
            }
            break;
        default:
            LOG_I("op:%d,seq:%d,status:%d",param->operation,param->seq_num,param->status);
            break;
    }
}


static void osapp_gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid, struct gapc_get_dev_info_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapc_get_dev_info_cfm *cfm;     
    switch(param->req)
    {
        case GAPC_DEV_NAME:
            cfm = AHI_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,src_id, gapc_get_dev_info_cfm,sizeof(UART_SVC_ADV_NAME));
            cfm->req = GAPC_DEV_NAME;
            cfm->info.name.length = sizeof(UART_SVC_ADV_NAME);
            memcpy(cfm->info.name.value,UART_SVC_ADV_NAME,sizeof(UART_SVC_ADV_NAME));
            os_ahi_msg_send(cfm, portMAX_DELAY);
            break;
        case GAPC_DEV_APPEARANCE:
            cfm=AHI_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM, src_id, gapc_get_dev_info_cfm);
            cfm->req = GAPC_DEV_APPEARANCE;
            cfm->info.appearance = 0;
            os_ahi_msg_send(cfm, portMAX_DELAY);
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
    }
}


static void osapp_gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,struct gattc_mtu_changed_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    conn_mtu = param->mtu;
}


static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param)
{
    LOG_I("bond_manage_evt_cb evt: %d",evt);

        switch(evt)
        {
            case BOND_MG_EVT_CONNECTED :
                {
                    conn_hdl = p_param->connected.conn_idx;
                    LOG_I("BOND_MG_EVT_CONNECTED conn_hdl=%d", conn_hdl);
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

static void uart_server_bond_manage_init(void)
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


static osapp_msg_handler_table_t const handler_table[]=
{
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
    {GATTC_CMP_EVT,(osapp_msg_handler_t)osapp_gattc_cmp_evt_handler},
    {GAPC_GET_DEV_INFO_REQ_IND,(osapp_msg_handler_t)osapp_gapc_get_dev_info_req_ind_handler},
    {GATTC_MTU_CHANGED_IND,(osapp_msg_handler_t)osapp_gattc_mtu_changed_ind_handler},
};
osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);

void user_init()
{    
    osapp_utils_set_dev_mtu(UART_SERVER_MAX_MTU);
    osapp_utils_set_dev_init(GAP_ROLE_PERIPHERAL,GAPM_CFG_ADDR_PUBLIC);
    uart_server_bond_manage_init();
    osapp_svc_manager_init();
    ahi_handler_register(&handler_info);
    uart_init();
    xTimer_creat_and_start();
}

