#ifndef HEARTH_SERVER__H
#define HEARTH_SERVER__H
#include "mesh_model.h"
#include "stdint.h"
#include "access_rx_process.h"
#include "model_servers_events_api.h"
#include "timer_wrapper.h"
#include "health_common.h"

#define  HEALTH_FAULT_NUM_MAX 20
#define FAST_PERIOD_DIVISOR_MAX  15
enum {
    HEALTH_NO_FAULT = 0,
    HEALTH_BATTERY_LOW_WARNING,
    HEALTH_BATTERY_LOW_ERROR,
    HEALTH_SUPPLY_VOLTAGE_TOO_LOW_WARNING,
    HEALTH_SUPPLY_VOLTAGE_TOO_LOW_ERROR,
    HEALTH_SUPPLY_VOLTAGE_TOO_HIGH_WARNING,
    HEALTH_SUPPLY_VOLTAGE_TOO_HIGH_ERROR,
    HEALTH_POWER_SUPPLY_INTERRUPTED_WEARNING,
    HEALTH_POWER_SUPPLY_INTERRUPTED_ERROR,
    HEALTH_NO_LOAD_WARNING,
    HEALTH_NO_LOAD_ERROR,
    HEALTH_OVERLOAD_WARNING,
    HEALTH_OVERLOAD_ERROR,
    HEALTH_OVERHEAT_WARNING,
    HEALTH_OVERHEAT_ERROR,
    HEALTH_CONDENSATION_WARNING,
    HEALTH_CONDENSATION_ERROR = 0x10,
    HEALTH_VIBRATION_WARNING,
    HEALTH_VIBRATION_ERROR,
    HEALTH_CONFIGRATION_WARNING,
    HEALTH_CONFIGRATION_ERROR,
    HEALTH_ELEMENT_NOT_CALIBRATED_WARNING,
    HEALTH_ELEMENT_NOT_CALIBRATED_ERROR,
    HEALTH_MEMORY_WARNING,
    HEALTH_MEMORY_ERROR,
    HEALTH_SELT_TEST_WARNING,
    HEALTH_SELT_TEST_ERROR,
    HEALTH_INPUT_TOO_LOW_WARNING,
    HEALTH_INPUT_TOO_LOW_ERROR,
    HEALTH_INPUT_TOO_HIGH_WARNING,
    HEALTH_INPUT_TOO_HIGH_ERROR,
    HEALTH_INPUT_NO_CHANGE_WARNING,
    HEALTH_INPUT_NO_CHANGE_ERROR = 0x20,
    HEALTH_ACTUATOR_BLOCKED_WARNING,
    HEALTH_ACTUATOR_BLOCKED_ERROR,
    HEALTH_HOSING_OPENED_WARNING,
    HEALTH_HOSING_OPENED_ERROR,
    HEALTH_TAMPER_WARNING,
    HEALTH_TAMPER_ERROR,
    HEALTH_DEVICE_MOVES_WARNING,
    HEALTH_DEVICE_MOVES_ERROR,
    HEALTH_DEVICE_DROPPED_WARNING,
    HEALTH_DEVICE_DROPPED_ERROR,
    HEALTH_OVERFLOW_WARNING,
    HEALTH_OVERFLOW_ERROR,
    HEALTH_EMPTY_WARNING,
    HEALTH_EMPTY_ERROR,
    HEALTH_INTERNAL_BUS_WARNING,
    HEALTH_INTERNAL_BUS_ERROR = 0x30,
    HEALTH_MECHANISM_JAMMED_WARNING,
    HEALTH_MECHANISM_JAMMED_ERROR,
    HEALTH_FAULT_MAX
};

typedef struct 
{
    model_server_base_t model;
    uint8_t test_id;
    uint16_t company_id;
    uint8_t  fast_period_divisor;
    publish_period_t health_publish_time;
    uint8_t registered_faults_array[HEALTH_FAULT_NUM_MAX];
    uint8_t current_faults_array[HEALTH_FAULT_NUM_MAX];
    uint8_t attention;       
    mesh_model_evt_cb_t cb;
    mesh_timer_t attention_Timer;
    mesh_timer_t publish_Timer;
}health_server_t;

#define DEF_HEALTH_SERVER_MODEL(name,app_key_max) \
    static health_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_HEALTH_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,HEALTH_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)health_period_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

 
void health_fault_attention_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_attention_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_attention_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_period_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_period_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_period_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_test_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_test_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_clear_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_clear_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void health_fault_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
int health_get_fault_array_size(uint8_t *fault_array);
void health_period_publish(health_server_t *server);
      
#endif


