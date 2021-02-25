#include <stddef.h>
#include "periph_recovery.h"
#include "bx_dbg.h"

void recovery_list_add(periph_inst_handle_t *recovery_buf,uint8_t idx,periph_inst_handle_t hdl)
{
    recovery_buf[idx] = hdl;
}

void recovery_list_remove(periph_inst_handle_t *recovery_buf,uint8_t idx)
{
    recovery_buf[idx] = NULL;
}

void periph_recovery(periph_inst_handle_t *recovery_buf,uint8_t size)
{
    uint8_t i;
    for(i=0;i<size;++i)
    {
        if(recovery_buf[i])
        {
            recovery_buf[i]->init_func(recovery_buf[i]);
        }
    }
}
