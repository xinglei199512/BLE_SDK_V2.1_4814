
#include "access_rx_process.h"
#include "model_rx.h"
#include "log.h"
#include "mesh_core_api.h"
#include "config_client.h"
#include "config_server.h"
#include "model_msg.h"


static bool use_devkey(uint32_t model_id,bool sig_model)
{
    return sig_model && (model_id == CONFIGURATION_CLIENT_MODEL_ID || model_id == CONFIGURATION_SERVER_MODEL_ID);
}


static void deliver_access_pdu_to_model_inst(model_base_t *model,access_pdu_rx_t *pdu,msg_handler_model_t *ptr)
{
    bool key_bound;
    
    if(access_is_access_pdu_type(pdu))
    {//appkey
        key_bound = appkey_model_check(model,access_get_pdu_app_key(pdu));
    }else
    {//devkey
        key_bound = use_devkey(model->model_id,model->sig_model);
    }
    if(key_bound)
    {
         ptr->hdl(model->elmt,model,pdu);
    }else
    {
        LOG(LOG_LVL_WARN,"key not bound\n");
    }
}

static void deliver_access_pdu_to_elmt_model(mesh_elmt_t *elmt,access_pdu_rx_t *pdu,msg_handler_model_t *ptr,bool sig_model)
{
    model_base_t *model_inst = get_model_inst_from_elmt(elmt,ptr->model_id,sig_model);
    //LOG(3,"elmt=0x%x model_id:%x sig_model:%d\n", elmt, ptr->model_id, sig_model);
    if(model_inst == 0)
    {
        LOG(3,"model_inst=0\n");
        return;
    }
    //BX_ASSERT(model_inst);
    deliver_access_pdu_to_model_inst(model_inst,pdu,ptr);
}

static bool subscription_list_search(model_base_t *model,uint16_t dst_addr,virt_addr_mngt_t *virt_addr)
{
    uint8_t j;
    for(j=0;j<model->subscription_list_size;++j)
    {
        //LOG(3, "subscription_list_search:%x %x\n", model->subscription_list[j].is_virt, IS_VIRTUAL_ADDR(dst_addr));
        if(IS_VIRTUAL_ADDR(dst_addr) && model->subscription_list[j].is_virt)
        {
            if(model->subscription_list[j].addr.virt == virt_addr)
            {
                return true;
            }
        }else if(!IS_VIRTUAL_ADDR(dst_addr) && !model->subscription_list[j].is_virt)
        {
            //LOG(3, "subscription_list_search addr:%x %x\n", model->subscription_list[j].addr.addr, dst_addr);
            if(model->subscription_list[j].addr.addr == dst_addr)
            {
                return true;
            }
        }
    }
    return false;
}

void deliver_to_model(upper_pdu_rx_t* uppdu,void * access_data,access_pdu_decrypt_callback_param_t*param)
{
    access_pdu_rx_t access_rx_pdu = {
        .uppdu = uppdu,
        .access_payload = access_data,
        .param = param,
    };
    access_pdu_rx_t *access_pdu = &access_rx_pdu;
    uint16_t dst_addr = access_get_pdu_dst_addr(access_pdu);
    uint8_t opcode_head= access_get_pdu_op_head(access_pdu);
    msg_handler_model_t *ptr;
    bool sig_model;
		
    LOG(3,"access_pdu_rx seq_auth:%x  uppdu->src:%x   uppdu->src_addr:%x   uppdu->dst_addr:%x    uppdu->iv_index:%x\n", uppdu->seq_auth,uppdu->src,uppdu->src_addr,uppdu->dst_addr,uppdu->iv_index);
    if((opcode_head & 0xc0) == 0xc0)
    {
        sig_model = false;
    }else
    {
        sig_model = true;
    }
		extern void show_buf(const char *name, uint8_t *buf, int len);\
    show_buf("pdu_rx:",(uint8_t *) uppdu,sizeof(upper_pdu_rx_t));
		
    show_buf("mesh_sig_msg_handler_search", access_get_pdu_payload(access_pdu),access_get_pdu_payload_length(access_pdu));
    ptr = mesh_sig_msg_handler_search(access_get_pdu_payload(access_pdu));
    LOG(3,"ptr=mesh_sig_msg_handler_search=0x%x dst_addr:%x\n", ptr, dst_addr);
    if(ptr&&ptr->hdl)
    {
        if(IS_UNICAST_ADDR(dst_addr))
        {
            mesh_elmt_t *elmt = get_elmt_by_uni_addr(dst_addr);
            BX_ASSERT(elmt);
            deliver_access_pdu_to_elmt_model(elmt,access_pdu,ptr,sig_model);
        }else if(dst_addr == MESH_ALL_NODES_ADDR)
        {
            mesh_elmt_t *primary_elmt = get_mesh_elmt();
            deliver_access_pdu_to_elmt_model(primary_elmt,access_pdu,ptr,sig_model);
        }else
        {
            uint8_t i;
            for(i=0;i<get_element_num();++i)
            {
                model_base_t *dst_model = get_model_inst_from_elmt(&(get_mesh_elmt()[i]),ptr->model_id,sig_model);
                if(dst_model && subscription_list_search(dst_model,dst_addr,access_get_pdu_virt_addr(access_pdu)))
                {
                    deliver_access_pdu_to_model_inst(dst_model,access_pdu,ptr);
                }
            }
        }
    }else
    {
        LOG(LOG_LVL_WARN,"no matching handler\n");
    }
   
}

