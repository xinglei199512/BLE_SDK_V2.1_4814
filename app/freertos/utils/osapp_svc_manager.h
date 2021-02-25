#ifndef OSAPP_SVC_MANAGER_H_
#define OSAPP_SVC_MANAGER_H_
#include <stdint.h>
#include "osapp_task.h"

typedef struct osapp_svc_helper_s
{
    struct co_list_hdr hdr;
    struct gattm_svc_desc const *svc_desc;
    struct gattm_att_desc const *att_desc;
    void (*read)(struct osapp_svc_helper_s const *,ke_task_id_t const,uint16_t);
    void (*write)(struct osapp_svc_helper_s const *,ke_task_id_t const,const uint16_t,const uint16_t,const uint16_t,uint8_t const*);
    uint16_t start_hdl;
    uint8_t att_num;
}osapp_svc_helper_t;

uint16_t osapp_get_att_handle_helper(osapp_svc_helper_t const *svc_helper,uint8_t att_idx);

void osapp_add_svc_req_helper(osapp_svc_helper_t *svc_helper_array,uint8_t num,void (*callback)(uint8_t,osapp_svc_helper_t *));

void osapp_add_svc_req_helper_2(osapp_svc_helper_t **svc_helper_ptr_array,uint8_t num,void (*callback)(uint8_t,osapp_svc_helper_t *));

void osapp_svc_manager_init(void);

#endif
