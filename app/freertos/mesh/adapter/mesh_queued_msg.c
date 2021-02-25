#include "mesh_queued_msg.h"
#include "log.h"

void mesh_queued_msg_send(void (*func)(void *),void *param)
{
    osapp_async_call_wrapper(func,param);
}

void mesh_run(void (*cb)(),uint32_t xTicksToWait,bool inISR)
{
    osapp_async_call_wrapper(cb,NULL);
}

