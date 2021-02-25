#define LOG_TAG        "osapp.utils"
#define LOG_LVL        LVL_DBG
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "osapp_utils.h"
#include "co_utils.h"
#include "gattm_task.h"
#include "osapp_task.h"
#include "osapp_config.h"
#include "gattc_task.h"
#include "log.h"
#include "nvds.h"
#include "bx_log.h"

/*
 * MACROS
 ****************************************************************************************
 */
//#define UTILS_BD_ADDR_VALIAD_MASK       0xC0//core v5.0   1.3.2.1
//#define UTILS_BD_ADDR_STATIC_MASK       0xC0//core v5.0   1.3.2.1
//#define UTILS_BD_ADDR_NPA_MASK          0x00//core v5.0   1.3.2.2
//#define UTILS_BD_ADDR_RPA_MASK          0x40//core v5.0   1.3.2.2

#define UTILS_SET_BD_ADDR_STATIC(addr)  \
                                        do{\
                                              addr[GAP_BD_ADDR_LEN-1] = addr[GAP_BD_ADDR_LEN-1]|GAP_STATIC_ADDR;\
                                          }while(0)
//===========================handler help
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    enum gap_role role;
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
}osapp_utils_database_t;

/* LOCAL FUNCTIONS DEFINITIONS
****************************************************************************************
*/
static void osapp_device_ready_ind_handler(ke_msg_id_t const msgid, void const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_set_dev_config(void);
static void osapp_gapc_param_update_req_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
static void osapp_gapc_param_update_ind_handler(ke_msg_id_t const msgid, struct gapc_param_updated_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
static void osapp_le_phy_ind_handler(ke_msg_id_t const msgid, struct gapc_le_phy_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);



/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

static uint16_t user_max_mtu = GAP_MAX_LE_MTU;

static osapp_utils_database_t l_utils={
        .role = GAP_ROLE_NONE,
        .addr_type = GAPM_CFG_ADDR_PUBLIC,
};

static osapp_msg_handler_table_t const handler_table[]=
{
    {GAPM_DEVICE_READY_IND,(osapp_msg_handler_t)osapp_device_ready_ind_handler},
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
    {GAPC_PARAM_UPDATE_REQ_IND,(osapp_msg_handler_t)osapp_gapc_param_update_req_handler},
    {GAPC_PARAM_UPDATED_IND,(osapp_msg_handler_t)osapp_gapc_param_update_ind_handler},
    {GAPC_LE_PHY_IND,(osapp_msg_handler_t)osapp_le_phy_ind_handler},
};

static osapp_msg_handler_info_t  handler_info = HANDLER_ARRAY_INFO(handler_table);


/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
uint8_t * adv_data_pack(uint8_t *buf,uint8_t field_nums,...)
{
    va_list list;
    uint8_t i;
    va_start(list,field_nums);
    for(i=0;i<field_nums;++i)
    {
        uint8_t ad_type = va_arg(list,uint32_t);
        uint8_t *ad_data = va_arg(list,uint8_t *);
        uint8_t ad_data_length = va_arg(list,uint32_t);
        buf[0] = ad_data_length + 1;
        buf[1] = ad_type;
        memcpy(&buf[2],ad_data,ad_data_length);
        buf += buf[0] + 1;
    }
    va_end(list);
    return buf;
}

//extract ad_type content
uint8_t adv_data_extract(uint8_t **buff , uint8_t ad_type)
{
    uint8_t *ptr = *buff;
    uint8_t *buff_end = ptr + ADV_DATA_LEN;
    while(ptr <= buff_end)
    {
        if(ad_type == ptr[1])
        {
            *buff = &ptr[2];
            return ptr[0] - 1;
        }
        ptr += ptr[0] + 1;
    }
    return 0;
}

/**
 ****************************************************************************************
 * @brief  tools for random generate.
 *
 * @param[out] p_out      Pointer to the random data.
 * @param[in]  len        generate random length in byte.
 *
 ****************************************************************************************
 */
void osapp_utils_random_generate(uint8_t *p_out,uint8_t len)
{
    uint32_t tmp=0;
    uint8_t random_idx = 0;
    uint8_t random_remain = 0;


    while(random_idx <len)
    {
        tmp = rand();
        random_remain = len - random_idx;

        if(random_remain>=4)
        {
            memcpy((p_out+random_idx),(uint8_t *)&tmp,4);
            random_idx += 4;
        }
        else
        {
            memcpy((p_out+random_idx),(uint8_t *)&tmp,random_remain);
            random_idx += random_remain;
        }
    }
}
/**
 ****************************************************************************************
 * @brief   Is all of the array elements set to a value.
 *
 * @param[in]   buf - buffer to check
 * @param[in]   val - value to check each array element for
 * @param[in]   len - length to check
 *
 * @return  TRUE if all "val", FALSE otherwise
 ****************************************************************************************
 */
bool osapp_utils_is_buffer_set( uint8_t *buf, uint8_t val, uint8_t len )
{
  uint8_t x;

  if ( buf == NULL )
  {
    return ( false );
  }

  for ( x = 0; x < len; x++ )
  {
    // Check for non-initialized value
    if ( buf[x] != val )
    {
      return ( false );
    }
  }
  return ( true );
}
/**
 ****************************************************************************************
 * @brief  used to set max_mtu.
 *
 * @param[in]  max_mtu      the att max mtu
 *
 ****************************************************************************************
 */
void osapp_utils_set_dev_mtu(uint16_t max_mtu)
{ 
    if ((max_mtu < 23) || (max_mtu > GAP_MAX_LE_MTU))
    {
        BX_ASSERT(0);
    }
    else
    {
        user_max_mtu = max_mtu;
    }
}
/**
 ****************************************************************************************
 * @brief   Func osapp_utils_set_dev_init
 *
 * @param[in] role           The role of the device.
 * @param[in] addr_type      Address type of the device 0=public/1=private random
 *
 *                           /// Device Address is a Public Static address
 *                           GAPM_CFG_ADDR_PUBLIC        = 0,
 *                           /// Device Address is a Private Static address
 *                           GAPM_CFG_ADDR_PRIVATE       = 1,
 *                           /// Device Address generated using host-based Privacy feature
 *                           GAPM_CFG_ADDR_HOST_PRIVACY  = 2,
 *                           /// Device Address generated using controller-based Privacy feature
 *                           GAPM_CFG_ADDR_CTNL_PRIVACY  = 4,
 *
 ****************************************************************************************
 */
void osapp_utils_set_dev_init(enum gap_role role,uint8_t addr_type)
{
    LOG_I("osapp_utils_set_dev_init,role=0x%x",role);

    l_utils.role = role;
    l_utils.addr_type = addr_type;
    //register handle table
    ahi_handler_register(&handler_info);
}
/**
 ****************************************************************************************
 * @brief   Func osapp_utils_get_params
 *
 * @param[in]  type           The type osapp utils params type.
 * @param[out] p_param        The point of the params.
 *
 ****************************************************************************************
 */
bool osapp_utils_get_params(osapp_utils_param_type_t type,osapp_utils_param_t *p_param)
{
    bool state = false;
    uint8_t addr_len = GAP_BD_ADDR_LEN;
    uint8_t irk_len = GAP_KEY_LEN;
    if(p_param)
    {
        switch(type)
        {
            case OSAPP_UTILS_TYPE_GET_BD_ADDR :
            {
                p_param->addr.addr_type = l_utils.addr_type;
                uint8_t tag = (l_utils.addr_type & GAPM_CFG_ADDR_PRIVATE) ? NVDS_TAG_STATIC_DEV_ADDR : NVDS_TAG_BD_ADDRESS;
                if(nvds_get(tag,&addr_len,p_param->addr.addr.addr) == NVDS_OK)
                {
                    state = true;
                }
            }
                break;
            case OSAPP_UTILS_TYPE_GET_IRK:
                if(nvds_get(NVDS_TAG_LOCAL_IRK,&irk_len,p_param->irk.key) == NVDS_OK)
                {
                    state = true;
                }
                break;
            default:
                break;
        }
    }
    return state;
}

/**
 ****************************************************************************************
 * @brief  tools for printf to segger rtt log data.(temp buff use malloc)
 *
 * @param[in]  data      Pointer to the data to be printf to segger rtt log.
 * @param[in]  length    The length of data to be printf.
 *
 ****************************************************************************************
 */
void osapp_utils_log_hex_data(const uint8_t * data , uint16_t length)
{
    static const uint8_t l_osapp_utils_log_hex_tab[] = "0123456789ABCDEF";

    uint8_t tmp_h,tmp_l;
    uint8_t *mesh_log_format_buff=0;
    uint8_t *bufptr=0;
    uint32_t total_length;
    //init
    total_length = length * 2 + 1;
    mesh_log_format_buff = pvPortMalloc(total_length);
    bufptr = mesh_log_format_buff;
    //content
    for(uint16_t i=0;i<length;i++)
    {
        tmp_h = data[i] >> 4;
        tmp_l = data[i] & 0x0F;
        *bufptr = l_osapp_utils_log_hex_tab[tmp_h];  bufptr++;
        *bufptr = l_osapp_utils_log_hex_tab[tmp_l];  bufptr++;
    }
    //end
    *bufptr = '\n'; bufptr ++;
    //print
    SEGGER_RTT_Write(0,mesh_log_format_buff,total_length);
    #if (USE_INTERNAL_LOG == 1)
    internal_log_write(mesh_log_format_buff,total_length);
    #endif
    //free
    vPortFree(mesh_log_format_buff);
}





//static function
/*
static void send_get_version(uint16_t conn_idx)
{
    struct gapc_get_info_cmd* cmd = AHI_MSG_ALLOC(GAPC_GET_INFO_CMD,KE_BUILD_ID(TASK_ID_GAPC, conn_idx),gapc_get_info_cmd);
    cmd->operation = GAPC_GET_PEER_VERSION;
    osapp_ahi_msg_send(cmd, sizeof(struct gapc_get_info_cmd),portMAX_DELAY);
}

static void send_get_feature(uint16_t conn_idx)
{
    struct gapc_get_info_cmd* cmd = AHI_MSG_ALLOC(GAPC_GET_INFO_CMD,KE_BUILD_ID(TASK_ID_GAPC, conn_idx),gapc_get_info_cmd);
    cmd->operation = GAPC_GET_PEER_FEATURES;
    osapp_ahi_msg_send(cmd, sizeof(struct gapc_get_info_cmd),portMAX_DELAY);
}

*/

static int32_t osapp_gapc_param_update_cfm(ke_task_id_t const src_id)
{
    struct gapc_param_update_cfm *cfm = AHI_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM, src_id, gapc_param_update_cfm);
    cfm->accept = 0x01;
    cfm->ce_len_max = 0xffff;
    cfm->ce_len_min = 0xffff;
    return os_ahi_msg_send(cfm, portMAX_DELAY);

}

static void osapp_gapc_param_update_req_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    LOG_I("param update request");
    osapp_gapc_param_update_cfm(src_id);
}

static void osapp_gapc_param_update_ind_handler(ke_msg_id_t const msgid, struct gapc_param_updated_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
   LOG_I("param update ind:0x%x, 0x%x, 0x%x",param->con_interval,param->con_latency,param->sup_to);
}

static void osapp_le_phy_ind_handler(ke_msg_id_t const msgid, struct gapc_le_phy_ind const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    LOG_I( "le phy ind handler, tx:%d  rx:%d mb/s", param->tx_rate, param->rx_rate);
}



static void osapp_device_ready_ind_handler(ke_msg_id_t const msgid, void const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_reset_cmd *cmd = AHI_MSG_ALLOC(GAPM_RESET_CMD,TASK_ID_GAPM,gapm_reset_cmd);
    cmd->operation = GAPM_RESET;
    os_ahi_msg_send(cmd, portMAX_DELAY);
}
static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;

    switch(cmp_evt->operation)
    {
    case GAPM_RESET:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_set_dev_config();
        break;
    default:
        break;
    }
}

static void osapp_set_dev_config(void)
{

    // Set Device configuration
    struct gapm_set_dev_config_cmd* cmd = AHI_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,TASK_ID_GAPM,gapm_set_dev_config_cmd);
    cmd->operation = GAPM_SET_DEV_CONFIG;
    cmd->role      = l_utils.role;
    //privacy configuration
    cmd->renew_dur = GAP_TMR_PRIV_ADDR_INT;

    uint8_t addr_len = GAP_BD_ADDR_LEN;
    uint8_t irk_len = GAP_KEY_LEN;
    bool nvds_update = false;
    if(nvds_get(NVDS_TAG_STATIC_DEV_ADDR,&addr_len,cmd->addr.addr)!=NVDS_OK)
    {
        osapp_utils_random_generate(cmd->addr.addr,addr_len);//random addr;
        UTILS_SET_BD_ADDR_STATIC(cmd->addr.addr);//set mask
        nvds_put(NVDS_TAG_STATIC_DEV_ADDR,addr_len,cmd->addr.addr);
        nvds_update = true;
    }
    if(nvds_get(NVDS_TAG_LOCAL_IRK,&irk_len,cmd->irk.key)!=NVDS_OK)
    {
        osapp_utils_random_generate(cmd->irk.key, irk_len);//random addr;
        nvds_put(NVDS_TAG_LOCAL_IRK,irk_len,cmd->irk.key);\
        nvds_update = true;
    }
    if(nvds_update)
    {
        nvds_write_through();
    }

//    cmd->addr = l_utils.static_addr;
//    cmd->irk = l_utils.irk;

    cmd->addr_type = l_utils.addr_type;
    //security configuration
    cmd->pairing_mode = GAPM_PAIRING_LEGACY|GAPM_PAIRING_SEC_CON ;
    //attribute database configuration
    cmd->gap_start_hdl = 0;
    cmd->gatt_start_hdl = 0;
    cmd->att_cfg = GAPM_MASK_ATT_SVC_CHG_EN | GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN;
    //LE Data Length Extension configuration
    cmd->sugg_max_tx_octets = BLE_MAX_OCTETS;
    cmd->sugg_max_tx_time   = BLE_MAX_TIME;
    //L2CAP Configuration
    cmd->max_mps = GAP_MAX_LE_MTU;
    cmd->max_mtu = user_max_mtu;
    cmd->max_nb_lecb = 0;
    //LE Audio Mode Supported
    cmd->audio_cfg = 0;//GAPM_MASK_AUDIO_AM0_SUP;
    //LE PHY Management
    cmd->tx_pref_rates = GAP_RATE_LE_1MBPS | GAP_RATE_LE_2MBPS;
    cmd->rx_pref_rates = GAP_RATE_LE_1MBPS | GAP_RATE_LE_2MBPS;

    os_ahi_msg_send(cmd, portMAX_DELAY);
}


/**
 ****************************************************************************************
 * @brief  tools for memory reverse and copy.
 *
 * @param[in]  p_src      Pointer to the memcpy source data.
 * @param[out] p_dst      Pointer to the memcpy destination data.
 * @param[in]  length     The length of data to be memcpy.
 *
 * @note  Source and destination addresses cannot have an intersection.
 *
 ****************************************************************************************
 */
void memcpy_rev(void * restrict p_dst, const void * restrict p_src, uint8_t length)
{
    uint8_t * src = (uint8_t *) p_src;
    uint8_t * dst = (uint8_t *) p_dst;
    
    dst += (length - 1);
    while(length --)
    {
        *dst = * src;
        dst--;
        src ++;
    }
}


/**
 ****************************************************************************************
 * @brief  tools for printf to segger rtt log data.(temp buff use malloc)
 *
 * @param[in]  data      Pointer to the data to be printf to segger rtt log.
 * @param[in]  length    The length of data to be printf.
 *
 ****************************************************************************************
 */
void log_hex_data(const uint8_t * data , uint16_t length)
{
    const uint8_t log_hex_tab[] = "0123456789ABCDEF";

    uint8_t tmp_h,tmp_l;
    uint8_t *mesh_log_format_buff=0;
    uint8_t *bufptr=0;
    uint32_t total_length;
    //init
    total_length = length * 2 + 1;
    mesh_log_format_buff = pvPortMalloc(total_length);
    bufptr = mesh_log_format_buff;
    //content
    for(uint16_t i=0;i<length;i++)
    {
        tmp_h = data[i] >> 4;
        tmp_l = data[i] & 0x0F;
        *bufptr = log_hex_tab[tmp_h];  bufptr++;
        *bufptr = log_hex_tab[tmp_l];  bufptr++;
    }
    //end
    *bufptr = '\n'; bufptr ++;
    //print
    SEGGER_RTT_Write(0,mesh_log_format_buff,total_length);
    #if (USE_INTERNAL_LOG == 1)
    internal_log_write(mesh_log_format_buff,total_length);
    #endif
    //free
    vPortFree(mesh_log_format_buff);
}

/**
 ****************************************************************************************
 * @brief  tools for reverse the src data.
 *
 * @param[in]  p_src      Pointer to the memcpy source data.
 * @param[in]  length     The length of data to be reverse.
 *
 ****************************************************************************************
 */
void reverse_self(void * p_src , uint8_t length)
{
    uint8_t *src1 = (uint8_t*)p_src;
    uint8_t *src2 = src1 + length - 1;
    uint8_t i=length / 2;
    uint8_t tmp=0;
    while(i--)
    {
        tmp = *src1;
        *src1 = *src2;
        *src2 = tmp;
        src1 ++;
        src2 --;
    }
}

/**
 ****************************************************************************************
 * @brief  tools for random generate.
 *
 * @param[out] p_out      Pointer to the random data.
 * @param[in]  len        generate random length in byte.
 *
 ****************************************************************************************
 */
void tools_random_generate(uint8_t *p_out,uint8_t len)
{
    uint32_t tmp=0;
    uint8_t random_idx = 0;
    uint8_t random_remain = 0;


    while(random_idx <len)
    {
        tmp = rand();
        random_remain = len - random_idx;

        if(random_remain>=4)
        {
            memcpy((p_out+random_idx),(uint8_t *)&tmp,4);
            random_idx += 4;
        }
        else
        {
            memcpy((p_out+random_idx),(uint8_t *)&tmp,random_remain);
            random_idx += random_remain;
        }
    }
}

/**
 ****************************************************************************************
 * @brief  tools for get the gap public bd address.
 *
 * @param[out] p_out_addr      Pointer to the gap public bd address data.
 *
 ****************************************************************************************
 */
void tools_gap_public_bd_addr_get(uint8_t *p_out_addr)
{
    const uint8_t default_bdaddr[BD_ADDR_LEN] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
    uint8_t len = BD_ADDR_LEN;

    // get BD address
#if (NVDS_SUPPORT)
    if (nvds_get(NVDS_TAG_BD_ADDRESS, &len, p_out_addr) != NVDS_OK)
#endif //(NVDS_SUPPORT)
    {
        memcpy(&p_out_addr, (uint8_t *)&default_bdaddr[0], sizeof(default_bdaddr));
    }
}

void output_rf_debug_pin(void)
{
    uint32_t tmp;
    //20132020 ¡¾19:16¡¿=0xF
    tmp = *(volatile uint32_t *) 0x20132020 ;
    tmp |= (uint32_t)0xF << 16;
    *(volatile uint32_t *) 0x20132020 = tmp;
    
    //20110024 ¡¾2:0¡¿=0x0
    tmp = *(volatile uint32_t *) 0x20110024 ;
    tmp &= 0xFFFFFFF8;
    *(volatile uint32_t *) 0x20110024 = tmp;

    //20100050 ¡¾7:0¡¿=0x83
    tmp = *(volatile uint32_t *) 0x20100050 ;
    tmp &= 0xFFFFFF00;
    tmp |= 0x83;
    *(volatile uint32_t *) 0x20100050 = tmp;

    //     GPIO14            GPIO13           GPIO12            GPIO11           GPIO10           GPIO9    GPIO8         GPIO7
    //     ble_error_irq   ble_rx_irq  ble_event_irq    event_in_process    syncfound_pulse sync_window radcntl_rxen    radcntl_txen
}


