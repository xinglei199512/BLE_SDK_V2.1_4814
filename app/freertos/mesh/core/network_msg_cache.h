#ifndef NETWORK_MSG_CACHE_H_
#define NETWORK_MSG_CACHE_H_


#include <stdint.h>
#include <stdbool.h>
#include "network_pdu.h"

#define NETWORK_MSG_CACHE_DATA_LENGTH 7

typedef struct
{
    uint8_t pdu_2_to_8[NETWORK_MSG_CACHE_DATA_LENGTH];
}network_msg_cache_data_t;


void network_msg_cache_init(void);

bool network_msg_cache_full(void);

void network_msg_cache_add(network_pdu_packet_u *pkt_u);

void network_msg_cache_remove_oldest(void);

bool network_msg_cache_search(network_pdu_packet_u *pkt_u);





#endif
