#ifndef CONFIG_SERVER_KEY_H_
#define CONFIG_SERVER_KEY_H_
#include "access_rx_process.h"
#include "mesh_model.h"

#define CONFIG_APPKEY_STATUS_PARAM_LENGTH 4

void config_appkey_add_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_appkey_update_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_appkey_delete_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_appkey_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_model_app_bind_rx(mesh_elmt_t *,model_base_t *,access_pdu_rx_t *);

void config_model_app_unbind_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_netkey_add_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_netkey_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_netkey_delete_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_server_update_netkey_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_SIG_model_app_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_vendor_model_app_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);



#endif
