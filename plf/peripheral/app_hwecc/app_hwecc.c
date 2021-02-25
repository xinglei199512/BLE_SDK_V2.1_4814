#include <string.h>
#include "app_hwecc.h"
#include "hwecc_typedef.h"
#include "bx_ring_queue.h"
#include "log.h"
#include "ll.h"
#include "co_utils.h"
#include "field_manipulate.h"

#define ECC_RAM(reg) ((ecc_ram_t *)(reg)->ECC_RAM)

extern periph_universal_func_set_t hwecc_universal_func;

//Memory copy to register.(length unit is BYTE)
void mem_to_reg(void * dst_addr,const void * src_addr,uint32_t length)
{
    BX_ASSERT(length%sizeof(uint32_t)==0);
    uint32_t val;
    uint32_t i;
    uint32_t * dst = (uint32_t *)dst_addr;
    uint8_t *src = (uint8_t *)src_addr;
    for(i=0;i<length/sizeof(uint32_t);++i)
    {
        val = src[3]<<24|src[2]<<16|src[1]<<8|src[0];
        *dst ++ = val;
        src += sizeof(uint32_t);
    }
}

//Register set to value.(length unit is WORD)
void reg_set(void * src_addr,uint32_t val,uint32_t length)
{
    uint32_t i;
    uint32_t *src = (uint32_t *)src_addr;
    for(i=0;i<length;++i)
    {
        *src = val;
        src ++;
    }
}

static void hwecc_calculate_start(app_hwecc_inst_t *inst,ecc_in_t *in)
{
    hwecc_universal_func.sys_stat_func(inst,HWECC_START);
    hwecc_universal_func.clk_gate_func(inst,SET_CLK);
    mem_to_reg(&ECC_RAM(inst->reg)->block[15],in->secret_key,ECC_BLOCK_SIZE);
    mem_to_reg(&ECC_RAM(inst->reg)->block[5],in->public_key[0],ECC_BLOCK_SIZE);
    mem_to_reg(&ECC_RAM(inst->reg)->block[6],in->public_key[1],ECC_BLOCK_SIZE);
    reg_set   (&ECC_RAM(inst->reg)->block[2],0,ECC_BLOCK_SIZE/sizeof(uint32_t));
    reg_set   (&ECC_RAM(inst->reg)->block[3],0,ECC_BLOCK_SIZE/sizeof(uint32_t));
    reg_set   (&ECC_RAM(inst->reg)->block[4],0,ECC_BLOCK_SIZE/sizeof(uint32_t));
    reg_set   (&ECC_RAM(inst->reg)->block[7],0,ECC_BLOCK_SIZE/sizeof(uint32_t));
    reg_set   (&ECC_RAM(inst->reg)->block[3],1,1);
    reg_set   (&ECC_RAM(inst->reg)->block[7],1,1);
    inst->reg->START_BUSY = 1;
}

void app_hwecc_isr(app_hwecc_inst_t *inst)
{
    inst->reg->INTR_CLR = 1;
    ecc_rslt_t *ptr = &inst->rslt;
    void (*callback)(void *) = ptr->callback;
    void *cb_param = ptr->dummy;
    memcpy(ptr->out.key[0],&ECC_RAM(inst->reg)->block[12],ECC_BLOCK_SIZE);
    memcpy(ptr->out.key[1],&ECC_RAM(inst->reg)->block[13],ECC_BLOCK_SIZE);
    hwecc_universal_func.clk_gate_func(inst,CLR_CLK);
    hwecc_universal_func.sys_stat_func(inst,HWECC_DONE);
    periph_unlock(&inst->ecc_lock);
    callback(cb_param);
}

periph_err_t app_hwecc_init(periph_inst_handle_t hdl)
{
    app_hwecc_inst_t *inst = CONTAINER_OF(hdl, app_hwecc_inst_t, inst);
    hwecc_universal_func.sw_rst_func(inst);
    hwecc_universal_func.clk_gate_func(inst,SET_CLK);
    hwecc_universal_func.intr_op_func(inst,INTR_CLR);
    hwecc_universal_func.intr_op_func(inst,INTR_ENABLE);
    reg_ecc_t *reg = inst->reg;
    reg->START_SEL = FIELD_BUILD(ECC_ECC_WO_INV,With_Inversion) | FIELD_BUILD(ECC_START_SEL_R, Full_ECC_Calc);
    reg->INTR_MASK = 1;
    hwecc_universal_func.clk_gate_func(inst,CLR_CLK);
    hwecc_universal_func.sys_stat_func(inst,HWECC_INIT);
    return PERIPH_NO_ERROR;
}

periph_err_t app_hwecc_uninit(periph_inst_handle_t hdl)
{
    app_hwecc_inst_t *inst = CONTAINER_OF(hdl, app_hwecc_inst_t, inst);
    hwecc_universal_func.clk_gate_func(inst,CLR_CLK);
    hwecc_universal_func.intr_op_func(inst,INTR_DISABLE);
    hwecc_universal_func.sys_stat_func(inst,HWECC_UNINIT);
    return PERIPH_NO_ERROR;
}

periph_err_t app_hwecc_calculate(periph_inst_handle_t hdl,ecc_in_t *in,ecc_rslt_t *rslt)
{
    app_hwecc_inst_t *inst = CONTAINER_OF(hdl, app_hwecc_inst_t, inst);
    if(periph_lock(&inst->ecc_lock)==false)
    {
        return PERIPH_BUSY;
    }
    inst->rslt = *rslt;
    hwecc_calculate_start(inst,in);
    return PERIPH_NO_ERROR;
}

