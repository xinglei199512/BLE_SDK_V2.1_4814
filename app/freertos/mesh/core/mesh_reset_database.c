
#include "osapp_config.h"
#include "mesh_reset_database.h"
#include "timers.h"
#include "ble_task.h"
#include "gapm_task.h"
#include "mesh_gatt.h"
#include "security.h"
#include "adv_bearer_tx.h"
#include "network_tx_process.h"
#include "upper_tx_process.h"
#include "mesh_sched.h" 
#include "unprov_device_intf.h" 
#include "mesh_gatt_proxy.h" 
#include "provisioning_s.h"
#include "unprov_device_fsm.h"

typedef enum
{
    DATABASE_RESET_MASK_INVALID=0,
    DATABASE_RESET_MASK_DISCONNECT = 0x01,
    DATABASE_RESET_MASK_PROVISION = 0x02,
    DATABASE_RESET_MASK_FLASHSAVE = 0x04,
    DATABASE_RESET_MASK = 0x07,
}database_mask_t;

static bool             reset_database_flag = false;
static uint8_t          database_reset_pending = false;

static int32_t osapp_reset()
{
    struct gapm_reset_cmd *cmd = AHI_MSG_ALLOC(GAPM_RESET_CMD,TASK_ID_GAPM,gapm_reset_cmd);
    cmd->operation = GAPM_RESET;
    return osapp_msg_build_send(cmd, sizeof(struct gapm_reset_cmd));
}

void provision_set_database_reset(void)
{
    reset_database_flag = true;
}


void provision_clear_database_pending(void)
{
    database_reset_pending = false;
}

void provision_set_database_pending(void)
{
    database_reset_pending = true;
}
void provision_reset_database_finish(void)
{
    provision_clear_database_pending();
    ble_mesh_gatt_adv_start(PROXY_ADV_EVT_PROVISIONING_DONE,0);
    
}

bool provision_is_database_pending(void)
{
    return database_reset_pending;
}

void provision_reset_database_start(void)
{
    extern void mesh_stack_reinit(void);
    mesh_stack_reinit();
    osapp_reset();
    LOG(LOG_LVL_INFO,"provision_reset_database_start\n");
    return;
}

bool get_is_node_can_rx_pkt(void)
{


    #if MESH_PROVISION_SERVER_SUPPORT
     if(provision_is_database_pending())
     {
        //LOG(3, "%s:%d\n", __func__, __LINE__);
         return false;
     }
     if(get_is_provisioned())
     {
         return true;
     }
    #endif
    #if MESH_PROVISION_CLIENT_SUPPORT
        return true;
    #endif
    return false;
}


bool get_is_node_can_tx_pkt(void)
{
    #if MESH_PROVISION_SERVER_SUPPORT
     if(provision_is_database_pending())
     {
         return false;
     }
     if(get_is_provisioned())
     {
         return true;
     }
    #endif
    #if MESH_PROVISION_CLIENT_SUPPORT
        return true;
    #endif
    return false;
}


void provision_delete_provision_database(void)
{
      if(reset_database_flag)
      {
         if((provision_system_method_get() & PROVISION_BY_GATT) == PROVISION_BY_GATT)
          {
             // platform_reset(0x0);
                 mesh_sched_stop_scan(provision_reset_database_start);
          }
          reset_database_flag  = false;
      }
}

/**
 ****************************************************************************************
 * @brief The ble mesh gatt events.
 *
 * @param[in] evt  gatt proxy adv evt.
 * @param[in] p_param   Pointer to the param data.
 *
 ****************************************************************************************
 */
void ble_mesh_gatt_evt_update(mesh_gatt_evt_t evt,const uint8_t * p_param)
{
    static volatile database_mask_t l_database_mask = DATABASE_RESET_MASK_DISCONNECT;

    LOG(LOG_LVL_INFO, "ble_mesh_gatt_evt_update %d \n",evt);

    taskENTER_CRITICAL();

    switch(evt)
    {
    case BLE_MESH_EVT_PROVISIONING_DONE :
    {
        gatt_provisioning_states_t state = PROVISIONING_DONE;
        ble_mesh_gatt_adv_start(PROXY_ADV_EVT_PROVISIONING_DONE,&state);
        l_database_mask |= DATABASE_RESET_MASK_PROVISION;
    }
        break;
    case BLE_MESH_EVT_DISCONNECT :
        l_database_mask |= DATABASE_RESET_MASK_DISCONNECT;
        break;
    case BLE_MESH_EVT_FLASHSAVE_DONE :
        l_database_mask |= DATABASE_RESET_MASK_FLASHSAVE;
        break;
    case BLE_MESH_EVT_CONNECTED :
        l_database_mask &= ~DATABASE_RESET_MASK_DISCONNECT;
        break;
    default:
        l_database_mask = DATABASE_RESET_MASK_INVALID;
        break;
    }

    //check evt is all ready
    if((l_database_mask&DATABASE_RESET_MASK) == DATABASE_RESET_MASK)
    {
        // device is ready to reset.
        provision_delete_provision_database();//TODO: ！！！考虑多次重新入网，设备角色属性
    }

    taskEXIT_CRITICAL();
}

