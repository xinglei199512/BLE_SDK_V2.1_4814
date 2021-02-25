#ifndef BEACON_MESH_MSG_CACHE_H_
#define BEACON_MESH_MSG_CACHE_H_
#include <stdint.h>
#include <stdbool.h>
#include "network_pdu.h"
#include "beacon.h"


void beacon_mesh_msg_cache_init(void);

bool beacon_mesh_msg_cache_full(void);

void beacon_mesh_msg_cache_add(mesh_beacon_t *element);

mesh_beacon_t *beacon_mesh_msg_cache_remove_oldest(void);

bool beacon_mesh_msg_cache_search(mesh_beacon_t *element);
void beacon_mesh_msg_cache_clear(void);


#endif
