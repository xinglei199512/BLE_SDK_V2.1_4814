#include "periph_common.h"
#include "clk_gate.h"
#include "rst_gen.h"
#include "reg_sysc_cpu.h"
#include "compiler_flag.h"

N_XIP_SECTION static void cache_clk_gate(void *inst,clk_gate_operation_t op)
{
    if(op == SET_CLK)
    {
        clk_gate_cpu_g1(CPU_CLKG_SET_CACHE);
    }else
    {
        clk_gate_cpu_g1(CPU_CLKG_CLR_CACHE);
    }
    
}

N_XIP_SECTION static void cache_sys_stat(void *inst,uint32_t sys_stat)
{
    sysc_cpu_cache_has_sram_setf((uint8_t)sys_stat);
}

N_XIP_VARIABLE  clk_gate_func_t const cache_clk_gate_func = cache_clk_gate;
N_XIP_VARIABLE  sys_stat_func_t const cache_sys_stat_func = cache_sys_stat;
