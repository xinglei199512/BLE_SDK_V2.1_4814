#ifndef CACHE_MANAGEMENT_H_
#define CACHE_MANAGEMENT_H_
#include <stdbool.h>
#include <stdint.h>
#include "co_list.h"
#include "static_buffer.h"

typedef struct
{
    static_buffer_t *cache_buf;
    int32_t (*compare)(void *,void *);
    struct co_list list;
}cache_management_t;

typedef struct co_list_hdr cache_head_t;

#define DEF_CACHE(cache_hdl,type,size) \
    typedef struct\
    {\
        cache_head_t hdr;\
        type data;\
    }_##cache_hdl##cache_t;\
    DEF_ARRAY_BUF(_##cache_hdl##_cache_buf,_##cache_hdl##cache_t,size);\
    cache_management_t cache_hdl = {\
        .cache_buf = &_##cache_hdl##_cache_buf,\
    }

void cache_init(cache_management_t *cache_hdl,int32_t (*compare)(void *,void *));

bool cache_full(cache_management_t *cache_hdl);

void *cache_add(cache_management_t *cache_hdl,void *element);

void *cache_del(cache_management_t *cache_hdl,void *element);

void *cache_remove_oldest(cache_management_t *cache_hdl);

void *cache_search(cache_management_t *cache_hdl,void *element);

void cache_clean_up(cache_management_t *cache_hdl,void (*free)(void *));

#endif
