#ifndef GENERIC_LEVEL_SERVER__H
#define GENERIC_LEVEL_SERVER__H
#include <stdint.h>
#include "mesh_model.h"
#include "mesh_env.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
//#include "access.h"
#include "generic_level_common.h"
#include "access_rx_process.h"

#define GENERCI_LEVEL_BOUND_HANDLER_MAX 5

typedef void(*user_generic_level_server_cb)(access_pdu_tx_t *,uint8_t);

enum
{
    MSG_GENERCI_LEVEL_SET,
    MSG_GENERCI_DELTA_SET,
    MSG_GENERCI_MOVE_SET
};


/** Internal structure for holding Set/Delta Set transition related variables */
typedef struct
{
    /** For storing actual required amount of level change. */
    int32_t required_delta;
    /** Initial present level required for handling Set/Delta Set message. */
    int16_t initial_present_level;
}set_transition_t;

/** Internal structure for holding Move transition related variables */
typedef struct
{
    /** Scaled representation of the Level value. */
    int16_t required_move;
    /** Initial present level required for handling Set/Delta Set message. */
    int16_t initial_present_level;
}move_transition_t;

typedef struct
{
    uint16_t target_level;
    uint16_t present_level;
    uint8_t remaining_time;
    int8_t  delta_last_tid;
    uint16_t origin_present_level;
    union {
        /* Parameters for Set Transition */
        set_transition_t set;
        /* Parameters for Move Transition */
        move_transition_t move;
    } params;
}generic_level_msg_format_t;


typedef struct
{
    model_server_base_t model;
    model_state_bound_field_t *state_bound;
    generic_level_msg_format_t msg_format;
    generic_valid_field_queue_t tid_queue;
    generic_delay_trans_param_t *delay_trans_timer;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}generic_level_server_t;

#define DEF_GENERIC_LEVEL_SERVER_MODEL(name,app_key_max) \
      static generic_level_server_t name;\
      static model_publish_state_t name##_publish_state;\
      static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_LEVEL_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,GENERIC_LEVEL_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)generic_level_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;


void generic_level_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_level_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_level_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_delta_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_delta_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_move_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_move_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_level_status_publish(generic_level_server_t *server);


#endif /* GENERIC_LEVEL_SERVER__H */


