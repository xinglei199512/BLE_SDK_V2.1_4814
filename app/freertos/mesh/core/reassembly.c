#include "reassembly.h"
#include "static_buffer.h"
#include "network_pdu_decrypt.h"
#include "lower_rx_process.h"
#include "stack_mem_cfg.h"

#define IMCOMPLETE_TIMER_MIN 10000

#define LOCAL_REASSEMBLY_BUF_SIZE REASSEMBLY_ENV_BUF_SIZE
DEF_ARRAY_BUF(local_reassembly,local_reassembly_t, LOCAL_REASSEMBLY_BUF_SIZE);
DEF_ARRAY_BUF(reassembly_buf,reassembly_env_t,REASSEMBLY_ENV_BUF_SIZE);

reassembly_env_t *reassembly_env_alloc(bool local,lower_rx_env_t *env)
{
    reassembly_env_t *ptr = array_buf_alloc(&reassembly_buf);
    if(NULL == ptr)
    {
        return NULL;
    }
    if(local)
    {
        ptr->local = array_buf_alloc(&local_reassembly);
        BX_ASSERT(ptr->local);
    }else
    {
        // Do note: 1:  we use "->local" as condition in function: lower_transport_rx; 
        //          2:  we do array_buf_release in reassembly_env_releae 
        // in case: local != NULL;
        ptr->local = NULL;
    }
    ptr->timer.incomplete = mesh_timer_create("incomplete",pdMS_TO_TICKS(IMCOMPLETE_TIMER_MIN),false,
        env,incomplete_timer_callback);
    ptr->timer.acknowledgment = mesh_timer_create("ack",1,false,
        env,ack_timer_callback);
    return ptr;
    
}

bool reassembly_env_release(reassembly_env_t *ptr)
{
    reassembly_timer_t timer = ptr->timer;
    local_reassembly_t * local = ptr->local;
    if(array_buf_release(&reassembly_buf,ptr))
    {
        bool released;
        // Both friend and local will reach here when do release.    
        if(local != NULL)
        {
            released = array_buf_release(&local_reassembly, local);
            BX_ASSERT(released);
        }
        mesh_timer_delete(timer.incomplete);
        mesh_timer_delete(timer.acknowledgment);
        return true;
    }else
    {
        return false;
    }

}
