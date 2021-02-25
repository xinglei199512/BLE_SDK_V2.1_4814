#include "rwip_config.h"

#include "co_utils.h"
#include "co_endian.h"
#include "gapm_task.h"
#include "gapm.h"
#include "l2cc_pdu.h"


#include "patch.h"

#define SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH_ADDR 0x0001bfc8
uint8_t smpc_construct_id_addr_type_patch_c(struct l2cc_pdu *pdu)
{
    return gapm_get_address_type() & GAPM_CFG_ADDR_PRIVATE ? ADDR_RAND : ADDR_PUBLIC;
}

void SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH(void);

void set_smpc_construct_id_addr_type_patch()
{
    uint32_t code = cal_patch_bl(SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH_ADDR,(uint32_t)SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH - 1);
    uint8_t patch_no[1];
    if(patch_alloc(&patch_no[0])==false)
    {
        BX_ASSERT(0);
    }
    patch_entrance_exit_addr(patch_no[0],SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH_ADDR,code);
    PATCH_ENABLE(patch_no[0]);
}
