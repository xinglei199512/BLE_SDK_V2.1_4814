/*
 * provisioning_s.c
 *
 *  Created on: 2018��8��2��
 *      Author: huichen
 */

/**
 ****************************************************************************************
 * @addtogroup BLE_BX2400_MESH_GATT_PROVISIONING BLE mesh gatt provisioning
 * @ingroup BLE_BX2400_MESH_GATT
 * @brief defines for BLE mesh gatt provisioning
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

#include "provisioning_s.h"
#include "sdk_mesh_definitions.h"
#include "mesh_gatt_error.h"
#include "mesh_gatt.h"
//#include "osapp_mesh.h"
#include "osapp_utils.h"
#include "adv_bearer_tx.h"
#include "timer_wrapper.h"
#include "mesh_reset_database.h"
#include "mesh_queued_msg.h"
#include "mesh_gatt_proxy.h"
//#include "provision_tx_process.h"
#include "mesh_gatt_serv_adv.h"
/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum
{
    GATT_PROV_BEACON_IDLE,
    GATT_PROV_BEACON_START,
    GATT_PROV_BEACON_STOP
}gatt_provisioning_beacon_state_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/**
 * @brief struct to mesh gatt provisioning env information
 */
typedef struct
{
    /// beaconing state
    gatt_provisioning_beacon_state_t beaconing_state;
    gatt_provisioning_states_t provisioning_state;
    uint16_t gatt_notify_cfg;
}mesh_gatt_provisioning_env_t;

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
static uint16_t provisioning_notify_seqnum_updata(void);
static void provision_service_adv_data_set(const uint8_t *dev_uuid,const uint16_t OOB_info);
//static void provision_service_beacon_continue(void);
//static void provision_service_adv_start(void (*callback)());
static void provision_service_adv_start(void);

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
struct gattm_svc_desc const provision_svc_desc ={
        .start_hdl = 0,
        .task_id = TASK_ID_AHI,
        .perm = PERM(SVC_MI,DISABLE)|PERM(SVC_EKS,DISABLE)|\
            PERM(SVC_AUTH,NO_AUTH)|PERM(SVC_UUID_LEN,UUID_16),PERM_VAL(SVC_SECONDARY,0),
        .nb_att = PROVISION_SVC_ATT_NUM,
        .uuid = TO_2BYTES_ARRAY(ATT_SVC_MESH_PROVISIONING),
};

struct gattm_att_desc const provision_svc_att_db[PROVISION_SVC_ATT_NUM] = {
    [PROVISION_SVC_IDX_IN_CHAR] = {
        .uuid = TO_2BYTES_ARRAY(ATT_DECL_CHARACTERISTIC),
        .perm = PERM(RD,ENABLE),
        .max_len = 0,
        .ext_perm= PERM(UUID_LEN,UUID_16),
    },
    [PROVISION_SVC_IDX_IN_VAL] = {
        .uuid = TO_2BYTES_ARRAY(ATT_CHAR_MESH_PROVISIONING_DATA_IN),
        .perm = PERM(WRITE_COMMAND,ENABLE)|PERM(WP,NO_AUTH),
        .max_len = PROXY_PDU_LENGTH_MAX,
        .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
    },
    [PROVISION_SVC_IDX_OUT_CHAR] = {
        .uuid = TO_2BYTES_ARRAY(ATT_DECL_CHARACTERISTIC),
        .perm = PERM(RD,ENABLE),
        .max_len = 0,
        .ext_perm = PERM(UUID_LEN,UUID_16),
    },
    [PROVISION_SVC_IDX_OUT_VAL] = {
        .uuid = TO_2BYTES_ARRAY(ATT_CHAR_MESH_PROVISIONING_DATA_OUT),
        .perm = PERM(NTF,ENABLE),
        .max_len = PROXY_PDU_LENGTH_MAX,
        .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
    },
    [PROVISION_SVC_IDX_OUT_NTF_CFG] = {
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
static uint8_t pb_gatt_adv_data[29] = {
0x2,GAP_AD_TYPE_FLAGS,GAP_LE_GEN_DISCOVERABLE_FLG|GAP_BR_EDR_NOT_SUPPORTED,
0x3,GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID,ATT_SVC_MESH_PROVISIONING&0xff,ATT_SVC_MESH_PROVISIONING>>8&0xff,
0x15,GAP_AD_TYPE_SERVICE_16_BIT_DATA,ATT_SVC_MESH_PROVISIONING&0xff,ATT_SVC_MESH_PROVISIONING>>8&0xff,
};

static mesh_gatt_provisioning_env_t l_provisioning_env=
{
        .beaconing_state = GATT_PROV_BEACON_IDLE,
        .provisioning_state = PROVISIONING_IDLE,
        .gatt_notify_cfg = 0,
};
static  mesh_timer_t provision_service_timer;

//DEF_CIRCLE_SEND_TIMER(provision_service, 500, provision_service_adv_start);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief The provision read req ind callback.
 *
 ****************************************************************************************
 */
void provision_service_timer_callback(mesh_timer_t tTimer)
{
    //LOG(3, "provision_service_timer_callback\n");
     mesh_queued_msg_send((void (*)(void *))provision_service_adv_start,NULL);
}

void provision_service_timer_init(void)
{
     provision_service_timer = mesh_timer_create("provision_timer",pdMS_TO_TICKS(70000),true,(void *)0,provision_service_timer_callback);
}

void provision_read_req_ind_callback(struct osapp_svc_helper_s const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx)
{
    LOG(LOG_LVL_INFO,"provision_read_req_ind_callback att = %d \n",att_idx);

    switch(att_idx)
    {
    case PROVISION_SVC_IDX_OUT_NTF_CFG :
        {
            struct gattc_read_cfm *cfm = AHI_MSG_ALLOC_DYN(GATTC_READ_CFM,src_id, gattc_read_cfm,2);

            cfm->handle =  osapp_get_att_handle_helper((osapp_svc_helper_t *)svc_helper,att_idx);
            cfm->status = ATT_ERR_NO_ERROR;
            cfm->length = 2;
            memcpy(cfm->value,(uint8_t *)&l_provisioning_env.gatt_notify_cfg,cfm->length);

            osapp_msg_build_send(cfm,(sizeof(struct gattc_read_cfm)+2));
        }
        break;
    default:break;
    }
}
/**
 ****************************************************************************************
 * @brief The provision write req ind callback.
 *
 ****************************************************************************************
 */
void provision_write_req_ind_callback(struct osapp_svc_helper_s const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx,uint16_t offset,uint16_t length,uint8_t const*value)
{
//1 write cfg
    struct gattc_write_cfm *cfm = AHI_MSG_ALLOC(GATTC_WRITE_CFM, src_id, gattc_write_cfm);

    LOG(LOG_LVL_INFO,"provision_write_req_ind_callback att = %d \n",att_idx);

    cfm->handle =  osapp_get_att_handle_helper((osapp_svc_helper_t *)svc_helper,att_idx);
    switch(att_idx)
    {
    case PROVISION_SVC_IDX_IN_VAL :
        if(offset != 0) cfm->status = ATT_ERR_INVALID_OFFSET;
        else            cfm->status = ATT_ERR_NO_ERROR;
        break;
    case PROVISION_SVC_IDX_OUT_NTF_CFG :
        if(offset != 0) cfm->status = ATT_ERR_INVALID_OFFSET;
        else if(length != 2) cfm->status = ATT_ERR_REQUEST_NOT_SUPPORTED;
        else
        {
            cfm->status = ATT_ERR_NO_ERROR;
            memcpy((uint8_t *)&l_provisioning_env.gatt_notify_cfg,value,length);

            if(l_provisioning_env.gatt_notify_cfg == MESH_GATT_NOTIFICATION)
            {
                provision_active_method_set(PROVISION_BY_GATT);//set active method by gatt
            }
            else
            {
                provision_active_method_set(PROVISION_BY_ADV);//set active method by adv
            }
        }
        break;
    default:
        cfm->status = ATT_ERR_INVALID_HANDLE;
        break;
    }
//2 cfm
    osapp_msg_build_send(cfm,sizeof(struct gattc_write_cfm));
//3 write exe
    switch(att_idx)
    {
    case PROVISION_SVC_IDX_IN_VAL :
        mesh_provisioning_data_in(MESH_CONNECT_SRCID_TO_HANDLE(src_id),value,length);
        break;
    case PROVISION_SVC_IDX_OUT_NTF_CFG :
        break;
    default:
        break;
    }
}

/**
 ****************************************************************************************
 * @brief The Mesh Provisioning Data In characteristic can be written to send a Proxy PDU message
 *   (see Section 6.3.1) containing Provisioning PDU (see Section 5.3) to the Provisioning Server.
 *
 * @param[in] conn_handle  gatt connect handle.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      he length of write data.
 *
 *
 ****************************************************************************************
 */
void mesh_provisioning_data_in(uint16_t conn_handle, const uint8_t * p_data, uint16_t length)
{
    const mesh_gatt_proxy_pdu_t * p_pdu = (const mesh_gatt_proxy_pdu_t *) p_data;

    LOG(LOG_LVL_INFO,"mesh_provisioning_data_in \n");

    switch(p_pdu->pdu_type)
    {
        case MESH_GATT_PDU_TYPE_PROV_PDU:
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
 * @brief The Mesh Provisioning Data Out characteristic can be notified to send a Proxy PDU message
 *  containing Provisioning PDU from a Provisioning Server to a Provisioning Client.
 *
 * @param[in] conn_handle  gatt connect handle.
 * @param[in] p_data      Pointer to the write data.
 * @param[in] length      he length of write data.
 *
 *
 ****************************************************************************************
 */
void mesh_provisioning_data_out(uint16_t conn_handle, const uint8_t * p_data, uint16_t length)
{
    const mesh_gatt_proxy_pdu_t * p_pdu = (const mesh_gatt_proxy_pdu_t *) p_data;

    switch(p_pdu->pdu_type)
    {
        case MESH_GATT_PDU_TYPE_PROV_PDU:
        {
            //send_notification the date to air
            uint8_t * p_msg = gatt_alloc_notify_msg(conn_handle,length,PROVISION_NOTIFY);

            if(p_msg)
            {
                memcpy(p_msg,p_data,length);
                gatt_notify_msg_send(p_msg,provisioning_notify_seqnum_updata());
            }
            else
            {
                LOG(LOG_LVL_ERROR,"!!! mesh_provisioning_data_out p_msg error !!! \n");
            }

            break;
        }
        default:
            /* Invalid PDU in this state. */
            break;
    }
}
/**
 ****************************************************************************************
 * @brief The Mesh Provisioning state set.
 *
 * @param[in] state  Provisioning state. @enum gatt_provisioning_states_t
 *
 ****************************************************************************************
 */
void mesh_provisioning_state_set(gatt_provisioning_states_t state)
{
    l_provisioning_env.provisioning_state = state;
}
/**
 ****************************************************************************************
 * @brief The provision service beacon restart.
 *
 ****************************************************************************************
 */
void provision_service_beacon_restart(void)
{
    LOG(LOG_LVL_ERROR,"provision_service_beacon_restart  \n");
    if(l_provisioning_env.provisioning_state == PROVISIONING_START)
    {
        if(l_provisioning_env.beaconing_state != GATT_PROV_BEACON_IDLE)
        {
		LOG(LOG_LVL_ERROR,"GATT_PROV_BEACON_IDLE  \n");
			
            //1.updata env
            l_provisioning_env.beaconing_state = GATT_PROV_BEACON_START;
            //2.start adv
            //provision_service_beacon_continue();
            //mesh_circle_timer_restart_timer(&provision_service);
             mesh_timer_reset(provision_service_timer);
LOG (3,"-----provision_service_timer  ----reset----\n");
					
        }
    }
}
/**
 ****************************************************************************************
 * @brief The provision service beacon start.
 *
 * @param[in] dev_uuid  Pointer to the device uuid.
 * @param[in] OOB_info  the OOB info data.
 *
 ****************************************************************************************
 */
void provision_service_beacon_start(const uint8_t *dev_uuid,const uint16_t OOB_info)
{
    //1.set mesh_provisioning_state
    mesh_provisioning_state_set(PROVISIONING_START);
    //2.set adv data
    provision_service_adv_data_set(dev_uuid,OOB_info);
    //3.updata env
    l_provisioning_env.beaconing_state = GATT_PROV_BEACON_START;
    //4.start adv
    //provision_service_beacon_continue();
    mesh_timer_start(provision_service_timer);
		LOG(3,"-------provision_service_timer   1----------\n");


}
/**
 ****************************************************************************************
 * @brief The provision service beacon stop.
 *
 ****************************************************************************************
 */
void provision_service_beacon_stop(void)
{
    if(l_provisioning_env.beaconing_state != GATT_PROV_BEACON_IDLE)
    {
        l_provisioning_env.beaconing_state = GATT_PROV_BEACON_STOP;
        mesh_timer_stop(provision_service_timer);
			
LOG (3,"-----provision_service_timer  ----stop----\n");
		}
	}
/**
 ****************************************************************************************
 * @brief The provision service adv data set.
 *
 * @param[in] dev_uuid  Pointer to the device uuid.
 * @param[in] OOB_info  the OOB info data.
 *
 ****************************************************************************************
 */
static void provision_service_adv_data_set(const uint8_t *dev_uuid,const uint16_t OOB_info)
{
    memcpy(&pb_gatt_adv_data[11],dev_uuid,MESH_DEVICE_UUID_LENGTH);
    pb_gatt_adv_data[27] = (OOB_info>>8)&0xff;
    pb_gatt_adv_data[28] = OOB_info&0xff;
}
/**
 ****************************************************************************************
 * @brief The provision service beacon continue.
 *
 ****************************************************************************************
 */
 /*
static void provision_service_beacon_continue(void)
{
    switch(l_provisioning_env.beaconing_state)
    {
        case GATT_PROV_BEACON_START :
        {
            LOG(LOG_LVL_INFO,"provision_service_beacon_continue \n");
            provision_service_adv_start(provision_service_beacon_continue);
        }break;
        default:break;
    }
}
*/
/**
 ****************************************************************************************
 * @brief The provision adv start (send tx queue).
 *
 * @param[in] callback  adv is done to callback the func.
 *
 ****************************************************************************************
 */
static void provision_service_adv_start(void)
{
    if(provision_is_database_pending())
    {
         return;
    }
    gatt_service_adv_tx_t * ptr = gatt_service_adv_pkt_prepare(pb_gatt_adv_data,sizeof(pb_gatt_adv_data),GATT_SERVICE_ADV_PKT);
    if(NULL == ptr)
    {
        BX_ASSERT(NULL != ptr);
        return;
    }
     gatt_service_adv_tx_start(ptr);
}
/**
 ****************************************************************************************
 * @brief  provisioning update notify seq num
 *
 * @return seqnum  0~0xffe;
 ****************************************************************************************
 */
static uint16_t provisioning_notify_seqnum_updata(void)
{
    static uint16_t l_prov_ntf_seqnum = 0;

    l_prov_ntf_seqnum++;
    l_prov_ntf_seqnum %= 0x0fff;

    return l_prov_ntf_seqnum;
}

/// @} BLE_BX2400_MESH_GATT_PROVISIONING
