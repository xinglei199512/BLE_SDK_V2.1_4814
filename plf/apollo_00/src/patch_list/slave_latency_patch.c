#include <string.h>
#include "arch.h"
#include "co_bt.h"
#include "co_endian.h"
#include "co_list.h"
#include "patch.h"

#define LATENCY_PATCH_ADDR  0x00017970


void set_slave_latency_patch(void)
{
    uint32_t code = 0x43700000;
    uint8_t patch_no;
    if(patch_alloc(&patch_no)==false)
    {
        BX_ASSERT(0);
    }    
    patch_entrance_exit_addr(patch_no,LATENCY_PATCH_ADDR,code);
    PATCH_ENABLE(patch_no);
}
