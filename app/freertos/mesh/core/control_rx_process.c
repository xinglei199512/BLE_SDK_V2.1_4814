#define LOG_TAG        "control_rx_process.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include "upper_pdu.h"
#include "control_rx_process.h"
#include "upper_rx_process.h"
#include "log.h"
#include "aes_ccm_cmac.h"
#include "mesh_env.h"
#include "stack_mem_cfg.h"
#include "upper_rx_process.h"
#include "model_rx.h"
#include "heartbeat.h"
#include "friend.h"
#include "low_power.h"
#include "heartbeat.h"

static void (*const control_pdu_rx_handler[CONTROL_PDU_TYPE_MAX - 1])(upper_pdu_rx_t *) = 
{
    #if (MESH_SUPPORT_LOW_POWER != MESH_FEATURE_NOT_SUPPORT)
    [FRIEND_UPDATE - 1] = friend_update_rx_handler,
    [FRIEND_OFFER - 1] = friend_offer_rx_handler, 
    [FRIEND_SUBSCRIPTION_LIST_CONFIRM - 1] = friend_subscription_list_confirm_rx_handler,
    #endif
    #if (MESH_SUPPORT_FRIEND != MESH_FEATURE_NOT_SUPPORT)
    [FRIEND_POLL - 1] = friend_poll_rx_handler,
    [FRIEND_REQUEST - 1] = friend_request_rx_handler,
    [FRIEND_CLEAR - 1] = friend_clear_rx_handler,
    [FRIEND_CLEAR_CONFIRM -1] = friend_clear_confirm_rx_handler,
    [FRIEND_SUBSCRIPTION_LIST_ADD - 1] = friend_subscription_list_add_rx_handler,
    [FRIEND_SUBSCRIPTION_LIST_REMOVE - 1] = friend_subscription_list_remove_rx_handler,
    #endif
    [HEARTBEAT - 1] = heartbeat_rx_handler,
};


void control_pdu_rx_process(upper_pdu_rx_t *pdu)
{
    uint8_t opcode = pdu->head.control.opcode;
    LOG_D("%s,opcode=0x%x",__func__,opcode);

	if(opcode >= CONTROL_PDU_TYPE_MAX)
    {
    	// opcode error : do nothing but do release pdu;
        LOG_D("%s: error opcode=0x%x",__func__,opcode);
    }else
    {
        if(control_pdu_rx_handler[opcode-1])
        {
            control_pdu_rx_handler[opcode-1](pdu);
        }else
        {
            //LOG
        }
    }
    upper_rx_pdu_free(pdu);
}
