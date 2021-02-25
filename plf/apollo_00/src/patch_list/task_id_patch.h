#ifndef TASK_ID_PATCH_H_
#define TASK_ID_PATCH_H_
#include "bx_config.h"
#include "rwip_config.h"
#if(LOCAL_NVDS==0)
enum extended_task_type
{
    #if(LOCAL_NVDS == 0)
    TASK_NVDS = (TASK_AHI - 1),
    #endif
};
#endif

#endif
