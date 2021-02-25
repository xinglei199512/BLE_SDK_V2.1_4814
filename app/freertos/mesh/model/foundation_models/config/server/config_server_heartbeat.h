#ifndef CONFIG_SERVER_HEARTBEAT_H_
#define CONFIG_SERVER_HEARTBEAT_H_
#include "access_rx_process.h"
#include "mesh_model.h"


void config_heartbeat_subscription_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_heartbeat_subscription_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_heartbeat_publication_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_heartbeat_publication_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);


#endif
