/*
 * proxy_s.c
 *
 *  Created on: 2018��7��31��
 *      Author: huichen
 */
/**
 ****************************************************************************************
 * @addtogroup BLE_BX2400_MESH_GATT_PROXY BLE mesh gatt proxy
 * @ingroup BLE_BX2400_MESH_GATT
 * @brief defines for BLE mesh gatt proxy
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

#include "proxy_s.h"
//#include "osapp_mesh.h"
#include "osapp_utils.h"
#include "security.h"
#include "osapp_utils.h"
#include "config_server.h"
#include "mesh_env.h"

#include "sdk_mesh_definitions.h"
#include "sdk_mesh_config_pro.h"
#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "mesh_core_api.h"
#include "adv_bearer_tx.h"
#include "timer_wrapper.h"
#include "mesh_reset_database.h"
#include "mesh_queued_msg.h"
#include "mesh_gatt_proxy.h"
//#include "provision_tx_process.h"
#include "mesh_gatt_serv_adv.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define PROXY_NODE_IDENTITY_ADV_DURATION   pdMS_TO_TICKS(60000)// 60s

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum
{
    GATT_PROXY_BEACON_IDLE,
    GATT_PROXY_BEACON_NETWORK_ID_START,
    GATT_PROXY_BEACON_NODE_IDENTITY_START,
    GATT_PROXY_BEACON_STOP
}gatt_proxy_beacon_state_t;

/*
typedef enum
{
    GATT_PROXY_ADV_ST_IDLE,
    GATT_PROXY_ADV_ST_SENDING,
    GATT_PROXY_ADV_ST_TOSEND,
    GATT_PROXY_ADV_ST_SENDED
}gatt_proxy_adv_state_t;
*/
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/**
 * @brief struct to mesh gatt network id information
 */
typedef struct
{
    /// beaconing state
    uint8_t * p_network_id[DM_CFG_NETKEY_MAX_NUM];//
    uint8_t index;//now index
    uint8_t max;//max index
}mesh_gatt_networkid_cache_t;
/**
 * @brief struct to mesh gatt provisioning env information
 */
typedef struct
{
    /// beaconing state
    gatt_proxy_beacon_state_t beaconing_state;//now state
    gatt_proxy_beacon_state_t beaconing_state_pre;//Previous state
    mesh_gatt_networkid_cache_t adv_netid;//networkcache
    mesh_timer_t    node_identity_timer;
    mesh_timer_t    adv_period_timer;//adv period timer
//    volatile gatt_proxy_adv_state_t    adv_state;//Does adv send now? if adv  send in the adv period,delay one period,then send
    uint16_t gatt_notify_cfg;
}mesh_gatt_proxy_env_t;

/*
 * DEFINES
 ****************************************************************************************
 */
#define PROXY_ADV_TYPE_ARRAY_IDX         11//array index
#define PROXY_NETWORK_ID_ARRAY_IDX         12//array index
#define PROXY_HAND_ARRAY_IDX              12//array index
#define PROXY_RANDOM_ARRAY_IDX             20//array index

#define PROXY_SERVER_DATA_LEN_ARRAY_IDX  7//array index
#define PROXY_NODE_IDENTITY_SERVER_DATA_LEN  0x14
#define PROXY_NETWORK_ID_SERVER_DATA_LEN     0x0C

#define PROXY_NODE_IDENTITY_ADV_LENGTH   28
#define PROXY_NETWORK_ID_ADV_LENGTH      20
#define NODE_IDENTITY_HASH_LENGTH          8
#define NODE_IDENTITY_RANDOM_LENGTH         8
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static uint16_t proxy_notify_seqnum_updata(void);
static uint8_t proxy_adv_set_network_id(const uint8_t *p_network_id);
static uint8_t proxy_adv_set_node_identity(const uint8_t *p_hash,const uint8_t *p_random);
static void proxy_service_adv_start(void);
static void proxy_node_identity_timer_callback( mesh_timer_t xTimer );
static void proxy_service_beacon_node_identity_adv_start(const uint8_t *p_hash,const uint8_t *p_random);
static void proxy_service_beacon_node_identity_hash_calc(const uint8_t *p_identity_key);
static void proxy_service_beacon_node_identity_hash_calc_done(const uint8_t *p_hash,const uint8_t *p_random);
static void proxy_service_beacon_network_id_update(void);
static void proxy_service_beacon_network_id_multiple_update(void);
static void proxy_service_beacon_network_id_ready(void);
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
struct gattm_svc_desc const proxy_svc_desc ={
        .start_hdl = 0,
        .task_id = TASK_ID_AHI,
        .perm = PERM(SVC_MI,DISABLE)|PERM(SVC_EKS,DISABLE)|\
            PERM(SVC_AUTH,NO_AUTH)|PERM(SVC_UUID_LEN,UUID_16),PERM_VAL(SVC_SECONDARY,0),
        .nb_att = PROXY_SVC_ATT_NUM,
        .uuid = TO_2BYTES_ARRAY(ATT_SVC_MESH_PROXY),
};

struct gattm_att_desc const proxy_svc_att_db[PROXY_SVC_ATT_NUM] = {
    [PROXY_SVC_IDX_IN_CHAR] = {
        .uuid = TO_2BYTES_ARRAY(ATT_DECL_CHARACTERISTIC),
        .perm = PERM(RD,ENABLE),
        .max_len = 0,
        .ext_perm= PERM(UUID_LEN,UUID_16),
    },
    [PROXY_SVC_IDX_IN_VAL] = {
        .uuid = TO_2BYTES_ARRAY(ATT_CHAR_MESH_PROXY_DATA_IN),
        .perm = PERM(WRITE_COMMAND,ENABLE)|PERM(WP,NO_AUTH),
        .max_len = PROXY_PDU_LENGTH_MAX,
        .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
    },
    [PROXY_SVC_IDX_OUT_CHAR] = {
        .uuid = TO_2BYTES_ARRAY(ATT_DECL_CHARACTERISTIC),
        .perm = PERM(RD,ENABLE),
        .max_len = 0,
        .ext_perm = PERM(UUID_LEN,UUID_16),
    },
    [PROXY_SVC_IDX_OUT_VAL] = {
        .uuid = TO_2BYTES_ARRAY(ATT_CHAR_MESH_PROXY_DATA_OUT),
        .perm = PERM(NTF,ENABLE),
        .max_len = PROXY_PDU_LENGTH_MAX,
        .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
    },
    [PROXY_SVC_IDX_OUT_NTF_CFG] = {
      .uuid = TO_2BYTES_ARRAY(ATT_DESC_CLIENT_CHAR_CFG),
      .perm = PERM(RD,ENABLE)|PERM(WRITE_REQ,ENABLE),
       .max_len = 0,
       .ext_perm = PERM(UUID_LEN,UUID_16),
    },
};
/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static uint8_t proxy_adv_data[PROXY_NODE_IDENTITY_ADV_LENGTH] = {
0x2,GAP_AD_TYPE_FLAGS,GAP_LE_GEN_DISCOVERABLE_FLG|GAP_BR_EDR_NOT_SUPPORTED,
0x3,GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID,ATT_SVC_MESH_PROXY&0xff,ATT_SVC_MESH_PROXY>>8&0xff,
0x15,GAP_AD_TYPE_SERVICE_16_BIT_DATA,ATT_SVC_MESH_PROXY&0xff,ATT_SVC_MESH_PROXY>>8&0xff,
};

static mesh_gatt_proxy_env_t l_proxy_env=
{
        .beaconing_state = GATT_PROXY_BEACON_IDLE,
        .beaconing_state_pre = GATT_PROXY_BEACON_IDLE,
        .adv_netid.max = 0,
        .adv_netid.index = 0,
        .gatt_notify_cfg = 0,
};




/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief The proxy read req ind callback.
 *
 ****************************************************************************************
 */
void proxy_read_req_ind_callback(struct osapp_svc_helper_s const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx)
{
    LOG(LOG_LVL_INFO,"proxy_read_req_ind_callback att = %d \n",att_idx);

    switch(att_idx)
    {
    case PROXY_SVC_IDX_OUT_NTF_CFG :
        {
            struct gattc_read_cfm *cfm = AHI_MSG_ALLOC_DYN(GATTC_READ_CFM,src_id, gattc_read_cfm,2);

            cfm->handle =  osapp_get_att_handle_helper((osapp_svc_helper_t *)svc_helper,att_idx);
            cfm->status = ATT_ERR_NO_ERROR;
            cfm->length = 2;
            memcpy(cfm->value,(uint8_t *)&l_proxy_env.gatt_notify_cfg,cfm->length);

            osapp_msg_build_send(cfm,(sizeof(struct gattc_read_cfm)+2));
        }
        break;
    default:break;
    }
}
/**
 ****************************************************************************************
 * @brief The proxy write req ind callback.
 *
 ****************************************************************************************
 */
void proxy_write_req_ind_callback(struct osapp_svc_helper_s const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx,uint16_t offset,uint16_t length,uint8_t const*value)
{
//1 write cfg
    struct gattc_write_cfm *cfm = AHI_MSG_ALLOC(GATTC_WRITE_CFM, src_id, gattc_write_cfm);

    LOG(LOG_LVL_INFO,"proxy_write_req_ind_callback att = %d \n",att_idx);

    cfm->handle =  osapp_get_att_handle_helper((osapp_svc_helper_t *)svc_helper,att_idx);
    switch(att_idx)
    {
    case PROXY_SVC_IDX_IN_VAL :
        if(offset != 0)    cfm->status = ATT_ERR_INVALID_OFFSET;
        else            cfm->status = ATT_ERR_NO_ERROR;
        break;
    case PROXY_SVC_IDX_OUT_NTF_CFG :
        if(offset != 0) cfm->status = ATT_ERR_INVALID_OFFSET;
        else if(length != 2) cfm->status = ATT_ERR_REQUEST_NOT_SUPPORTED;
        else
        {
            cfm->status = ATT_ERR_NO_ERROR;
            memcpy((uint8_t *)&l_proxy_env.gatt_notify_cfg,value,length);

            if(l_proxy_env.gatt_notify_cfg == MESH_GATT_NOTIFICATION)
            {
                provision_active_method_set(PROVISION_BY_GATT);//set active method by gatt
            }
        }
        break;
    default:
        cfm->status = ATT_ERR_INVALID_HANDLE;
        break;
    }
//2
    osapp_msg_build_send(cfm,sizeof(struct gattc_write_cfm));
//3 write exe
    switch(att_idx)
    {
    case PROXY_SVC_IDX_IN_VAL :
        mesh_proxy_data_in(MESH_CONNECT_SRCID_TO_HANDLE(src_id),value,length);
        break;
    case PROXY_SVC_IDX_OUT_NTF_CFG :
        break;
    default:
        break;
    }
}
/**
 ****************************************************************************************
 * @brief The Mesh Proxy Data In characteristic is used by the client to send Proxy PDUs
 *  (see Section 6.3) to the server.
 *
 * @param[in] conn_handle  gatt connect handle.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      he length of write data.
 *
 *
 ****************************************************************************************
 */
void mesh_proxy_data_in(uint16_t conn_handle, const uint8_t * p_data, uint16_t length)
{
    const mesh_gatt_proxy_pdu_t * p_pdu = (const mesh_gatt_proxy_pdu_t *) p_data;

    switch(p_pdu->pdu_type)
    {
        case MESH_GATT_PDU_TYPE_NETWORK_PDU:
        case MESH_GATT_PDU_TYPE_MESH_BEACON:
        case MESH_GATT_PDU_TYPE_PROXY_CONFIG:
        {
            mesh_gatt_proxy_pdu_rx(conn_handle,p_data,length);
            break;
        }
        default:
            /* Invalid PDU in this state. */
            mesh_gatt_disconnect(conn_handle,CO_ERROR_PARAM_OUT_OF_MAND_RANGE);
            break;
    }
}
/**
 ****************************************************************************************
 * @brief The Mesh Proxy Data Out characteristic is used by the server to send Proxy PDUs to the client.
 *
 * @param[in] conn_handle  gatt connect handle.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      he length of write data.
 *
 *
 ****************************************************************************************
 */
void mesh_proxy_data_out(uint16_t conn_handle, const uint8_t * p_data, uint16_t length)
{
    const mesh_gatt_proxy_pdu_t * p_pdu = (const mesh_gatt_proxy_pdu_t *) p_data;

    switch(p_pdu->pdu_type)
    {
        case MESH_GATT_PDU_TYPE_NETWORK_PDU:
        case MESH_GATT_PDU_TYPE_MESH_BEACON:
        case MESH_GATT_PDU_TYPE_PROXY_CONFIG:
        {
            //send_notification the date to air
            uint8_t * p_msg = gatt_alloc_notify_msg(conn_handle,length,PROXY_NOTIFY);

            if(p_msg)
            {
                memcpy(p_msg,p_data,length);
                gatt_notify_msg_send(p_msg,proxy_notify_seqnum_updata());
            }
            else
            {
                LOG(LOG_LVL_ERROR,"!!! mesh_proxy_data_out p_msg error !!! \n");
            }
            break;
        }
        default:
            /* Invalid PDU in this state. */
            break;
    }
}


void proxy_service_adv_callback(mesh_timer_t tTimer)
{
    //LOG(3, "proxy_service_adv_callback\n");
    mesh_queued_msg_send((void (*)(void *))proxy_service_adv_start,NULL);
}

/**
 ****************************************************************************************
 * @brief The Mesh Proxy Data Init.
 *
 ****************************************************************************************
 */
void mesh_proxy_service_data_init(void)
{
    //1. get config
    mesh_core_params_t param_proxy_services_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT   , &param_proxy_services_transmit);
    //2. create timer
    l_proxy_env.node_identity_timer = mesh_timer_create("node_identity_timer",PROXY_NODE_IDENTITY_ADV_DURATION,pdFALSE,(void *)0,proxy_node_identity_timer_callback);
    BX_ASSERT(l_proxy_env.node_identity_timer !=0 );
    if(param_proxy_services_transmit.proxy_services_transmit.period)//check period is zero
    {
         l_proxy_env.adv_period_timer = mesh_timer_create("adv_period_timer",pdMS_TO_TICKS(param_proxy_services_transmit.proxy_services_transmit.period),pdTRUE,(void *)0,proxy_service_adv_callback);
         BX_ASSERT(l_proxy_env.adv_period_timer !=0 );
    }
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon network id ready.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_network_id_ready(void)
{
    LOG(LOG_LVL_INFO,"proxy_service_beacon_network_id_ready \n");

    //1.update network key list
    proxy_service_beacon_network_id_update();
    //2.set adv data
    proxy_service_beacon_network_id_multiple_update();
    //3.updata env
    l_proxy_env.beaconing_state = GATT_PROXY_BEACON_NETWORK_ID_START;
    //4. save GATT_PROXY_BEACON_NETWORK_ID_START
    proxy_service_beacon_stop();
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon network id start.
 *
 ****************************************************************************************
 */
void proxy_service_beacon_network_id_start(void)
{
    LOG(LOG_LVL_INFO,"proxy_service_beacon_network_id_start \n");

    //1.update network key list
    proxy_service_beacon_network_id_update();
    //2.set adv data
    proxy_service_beacon_network_id_multiple_update();
    //3.updata env
    l_proxy_env.beaconing_state = GATT_PROXY_BEACON_NETWORK_ID_START;
    //4.start adv
    //proxy_service_beacon_continue();
    mesh_timer_reset(l_proxy_env.adv_period_timer);

}
/**
 ****************************************************************************************
 * @brief The proxy beacon node identity start.
 *
 * @param[in] p_identity_key  Pointer to the identity key.
 *
 ****************************************************************************************
 */
void proxy_service_beacon_node_identity_start(const uint8_t *p_identity_key)
{
    //1.hash calc
    proxy_service_beacon_node_identity_hash_calc(p_identity_key);
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon stop.
 *
 ****************************************************************************************
 */
void proxy_service_beacon_stop(void)
{
    if( l_proxy_env.beaconing_state != GATT_PROXY_BEACON_IDLE )
    {
        //1. update pre state
        l_proxy_env.beaconing_state_pre = l_proxy_env.beaconing_state;
        //2. update now state
        l_proxy_env.beaconing_state = GATT_PROXY_BEACON_STOP;
        //3. update timer stop
        //l_proxy_env.adv_state = GATT_PROXY_ADV_ST_IDLE;
//        xTimerStop(l_proxy_env.adv_period_timer,portMAX_DELAY);

    }
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon restart (adv data not update).
 *
 ****************************************************************************************
 */
void proxy_service_beacon_restart(void)
{
    LOG(LOG_LVL_INFO,"proxy_service_beacon_restart \n");
	
    if(    (l_proxy_env.beaconing_state != GATT_PROXY_BEACON_IDLE)
        && (l_proxy_env.beaconing_state_pre != GATT_PROXY_BEACON_IDLE)
        )
    {
        //1.updata env
        l_proxy_env.beaconing_state = l_proxy_env.beaconing_state_pre;
        //2.start adv
        //proxy_service_beacon_continue();
        mesh_timer_reset(l_proxy_env.adv_period_timer);

    }
}


/**
 ****************************************************************************************
 * @brief The proxy node identity timer time out callback.
 *
 * @param[in] xTimer  Timer Handle.
 *
 ****************************************************************************************
 */
static void proxy_node_identity_timer_callback( mesh_timer_t xTimer )
{
    LOG(LOG_LVL_INFO,"proxy_node_identity_timer_callback \n");

    switch(l_proxy_env.beaconing_state)
    {
        case GATT_PROXY_BEACON_STOP ://stop -> net id
        {
            proxy_service_beacon_network_id_ready();
        }
        break;
        case GATT_PROXY_BEACON_NODE_IDENTITY_START ://identity -> net id
        {
            //1.node identity beacon stop
            proxy_service_beacon_stop();
            //2. network id beacon start
            proxy_service_beacon_network_id_start();
        }
        break;
        default:break;
    }
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon network id multiple adv data update.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_network_id_multiple_update(void)
{
    //LOG(LOG_LVL_INFO,"proxy_service_beacon_network_id_multiple_update %d \n",l_proxy_env.adv_netid.index);

    if(l_proxy_env.adv_netid.index >= l_proxy_env.adv_netid.max)
    {
        //1.update network key list
        //proxy_service_beacon_network_id_update();
        l_proxy_env.adv_netid.index = 0;
    }

    //2.set adv data
    proxy_adv_set_network_id(l_proxy_env.adv_netid.p_network_id[l_proxy_env.adv_netid.index]);
    l_proxy_env.adv_netid.index++;
}
/**
 ****************************************************************************************
 * @brief The proxy adv set network id.
 *
 * @param[in] p_network_id  Pointer to the identity key.
 *
 * @return The proxy nerwork adv total length.
 *
 ****************************************************************************************
 */
static uint8_t proxy_adv_set_network_id(const uint8_t *p_network_id)
{
    proxy_adv_data[PROXY_ADV_TYPE_ARRAY_IDX] = Network_ID_Type;
    proxy_adv_data[PROXY_SERVER_DATA_LEN_ARRAY_IDX] = PROXY_NETWORK_ID_SERVER_DATA_LEN;
    memcpy(&proxy_adv_data[PROXY_NETWORK_ID_ARRAY_IDX],p_network_id,NETWORK_ID_LENGTH);
    return PROXY_NETWORK_ID_ADV_LENGTH;
}
/**
 ****************************************************************************************
 * @brief The proxy adv set node identity.
 *
 * @param[in] p_hash  Pointer to the hash value.
 * @param[in] p_random  Pointer to the random value.
 *
 * @return The proxy node identity adv total length.
 *
 ****************************************************************************************
 */
static uint8_t proxy_adv_set_node_identity(const uint8_t *p_hash,const uint8_t *p_random)
{
    proxy_adv_data[PROXY_ADV_TYPE_ARRAY_IDX] = Node_Identity_Type;
    proxy_adv_data[PROXY_SERVER_DATA_LEN_ARRAY_IDX] = PROXY_NODE_IDENTITY_SERVER_DATA_LEN;
    memcpy(&proxy_adv_data[PROXY_HAND_ARRAY_IDX],p_hash,NODE_IDENTITY_HASH_LENGTH);
    memcpy(&proxy_adv_data[PROXY_RANDOM_ARRAY_IDX],p_random,NODE_IDENTITY_RANDOM_LENGTH);
    return PROXY_NODE_IDENTITY_ADV_LENGTH;
}


static void proxy_send_pkt(void)
{
    uint8_t length  = 0;
    if(provision_is_database_pending())
    {
         return;
    }
    if(proxy_adv_data[PROXY_ADV_TYPE_ARRAY_IDX]==Network_ID_Type)
    {
       length = PROXY_NETWORK_ID_ADV_LENGTH;
    }else
    {
       length = PROXY_NODE_IDENTITY_ADV_LENGTH;
    }
    gatt_service_adv_tx_t * ptr = gatt_service_adv_pkt_prepare(proxy_adv_data,length,GATT_SERVICE_ADV_PKT);
    gatt_service_adv_tx_start(ptr);

}
/**
 ****************************************************************************************
 * @brief The proxy adv start (send tx queue).
 *
 * @param[in] callback  adv is done to callback the func.
 *
 ****************************************************************************************
 */
static void proxy_service_adv_start(void)
{
    switch(l_proxy_env.beaconing_state)
    {
        case GATT_PROXY_BEACON_NETWORK_ID_START :
        {
            //LOG(LOG_LVL_INFO,"GATT_PROXY_BEACON_NETWORK_ID_START \n");
            proxy_service_beacon_network_id_multiple_update();
            proxy_send_pkt();
        }break;
        case GATT_PROXY_BEACON_NODE_IDENTITY_START :
        {
            //LOG(LOG_LVL_INFO,"GATT_PROXY_BEACON_NODE_IDENTITY_START \n");
            proxy_send_pkt();
        }break;
        default:break;
    }
}
/**
 ****************************************************************************************
 * @brief  proxy update notify seq num
 *
 * @return seqnum  0x1000~0x1ffe;
 ****************************************************************************************
 */
static uint16_t proxy_notify_seqnum_updata(void)
{
    static uint16_t l_proxy_ntf_seqnum = 0;

    l_proxy_ntf_seqnum++;
    l_proxy_ntf_seqnum %= 0x0fff;
    l_proxy_ntf_seqnum |= 0x1000;

    return l_proxy_ntf_seqnum;
}
/**
 ****************************************************************************************
 * @brief The proxy_service_beacon_network_id_update.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_network_id_update(void)
{
    uint8_t i=0;
    dm_netkey_handle_t netkeylist[DM_CFG_NETKEY_MAX_NUM];
    uint32_t counter = 0;
    
    //1. clean
    memset(&l_proxy_env.adv_netid,0,sizeof(mesh_gatt_networkid_cache_t));
    dm_netkey_handle_list_get(netkeylist,&counter);
    //2. find
    for(i=0;i<counter;i++)
    {
        l_proxy_env.adv_netid.p_network_id[l_proxy_env.adv_netid.max] = (uint8_t *)&(netkeylist[i]->key[netkeylist[i]->primary_used].network_id[0]);
        l_proxy_env.adv_netid.max +=1;
    }
}
/**
 ****************************************************************************************
 * @brief The proxy_service beacon_node identity adv start.
 *
 * @param[in] p_hash      Pointer to the hash calc data.
 * @param[in] p_random    Pointer to the hash random data.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_node_identity_adv_start(const uint8_t *p_hash,const uint8_t *p_random)
{
    //1.set adv data
    proxy_adv_set_node_identity(p_hash,p_random);
    //2.updata env
    l_proxy_env.beaconing_state = GATT_PROXY_BEACON_NODE_IDENTITY_START;
    //3. start timer
    mesh_timer_start(l_proxy_env.node_identity_timer);
    //4. start adv
    //proxy_service_beacon_continue();
     mesh_timer_start(l_proxy_env.adv_period_timer);

}
/**
 ****************************************************************************************
 * @brief The proxy service beacon node identity hash calc done.
 *
 * @param[in] p_hash      Pointer to the hash calc data.
 * @param[in] p_random    Pointer to the hash random data.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_node_identity_hash_calc_done(const uint8_t *p_hash,const uint8_t *p_random)
{
    LOG(LOG_LVL_INFO,"proxy_service_beacon_node_identity_hash_calc_done \n");
    proxy_service_beacon_node_identity_adv_start(p_hash,p_random);
}
/**
 ****************************************************************************************
 * @brief The proxy service beacon node identity hash calc.
 *
 * @param[in] p_identity_key      Pointer to the node identity key data.
 *
 ****************************************************************************************
 */
static void proxy_service_beacon_node_identity_hash_calc(const uint8_t *p_identity_key)
{
    proxy_node_identity_hash_t hash;
    uint16_t primary_element_addr = mesh_node_get_primary_element_addr();

    LOG(LOG_LVL_INFO,"proxy_service_beacon_node_identity_hash_calc \n");
    log_hex_data((uint8_t *)p_identity_key,MESH_KEY_LENGTH);
    LOG(LOG_LVL_INFO,"p_identity_key\n");
    //1. set padding
    memset(hash.metadata.params.padding,0,PROXY_NODE_IDENTITY_HASH_PADDING_LEN);
    //2. set random
    tools_random_generate(&hash.metadata.params.random[0],PROXY_NODE_IDENTITY_HASH_RANDOM_LEN);
    //3. set identity
    hash.p_identiykey =(uint8_t *)p_identity_key;
    //4.set addr
    hash.metadata.params.source_addr[0] = primary_element_addr>>8;//high byte
    hash.metadata.params.source_addr[1] = primary_element_addr;//low byte
    //5.calc
    proxy_node_identity_hash_generation((const uint8_t *)&hash,proxy_service_beacon_node_identity_hash_calc_done);
}

/// @} BLE_BX2400_MESH_GATT_PROXY
