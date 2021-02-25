#include "app_dmac.h"
#include "compiler_flag.h"
app_dmac_inst_t dmac_inst = DMAC_INSTANCE();

void app_dmac_init_wrapper(void)
{
    app_dmac_init(&dmac_inst.inst);
}

void app_dmac_uninit_wrapper(void)
{
    app_dmac_uninit(&dmac_inst.inst);
}

N_XIP_SECTION periph_err_t app_dmac_start_wrapper(app_dmac_transfer_param_t *param,uint8_t *ch_idx)
{
    return app_dmac_start(&dmac_inst.inst,param,ch_idx);
}

periph_err_t app_dmac_transfer_cancel_wrapper(uint8_t ch_idx,uint32_t *remaining_size)
{
    return app_dmac_transfer_cancel(&dmac_inst.inst,ch_idx,remaining_size);
}

N_XIP_SECTION periph_err_t app_dmac_transfer_wait_wrapper(uint8_t ch_idx)
{
    return app_dmac_transfer_wait(&dmac_inst.inst,ch_idx);
}


