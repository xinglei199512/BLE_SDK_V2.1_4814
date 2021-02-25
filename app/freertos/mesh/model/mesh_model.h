#ifndef MESH_MODEL_H_
#define MESH_MODEL_H_
#include <stdint.h>
#include <stdbool.h>
#include "co_list.h"
#include "co_utils.h"
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
//#include "sdk_mesh_definitions.h"
#include "device_manager_api.h"


/*
 * ENUMERATIONS
 ****************************************************************************************
 */
enum Step_Resolution
{
    Milliseconds_100,
    Second_1,
    Seconds_10,
    Minutes_10,
};

enum MESH_BOUND_EVENT
{
    BOUND_SYS = 0x01,
    BOUND_STATE_CHANGE,
    BOUND_DELETE
};
enum MESH_BOUND_STATE
{
    GENERIC_ONOFF_STATE = 0x01,
    GENERIC_LEVEL_STATE
};
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef void (*model_publish_timeout_cb_t)(void * cookie);
typedef struct
{
    union{
        virt_addr_mngt_t *virt;
        uint16_t addr;
    }addr;
    bool is_virt;
}mesh_addr_t;
typedef struct
{
    struct co_list model_list;
    uint16_t uni_addr;
}mesh_elmt_t;

typedef struct
{
    uint8_t num_steps: 6,
        step_resolution: 2;
}publish_period_t;
typedef struct
{
    uint8_t count:3,
        interval_steps:5;
}transmit_state_t;
typedef struct
{
    mesh_addr_t addr;
    uint16_t appkey_idx : 12,
             credential_flag : 1,
             rfu:3;
    uint8_t ttl;
    publish_period_t period;
    transmit_state_t retransmit;
    model_publish_timeout_cb_t publish_period_cb;
}model_publish_state_t;

typedef struct
{
    struct co_list_hdr hdr; 
    mesh_elmt_t *elmt;
    uint32_t model_id;
    model_publish_state_t *publish;
    mesh_addr_t *subscription_list;
    app_key_t **bound_key_buf;
    uint8_t bound_key_buf_size;
    uint8_t subscription_list_size;
    bool sig_model;
}model_base_t;
typedef struct
{
    model_base_t base;
}model_server_base_t;
typedef struct
{
    model_base_t base;
}model_client_base_t;

/*
 * MACROS
 ****************************************************************************************
 */
#define IS_MESH_ADDR_STRUCT_VALID(ptr) (*(uint32_t *)&((ptr)->addr)!=0)
#define GET_SERVER_MODEL_PTR(type,ptr)   CONTAINER_OF(CONTAINER_OF((ptr),model_server_base_t,base),type,model)
#define GET_CLIENT_MODEL_PTR(type,ptr)   CONTAINER_OF(CONTAINER_OF((ptr),model_client_base_t,base),type,model)


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
extern void bind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status);

extern void unbind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status);

extern void unbind_appkey_to_all_model(dm_appkey_handle_t p_appkey_handle);

extern err_t bind_appkey_to_model_by_element(uint8_t elem_idx,uint32_t model_id,uint16_t appkey_idx);

extern uint16_t get_first_appkey_global_idx_in_model(model_base_t *model);

#endif



