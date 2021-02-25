#include "periph_common.h"
#include "app_iic.h"
#include "periph_recovery.h"
#include "bx_config.h"
#include "apollo_00.h"
#include "bx_dbg.h"
#include "rst_gen.h"
#include "clk_gate.h"
#include "reg_sysc_per.h"
#include "pshare.h"
#include "log.h"
#include "sysctrl.h"
#include "ll.h"



static app_iic_inst_t *iic_inst_env[2];



static void iic_intr_op(app_iic_inst_t * inst,intr_operation_t op)
{
    IRQn_Type iic_irq = inst->idx ? IIC1_IRQn : IIC0_IRQn;
    if(op == INTR_ENABLE)
    {
        iic_inst_env[inst->idx] = inst;
        NVIC_EnableIRQ(iic_irq);
    }else if(op == INTR_DISABLE)
    {
        NVIC_DisableIRQ(iic_irq);
    }else if(op == INTR_CLR)
    {
        NVIC_ClearPendingIRQ(iic_irq);
    }else
    {
        BX_ASSERT(0);
    }
}

static void iic_sw_rst(app_iic_inst_t * inst)
{
    uint32_t rst_mask = inst->idx ? IIC1_SRST_PER : IIC0_SRST_PER;
    srst_per(rst_mask);
}

//parameter is speed mode
static void iic_clk_src_cfg(app_iic_inst_t * inst,uint32_t clk_cfg)
{
    uint8_t idx =inst->idx;
    //set clock source
    ATOMIC_OP(
    if(idx)
    {
        #if USE_PLL == 1
            sysc_per_clkg0_set(1<<7);
            sysc_per_clk_div_iic1_para_m1_setf(2&0xf);
            sysc_per_clk_sel_iic1_setf(3);
            sysc_per_clkg0_set(1<<6);
        #else
            sysc_per_clkg0_set(1<<5);
            sysc_per_clk_sel_iic1_setf(0);
            sysc_per_clkg0_set(1<<4);
            clk_gate_per_g1(PER_CLKG0_SET_IIC0);
        #endif
    }
    else
    {
        #if USE_PLL == 1
            sysc_per_clkg0_set(1<<3);
            sysc_per_clk_div_iic0_para_m1_setf(2&0xf);
            sysc_per_clk_sel_iic0_setf(3);
            sysc_per_clkg0_set(1<<2);
        #else
            sysc_per_clkg0_set(1<<1);
            sysc_per_clk_sel_iic0_setf(0);
            sysc_per_clkg0_set(1<<0);
            clk_gate_per_g0(PER_CLKG0_SET_IIC0);
        #endif
    }
    );
}

static void iic_clk_gate(app_iic_inst_t * inst,clk_gate_operation_t op)
{
    uint8_t idx = inst->idx;
    #if (defined(CFG_ON_CHIP)&&(CFG_ON_CHIP==1))
    if(op == SET_CLK )
    {
        if(idx)
        {
            clk_gate_per_g0(PER_CLKG0_SET_IIC1|PER_CLKG1_SET_IIC1);
        }else
        {
            clk_gate_per_g0(PER_CLKG0_SET_IIC0|PER_CLKG1_SET_IIC0);
        }
    }else
    {
        if(idx)
        {
            clk_gate_per_g0(PER_CLKG0_CLR_IIC1|PER_CLKG1_CLR_IIC1);
        }else
        {
            clk_gate_per_g0(PER_CLKG0_CLR_IIC0|PER_CLKG1_CLR_IIC0);
        }
    }   
    #endif
}

static void iic_pin_cfg(app_iic_inst_t * inst,uint8_t pin_num,uint32_t pin_role,bool enable)
{
    uint8_t idx = inst->idx;
    switch(pin_role)
    {
    case IIC_SCL_PIN:
        sysctrl_io_config(pin_num, enable ? IIC_SCL : GPIO_INPUT);
        if(idx)
        {
            pshare_funcio_set(pin_num - 2,IO_IIC1_SCL, enable);
        }else
        {
            pshare_funcio_set(pin_num - 2,IO_IIC0_SCL, enable);
        }
        break;
    case IIC_SDA_PIN:
        sysctrl_io_config(pin_num, enable ? IIC_SDA : GPIO_INPUT);
        if(idx)
        {
            pshare_funcio_set(pin_num - 2,IO_IIC1_SDA, enable);
        }else
        {
            pshare_funcio_set(pin_num - 2,IO_IIC0_SDA, enable);
        }
        break;
    default:
        BX_ASSERT(0);
    break;
    }
    //pull up
    ATOMIC_OP(
    if((enable) && (inst->param.enable_pull_up))
    {
        hwp_sysc_awo->gpio_ps.val |= (1 << pin_num);//pull up select
        hwp_sysc_awo->gpio_pe.val |= (1 << pin_num);//pull up enable
    }
    else
    {
        hwp_sysc_awo->gpio_ps.val &= ~(1 << pin_num);//pull up select
        hwp_sysc_awo->gpio_pe.val &= ~(1 << pin_num);//pull up enable
    }
    );
}

static void iic_sys_stat(app_iic_inst_t *inst,uint32_t sys_stat)
{
    uint8_t idx = inst->idx;
    switch(sys_stat)
    {
    case IIC_INIT:
        recovery_list_add(periph_domain_recovery_buf,PERIPH_DOMAIN_IIC0+inst->idx,&inst->inst);
    break;
    case IIC_UNINIT:
        
        recovery_list_remove(periph_domain_recovery_buf,PERIPH_DOMAIN_IIC0+inst->idx);
    break;
    case IIC_READ_START:
    case IIC_WRITE_START:
        if(idx)
        {
            periph_domain_stat.iic1 = 1;
        }else
        {
            periph_domain_stat.iic0 = 1;
        }
    break;
    case IIC_READ_DONE:
    case IIC_WRITE_DONE:
        if(idx)
        {
            periph_domain_stat.iic1 = 0;
        }else
        {
            periph_domain_stat.iic0 = 0;
        }
    break;
    default:
        LOG(LOG_LVL_WARN,"unexpected sys stat: %d\n",sys_stat);
    break;
    }
}

const periph_universal_func_set_t iic_universal_func = 
{
    .intr_op_func     = (intr_op_func_t    )iic_intr_op,
    .sw_rst_func      = (sw_rst_func_t     )iic_sw_rst,
    .clk_src_cfg_func = (clk_src_cfg_func_t)iic_clk_src_cfg,
    .clk_gate_func    = (clk_gate_func_t   )iic_clk_gate,
    .pin_cfg_func     = (pin_cfg_func_t    )iic_pin_cfg,
    .sys_stat_func    = (sys_stat_func_t   )iic_sys_stat,
};



void IIC0_IRQHandler(void)
{
    app_iic_isr(iic_inst_env[0]);
}

void IIC1_IRQHandler(void)
{
    app_iic_isr(iic_inst_env[1]);
}

