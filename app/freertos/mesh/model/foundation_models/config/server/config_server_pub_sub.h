#ifndef CONFIG_SERVER_PUB_SUB_H_
#define CONFIG_SERVER_PUB_SUB_H_
#include "access_rx_process.h"
#include "mesh_model.h"
typedef struct
{
    uint16_t appkey_index : 12;
    uint16_t credential_flag : 1;
    uint16_t rfu : 3;
    uint8_t  publish_ttl;
    uint8_t num_steps: 6;
    uint8_t step_resolution: 2;
    uint8_t  retransmit_count : 3;
    uint8_t  retransmit_interval : 5;
    uint32_t  model_id;
} __attribute((packed)) config_publication_params_t;


typedef struct
{
    uint8_t status;
    uint16_t element_address;
    uint16_t publish_address;
    config_publication_params_t state;
} __attribute((packed))config_model_publication_status_t;

void config_model_publication_set_rx(mesh_elmt_t *,model_base_t *,access_pdu_rx_t *);

void config_model_publication_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_model_publication_vir_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_model_subscription_update_rx(mesh_elmt_t *,model_base_t *,access_pdu_rx_t *);

void config_model_subscription_delete_all_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_model_subscription_virt_update_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_SIG_model_subscription_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_vendor_model_subscription_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

uint8_t config_model_subscription_add(mesh_elmt_t *elmt,model_base_t *model,uint16_t addr);
#endif
