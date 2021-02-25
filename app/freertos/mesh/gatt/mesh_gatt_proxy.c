/*
 * mesh_gatt_proxy.c
 *
 *  Created on: 2018-8-7
 *      Author: huichen
 */

/**
 ****************************************************************************************
 * @addtogroup BLE_MESH_GATT_PROXY  BLE Mesh Gatt proxy Internal
 * @ingroup BLE_MESH_GATT
 * @brief defines for BLE mesh gatt proxy contral
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "mesh_gatt_error.h"
#include "mesh_gatt_filter.h"
#include "mesh_gatt_proxy.h"
#include "provisioning_s.h"
#include "proxy_s.h"
#include "mesh_env.h"
#include "beacon.h"
#include "mesh_iv_operation_ex.h"
#include "osapp_utils.h"
#include "co_endian.h"
#include "network_tx_process.h"
#include "network_pdu_decrypt.h"
#include "adv_bearer_tx.h"
#include "adv_bearer_rx.h"
#include "mesh_queued_msg.h"
#include "network_tx_process.h"
#include "upper_pdu.h"
#include "lower_tx_process.h"
#include "provision_comm.h"
#include "unprov_device_fsm.h"
#include "unprov_device_intf.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    proxy_filter_t filter;
    bool connected;
    net_key_t *netkey;
    net_key_box_t *netkey_box;
} proxy_connection_t;

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * MACROS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
//rx
static void mesh_gatt_proxy_config_pdu_rx(uint16_t conn_index,const uint8_t * p_data,uint32_t length);
static void mesh_gatt_proxy_config_pdu_decrypt(uint16_t conn_index,const uint8_t * p_data,uint32_t length);
//tx
static void mesh_gatt_proxy_config_pdu_send_filter_status(void);
static void mesh_gatt_proxy_config_pdu_encrypt(uint16_t conn_index);
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static mesh_provsion_method_t m_active_methon=PROVISION_BY_ADV;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static proxy_connection_t m_connections[MESH_GATT_CONNECTION_COUNT_MAX];

static uint16_t m_conn_index = MESH_GATT_CONN_INDEX_INVALID;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

static int32_t  provision_gatt_compare(void * cookie, void * p_item)
{
     if(*(uint16_t*)cookie == *(uint16_t*) p_item)
     {
         return 0;
     }
     if(*(uint16_t*)cookie < *(uint16_t*) p_item)
     {
         return -1;
     }
     else
     {
        return 1;
     }
}
void provision_gatt_prepare_link_close(void * instance,uint8_t reason,void (*cb)(void *tcb, Tx_Reason reason))
{

     int16_t conn_handle =  mesh_gatt_conn_index_to_handle(*(uint16_t*)instance);

     if(conn_handle == MESH_GATT_CONN_INDEX_INVALID )
     {
         void * tcb  = prov_fsm_get_gatt_protocol_instance(instance);
         if(NULL != tcb)
         {
            cb(tcb,(Tx_Reason)reason);
         }
     }
     else
     {
         mesh_gatt_disconnect(conn_handle,reason);
     }
}

static void provisioning_pdu_gatt_cancel_tx(void *instance,void (*cb)(void *tcb, Tx_Reason reason))
{
    void * tcb  = prov_fsm_get_gatt_protocol_instance(instance);
    cb(tcb,Tx_Cancel_Finish);
}

static void provisioning_pdu_gatt_tx(void *instance,  uint8_t *p_data,uint8_t length,void (*cb)(void *tcb, Tx_Reason reason))
{
    LOG(LOG_LVL_INFO,"provisioning_pdu_gatt_tx");
    mesh_gatt_proxy_pdu_tx(MESH_GATT_PDU_TYPE_PROV_PDU,p_data,length,MESH_ADDR_UNASSIGNED);
    void * tcb  = prov_fsm_get_gatt_protocol_instance(instance);
    cb(tcb,Tx_Success_Finish);
}

void provisioning_pdu_gatt_rx(uint16_t conn_index, uint8_t *p_data, uint16_t length)
{
    void * tcb  = prov_fsm_get_gatt_protocol_instance(conn_get_handle_ptr(conn_index));
    if(tcb == NULL)
    {
        tcb = prov_fsm_create_gatt_protocol_instance(conn_get_handle_ptr(conn_index),provisioning_pdu_gatt_tx,provision_gatt_compare,provisioning_pdu_gatt_cancel_tx);
    }
    if(NULL == tcb)
    {
        LOG(LOG_LVL_INFO,"Invalid TCB in GATT\n");
        return;
    }
    provision_fsm_entry(tcb, p_data, length);
    prov_fsm_ack_finish(tcb,&conn_index);
}

/**
 ****************************************************************************************
 * @brief   provision user set method   maybe gatt/adv/gatt+aadv  (3)
 ****************************************************************************************
 */
 /*

only used by unprov device

 */
mesh_provsion_method_t provision_system_method_get(void)
{
    mesh_provsion_method_t method = PROVISION_BY_GATT;
    method = unprov_priovision_method_get();
    return method;
}
/**
 ****************************************************************************************
 * @brief   provision active method   gatt/adv  (2)
 ****************************************************************************************
 */
void provision_active_method_set(mesh_provsion_method_t method)
{
    if(PROVISION_BY_ADV_GATT ==provision_system_method_get())
    {
        if(PROVISION_BY_ADV == method)//only adv
        {
            provision_service_beacon_stop();//gatt stop
        }
        else
        {
            stop_current_beacon();//adv stop
        }
        m_active_methon = method;
    }
    else
    {
        m_active_methon = provision_system_method_get();
    }
}
/**
 ****************************************************************************************
 * @brief   provision active method   gatt/adv (2)
 ****************************************************************************************
 */
mesh_provsion_method_t  provision_active_method_get(void)
{
    return m_active_methon;
}





 
/**
 ****************************************************************************************
 * @brief The ble Gatt porxy connect event reset inter database.
 *
 * @param[in] conn_index  gatt connect index.
 *
 ****************************************************************************************
 */
void mesh_gatt_proxy_connection_reset(uint16_t conn_index)
{
    proxy_connection_t * p_connection = &m_connections[conn_index];

    p_connection->connected = true;
    mesh_proxy_filter_reset(&p_connection->filter);
//    p_connection->p_pending_beacon_info = NULL;
}
/**
 ****************************************************************************************
 * @brief The ble Gatt porxy disconnect event reset inter database.
 *
 * @param[in] conn_index  gatt connect index.
 *
 ****************************************************************************************
 */
void mesh_gatt_proxy_disconnection_reset(uint16_t conn_index)
{
    proxy_connection_t * p_connection = &m_connections[conn_index];

    p_connection->connected = false;
}
/**
 ****************************************************************************************
 * @brief The ble Gatt porxy rx handler.
 *
 * @param[in] conn_index  gatt connect index.
 * @param[in] pdu_type    The gatt proxy pdu type.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      The length of write data.
 *
 ****************************************************************************************
 */

void mesh_gatt_proxy_rx_handle(uint16_t conn_index,mesh_gatt_pdu_type_t pdu_type, uint8_t * p_data,uint16_t length)
{


    LOG(LOG_LVL_INFO, "RX_GATT_PDU_type 0x%x, len %u\n", pdu_type, length);

    switch (pdu_type)
    {
        case MESH_GATT_PDU_TYPE_NETWORK_PDU:
            gatt_mesh_msg_rx((uint8_t *)p_data,length,NORMAL_NETWORK_PKT_RX,conn_index);
            break;
        case MESH_GATT_PDU_TYPE_MESH_BEACON:
            beacon_rx(p_data, length);
            break;
        case MESH_GATT_PDU_TYPE_PROXY_CONFIG:
            mesh_gatt_proxy_config_pdu_rx(conn_index, p_data, length);
            break;
        case MESH_GATT_PDU_TYPE_PROV_PDU:
            provisioning_pdu_gatt_rx(conn_index,p_data,length);
            break;
        default:
            /* Ignore unknown PDU type, according to Mesh Profile Specification v1.0, section 6.6 */
            break;
    }
}
/**
 ****************************************************************************************
 * @brief The ble Gatt Proxy gatt network messages pdu decrypt done callback.
 *
 * @param[in] p_pdu       Pointer to the network pdu database.
 * @param[in] length      The length of the decrypt data.
 * @param[in] conn_index  gatt connect index.
 *
 ****************************************************************************************
 */
void mesh_gatt_proxy_gatt_list_filter_set(network_pdu_decrypt_callback_param_t * p_pdu,uint16_t conn_index)
{
    if( conn_index < MESH_GATT_CONNECTION_COUNT_MAX )
    {

        proxy_connection_t * p_connection = &m_connections[conn_index];

        if (p_connection->connected)
        {
             uint16_t src_addr = network_pdu_get_src_addr(p_pdu);
            if (p_connection->filter.type == PROXY_FILTER_TYPE_WHITELIST)
            {
                /*
                 * If the proxy filter is a white list filter, upon receiving a valid Mesh message from the Proxy Client,
                 * the Proxy Server shall add the unicast address contained in the SRC field of the message to the white list.
                 * */
                mesh_proxy_add_filter_addr(&p_connection->filter, &src_addr, 1);
            }
            else
            {
                /*
                 * If the proxy filter is a black list filter, upon receiving a valid Mesh message from the Proxy Client,
                 * the Proxy Server shall remove the unicast address contained in the SRC field of the message from the black list.
                 * */
                mesh_proxy_remove_filter_addr(&p_connection->filter, &src_addr, 1);
            }
        }
    }
    else
    {
        LOG(LOG_LVL_ERROR, "!!!mesh_gatt_proxy_gatt_list_filter_set error !!!\n");
    }
}
/**
 ****************************************************************************************
 * @brief The ble Gatt Proxy configuration messages rx handler.
 *           (Proxy configuration messages are used to configure the proxy filters.)
 *
 * @param[in] conn_index  gatt connect index.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      The length of write data.
 *
 ****************************************************************************************
 */
static void mesh_gatt_proxy_config_pdu_rx(uint16_t conn_index,const uint8_t * p_data,uint32_t length)
{
    if (length <= MESH_GATT_PROXY_PDU_MAX_SIZE && length >= PROXY_CONFIG_PARAM_OVERHEAD)
    {
        mesh_gatt_proxy_config_pdu_decrypt(conn_index,p_data,length);
    }
    else
    {
         LOG(LOG_LVL_INFO, "mesh_gatt_proxy_config_pdu_rx too long 0x%x \n", length);
    }
}
/**
 ****************************************************************************************
 * @brief The ble Gatt Proxy configuration messages pdu decrypt.
 *
 * @param[in] conn_index  gatt connect index.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      The length of write data.
 *
 ****************************************************************************************
 */
static void mesh_gatt_proxy_config_pdu_decrypt(uint16_t conn_index,const uint8_t * p_data,uint32_t length)
{
    gatt_mesh_msg_rx((uint8_t *)p_data,length,PROXY_CONFIG_NETWORK_PKT_RX,conn_index);
}
/**
 ****************************************************************************************
 * @brief The Gatt Proxy configuration messages pdu send filter status.
 *
 ****************************************************************************************
 */
static void mesh_gatt_proxy_config_pdu_send_filter_status(void)
{
    if( m_conn_index != MESH_GATT_CONN_INDEX_INVALID)
    {
        proxy_connection_t * p_connection = &m_connections[m_conn_index];

        if(p_connection->connected == true)
        {
            //tx
            mesh_gatt_proxy_config_pdu_encrypt(m_conn_index);
        }
    }
}
/**
 ****************************************************************************************
 * @brief The mesg gatt proxy config pdu encrypt callback.
 *
 ****************************************************************************************
 */
/*
static void mesh_gatt_proxy_config_pdu_encrypt_callback(void)
{
    LOG(LOG_LVL_INFO,"mesh_gatt_proxy_config_pdu_encrypt_callback \n");
}
*/


/**
 ****************************************************************************************
 * @brief The ble Gatt Proxy configuration messages pdu encrypt.
 *
 * @param[in] conn_index  gatt connect index.
 *
 ****************************************************************************************
 */
static void mesh_gatt_proxy_config_pdu_encrypt(uint16_t conn_index)
{
   
    proxy_connection_t * p_connection = &m_connections[conn_index];
    net_key_t *netkey = p_connection->netkey;
    net_key_box_t *netkey_box = p_connection->netkey_box;
    uint32_t iv_index = mesh_tx_iv_index_get();
    uint32_t seq_num = mesh_seqnum_alloc(1);
    //uint8_t raw_pdu[MESH_PROXY_CFG_FILTER_STATUS_LENGHT];
    dm_netkey_handle_t netkey_handle = NULL;
    if(NULL == netkey || NULL == netkey_box)
    {
         if( MESH_CORE_SUCCESS != dm_netkey_get_first_handle(0,&netkey_handle))
         {
             return;
         }
         netkey = netkey_handle;
         dm_netkey_get_tx_info(netkey_handle,&netkey_box);
    }
    network_pdu_packet_head_t head = 
    {
        .ivi = iv_index & 0x1,
        .nid = netkey_box->master.nid,
        .ctl = 1,
        .ttl = 0,
        .seq_be = SEQ_ENDIAN_REVERSE(seq_num),
        .src_be = co_bswap16(mesh_node_get_primary_element_addr()),
        .dst_be = MESH_ADDR_UNASSIGNED,
    };
    network_pdu_tx_t *msg = network_tx_pdu_head_build(&head);
    mesh_proxy_fill_filter_status(&p_connection->filter, msg->src.lower_pdu);
    
    msg->src.lower_pdu_length = MESH_PROXY_CFG_FILTER_STATUS_LENGHT;
    msg->iv_index = iv_index;
    msg->netkey_credentials = &netkey_box->master;
    msg->pkt_type = NON_ADV_PROXY_CONFIG_PKT;
    //msg->bearer.local = 0;
   // msg->bearer.adv = 0;
    //msg->bearer.gatt = 1<<conn_index;
    network_tx_process_start(msg);
    LOG(LOG_LVL_INFO,"mesh_gatt_proxy_config_pdu_encrypt \n");

}
/**
 ****************************************************************************************
 * @brief The ble Gatt Proxy configuration messages pdu decrypt done callback.
 *
 * @param[in] p_pdu       Pointer to the network pdu database.
 * @param[in] length      The length of the decrypt data.
 * @param[in] conn_index  gatt connect index.
 *
 ****************************************************************************************
 */

 
void mesh_gatt_proxy_config_pdu_decrypt_callback(network_pdu_decrypt_callback_param_t * p_pdu,uint16_t length,uint16_t conn_index)
{
    if( (length >= sizeof(network_pdu_packet_head_t)) && (conn_index < MESH_GATT_CONNECTION_COUNT_MAX) )
    {

        proxy_connection_t * p_connection = &m_connections[conn_index];
        proxy_config_network_pdu_t pdu;
        uint32_t status = MESH_GATT_SUCCESS;
        const uint8_t * p_data = network_pdu_get_transport_pdu(p_pdu);
        uint16_t msg_len = length - network_pdu_mic_length(p_pdu->decrypted.pkt.ctl)-TRANSPORT_DATA_OFFSET -1;//dst+op = 2+1 = 3

        //1. save proxy decrypt pdu data
        //memcpy(&p_connection->net_pdu,p_pdu,sizeof(network_pdu_base_t));
        //2. set config opcode
        pdu.src_addr = network_pdu_get_src_addr(p_pdu);
        pdu.dst_addr = network_pdu_get_dst_addr(p_pdu);
        pdu.msg.opcode = p_data[0] ;

        switch (pdu.msg.opcode)
        {
            case PROXY_CONFIG_OPCODE_SET_FILTER_TYPE:
                {
                    if(msg_len == 1)
                    {
                        pdu.msg.params.set_type.filter_type = p_data[1];

                        status = mesh_proxy_set_filter_type(&p_connection->filter,(proxy_filter_type_t)pdu.msg.params.set_type.filter_type);
                    }
                    else
                    {
                        status = MESH_GATT_ERROR_INVALID_LENGTH;
                    }
                }
                break;
            case PROXY_CONFIG_OPCODE_ADD_FILTER_ADDR:
                {
                    if(msg_len>1&&!(msg_len%0x01)&&msg_len<=10)//(2~10  2N) 96/8 = 12  12-1=11 ~2N = 10
                    {
                        pdu.msg.params.add_addr.count = msg_len/2;

                        for(uint8_t i=0;i<pdu.msg.params.add_addr.count;i++)
                        {
                            pdu.msg.params.add_addr.addrs[i]= (p_data[i*2+1]<<8) | (p_data[i*2+2]);
                        }
                        mesh_proxy_add_filter_addr(&p_connection->filter,pdu.msg.params.add_addr.addrs,pdu.msg.params.add_addr.count);
                    }
                    else
                    {
                        status = MESH_GATT_ERROR_INVALID_LENGTH;
                    }
                }
                break;
            case PROXY_CONFIG_OPCODE_REMOVE_FILTER_ADDR:
                {
                    if(msg_len>1&&!(msg_len%0x01)&&msg_len<=10)//(2~10  2N) 96/8 = 12  12-1=11 ~2N = 10
                    {
                        pdu.msg.params.remove_addr.count = msg_len/2;

                        for(uint8_t i=0;i<pdu.msg.params.remove_addr.count;i++)
                        {
                            pdu.msg.params.remove_addr.addrs[i]= (p_data[i*2+1]<<8) | (p_data[i*2+2]);
                        }
                        mesh_proxy_remove_filter_addr(&p_connection->filter,pdu.msg.params.remove_addr.addrs,pdu.msg.params.remove_addr.count);
                    }
                    else
                    {
                        status = MESH_GATT_ERROR_INVALID_LENGTH;
                    }
                }
                break;
            default:
                status = MESH_GATT_ERROR_INVALID_PARAM;
                LOG(LOG_LVL_ERROR, "!!!proxy_config_network_pdu msg.opcode 0x%x !!!\n", pdu.msg.opcode);
                break;
        }

        //3. send state
        if (status == MESH_GATT_SUCCESS)
        {
            m_conn_index = conn_index;
            mesh_run(mesh_gatt_proxy_config_pdu_send_filter_status,portMAX_DELAY,false);
            
        }
        else
        {
            LOG(LOG_LVL_ERROR, "!!!proxy_config_network_pdu status 0x%x !!!\n", status);
        }
    }
    else
    {
        LOG(LOG_LVL_ERROR, "!!!mesh_gatt_proxy_config_pdu_decrypt_callback error !!!\n");
    }
}
/**
 ****************************************************************************************
 * @brief The ble Gatt porxy tx handler.
 *
 * @param[in] conn_index  gatt connect index.
 * @param[in] pdu_type    The gatt proxy pdu type.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      The length of write data.
 *
 ****************************************************************************************
 */
void mesh_gatt_proxy_tx_handle(uint16_t conn_index,mesh_gatt_pdu_type_t pdu_type,const uint8_t * p_data,uint16_t length)
{
//    proxy_connection_t * p_connection = &m_connections[conn_index];
    uint16_t conn_handle = mesh_gatt_conn_index_to_handle(conn_index);

    if(conn_handle != MESH_GATT_CONN_INDEX_INVALID )
    {
        LOG(LOG_LVL_INFO, "TX GATT PDU type 0x%x, len %u\n", pdu_type, length);
        LOG(LOG_LVL_INFO,"mesh_gatt_proxy_tx_handle: pdu=");
        log_hex_data((uint8_t *)p_data,length);

        switch (pdu_type)
        {
            case MESH_GATT_PDU_TYPE_NETWORK_PDU:
            case MESH_GATT_PDU_TYPE_MESH_BEACON:
            case MESH_GATT_PDU_TYPE_PROXY_CONFIG:
                mesh_proxy_data_out(conn_handle,p_data,length);
                break;
            case MESH_GATT_PDU_TYPE_PROV_PDU:
                mesh_provisioning_data_out(conn_handle,p_data,length);
                break;
            default:
                /* Ignore unknown PDU type, according to Mesh Profile Specification v1.0, section 6.6 */
                break;
        }
    }
    else
    {
        LOG(LOG_LVL_ERROR,"!!! mesh_gatt_proxy_tx_handle conn_handle error !!! \n");
    }
}
/**
 ****************************************************************************************
 * @brief The ble Gatt porxy tx addr is in the filter valid.
 *
 * @param[in] conn_index  The gatt connect index.
 * @param[in] dst_addr    The destination address.
 *
 * @return Whether or not the tx address is valid( true/false).
 *
 ****************************************************************************************
 */
bool mesh_gatt_proxy_tx_addr_is_valid(uint16_t conn_index,uint16_t dst_addr)
{
    proxy_connection_t * p_connection = &m_connections[conn_index];

    return ((p_connection->connected) && mesh_proxy_filter_addr_is_accept(&p_connection->filter,dst_addr));
}

/// @} BLE_MESH_GATT_PROXY

