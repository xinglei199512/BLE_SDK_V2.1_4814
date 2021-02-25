#include "periph_common.h"
#include "periph_mngt.h"
#include "app_qspi.h"
#include "clk_gate.h"
#include "sysctrl.h"
#include "bx_config.h"
#include "rst_gen.h"
#include "log.h"
#include "compiler_flag.h"
#define QSPI_FIFO_DEPTH 64

N_XIP_SECTION static void qspi_sw_rst(app_qspi_inst_t *inst)
{
    srst_cpu(QSPI_SRST_CPU);
}

static N_XIP_SECTION void qspi_clk_gate(app_qspi_inst_t *inst,clk_gate_operation_t op)
{
    if(op == SET_CLK)
    {
        clk_gate_cpu_g1(CPU_CLKG_SET_QSPI);
    }else
    {
        clk_gate_cpu_g1(CPU_CLKG_CLR_QSPI);
    }
}

N_XIP_SECTION static void qspi_pin_cfg(app_qspi_inst_t * inst,uint8_t pin_num,uint32_t pin_role,bool enable)
{
    void sysc_cpu_qspi_en_setf(uint8_t) N_XIP_SECTION;
    switch(pin_role)
    {
    case QSPI_CLK_PIN:
        sysc_cpu_qspi_en_setf(0x3);
        sysctrl_io_config(25, QSPI_CLK);
    break;
    case QSPI_D0_PIN:
        sysctrl_io_config(26, QSPI_D0);
    break;
    case QSPI_D1_PIN:
        sysctrl_io_config(27, QSPI_D1);
    break;
    case QSPI_D2_PIN:
        sysc_cpu_qspi_en_setf(0xf);
        sysctrl_io_config(28, QSPI_D2);
    break;
    case QSPI_D3_PIN:
        sysctrl_io_config(29, QSPI_D3);
    break;
    case QSPI_CS0_PIN:
        sysctrl_io_config(24, QSPI_CS);
    break;
    default:
        BX_ASSERT(0);
    break;
    }
}

N_XIP_SECTION static void qspi_sys_stat(app_qspi_inst_t *inst,uint32_t sys_stat)
{
    switch(sys_stat)
    {
    case QSPI_INIT:
    break;
    case QSPI_UNINIT:
    break;
    case QSPI_OP_START:
    break;
    case QSPI_OP_DONE:
    break;
    default:
        LOG(LOG_LVL_WARN,"unexpected sys stat: %d\n",sys_stat);
    break;
    }    
    
}

const periph_universal_func_set_t qspi_universal_func N_XIP_VARIABLE = {
    .intr_op_func = (intr_op_func_t)NULL,
    .sw_rst_func = (sw_rst_func_t)qspi_sw_rst,
    .clk_src_cfg_func = (clk_src_cfg_func_t)NULL,
    .clk_gate_func = (clk_gate_func_t)qspi_clk_gate,
    .pin_cfg_func = (pin_cfg_func_t)qspi_pin_cfg,
    .sys_stat_func = (sys_stat_func_t)qspi_sys_stat,
};

N_XIP_SECTION uint16_t qspi_get_fifo_depth(app_qspi_inst_t *inst)
{
    return QSPI_FIFO_DEPTH;
}
