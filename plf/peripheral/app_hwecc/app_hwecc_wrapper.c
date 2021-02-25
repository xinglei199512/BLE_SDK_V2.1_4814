#include "app_hwecc_wrapper.h"

app_hwecc_inst_t hwecc_inst = HWECC_INSTANCE();

void app_hwecc_init_wrapper()
{
    app_hwecc_init(&hwecc_inst.inst);
}

void app_hwecc_uninit_wrapper()
{
    app_hwecc_uninit(&hwecc_inst.inst);   
}

periph_err_t app_hwecc_calculate_wrapper(ecc_queue_t *param)
{
    ecc_rslt_t rslt = {
        .out = param->out,
        .callback = param->cb,
        .dummy = param->dummy,
    };
    return app_hwecc_calculate(&hwecc_inst.inst,&param->in,&rslt);
}
