#include <string.h>
#include "beacon_mesh_msg_cache.h"
#include "cache_management.h"
#include "stack_mem_cfg.h"


DEF_CACHE(beacon_mesh_msg_cache,mesh_beacon_t ,BEACON_MESH_MSG_CACHE_SIZE);

static int32_t beacon_mesh_msg_cache_cmp(void *a,void *b)
{
    mesh_beacon_t *x = (mesh_beacon_t *)a;
    mesh_beacon_t *y = (mesh_beacon_t *)b;
    return memcmp(x,y,sizeof(mesh_beacon_t));
}

void beacon_mesh_msg_cache_init()
{
    cache_init(&beacon_mesh_msg_cache,beacon_mesh_msg_cache_cmp);
}

bool beacon_mesh_msg_cache_full()
{
    return cache_full(&beacon_mesh_msg_cache);    
}

void beacon_mesh_msg_cache_add(mesh_beacon_t *element)
{
    cache_add(&beacon_mesh_msg_cache,element);
}

mesh_beacon_t *beacon_mesh_msg_cache_remove_oldest()
{
    mesh_beacon_t *ptr = (mesh_beacon_t *)cache_remove_oldest(&beacon_mesh_msg_cache);
    return ptr ? ptr:NULL;
}

bool beacon_mesh_msg_cache_search(mesh_beacon_t *element)
{
    return cache_search(&beacon_mesh_msg_cache,(void *)element) ? true : false;
}

void beacon_mesh_msg_cache_clear(void)
{
    cache_clean_up(&beacon_mesh_msg_cache,NULL);
}

