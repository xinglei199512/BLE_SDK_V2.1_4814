#include <stdint.h>
#include "patch.h"
#include "log.h"
#include "lld_evt.h"
#define LLD_EVT_DRIFT_COMPUTE_ENTRY_PATCH_ADDR 0x16180
#define LLD_EVT_DRIFT_COMPUTE_ENTRY_PATCH_CODE 0xb510
#define LLD_EVT_DRIFT_COMPUTE_PATCH_ADDR 0x16184
#define RC32K_DRIFT_PPM 20000
#define co_sca2ppm ((uint16_t *)0x1e77c)
#define DRIFT_ACCURRACY_REDUCE_FACTOR 2
uint16_t LLD_EVT_DRIFT_COMPUTE_PATCH(uint16_t delay, uint8_t master_sca);

uint16_t lld_evt_drift_compute_patch(uint16_t delay, uint8_t master_sca)
{
    // Compute the total accuracy in ppm
    uint32_t accuracy = RC32K_DRIFT_PPM + ((uint32_t) co_sca2ppm[master_sca]);

    // Compute the drift from the interval and the accuracy and add max max expected drift
    // (41 / 2^16) = 0.0006256103515625 means approximate slot duration in seconds
    // delay is in slot
    // accuracy is in ppm
    // so result of this function is in us
    uint32_t drift = (delay * 41) * (accuracy>> DRIFT_ACCURRACY_REDUCE_FACTOR) >> (16 - DRIFT_ACCURRACY_REDUCE_FACTOR) ;
    uint32_t retval = (drift + 1) + LLD_EVT_MAX_JITTER;
    return retval >= 0x10000 ? 0xffff : retval;
}

void set_rc32k_patch()
{
    uint8_t patch_no[2];
    if(patch_alloc(&patch_no[0])==false)
    {
        BX_ASSERT(0);
    }    
    uint32_t code = cal_patch_bl(LLD_EVT_DRIFT_COMPUTE_PATCH_ADDR,(uint32_t)LLD_EVT_DRIFT_COMPUTE_PATCH - 1);
    patch_entrance_exit_addr(patch_no[0],LLD_EVT_DRIFT_COMPUTE_PATCH_ADDR,code);
    PATCH_ENABLE(patch_no[0]);
    if(patch_alloc(&patch_no[1])==false)
    {
        BX_ASSERT(0);
    }    
    patch_entrance_exit_addr(patch_no[1],LLD_EVT_DRIFT_COMPUTE_ENTRY_PATCH_ADDR,LLD_EVT_DRIFT_COMPUTE_ENTRY_PATCH_CODE);
    PATCH_ENABLE(patch_no[1]);
}
