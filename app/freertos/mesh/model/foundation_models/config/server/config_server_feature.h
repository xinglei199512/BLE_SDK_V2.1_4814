#ifndef CONFIG_SERVER_FEATURE_H_
#define CONFIG_SERVER_FEATURE_H_
#include "access_rx_process.h"
#include "mesh_model.h"

typedef struct
{
    /// the retransmit count of the message.
    uint8_t count:3;            //LSB
    /// the interval between two messages.(unit is 10ms)
    uint8_t interval_steps:5;   //MSB
}network_transmit_state_t;

typedef struct
{
    mesh_feature_stat_t relay;
    network_transmit_state_t retransmit;
}config_relay_param_t;

typedef struct
{
    uint8_t proxy_state;
}proxy_state_t;

typedef struct
{
    proxy_state_t proxy;
}config_proxy_param_t;

void config_friend_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_friend_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void config_proxy_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_proxy_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_relay_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_relay_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void config_low_power_node_polltimeout_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);


#endif
