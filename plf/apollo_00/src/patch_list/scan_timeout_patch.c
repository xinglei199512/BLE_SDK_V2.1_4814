#include "patch.h"
#include "bx_dbg.h"


void set_scan_timeout_patch()
{
    uint8_t patch_no;
    if(patch_alloc(&patch_no)==false)
    {
        BX_ASSERT(0);
    }
    patch_entrance_exit_addr(patch_no,0x7cac,0x02122204);
    PATCH_ENABLE(patch_no);
}  

