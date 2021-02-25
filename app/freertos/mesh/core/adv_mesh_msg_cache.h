#ifndef ADV_MESH_MSG_CACHE_H_
#define ADV_MESH_MSG_CACHE_H_
#include <stdint.h>
#include <stdbool.h>
#include "network_pdu.h"




void adv_mesh_msg_cache_init(void);

bool adv_mesh_msg_cache_full(void);

void adv_mesh_msg_cache_add(network_pdu_rx_t *element);

network_pdu_rx_t *adv_mesh_msg_cache_remove_oldest(void);

bool adv_mesh_msg_cache_search(network_pdu_rx_t *element);
void adv_mesh_msg_cache_clear(void);


#endif
