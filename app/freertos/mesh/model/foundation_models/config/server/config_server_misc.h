#ifndef CONFIG_SERVER_MISC_H_
#define CONFIG_SERVER_MISC_H_
#include "access_rx_process.h"
#include "mesh_model.h"

void config_composition_data_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_beacon_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_beacon_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_default_ttl_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_default_ttl_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void handle_config_key_refresh_phase_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void handle_config_key_refresh_phase_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_network_transmit_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_network_transmit_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_server_node_identity_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_server_node_identity_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_server_node_reset_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

#endif
