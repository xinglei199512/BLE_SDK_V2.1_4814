#include "osapp_config.h"
#include <stddef.h>
#include "security.h"
#include "log.h"
#include "mesh_errors.h"
#include "access_rx_process.h"
#include "mesh_model.h"
#include "sdk_mesh_definitions.h"
#include "virt_addr_mngt.h"
#include "foundation_common.h"


/*
 * DEFINES
 ****************************************************************************************
 */
#define VIRT_ADDR_SALT_PARAM "vtad"

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

typedef struct {
    model_base_t *model;
    access_pdu_param_t access_param;
    void (*callback)(model_base_t *, access_pdu_param_t *, virt_addr_mngt_t *);
}vtad_derivation_env_t;

static uint8_t virt_addr_salt[MESH_KEY_LENGTH];
static virt_addr_mngt_t virt_addr_set[VIRT_ADDR_SET_BUF_SIZE];
static virt_addr_mngt_t *virt_addr_add_pr_state;
static uint8_t vitrual_addr_cmac_buf[MESH_KEY_LENGTH];
static vtad_derivation_env_t tad_derivation_env;

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

static virt_addr_mngt_t *alloc_virt_addr_buf()
{
    uint8_t i;
    for(i=0;i<VIRT_ADDR_SET_BUF_SIZE;++i)
    {
        if(virt_addr_set[i].virt_addr == 0)
        {
            virt_addr_set[i].virt_addr = 0x8000;
            return &virt_addr_set[i];
        }
    }
    return NULL;
}

err_t free_virt_addr_buf( virt_addr_mngt_t * ptr)
{
    uint8_t i;
    for(i=0;i<VIRT_ADDR_SET_BUF_SIZE;++i)
    {
        if(&virt_addr_set[i] == ptr)
        {
            virt_addr_set[i].virt_addr = 0x0000;
            return Config_Success;
        }
    }
    return Storage_Failure;
}
 
static void virt_addr_add_complete()
{
    virt_addr_add_pr_state->virt_addr = (((uint16_t)vitrual_addr_cmac_buf[14]) << 8) | vitrual_addr_cmac_buf[15];

    virt_addr_add_pr_state->virt_addr &= 0x3fff;
    virt_addr_add_pr_state->virt_addr |= 0x8000;

    if(tad_derivation_env.callback)
        tad_derivation_env.callback(tad_derivation_env.model, &tad_derivation_env.access_param, virt_addr_add_pr_state);
}


void virt_addr_search(uint16_t virt_addr,void (*candidate_add)(virt_addr_mngt_t *virt_addr))
{
    uint8_t i;
    for(i=0;i<VIRT_ADDR_SET_BUF_SIZE;++i)
    {
        if(virt_addr_set[i].virt_addr == virt_addr)
        {
            candidate_add(&virt_addr_set[i]);
        }
    }
}


uint8_t * virt_addr_get_salt(void)
{
    return virt_addr_salt;
}

void save_virt_addr_param(model_base_t *model, access_pdu_param_t *access_param, void (*cb)(model_base_t *, access_pdu_param_t *, virt_addr_mngt_t *))
{
    tad_derivation_env.model = model;
    memcpy(&tad_derivation_env.access_param, access_param, sizeof(access_pdu_param_t));
    tad_derivation_env.callback = cb;
}

virt_addr_mngt_t* virt_addr_add(uint8_t *label_uuid, uint8_t *status)
{
    virt_addr_mngt_t *buf = alloc_virt_addr_buf();

    if(buf)
    {
        memcpy(buf->label_uuid,label_uuid,LABEL_UUID_SIZE);

        virt_addr_add_pr_state = buf;
        aes_cmac(virt_addr_salt, buf->label_uuid, 16, vitrual_addr_cmac_buf, virt_addr_add_complete);
        *status = Config_Success;
        return buf;
    }else
    {
        *status = Insufficient_Resources;
        return NULL;
    }
}

static void virt_addr_salt_gen_done()
{
#if 0
    uint8_t status;
    uint8_t salt[16] = {0xce, 0xf7, 0xfa, 0x9d, 0xc4, 0x7b, 0xaf, 0x5d, 0xaa, 0xee, 0xd1, 0x94, 0x06, 0x09, 0x4f, 0x37};
    uint8_t uuid[16] = {0x5D, 0x41, 0x40, 0x2a, 0xbc, 0x4b, 0x3a, 0x76, 0xb9, 0x71, 0x9d, 0x91, 0x10, 0x17, 0xc5, 0x92};
    virt_addr_add(uuid, &status);
#endif
    LOG(LOG_LVL_INFO,"virt_addr_salt gen done\n");
}

void virt_addr_salt_init()
{
    s1_salt_generation(VIRT_ADDR_SALT_PARAM,4,virt_addr_salt, virt_addr_salt_gen_done);
}
