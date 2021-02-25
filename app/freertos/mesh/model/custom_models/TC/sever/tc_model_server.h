#ifndef TC_MODEL_SERVER_H_
#define TC_MODEL_SERVER_H_
#include <stdint.h>
#include "mesh_model.h"
#include "mesh_env.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"
#include "tc_model_common.h"

#define GENERCI_LEVEL_TC_BOUND_HANDLER_MAX 5
enum {
    TC_MODEL_ATTR_HSL = 0x0123,
    TC_MODEL_ATTR_ONOFF = 0x0100,
    TC_MODEL_ATTR_LIGHTNESS = 0x0121,
    TC_MODEL_ATTR_CTL = 0x122,
    TC_MODEL_ATTR_MAIN_LIGHTONOFF = 0x0534,
    TC_MODEL_ATTR_BACKGROUNDLIGHTONOFF = 0x0533,
    TC_MODEL_ATTR_MODEL_NUMBER = 0xf004,
    TC_MODEL_ATTR_ERRER_EVENT = 0xf009
};

typedef void(*user_custom_tc_server_cb)(access_pdu_tx_t *, uint8_t);

typedef struct
{
   // uint16_t attr_type;
    uint8_t value[200];
	  uint32_t length;
}custom_tc_msg_format_t;

typedef struct
{
    model_server_base_t model;
    custom_tc_msg_format_t msg_format[2];
    generic_valid_field_queue_t tid_queue;
    mesh_model_evt_cb_t cb;
}custom_tc_server_t;

#define DEF_TC_MODEL_SERVER_MODEL(name,app_key_max) \
      static custom_tc_server_t name;\
      static model_publish_state_t name##_publish_state;\
      static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_CUSTOM_TC_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,TC_MODEL_SERVER_MODEL_ID,false,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;


void set_tc_server(custom_tc_server_t *server);
void tc_msg_attr_indication_tx(void *msg, uint8_t len);
void show_buf(const char *name, uint8_t *buf, int len);

void tc_message_transparent_msg_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_attr_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_indication_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_confirmation_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
#endif /* TC_MODEL_SERVER_H_ */
