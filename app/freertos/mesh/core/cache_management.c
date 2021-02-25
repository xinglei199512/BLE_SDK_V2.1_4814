#include <string.h>
#include "cache_management.h"
#include "log.h"
typedef struct
{
    cache_head_t hdr;
    uint8_t data[];
}cache_buf_base_t;

void cache_init(cache_management_t *cache_hdl,int32_t (*compare)(void *,void *))
{
    co_list_init(&cache_hdl->list);
    cache_hdl->compare = compare;
}

bool cache_full(cache_management_t *cache_hdl)
{
    if(array_buf_available_size(cache_hdl->cache_buf))
    {
        return false;
    }else
    {
        return true;
    }
}

void *cache_add(cache_management_t *cache_hdl,void *element)
{
    cache_buf_base_t *ptr = array_buf_alloc(cache_hdl->cache_buf);
    BX_ASSERT(ptr);
    memcpy(ptr->data,element,array_buf_element_size(cache_hdl->cache_buf)-sizeof(cache_head_t));
    co_list_push_back(&cache_hdl->list,&ptr->hdr);
    return ptr->data;
}


void *cache_remove_oldest(cache_management_t *cache_hdl)
{
    cache_buf_base_t *ptr = (cache_buf_base_t *)co_list_pop_front(&cache_hdl->list);
    if(ptr)
    {
        array_buf_release(cache_hdl->cache_buf,ptr);
        return ptr->data;
    }else
    {
        return NULL;
    }
}

static cache_buf_base_t *cache_search_base(cache_management_t *cache_hdl,void *element)
{
    cache_buf_base_t *ptr =(cache_buf_base_t *) co_list_pick(&cache_hdl->list);
    while(ptr)
    {
        int32_t compare_rslt;
        if(cache_hdl->compare)
        {
            compare_rslt = cache_hdl->compare(ptr->data,element);
        }else
        {
            compare_rslt = memcmp(ptr->data,element,array_buf_element_size(cache_hdl->cache_buf)-sizeof(cache_head_t));
        }
        if(compare_rslt == 0)
        {
            return ptr;
        }
        ptr = (cache_buf_base_t *)co_list_next(( const struct co_list_hdr *)ptr);
    };
    return NULL;
}


void *cache_search(cache_management_t *cache_hdl,void *element)
{
    cache_buf_base_t *ptr = cache_search_base(cache_hdl, element);
    return ptr? ptr->data : NULL;
}

void *cache_del(cache_management_t *cache_hdl,void *element)
{
    cache_buf_base_t *ptr = cache_search_base(cache_hdl,element);
    if(ptr)
    {
        bool extracted = co_list_extract(&cache_hdl->list,&ptr->hdr,0);
        BX_ASSERT(extracted);
        array_buf_release(cache_hdl->cache_buf,ptr);
    }
    return ptr ? ptr->data : NULL;
}

void cache_clean_up(cache_management_t *cache_hdl,void (*free)(void *))
{
    void *data;
    do{
        data = cache_remove_oldest(cache_hdl);
        if(data && free)
        {
            free(data);
        }
    }while(data); 
}


