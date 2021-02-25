#ifndef _MESH_RESET_DATABASE_H_
#define _MESH_RESET_DATABASE_H_
#include <stdint.h>
#include "mesh_gatt.h"



bool get_is_node_can_rx_pkt(void);
void provision_set_database_reset(void);
void provision_delete_provision_database(void);
bool provision_is_database_pending(void);
void provision_reset_database_finish(void);
void provision_set_database_pending(void);
bool get_is_node_can_tx_pkt(void);
void ble_mesh_gatt_evt_update(mesh_gatt_evt_t evt,const uint8_t * p_param);

#endif

