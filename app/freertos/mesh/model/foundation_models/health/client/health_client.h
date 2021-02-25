#ifndef HERLTH_CLIENT__H
#define HERLTH_CLIENT__H
#include "mesh_model.h"
#include "access_rx_process.h"

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
}health_client_t;

void health_fault_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void health_current_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void health_period_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void health_attention_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
#endif


