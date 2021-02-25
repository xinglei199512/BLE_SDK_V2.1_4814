#include "osapp_svc_manager.h"
#include "osapp_task.h"
#include "co_utils.h"
#include "osapp_config.h"
static struct
{
    struct co_list svc_helper_list;
    osapp_svc_helper_t *current;
    void (*add_svc_cb)(uint8_t,osapp_svc_helper_t *);
}svc_helper_env;

uint16_t osapp_get_att_handle_helper(osapp_svc_helper_t const*svc_helper,uint8_t att_idx)
{
    return svc_helper->start_hdl + att_idx + 1;
}

static bool handle_in_svc(osapp_svc_helper_t *helper,uint16_t handle)
{
    return handle > helper->start_hdl && handle <= helper->start_hdl + helper->att_num;
}

static osapp_svc_helper_t *osapp_get_svc_helper_ptr(uint16_t handle)
{
    if(handle_in_svc(svc_helper_env.current, handle))
    {
        return svc_helper_env.current;
    }
    struct co_list_hdr *hdr = co_list_pick(&svc_helper_env.svc_helper_list);
    while(hdr)
    {
        osapp_svc_helper_t *helper = CONTAINER_OF(hdr, osapp_svc_helper_t, hdr);
        if(handle_in_svc(helper,handle))
        {
            svc_helper_env.current = helper;
            return helper;
        }
        hdr = co_list_next(hdr);
    }
    return NULL;
}

static void osapp_read_write_req_ind_helper(void const* param,ke_task_id_t const src_id,bool read)
{
    uint16_t handle = *(uint16_t *)param;
    osapp_svc_helper_t *helper = osapp_get_svc_helper_ptr(handle);
    if(helper)
    {
        uint16_t att_idx = handle-helper->start_hdl -1 ;
        if(read)
        {
            if(helper->read)
            {
                helper->read(helper,src_id,att_idx);
            }
        }else
        {
            struct gattc_write_req_ind const* wr_param = param;
            if(helper->write)
            {
                helper->write(helper,src_id,att_idx,wr_param->offset,wr_param->length,wr_param->value);
            }
        }
    }
}

static void osapp_write_req_ind_helper_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    osapp_read_write_req_ind_helper(param,src_id,false);
}

static void osapp_read_req_ind_helper_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    osapp_read_write_req_ind_helper(param,src_id,true);
}

static void add_svc_req()
{
    osapp_svc_helper_t *helper = svc_helper_env.current;
    struct gattm_add_svc_req *req = AHI_MSG_ALLOC_DYN(GATTM_ADD_SVC_REQ,TASK_ID_GATTM,gattm_add_svc_req,sizeof(struct gattm_att_desc)*helper->att_num);
    struct gattm_svc_desc *svc = &req->svc_desc;
    memcpy(svc,helper->svc_desc,sizeof(struct gattm_svc_desc));
    memcpy(svc->atts,helper->att_desc,sizeof(struct gattm_att_desc)*helper->att_num);
    os_ahi_msg_send(req, portMAX_DELAY);
}


void osapp_add_svc_req_helper(osapp_svc_helper_t *svc_helper_array,uint8_t num,void (*callback)(uint8_t,osapp_svc_helper_t *))
{
    svc_helper_env.add_svc_cb = callback;
    uint8_t i;
    for(i=0;i<num;++i)
    {
        co_list_push_back(&svc_helper_env.svc_helper_list,&svc_helper_array[i].hdr);
    }
    struct co_list_hdr *hdr = co_list_pick(&svc_helper_env.svc_helper_list);
    svc_helper_env.current = CONTAINER_OF(hdr, osapp_svc_helper_t, hdr);
    add_svc_req();
}

void osapp_add_svc_req_helper_2(osapp_svc_helper_t **svc_helper_ptr_array,uint8_t num,void (*callback)(uint8_t,osapp_svc_helper_t *))
{
    svc_helper_env.add_svc_cb = callback;
    uint8_t i;
    for(i=0;i<num;++i)
    {
        co_list_push_back(&svc_helper_env.svc_helper_list,&svc_helper_ptr_array[i]->hdr);
    }
    struct co_list_hdr *hdr = co_list_pick(&svc_helper_env.svc_helper_list);
    svc_helper_env.current = CONTAINER_OF(hdr, osapp_svc_helper_t, hdr);
    add_svc_req();
}

static void osapp_add_svc_rsp_helper(struct gattm_add_svc_rsp const * param)
{
    if(param->status == ATT_ERR_NO_ERROR)
    {
        svc_helper_env.current->start_hdl = param->start_hdl;
    }else
    {
        svc_helper_env.add_svc_cb(param->status,svc_helper_env.current);
        return;
    }
    struct co_list_hdr *hdr = co_list_next(&svc_helper_env.current->hdr);
    if(hdr)
    {
        svc_helper_env.current = CONTAINER_OF(hdr, osapp_svc_helper_t, hdr);
        add_svc_req();
    }else
    {
        svc_helper_env.add_svc_cb(param->status,svc_helper_env.current);
    }
}

static void osapp_add_svc_rsp_helper_handler(ke_msg_id_t const msgid, struct gattm_add_svc_rsp const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    osapp_add_svc_rsp_helper(param);    
}

static void osapp_att_info_req_ind_helper_handler(ke_msg_id_t const msgid, struct gattc_att_info_req_ind const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_att_info_cfm *cfm = AHI_MSG_ALLOC(GATTC_ATT_INFO_CFM,src_id,gattc_att_info_cfm);
    cfm->handle = param->handle;
    cfm->length = 0;
    cfm->status = GAP_ERR_NO_ERROR;
    os_ahi_msg_send(cfm,portMAX_DELAY);
}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_RESET:
        co_list_init(&svc_helper_env.svc_helper_list);
        break;
    default:
        break;
    }
}


static osapp_msg_handler_table_t osapp_svc_handler_table[] =
{
    {GATTM_ADD_SVC_RSP,(osapp_msg_handler_t)osapp_add_svc_rsp_helper_handler}, 
    {GATTC_WRITE_REQ_IND,(osapp_msg_handler_t)osapp_write_req_ind_helper_handler},
    {GATTC_READ_REQ_IND,(osapp_msg_handler_t)osapp_read_req_ind_helper_handler},
    {GATTC_ATT_INFO_REQ_IND,(osapp_msg_handler_t)osapp_att_info_req_ind_helper_handler},
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},

};

static osapp_msg_handler_info_t osapp_svc_handler = HANDLER_ARRAY_INFO(osapp_svc_handler_table);


void osapp_svc_manager_init()
{
    ahi_handler_register(&osapp_svc_handler);
}


