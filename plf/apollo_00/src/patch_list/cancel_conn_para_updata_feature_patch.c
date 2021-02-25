#include "patch.h"
#include "bx_dbg.h"

//cancel BLE4.2 BLE_CON_PARAM_REQ_PROC_FEATURE,use L2CAP conn para updata
void cancel_conn_para_updata_feature_patch()
{
    uint8_t patch_no,patch_no2;
    if(patch_alloc(&patch_no)==false)
    {
        BX_ASSERT(0);
    }
    patch_entrance_exit_addr(patch_no,0x1e9e4,0x3fd);
    PATCH_ENABLE(patch_no);
    //USE L2CAP param update message
    if(patch_alloc(&patch_no2)==false)
    {
        BX_ASSERT(0);
    }
    patch_entrance_exit_addr(patch_no2,0x0000632e,0x201D5979);
    PATCH_ENABLE(patch_no2);
}  

