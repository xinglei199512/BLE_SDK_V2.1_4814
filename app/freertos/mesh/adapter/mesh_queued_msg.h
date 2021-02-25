#ifndef MESH_QUEUED_MSG_H_
#define MESH_QUEUED_MSG_H_
#include "osapp_task.h"

void mesh_queued_msg_send(void (*func)(void *),void *param);


void mesh_run(void (*cb)(),uint32_t xTicksToWait,bool inISR);//deprecated

#endif
