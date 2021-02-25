#include "svcall.h"
#include "flash_integration.h"
#include "compiler_flag.h"
typedef uint32_t (*svcall_fn_t)(uint32_t,uint32_t,uint32_t,uint32_t);

const svcall_fn_t svcall_table[SVCALL_NUM_MAX] = {
    [SVCALL_FLASH_PROGRAM] = (svcall_fn_t)flash_program_execution,
    [SVCALL_FLASH_ERASE] = (svcall_fn_t)flash_erase_execution,
    [SVCALL_FLASH_READ] = (svcall_fn_t)flash_read_execution,
    [SVCALL_FLASH_MULTI_READ_32BITS] = (svcall_fn_t)flash_multi_read_32bits_execution,
    [SVCALL_FLASH_WRITE_STATUS_REG] = (svcall_fn_t)flash_write_status_reg_execution,
};

void SVC_Handler_C(uint32_t *svc_args)
{
    uint8_t svc_num = ((uint8_t *)svc_args[6])[-2];
    if(svc_num<SVCALL_NUM_MAX)
    {
        svc_args[0] = svcall_table[svc_num](svc_args[0],svc_args[1],svc_args[2],svc_args[3]);
    }
}

