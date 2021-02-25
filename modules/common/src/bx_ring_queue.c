#include "bx_ring_queue.h"
#include <string.h>
#include "bx_dbg.h"

void bx_enqueue_specific(ring_queue_t *rq,void (*copy)(uint8_t *,uint16_t,va_list *),...)
{
    uint8_t *ptr = rq->buf;
    ptr +=  rq->wr_idx*rq->element_size;
    va_list param_list;
    va_start(param_list,copy);
    copy(ptr,rq->element_size,&param_list);
    va_end(param_list);
    ++rq->count;
    ++rq->wr_idx;
    if(rq->wr_idx==rq->size)
    {
        rq->wr_idx = 0;
    }
}

static void normal_copy(uint8_t *dst,uint16_t size,va_list *param_list)
{
    void *ptr = va_arg(*param_list,void *);
    memcpy(dst,ptr,size);
}

void bx_enqueue(ring_queue_t *rq,void *element)
{
    bx_enqueue_specific(rq,normal_copy,element);
}

void *bx_dequeue(ring_queue_t *rq)
{
    uint8_t *ptr = rq->buf;
    ptr += rq->rd_idx*rq->element_size;
    BX_ASSERT(rq->count);
    --rq->count;
    ++rq->rd_idx;
    if(rq->rd_idx == rq->size)
    {
        rq->rd_idx = 0;
    }
    return ptr;
}

/*
uint16_t bx_ring_queue_array_offset(ring_queue_t *rq,void *element)
{
    uint8_t *ptr =rq->buf;
    ptrdiff_t diff = (uint8_t *)element - ptr;
    return diff/rq->element_size;
}
*/

uint16_t bx_ring_queue_amount(ring_queue_t *rq)
{
        return rq->count;
}

void *bx_ring_queue_glance(ring_queue_t *rq,uint16_t idx)
{
    if(idx>=rq->count)
    {
        return NULL;
    }
    uint8_t *ptr = rq->buf;
    if(idx+rq->rd_idx < rq->size)
    {
        ptr += (idx + rq->rd_idx)*rq->element_size;
    }else
    {
        ptr += (idx + rq->rd_idx - rq->size)*rq->element_size;
    }
    return ptr;
}

bool bx_ring_queue_full(ring_queue_t *rq)
{
    return rq->count == rq->size;
}

bool bx_ring_queue_empty(ring_queue_t *rq)
{
    return rq->count == 0;
}

void bx_ring_queue_flush_all(ring_queue_t *rq)
{
    rq->rd_idx = rq->wr_idx;
    rq->count = 0;
}

uint16_t bx_ring_queue_element_size(ring_queue_t *rq)
{
    return rq->element_size;
}


