#include "static_buffer.h"
#include "log.h"

static bool array_buf_hdl_sanity_check(static_buffer_t *buf_hdl,uint8_t *ptr);

void *array_buf_element_get(static_buffer_t *buf_hdl,uint16_t idx)
{
    BX_ASSERT(buf_hdl->buf_length>idx);
    if(buf_hdl->ref_cnt[idx])
    {
        return &buf_hdl->buf[buf_hdl->element_size*idx];
    }else
    {
        return NULL;
    }
}

uint16_t array_buf_idx_get(static_buffer_t *buf_hdl,void *ptr)
{
    BX_ASSERT(array_buf_hdl_sanity_check(buf_hdl,ptr));
    return ((uint8_t *)ptr-buf_hdl->buf)/buf_hdl->element_size;
}

void *array_buf_alloc_idx(static_buffer_t *buf_hdl,uint16_t idx)
{
    BX_ASSERT(buf_hdl->ref_cnt[idx]==0);
    ++buf_hdl->ref_cnt[idx];
    --buf_hdl->remaining_size;
    return &buf_hdl->buf[idx*buf_hdl->element_size];
}

void *array_buf_calloc_idx(static_buffer_t *buf_hdl,uint16_t idx)
{
    void *ptr = array_buf_alloc_idx(buf_hdl,idx);
    if(ptr)
    {
        memset(ptr,0,buf_hdl->element_size);
    }
    return ptr;
}

void *array_buf_alloc(static_buffer_t *buf_hdl)
{
    uint16_t i;
    for(i=0;i<buf_hdl->buf_length;++i)
    {
        if(buf_hdl->ref_cnt[i]==0)
        {
            return array_buf_alloc_idx(buf_hdl,i);
        }
    }
    return NULL;
}

void *array_buf_calloc(static_buffer_t *buf_hdl)
{
    void *ptr = array_buf_alloc(buf_hdl);
    if(ptr)
    {
        memset(ptr,0,buf_hdl->element_size);
    }
    return ptr;
}

uint8_t array_buf_retain(static_buffer_t *buf_hdl,void *ptr)
{
    BX_ASSERT(array_buf_hdl_sanity_check(buf_hdl,(uint8_t *)ptr));
    uint16_t i = ((uint8_t *)ptr-buf_hdl->buf)/buf_hdl->element_size;
    BX_ASSERT(buf_hdl->ref_cnt[i]);
    return ++buf_hdl->ref_cnt[i];
}

uint8_t array_buf_ref_cnt(static_buffer_t *buf_hdl,void *ptr)
{
    BX_ASSERT(array_buf_hdl_sanity_check(buf_hdl,(uint8_t *)ptr));
    uint16_t i = ((uint8_t *)ptr-buf_hdl->buf)/buf_hdl->element_size;
    return buf_hdl->ref_cnt[i];
}

static bool array_buf_hdl_sanity_check(static_buffer_t *buf_hdl,uint8_t *ptr)
{
    if((ptr-buf_hdl->buf)%buf_hdl->element_size)
    {
        return false;
    }
    if(ptr<buf_hdl->buf || ptr>= &buf_hdl->buf[buf_hdl->buf_length*buf_hdl->element_size])
    {
        return false;
    }
    return true;
}

uint16_t array_buf_available_size(static_buffer_t *buf_hdl)
{
    return buf_hdl->remaining_size;
}

uint16_t array_buf_element_size(static_buffer_t *buf_hdl)
{
    return buf_hdl->element_size;
}

bool array_buf_has_element(static_buffer_t *buf_hdl,void *ptr)
{
    bool state = false;
    if(array_buf_hdl_sanity_check(buf_hdl,ptr))// addr in buf range
    {
        uint16_t i = ((uint8_t *)ptr-buf_hdl->buf)/buf_hdl->element_size;
        if( buf_hdl->ref_cnt[i] ) state = true;
    }
    return state;    
}

bool array_buf_release(static_buffer_t *buf_hdl,void *ptr)
{
    BX_ASSERT(array_buf_hdl_sanity_check(buf_hdl,ptr));
    uint16_t i = ((uint8_t *)ptr-buf_hdl->buf)/buf_hdl->element_size;
    BX_ASSERT(buf_hdl->ref_cnt[i]);
    if(--buf_hdl->ref_cnt[i]==0)
    {
        ++buf_hdl->remaining_size;
        return true;
    }else
    {
        return false;
    }
}
