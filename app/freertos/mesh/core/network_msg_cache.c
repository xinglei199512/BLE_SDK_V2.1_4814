#include <string.h>
#include "network_msg_cache.h"
#include "cache_management.h"
#include "stack_mem_cfg.h"
#include "network_pdu_decrypt.h"

DEF_CACHE(network_msg_cache,network_msg_cache_data_t,NETWORK_MSG_CACHE_SIZE);

void network_msg_cache_init()
{
    cache_init(&network_msg_cache,NULL);
}

bool network_msg_cache_full()
{
    return cache_full(&network_msg_cache);
}

void network_msg_cache_add(network_pdu_packet_u *pkt_u)
{
    network_msg_cache_data_t data;
    uint8_t mic_length = network_pdu_mic_length(pkt_u->pkt.ctl);
    memcpy(data.pdu_2_to_8,&pkt_u->buf[2],NETWORK_MSG_CACHE_DATA_LENGTH);
    cache_add(&network_msg_cache,&data);
}

void network_msg_cache_remove_oldest()
{
    cache_remove_oldest(&network_msg_cache);
}

bool network_msg_cache_search(network_pdu_packet_u *pkt_u)
{
    network_msg_cache_data_t data;
    uint8_t mic_length = network_pdu_mic_length(pkt_u->pkt.ctl);
    memcpy(data.pdu_2_to_8,&pkt_u->buf[2],NETWORK_MSG_CACHE_DATA_LENGTH);
    return cache_search(&network_msg_cache,&data)?true:false;
}
