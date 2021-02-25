#include "control_tx_process.h"
#include "upper_tx_process.h"
#include "static_buffer.h"
#include "log.h"
#include "aes_ccm_cmac.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_env.h"
#include "stack_mem_cfg.h"
#include "mesh_core_api.h"

DEF_ARRAY_BUF(control_tx_pdu_buf, control_pdu_tx_t, CONTROL_TX_PDU_BUF_SIZE);

static control_pdu_tx_t *control_tx_pdu_buf_alloc()
{
    return array_buf_alloc(&control_tx_pdu_buf);
}

static void control_tx_pdu_buf_release(control_pdu_tx_t *ptr)
{
    array_buf_release(&control_tx_pdu_buf,ptr);
}

control_pdu_tx_t *control_tx_pdu_alloc(uint16_t pdu_length,bool seg, uint8_t opcode)
{
    control_pdu_tx_t *ptr = control_tx_pdu_buf_alloc();
    if(ptr)
    {
        ptr->pdu_base.seg = seg;
        ptr->pdu_base.total_length = pdu_length;
        ptr->payload = mesh_alloc(pdu_length);
        BX_ASSERT(ptr->payload);
        ptr->pdu_base.ctl = 1;
        ptr->pdu_base.seq_auth_valid = 0;
        ptr->opcode = opcode;
    }
    return ptr;
}

void control_set_tx_dst_addr(control_pdu_tx_t *ptr, virt_addr_mngt_t *virt,uint16_t dst_addr)
{
    if(virt)
    {
        ptr->pdu_base.dst_addr = virt->virt_addr;
    }else
    {
        ptr->pdu_base.dst_addr = dst_addr;
    }
}

void control_tx_complete(control_pdu_tx_t *tx,uint8_t status)
{
    LOG(3, "%s\n", __func__);
    tx->pdu_base.callback(tx,status);
    mesh_free(tx->payload);
    control_tx_pdu_buf_release(tx);
}

void control_send(control_pdu_tx_t *ptr)
{
    LOG(3, "%s\n", __func__);
    ptr->pdu_base.iv_index = mesh_tx_iv_index_get();
    upper_tx_env_add_new_pdu(&ptr->pdu_base);
}

control_pdu_tx_t *control_unseg_msg_build(control_msg_tx_param_t *param,void (*callback)(void *,uint8_t))
{
    control_pdu_tx_t *ptr = control_tx_pdu_alloc(param->length,false,param->opcode);
    if(ptr)
    {
        ptr->pdu_base.ttl = param->ttl;
        ptr->pdu_base.netkey_credentials = param->netkey_credentials;
        ptr->pdu_base.src_addr = mesh_node_get_primary_element_addr();
        ptr->pdu_base.dst_addr = param->dst_addr;
        if(param->opcode>0&&param->opcode<HEARTBEAT)
        {
            ptr->pdu_base.interval_ms = 0;
            ptr->pdu_base.repeats = 0;
        }else
        {
            mesh_core_params_t param_network_transmit;
            mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param_network_transmit);
            ptr->pdu_base.interval_ms = param_network_transmit.network_transmit.interval_steps* 10;
            ptr->pdu_base.repeats = param_network_transmit.network_transmit.count;
        }
        ptr->pdu_base.callback = callback;
        if(param->expected_tx_time)
        {
            ptr->pdu_base.expected_tx_time_valid = 1;
            ptr->pdu_base.expected_tx_time = *param->expected_tx_time;
        }else
        {
            ptr->pdu_base.expected_tx_time_valid = 0;
        }
    }
    return ptr;
}



