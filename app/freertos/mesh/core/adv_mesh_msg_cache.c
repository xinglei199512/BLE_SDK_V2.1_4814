#include <string.h>
#include "adv_mesh_msg_cache.h"
#include "cache_management.h"
#include "stack_mem_cfg.h"
#include "adv_bearer_rx.h"

DEF_CACHE(adv_mesh_msg_cache,network_pdu_rx_t *,ADV_MESH_MSG_CACHE_SIZE);

static int32_t adv_mesh_msg_cache_cmp(void *a,void *b)
{
    network_pdu_rx_t *x = *(network_pdu_rx_t **)a;
    network_pdu_rx_t *y = *(network_pdu_rx_t **)b;
    if(x->src.length < y->src.length)
    {
        return -1;
    }else if(x->src.length > y->src.length)
    {
        return 1;
    }else
    {
        return memcmp(x->src.data,y->src.data,x->src.length);
    }
}

void adv_mesh_msg_cache_init()
{
    cache_init(&adv_mesh_msg_cache,adv_mesh_msg_cache_cmp);
}

bool adv_mesh_msg_cache_full()
{
    return cache_full(&adv_mesh_msg_cache);    
}

void adv_mesh_msg_cache_add(network_pdu_rx_t *element)
{
    cache_add(&adv_mesh_msg_cache,&element);
}

network_pdu_rx_t *adv_mesh_msg_cache_remove_oldest()
{
    network_pdu_rx_t **ptr = (network_pdu_rx_t **)cache_remove_oldest(&adv_mesh_msg_cache);
    return ptr ? *ptr:NULL;
}

bool adv_mesh_msg_cache_search(network_pdu_rx_t *element)
{
    return cache_search(&adv_mesh_msg_cache,(void *)&element) ? true : false;
}

void adv_mesh_msg_cache_del(void * pptr)
{
    network_pdu_rx_t * ptr = *(network_pdu_rx_t**)pptr;
    rx_pdu_buf_release(ptr);
}
void adv_mesh_msg_cache_clear(void)
{
    cache_clean_up(&adv_mesh_msg_cache,adv_mesh_msg_cache_del);
}

