#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "reg_iic.h"
#include "field_manipulate.h"
#include "app_iic.h"
#include "bx_dbg.h"
#include "ll.h"
#include "rwip.h"
#include "log.h"
#include "co_utils.h"
#include "app_dmac_wrapper.h"
#include "string.h"
#include "plf.h"
#include "osal.h"

extern periph_universal_func_set_t iic_universal_func;

//set
#define app_iic_set_enable(reg,enable)              FIELD_WR(reg , IC_ENABLE , IIC_IC_EN               , enable)
#define app_iic_set_restart_en(reg,enable)          FIELD_WR(reg , IC_CON    , IIC_IC_RESTART_EN       , enable)
#define app_iic_set_addr_bit_master(reg,val)        FIELD_WR(reg , IC_CON    , IIC_IC_10BITADDR_MASTER , val)
#define app_iic_set_addr_bit_slave(reg,val)         FIELD_WR(reg , IC_CON    , IIC_IC_10BITADDR_SLAVE  , val)
#define app_iic_set_tx_empty_ctrl(reg,val)          FIELD_WR(reg , IC_CON    , IIC_TX_EMPTY_CTRL       , val)
#define app_iic_set_slave_addr(reg,val)             FIELD_WR(reg , IC_SAR    , IIC_SAR                 , val)
#define app_iic_set_rxtl(reg,val)                   FIELD_WR(reg , IC_RX_TL  , IIC_RX_TL               , val)
#define app_iic_set_txtl(reg,val)                   FIELD_WR(reg , IC_TX_TL  , IIC_TX_TL               , val)
#define app_iic_set_tar(reg,val)                    FIELD_WR(reg , IC_TAR    , IIC_TAR                 , val)
#define app_iic_set_int_unmask_tx_empty(reg)        FIELD_WR(reg , IC_INTR_MASK , IIC_M_TX_EMPTY       , 1  )
#define app_iic_set_int_mask_tx_empty(reg)          FIELD_WR(reg , IC_INTR_MASK , IIC_M_TX_EMPTY       , 0  )
#define app_iic_set_int_unmask_tx_abort(reg)        FIELD_WR(reg , IC_INTR_MASK , IIC_M_TX_ABRT        , 1  )
#define app_iic_set_int_mask_tx_abort(reg)          FIELD_WR(reg , IC_INTR_MASK , IIC_M_TX_ABRT        , 0  )
#define app_iic_set_int_unmask_rx_full(reg)         FIELD_WR(reg , IC_INTR_MASK , IIC_M_RX_FULL        , 1  )
#define app_iic_set_int_mask_rx_full(reg)           FIELD_WR(reg , IC_INTR_MASK , IIC_M_RX_FULL        , 0  )


//get
#define app_iic_get_txflr(reg)                      FIELD_RD(reg , IC_TXFLR             , IIC_TXFLR             )
#define app_iic_get_rxflr(reg)                      FIELD_RD(reg , IC_RXFLR             , IIC_RXFLR             )
#define app_iic_get_tfnf(reg)                       FIELD_RD(reg , IC_STATUS            , IIC_TFNF              )
#define app_iic_get_tfe(reg)                        FIELD_RD(reg , IC_STATUS            , IIC_TFE               )
#define app_iic_get_dat(reg)                        FIELD_RD(reg , IC_DATA_CMD          , IIC_DAT               )
#define app_iic_get_txabrt_7b_addr_nack(reg)        FIELD_RD(reg , IC_TX_ABRT_SOURCE    , IIC_ABRT_7BADDR_NOACK )


struct iic_hlcnt_rxtxhold_Config
{
    uint32_t lcnt;    // low level count
    uint32_t hcnt;    // high level count
    uint32_t rx_hold; // rx hold
    uint32_t tx_hold; // tx hold
};
static struct iic_hlcnt_rxtxhold_Config iic_hlcnt_array[IIC_HIGH_SPEED_MODE + 1] =
{
    {0,   0,   0, 0 }, //speed mode is 1-3,array[0] is no use.
    {160, 145, 3, 10}, // STANDARD_100K_32M
    {40,  30,  3, 6 }, // FAST_400K_32M
    {16,  8,   3, 3 }, // HIGH_1M_32M
};

static __IO uint8_t  bx_iic_rd_wr_ret[2] = {IIC_RD_WR_RET_ERROR,IIC_RD_WR_RET_ERROR};
static __IO uint8_t  bx_iic_rd_wr_completed[2] = {0};

//regisiter set function
static __inline void app_iic_set_data_cmd_pack(reg_iic_t *reg,uint8_t restart, uint8_t stop, uint8_t cmd, uint8_t dat)
{
    reg->IC_DATA_CMD = (((uint32_t)restart << 10) | ((uint32_t)stop << 9) | ((uint32_t)cmd << 8) | ((uint32_t)dat << 0));
}
void app_iic_send_addr_cmd(reg_iic_t *reg , uint8_t reg_addr, uint8_t rw)
{
    while(app_iic_get_txflr(reg) >= APP_IIC_RX_TX_FIFO_DEPTH); // tx fifo not full
    app_iic_set_data_cmd_pack(reg,0,0,rw,reg_addr);
}
static void iic_set_target_address(reg_iic_t *reg , uint32_t addr)
{
    app_iic_set_enable(reg,0);
    app_iic_set_tar(reg,addr);
    app_iic_set_enable(reg,1);
}


static void iic_rx_complete(app_iic_inst_t *inst , uint8_t stat)
{
    void (*callback) (void*,uint8_t) = NULL;
    void * dummy = NULL;
    reg_iic_t *reg = inst->reg;
    // Disable interrupt
    app_iic_set_int_mask_rx_full (reg);
    app_iic_set_int_mask_tx_empty(reg);
    app_iic_set_int_mask_tx_abort(reg);
    // Retrieve callback pointer
    callback = inst->env.rx.callback;
    dummy = inst->env.rx.dummy;
    BX_ASSERT(callback);
    // Clear callback pointer
    inst->env.rx.callback = NULL;
    inst->env.rx.dummy = NULL;
    //set iic idle status
    iic_universal_func.sys_stat_func(inst,IIC_READ_DONE);
    //thread unlock
    periph_unlock(&inst->env.iic_lock);
    // Call handler
    callback(dummy, stat);
}
static void iic_tx_complete(app_iic_inst_t *inst , uint8_t stat)
{
    void (*callback) (void*,uint8_t) = NULL;
    void * dummy = NULL;
    reg_iic_t *reg = inst->reg;
    // Disable interrupt
    app_iic_set_int_mask_tx_empty(reg);
    app_iic_set_int_mask_tx_abort(reg);
    // Retrieve callback pointer
    callback = inst->env.tx.callback;
    dummy = inst->env.tx.dummy;
    BX_ASSERT(callback);
    // Clear callback pointer
    inst->env.tx.callback = NULL;
    inst->env.tx.dummy = NULL;
    //set iic idle status
    iic_universal_func.sys_stat_func(inst,IIC_WRITE_DONE);
    //thread unlock
    periph_unlock(&inst->env.iic_lock);
    // Call handler
    callback(dummy, stat);
}




static void iic_rd_req_isr(app_iic_inst_t *inst)
{
    app_iic_set_int_unmask_tx_empty(inst->reg);
}
static void iic_rx_done_isr(app_iic_inst_t *inst)
{
    //no need
    //LOG(LOG_LVL_WARN , "iic_rx_done_isr\n");
}
static void iic_tx_abort_isr(app_iic_inst_t *inst)
{
    reg_iic_t *reg = inst->reg;
    //tx address but no ack
    if(app_iic_get_txabrt_7b_addr_nack(reg))
    {
        //nack error
        inst->env.errordetect = 1;
        //mask tx empty
        app_iic_set_int_mask_tx_empty(reg);
        //abort dma
        if(inst->param.use_dma)
        {
            if(inst->env.rw_mode == IIC_MODE_WRITE)
            {
                if(inst->env.dma_ch1_en)    
                {
                    app_dmac_transfer_cancel_wrapper(inst->env.dma_ch1_ch , NULL);
                    inst->env.dma_ch1_en = 0;
                }
            }
            else
            {
                if(inst->env.dma_ch1_en)    
                {
                    app_dmac_transfer_cancel_wrapper(inst->env.dma_ch1_ch , NULL);
                    inst->env.dma_ch1_en = 0;
                }
                if(inst->env.dma_ch2_en)    
                {
                    app_dmac_transfer_cancel_wrapper(inst->env.dma_ch2_ch , NULL);
                    inst->env.dma_ch2_en = 0;
                }
            }
        }
    }
    if(inst->env.rw_mode == IIC_MODE_WRITE)
    {
        iic_tx_complete(inst , inst->env.errordetect);
    }
    else
    {
        iic_rx_complete(inst , inst->env.errordetect);
    }
}

static void iic_rx_full_isr(app_iic_inst_t *inst)
{
    reg_iic_t *reg = inst->reg;
    //read received data
    while(app_iic_get_rxflr(reg) > 0)
    {
        //read current
        *inst->env.rx.bufptr = app_iic_get_dat(reg);
        inst->env.rx.bufptr ++;
        if(inst->env.rx.remain > 0) inst->env.rx.remain --;
        //callback
        if(inst->env.rx.remain == 0)
        {
            if(inst->env.rw_mode == IIC_MODE_READ)
            {
                iic_rx_complete(inst , inst->env.errordetect);
            }
            return;
        }
    }
    //tramsmit remain data ,generate clocks
    uint8_t rx_fifo_depth = (inst->env.rx.remain > APP_IIC_RX_TX_FIFO_DEPTH) ? APP_IIC_RX_TX_FIFO_DEPTH / 2 : inst->env.rx.remain-1;
    app_iic_set_rxtl(reg,rx_fifo_depth);
	
}

static bool iic_allow_tx(app_iic_inst_t *inst)
{
    bool retval = true;
    reg_iic_t *reg = inst->reg;
    if(inst->env.rw_mode == IIC_MODE_READ)
    {
        if((reg->IC_TXFLR + reg->IC_RXFLR) >= (APP_IIC_RX_TX_FIFO_DEPTH - 1))
        {
            retval = false;
        }
    }
    return retval;
}
static void iic_tx_empty_isr(app_iic_inst_t *inst)
{
    reg_iic_t *reg = inst->reg;
    if(inst->env.tx.remain == 0)
    {
        if(inst->env.rw_mode == IIC_MODE_WRITE)
        {
            iic_tx_complete(inst , inst->env.errordetect);
        }
        else
        {
            //read tx finish , disable tx empty.
            app_iic_set_int_mask_tx_empty(reg);
        }
        return;
    }
    while((app_iic_get_tfnf(reg) == 1) && (iic_allow_tx(inst) == true))// tx fifo not full
    {
        uint8_t rw,stop_bit;
        rw = (inst->env.rw_mode == IIC_MODE_READ) ? 1 : 0 ;
        stop_bit = (inst->env.tx.remain == 1) ? 1 : 0;
        //write one byte
        app_iic_set_data_cmd_pack(reg,0,stop_bit,rw,*inst->env.tx.bufptr);
        //increase buffer ptr
        inst->env.tx.bufptr ++;
        inst->env.tx.remain --;
        if(inst->env.tx.remain == 0)
        {
            app_iic_set_txtl(reg,0);
            return;
        }
    }
}


static void iic_clear_all_irq(reg_iic_t *reg , uint32_t irq_stat)
{
    volatile uint32_t temp_clear_irq_val;
    if(irq_stat & (1 << IIC_INTR_RX_UNDER   ))   temp_clear_irq_val = reg->IC_CLR_RX_UNDER ;
    if(irq_stat & (1 << IIC_INTR_RX_OVER    ))   temp_clear_irq_val = reg->IC_CLR_RX_OVER  ;
    if(irq_stat & (1 << IIC_INTR_TX_OVER    ))   temp_clear_irq_val = reg->IC_CLR_TX_OVER  ;
    if(irq_stat & (1 << IIC_INTR_RD_REQ     ))   temp_clear_irq_val = reg->IC_CLR_RD_REQ   ;
    if(irq_stat & (1 << IIC_INTR_TX_ABORT   ))   temp_clear_irq_val = reg->IC_CLR_TX_ABRT  ;
    if(irq_stat & (1 << IIC_INTR_RX_DONE    ))   temp_clear_irq_val = reg->IC_CLR_RX_DONE  ;
    if(irq_stat & (1 << IIC_INTR_ACTIVITY   ))   temp_clear_irq_val = reg->IC_CLR_ACTIVITY ;
    if(irq_stat & (1 << IIC_INTR_STOP_DET   ))   temp_clear_irq_val = reg->IC_CLR_STOP_DET ;
    if(irq_stat & (1 << IIC_INTR_START_DET  ))   temp_clear_irq_val = reg->IC_CLR_START_DET;
    if(irq_stat & (1 << IIC_INTR_GEN_CALL   ))   temp_clear_irq_val = reg->IC_CLR_GEN_CALL ;
}

void app_iic_isr(app_iic_inst_t *inst)
{
    reg_iic_t *reg = inst->reg;
    uint32_t irq_stat = reg->IC_INTR_STAT;
    uint32_t irq_clr  = irq_stat;
    
    //must before tx empty.(when slave send nack)
    //read status again,because iic_tx_abort_isr will mask the iic_tx_empty_isr.
    if(irq_stat & (1<<IIC_INTR_TX_ABORT)) {iic_tx_abort_isr(inst); irq_stat &= ~(1<<IIC_INTR_TX_ABORT); irq_stat &= ~(1<<IIC_INTR_TX_EMPTY);}
    if(irq_stat & (1<<IIC_INTR_RX_FULL )) {iic_rx_full_isr (inst); irq_stat &= ~(1<<IIC_INTR_RX_FULL );}
    if(irq_stat & (1<<IIC_INTR_TX_EMPTY)) {iic_tx_empty_isr(inst); irq_stat &= ~(1<<IIC_INTR_TX_EMPTY);}
    if(irq_stat & (1<<IIC_INTR_RD_REQ  )) {iic_rd_req_isr  (inst); irq_stat &= ~(1<<IIC_INTR_RD_REQ  );}
    if(irq_stat & (1<<IIC_INTR_RX_DONE )) {iic_rx_done_isr (inst); irq_stat &= ~(1<<IIC_INTR_RX_DONE );}
    //unexpected irq
    if(irq_stat)
    {
        LOG(LOG_LVL_WARN , "iic%d:unexpected irq,stat=0x%x\n" , inst->idx , irq_stat);
    }
    //clear all irq
    iic_clear_all_irq(reg , irq_clr);
}




static void app_iic_set_work_mode(reg_iic_t *reg , uint8_t work_mode)
{
    if(work_mode == IIC_SLAVE) //slave
    {
        GLOBAL_INT_DISABLE();
        FIELD_WR(reg , IC_CON    , IIC_IC_SLAVE_DISABLE , 0);
        FIELD_WR(reg , IC_CON    , IIC_MASTER_MODE      , 0);
        GLOBAL_INT_RESTORE(); 
    }
    else // master
    {
        GLOBAL_INT_DISABLE();
        FIELD_WR(reg , IC_CON    , IIC_IC_SLAVE_DISABLE , 1);
        FIELD_WR(reg , IC_CON    , IIC_MASTER_MODE      , 1);
        GLOBAL_INT_RESTORE(); 
    }
}

static void app_iic_set_speed_mode(app_iic_inst_t * inst,uint32_t clk_cfg)
{
    reg_iic_t *reg = inst->reg;
    app_iic_speed_mode_t speed_mode = (app_iic_speed_mode_t)clk_cfg;

    //set speed mode
    FIELD_WR(reg,IC_CON , IIC_SPEED , speed_mode);
    //set h/l cnt
    if(speed_mode == IIC_STANDARD_MODE)
    {
        FIELD_WR(reg , IC_SS_SCL_HCNT , IIC_SS_SCL_HCNT , iic_hlcnt_array[speed_mode].hcnt);
        FIELD_WR(reg , IC_SS_SCL_LCNT , IIC_SS_SCL_LCNT , iic_hlcnt_array[speed_mode].lcnt);
    }
    else if(speed_mode == IIC_FAST_MODE)
    {
        FIELD_WR(reg , IC_FS_SCL_HCNT , IIC_FS_SCL_HCNT , iic_hlcnt_array[speed_mode].hcnt);
        FIELD_WR(reg , IC_FS_SCL_LCNT , IIC_FS_SCL_LCNT , iic_hlcnt_array[speed_mode].lcnt);
    }
    else if(speed_mode == IIC_HIGH_SPEED_MODE)
    {
        FIELD_WR(reg , IC_HS_SCL_HCNT , IIC_HS_SCL_HCNT , iic_hlcnt_array[speed_mode].hcnt);
        FIELD_WR(reg , IC_HS_SCL_LCNT , IIC_HS_SCL_LCNT , iic_hlcnt_array[speed_mode].lcnt);
    }
    //set tx/rx hold
    FIELD_WR(reg , IC_SDA_HOLD , IIC_IC_SDA_RX_HOLD , iic_hlcnt_array[speed_mode].rx_hold);
    FIELD_WR(reg , IC_SDA_HOLD , IIC_IC_SDA_TX_HOLD , iic_hlcnt_array[speed_mode].tx_hold);

}

void app_iic_init(periph_inst_handle_t hdl)
{
    //get parameter
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    app_iic_comm_params_t *param = &inst->param;
    reg_iic_t *reg = inst->reg;
    
    //set system config
    iic_universal_func.sw_rst_func(inst);                           //software reset
    iic_universal_func.pin_cfg_func(inst,param->scl_pin,IIC_SCL_PIN,true); 
    iic_universal_func.pin_cfg_func(inst,param->sda_pin,IIC_SDA_PIN,true);
    
    //set parameter
    app_iic_set_enable(reg , 0);                                    //disable
    iic_universal_func.clk_src_cfg_func(inst , param->speed_mode);  //set clock src
    app_iic_set_speed_mode(inst , param->speed_mode);               //set speed mode
    iic_universal_func.clk_gate_func(inst,SET_CLK);                 //set clock gate
    app_iic_set_restart_en(reg , 1);                                //set restart en
    app_iic_set_addr_bit_master(reg,param->dev_addr_bit_num);       //set master address bit number
    app_iic_set_addr_bit_slave(reg,param->dev_addr_bit_num);        //set slave address bit number
    app_iic_set_work_mode(reg , param->work_mode);                  //set work mode
    app_iic_set_tx_empty_ctrl(reg,1);
    if(param->work_mode == IIC_SLAVE)                               //set slave address
    {
        app_iic_set_slave_addr(reg,param->slave_address >> 1);
    }
    
    //set enable
    iic_universal_func.intr_op_func(inst,INTR_CLR);                            //clear  interrupt
    iic_universal_func.intr_op_func(inst,INTR_ENABLE);                         //enable interrupt
    reg->IC_INTR_MASK = 0xef;                                                  //unmask all IRQ
    if(param->use_dma)
    {
        reg->IC_DMA_CR    =  3;     //set dma parameter
        reg->IC_DMA_TDLR  = 15;     //
        reg->IC_DMA_RDLR  =  0;     //
        reg->IC_INTR_MASK =  0;     //mask all iic IRQ
    }
    app_iic_set_enable(reg , 1);                            //enable
    iic_universal_func.sys_stat_func(inst,IIC_INIT);        //set state
    iic_universal_func.clk_gate_func(inst,CLR_CLK);         //clear clock gate
    
    //set thread lock
    periph_semaphore_init(&inst->env.iic_lock,1);
}

void app_iic_uninit(periph_inst_handle_t hdl)
{
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    reg_iic_t *reg = inst->reg;
    
    app_iic_set_enable(reg , 0);                                   //disable
    iic_universal_func.clk_gate_func(inst,CLR_CLK);                //
    iic_universal_func.intr_op_func (inst,INTR_DISABLE);           //
    iic_universal_func.sys_stat_func(inst,IIC_UNINIT);             //
    iic_universal_func.pin_cfg_func(inst,inst->param.scl_pin,IIC_SCL_PIN,false); 
    iic_universal_func.pin_cfg_func(inst,inst->param.sda_pin,IIC_SDA_PIN,false);
    //set thread lock
    periph_semaphore_init(&inst->env.iic_lock,0);
}



void iic_transfer_sanity_check(uint8_t *bufptr,uint32_t size,void (*callback)(void *,uint8_t))
{
    // Sanity check
    BX_ASSERT(bufptr != NULL);
    BX_ASSERT(size != 0);
    BX_ASSERT(callback != NULL);
}


static void iic_read_dma_send_clock_callback(app_iic_inst_t *inst)
{
    inst->env.dma_ch2_en = 0;
}

static void iic_read_dma_callback(app_iic_inst_t *inst)
{
    inst->env.dma_ch1_en = 0;
}


static periph_err_t app_iic_read_dma(app_iic_inst_t *inst,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address)
{
    periph_err_t retval = PERIPH_NO_ERROR;
    reg_iic_t *reg = inst->reg;
    uint32_t *p_rx_pointer = inst->env.rx.dma_buffer;
    memset((void*)inst->env.rx.dma_buffer,0,sizeof(inst->env.rx.dma_buffer));
    
    //--------------------------Read data DMA--------------------------------//
    app_dmac_transfer_param_t param = 
    {
        .callback = (void (*)(void*))iic_read_dma_callback,
        .callback_param = inst,
        .src = (uint8_t *)&reg->IC_DATA_CMD,
        .dst = (uint8_t *)bufptr,
        .length = size,
        .src_tr_width = Transfer_Width_8_bits,
        .dst_tr_width = Transfer_Width_8_bits,
        .src_msize = Burst_Transaction_Length_1,
        .dst_msize = Burst_Transaction_Length_1,
        .tt_fc = Peripheral_to_Memory_DMAC_Flow_Controller,
        .src_per = dmac_iic_rx_handshake_enum(inst->idx),
        .dst_per = 0,
        .int_en = 1,
    };
    inst->env.dma_ch1_en = 1;
    retval = app_dmac_start_wrapper(&param , &inst->env.dma_ch1_ch);
    if(retval != PERIPH_NO_ERROR) 
    {
        return retval;
    }        
    
    //--------------------------send clock DMA----------------------------------//
    param.src_tr_width = Transfer_Width_32_bits;
    param.dst_tr_width = Transfer_Width_32_bits;
    param.src = (uint8_t *)inst->env.rx.dma_buffer;//anything
    param.dst = (uint8_t *)&reg->IC_DATA_CMD;
    param.callback = (void (*)(void*))iic_read_dma_send_clock_callback;
    param.tt_fc = Memory_to_Peripheral_DMAC_Flow_Controller;
    param.src_per = 0;
    param.dst_per = dmac_iic_tx_handshake_enum(inst->idx);
    //memory address generation
    uint8_t stop_bit = (size == 1) ? 1 : 0;
    if(inst->param.mem_addr_bit_num == IIC_16BIT_MEMORY_ADDRESS)
    {
    	param.length = (size+2);// 16*4 bytes data written to EEPROM, and 1 write cmd. Write reg for 17 times.
    	*p_rx_pointer = (mem_address >> 8  );  p_rx_pointer ++;// restart = 0, stop = 0, cmd = write, data(addr) = 0
    	*p_rx_pointer = (mem_address & 0xFF);  p_rx_pointer ++;
        //Restart command
        *p_rx_pointer = (1 << 10)|(stop_bit << 9)|(1 << 8)|device_address;// restart = 1, stop = stop_bit, cmd = read, data(addr) = 0xA0
        p_rx_pointer++;
    }
    if(inst->param.mem_addr_bit_num == IIC_8BIT_MEMORY_ADDRESS)
	{
		param.length = (size+1);//new dma driver is no need *4
		*p_rx_pointer = (mem_address & 0xFF); p_rx_pointer ++;
	    //Restart command
	    *p_rx_pointer = (1 << 10)|(stop_bit << 9)|(1 << 8)|device_address;// restart = 1, stop = stop_bit, cmd = read, data(addr) = 0xA0
	    p_rx_pointer++;
	}
    if(inst->param.mem_addr_bit_num == IIC_NO_MEMORY_ADDRESS)
	{
		param.length = (size);
	}

    //data receive
    //need to reduce one byte (send restart will auto send a data byte)
    size --;
    for(uint8_t cnt = 0 ; cnt < size ; cnt ++) 
    {
        uint8_t stop_bits = (cnt == (size - 1)) ? 1 : 0;
    	*p_rx_pointer = (stop_bits<<9) | (1 << 8) | *bufptr;// restart = 0, stop = ?, cmd = write, 
    	p_rx_pointer++;
    	bufptr++;
    }
    //output pulse
    iic_set_target_address(reg , device_address >> 1);
    inst->env.dma_ch2_en = 1;
    retval = app_dmac_start_wrapper(&param , &inst->env.dma_ch2_ch);
    return retval;
}

static periph_err_t app_iic_read_no_dma(app_iic_inst_t *inst,uint8_t *bufptr, uint32_t size,uint16_t device_address, uint16_t mem_address)
{
    reg_iic_t *reg = inst->reg;
    uint8_t stop_bit = (size == 1) ? 1 : 0;
    uint8_t tl_value = 1;
    //set main data
    inst->env.tx.remain = inst->env.rx.remain = size;   //tx need send clock.
    inst->env.tx.bufptr = inst->env.rx.bufptr = bufptr;
    //set rxtl must before restart command sent.
    if(size >= 2)
    {
        tl_value = (size > APP_IIC_RX_TX_FIFO_DEPTH) ? APP_IIC_RX_TX_FIFO_DEPTH/2 : size-1;
    }
    else
    {
        tl_value = 1;
    }
    app_iic_set_txtl(reg, tl_value);
    app_iic_set_rxtl(reg, tl_value-1);
    //send device address,memory address and resart+first byte.
    ATOMIC_OP(
    if(inst->param.work_mode == IIC_MASTER)
    {
        iic_set_target_address(reg,device_address >> 1);//slave don't need set target addr.
        //send head data
        if(inst->param.mem_addr_bit_num == IIC_16BIT_MEMORY_ADDRESS) app_iic_send_addr_cmd(reg , mem_address >> 8  , 0);
        if(inst->param.mem_addr_bit_num != IIC_NO_MEMORY_ADDRESS)    app_iic_send_addr_cmd(reg , mem_address & 0xFF, 0);
        //restart = 1, r/w = 1, dat = 0xA0, stop bit=0
        //restart will send a byte of data
        if(inst->param.mem_addr_bit_num != IIC_NO_MEMORY_ADDRESS)    
        {
            app_iic_set_data_cmd_pack(reg,1, stop_bit, 1, 0);
            inst->env.tx.remain --;
        }
    }
    //send clocks
    app_iic_set_int_unmask_rx_full (reg);
    app_iic_set_int_unmask_tx_empty(reg);
    );
    
    return PERIPH_NO_ERROR;
}

periph_err_t app_iic_read(periph_inst_handle_t hdl,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address , void (*callback) (void*,uint8_t),void* dummy)
{
    periph_err_t retval;
    iic_transfer_sanity_check(bufptr,size,callback);
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    reg_iic_t *reg = inst->reg;
    //thread lock
    if(periph_lock(&inst->env.iic_lock)==false)
    {
        return PERIPH_BUSY;
    }
    inst->env.rx.callback = callback; 
    inst->env.rx.dummy = dummy;
    inst->env.rw_mode = IIC_MODE_READ;
    inst->env.errordetect = 0;
    iic_universal_func.sys_stat_func(inst,IIC_READ_START);
    iic_universal_func.clk_gate_func(inst,SET_CLK);
    //must before unmask tx empty.otherwise,if read/write 1 bytes and NACK , will into callback twice.
    app_iic_set_int_unmask_tx_abort(reg);
    if(inst->param.use_dma)
    {
        retval = app_iic_read_dma(inst,bufptr,size,device_address,mem_address);
    }else
    {
        retval = app_iic_read_no_dma(inst,bufptr,size,device_address,mem_address);
    }
    return retval;
}

static void iic_write_dma_callback(app_iic_inst_t *inst)
{
    reg_iic_t *reg = inst->reg;
    inst->env.dma_ch1_en = 0;
    inst->env.tx.remain = 0;
    app_iic_set_int_unmask_tx_empty(reg);
}

static periph_err_t app_iic_write_dma(app_iic_inst_t *inst,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address)
{
    periph_err_t retval = PERIPH_NO_ERROR;
    reg_iic_t *reg = inst->reg;
    uint32_t *p_tx_pointer = inst->env.tx.dma_buffer;
    
    app_dmac_transfer_param_t param = 
    {
        .callback = (void (*)(void*))iic_write_dma_callback,
        .callback_param = inst,
        .src = (uint8_t *)p_tx_pointer,
        .dst = (uint8_t *)&reg->IC_DATA_CMD,
        .length = 0,//set value later
        .src_tr_width = Transfer_Width_32_bits,
        .dst_tr_width = Transfer_Width_32_bits,
        .src_msize = Burst_Transaction_Length_1,
        .dst_msize = Burst_Transaction_Length_1,
        .tt_fc = Memory_to_Peripheral_DMAC_Flow_Controller,
        .src_per = 0,
        .dst_per = dmac_iic_tx_handshake_enum(inst->idx),
        .int_en = 1,
    };

    //memory address generation
    if(inst->param.mem_addr_bit_num == IIC_16BIT_MEMORY_ADDRESS)
    {
        param.length = (size+2);// 16*4 bytes data written to EEPROM, and 1 write cmd. Write reg for 17 times.
        *p_tx_pointer = mem_address >> 8;    p_tx_pointer ++;// restart = 0, stop = 0, cmd = write, data(addr) = 0
        *p_tx_pointer = mem_address & 0xFF;  p_tx_pointer ++;
    }
    if(inst->param.mem_addr_bit_num == IIC_8BIT_MEMORY_ADDRESS)
    {
		param.length = (size+1);//new dma driver is no need *4
		*p_tx_pointer  = mem_address & 0xFF; p_tx_pointer ++;
    }
    if(inst->param.mem_addr_bit_num == IIC_NO_MEMORY_ADDRESS)
	{
		param.length = (size);
	}
	//data make
    for(uint32_t cnt = 0 ; cnt < size ; cnt ++)
    {
        uint8_t stop_bit = (cnt == (size - 1)) ? 1 : 0;
        *p_tx_pointer = (stop_bit << 9) | *bufptr;// restart = 0, stop = ?, cmd = write
    	p_tx_pointer++;
    	bufptr++;
    }
    //output pulse
    iic_set_target_address(reg , device_address >> 1);
    inst->env.dma_ch1_en = 1;
    retval = app_dmac_start_wrapper(&param , &inst->env.dma_ch1_ch);
    return retval;
}

static periph_err_t app_iic_write_no_dma(app_iic_inst_t *inst,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address)
{
    reg_iic_t *reg = inst->reg;
    inst->env.tx.size = size;
    inst->env.tx.remain = size;
    inst->env.tx.bufptr = bufptr;
    app_iic_set_txtl(reg,APP_IIC_RX_TX_FIFO_DEPTH/2);
    ATOMIC_OP(
    if(inst->param.work_mode == IIC_MASTER)
    {
        iic_set_target_address(reg, device_address >> 1);
        //send head data
        if(inst->param.mem_addr_bit_num == IIC_16BIT_MEMORY_ADDRESS) app_iic_send_addr_cmd(reg , mem_address >> 8  , 0);
        if(inst->param.mem_addr_bit_num != IIC_NO_MEMORY_ADDRESS)    app_iic_send_addr_cmd(reg , mem_address & 0xFF, 0);
        //start transfer
        app_iic_set_int_unmask_tx_empty(reg);
    }
    );
    
    return PERIPH_NO_ERROR;
}

periph_err_t app_iic_write(periph_inst_handle_t hdl,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address , void (*callback) (void*,uint8_t),void* dummy)
{
    periph_err_t retval;
    iic_transfer_sanity_check(bufptr,size,callback);
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    reg_iic_t *reg = inst->reg;
    //thread lock
    if(periph_lock(&inst->env.iic_lock)==false)
    {
        return PERIPH_BUSY;
    }
    inst->env.tx.callback = callback; 
    inst->env.tx.dummy = dummy;
    inst->env.rw_mode = IIC_MODE_WRITE;
    inst->env.errordetect = 0;
    iic_universal_func.sys_stat_func(inst,IIC_WRITE_START);
    iic_universal_func.clk_gate_func(inst,SET_CLK);
    app_iic_set_int_unmask_tx_abort(reg);
    if(inst->param.use_dma)
    {
        retval = app_iic_write_dma(inst,bufptr,size,device_address,mem_address);
    }
    else
    {
        retval = app_iic_write_no_dma(inst,bufptr,size,device_address,mem_address);
    }
    return retval;
}

static void app_iic_master_complete_prepare(app_iic_inst_t* inst)
{
    app_iic_speed_mode_t speed = inst->param.speed_mode;
    
    if(speed == IIC_STANDARD_MODE  )  BX_DELAY_US(1000 / 100  * 3);
    if(speed == IIC_FAST_MODE      )  BX_DELAY_US(1000 / 400  * 3);
    if(speed == IIC_HIGH_SPEED_MODE)  BX_DELAY_US(1000 / 1000 * 3);
}

void bx_iic_read_write_callback0(void* ptr, uint8_t stat)
{
    bx_iic_rd_wr_completed[0] = 1;
    bx_iic_rd_wr_ret[0] = stat;
}

void bx_iic_read_write_callback1(void* ptr, uint8_t stat)
{
    
    bx_iic_rd_wr_completed[1] = 1;
    bx_iic_rd_wr_ret[1] = stat;
}

/*****************************************************************************************
 *
 * function name bx_iic_read
 *
 * @brief  readwith iic , have timeout loop, and return value
 *
 *  hdl: iic handle
 *  bufptr: data's address
 *  size : 
 *****************************************************************************************/
uint32_t bx_iic_read(periph_inst_handle_t hdl,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address , uint32_t timeout/*ms*/)
{
    static uint8_t error_counter = 0;
    uint32_t curseq = timeout;
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    if (0 != inst->idx && 1!= inst->idx)
    {
        return IIC_RD_WR_RET_ERROR;
    }
    
    bx_iic_rd_wr_ret[inst->idx]= IIC_RD_WR_RET_ERROR;
    error_counter = 0;
    
    while( IIC_RD_WR_RET_OK  != bx_iic_rd_wr_ret[inst->idx]   && curseq > 0)
    {
        bx_iic_rd_wr_completed[inst->idx] = 0;

        if (0 == inst->idx)
        {
            app_iic_read(hdl, bufptr, size, device_address, mem_address, bx_iic_read_write_callback0, 0);

        }
        else
        {
            app_iic_read(hdl, bufptr, size, device_address, mem_address, bx_iic_read_write_callback1, 0);
        }
    
        while( 0 == bx_iic_rd_wr_completed[inst->idx] && curseq > 0)
        {
            OS_DELAY_MS(2);
            //BX_DELAY_US(2000);
            curseq--;
        }
        
        if (IIC_RD_WR_RET_OK != bx_iic_rd_wr_ret[inst->idx])
        {
            LOG(LOG_LVL_WARN , "iic read error,,stat=0x%x\n" ,bx_iic_rd_wr_ret[inst->idx]);
            OS_DELAY_MS(2);
            if(error_counter++ == 50)
            {
                LOG(LOG_LVL_WARN , "iic read multile error!!!!!!!!!!\n" );
                while(1);
            }
        }
        else
        {
            app_iic_master_complete_prepare(inst);
        }
    }
    return bx_iic_rd_wr_ret[inst->idx];
}


/*****************************************************************************************
 *
 * function name bx_iic_write
 *
 * @brief  write with iic , have timeout loop, and return value
 *
 *  hdl: iic handle
 *  bufptr: data's address
 *  size : 
 *****************************************************************************************/
uint32_t bx_iic_write(periph_inst_handle_t hdl,uint8_t *bufptr, uint32_t size , uint16_t device_address, uint16_t mem_address , uint32_t timeout)
{
    uint32_t curseq = timeout;
    app_iic_inst_t *inst = CONTAINER_OF(hdl, app_iic_inst_t, inst);
    if (0 != inst->idx && 1!= inst->idx)
    {
        return IIC_RD_WR_RET_ERROR;
    }

    bx_iic_rd_wr_ret[inst->idx] = IIC_RD_WR_RET_ERROR;
    
    while( IIC_RD_WR_RET_OK  != bx_iic_rd_wr_ret[inst->idx]   && curseq > 0)
    {
        bx_iic_rd_wr_completed[inst->idx] = 0;

        if (0 == inst->idx)
        {
            app_iic_write(hdl, bufptr, size, device_address, mem_address, bx_iic_read_write_callback0, 0);
        }
        else
        {
            app_iic_write(hdl, bufptr, size, device_address, mem_address, bx_iic_read_write_callback1, 0);
        }
            
        while( 0 == bx_iic_rd_wr_completed[inst->idx] && curseq > 0)
        {
            OS_DELAY_MS(2);
            //BX_DELAY_US(2000);
            curseq--;
        }
        
        if (IIC_RD_WR_RET_OK != bx_iic_rd_wr_ret[inst->idx])
        {
            LOG(LOG_LVL_WARN , "iic write error,stat=0x%x\n" ,bx_iic_rd_wr_ret[inst->idx]);
        }
        else
        {
            app_iic_master_complete_prepare(inst);
        }
    }

    return bx_iic_rd_wr_ret[inst->idx];
}


