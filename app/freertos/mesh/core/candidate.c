#include <stddef.h>
#include "candidate.h"
#include "log.h"
#include "bx_ring_queue.h"

#define NETKEY_CANDIDATE_QUEUE_LENGTH 2
#define APPKEY_CANDIDATE_QUEUE_LENGTH 3
#define VIRTUAL_ADDRESS_CANDIDATE_QUEUE_LENGTH 3
typedef struct
{
    union{
        net_key_t *net;
        app_key_t *app;
    }ptr;
    union{
        security_credentials_t *net;
        app_key_box_t *app;
    }keybox;
}key_candidate_t;
DEF_RING_QUEUE(appkey_candidate_q,APPKEY_CANDIDATE_QUEUE_LENGTH,key_candidate_t);
DEF_RING_QUEUE(netkey_candidate_q,NETKEY_CANDIDATE_QUEUE_LENGTH,key_candidate_t);
DEF_RING_QUEUE(virt_addr_candidate_q,VIRTUAL_ADDRESS_CANDIDATE_QUEUE_LENGTH,virt_addr_mngt_t *);
void candidate_add(ring_queue_t *rq,void *ptr)
{
    BX_ASSERT(bx_ring_queue_full(rq)==false);
    bx_enqueue(rq,ptr);
}

void *candidate_current_pick(ring_queue_t *rq)
{
    if(bx_ring_queue_empty(rq))
    {
        return NULL;
    }else
    {
        return bx_ring_queue_glance(rq,0);
    }
}

void candidate_current_remove(ring_queue_t *rq)
{
    bx_dequeue(rq);
}

void candidate_remove_all(ring_queue_t *rq)
{
    bx_ring_queue_flush_all(rq);
}

void netkey_candidate_add(net_key_t *netkey,security_credentials_t* keybox)
{
    key_candidate_t candidate = 
    {
        .ptr.net = netkey,
        .keybox.net = keybox,
    };
    candidate_add(&netkey_candidate_q,&candidate);
}

void appkey_candidate_add(app_key_t *appkey,app_key_box_t *keybox)
{
    key_candidate_t candidate = 
    {
        .ptr.app = appkey,
        .keybox.app = keybox,
    };
    candidate_add(&appkey_candidate_q,&candidate);
}

void virt_addr_candidate_add(virt_addr_mngt_t *virt_addr)
{
    candidate_add(&virt_addr_candidate_q,&virt_addr);
}

virt_addr_mngt_t *virt_addr_candidate_current_pick()
{
    return candidate_current_pick(&virt_addr_candidate_q);
}

security_credentials_t *netkey_candidate_current_key_box_pick()
{
    key_candidate_t *candidate = candidate_current_pick(&netkey_candidate_q);
    if(candidate)
    {
        return candidate->keybox.net;
    }else
    {
        return NULL;
    }
}

app_key_box_t *appkey_candidate_current_key_box_pick()
{
    key_candidate_t *candidate = candidate_current_pick(&appkey_candidate_q);
    if(candidate)
    {
        return candidate->keybox.app;
    }else
    {
        return NULL;
    }
}

net_key_t *netkey_candidate_current_key_env_pick()
{
    key_candidate_t *candidate = candidate_current_pick(&netkey_candidate_q);
    if(candidate)
    {
        return candidate->ptr.net;
    }else
    {
        return NULL;
    }
}

app_key_t *appkey_candidate_current_key_env_pick()
{
    key_candidate_t *candidate = candidate_current_pick(&appkey_candidate_q);
    if(candidate)
    {
        return candidate->ptr.app;
    }else
    {
        return NULL;
    }    
}

void virt_addr_candidate_remove()
{
    candidate_current_remove(&virt_addr_candidate_q);
}

void netkey_candidate_remove()
{
    candidate_current_remove(&netkey_candidate_q);
}

void appkey_candidate_remove()
{
    candidate_current_remove(&appkey_candidate_q);
}

void virt_addr_candidate_remove_all()
{
    candidate_remove_all(&virt_addr_candidate_q);
}

void netkey_candidate_remove_all()
{
    candidate_remove_all(&netkey_candidate_q);    
}

void appkey_candidate_remove_all()
{
    candidate_remove_all(&appkey_candidate_q);    
}
