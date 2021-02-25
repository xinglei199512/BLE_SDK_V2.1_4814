#define LOG_TAG        "k2_derivation.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include "k2_derivation.h"
#include "queued_async_framework.h"
#include "aes_ccm_cmac.h"
#include "string.h"
#include "osapp_utils.h"


#define K2_DERIVATION_QUEUE_LENGTH 3
static void k2_derivation_pre_process(k2_derivation_buf_t *param);
QUEUED_ASYNC_PROCESS_SPECIFIC_CALLBACK(k2_derivation_async_process,K2_DERIVATION_QUEUE_LENGTH,k2_derivation_buf_t,k2_derivation_pre_process);

static struct{
    uint8_t key_t[GAP_KEY_LEN];
    uint8_t t1[GAP_KEY_LEN];
    uint8_t tmp[26];
}k2_env;

uint8_t salt_for_k2[GAP_KEY_LEN];


static void k2_derivation_done(ccm_cmac_buf_t *ccm,void *dummy,uint8_t status)
{
    queued_async_process_end(&k2_derivation_async_process,NULL,0);
}

static void k2_t2_generated(ccm_cmac_buf_t *ccm,void *dummy,uint8_t status)
{
    k2_derivation_buf_t *ptr = queued_async_process_get_current(&k2_derivation_async_process);
    uint8_t length = ptr->master ? 18 : 26;
    k2_env.tmp[length - 1] = 3;
    memcpy(&k2_env.tmp,ptr->rslt->encryption_key,GAP_KEY_LEN);
    ccm_cmac_buf_t buf = {
        .op_type = CMAC_CALC,
        .param.cmac = {
            .k = k2_env.key_t,
            .m = k2_env.tmp,
            .rslt = ptr->rslt->privacy_key,
            .length = length,
        },
    };
    ccm_cmac_start(&buf,k2_derivation_done);
}

static void k2_t1_generated(ccm_cmac_buf_t *ccm,void *dummy,uint8_t status)
{
    k2_derivation_buf_t *ptr = queued_async_process_get_current(&k2_derivation_async_process);
    ptr->rslt->nid = k2_env.t1[GAP_KEY_LEN-1]&0x7f;
    uint8_t length = ptr->master ? 18 : 26;
    memcpy(&k2_env.tmp,k2_env.t1,GAP_KEY_LEN);//T
    if(ptr->master)//P
    {
        k2_env.tmp[length - 2] = 0;
    }
    else
    {
        k2_env.tmp[GAP_KEY_LEN] = 0x01;//P = 0x01
        memcpy_rev(&k2_env.tmp[GAP_KEY_LEN+1] , (uint8_t *)&ptr->friend , 8);//friend
    }
    k2_env.tmp[length - 1] = 2;//tail
    ccm_cmac_buf_t buf = {
        .op_type = CMAC_CALC,
        .param.cmac = {
            .k = k2_env.key_t,
            .m = k2_env.tmp,
            .rslt = ptr->rslt->encryption_key,
            .length = length,
        },
    };
    ccm_cmac_start(&buf,k2_t2_generated);
}

static void k2_key_t_generated(ccm_cmac_buf_t *ccm,void *dummy,uint8_t status)
{
    k2_derivation_buf_t *ptr = queued_async_process_get_current(&k2_derivation_async_process);
    uint8_t length;
    if(ptr->master)
    {
        length = 2;
        k2_env.tmp[0] = 0;//P = 0x00
        k2_env.tmp[1] = 1;//tail
    }else
    {
        length = 10;
        k2_env.tmp[0] = 1;//P = 0x01
        memcpy_rev( &k2_env.tmp[1] , (uint8_t *)&ptr->friend ,8);//friend
        k2_env.tmp[9] = 1;//tail
    }
    ccm_cmac_buf_t buf = {
        .op_type = CMAC_CALC,
        .param.cmac = {
            .k = k2_env.key_t,
            .m = k2_env.tmp,
            .rslt = k2_env.t1,
            .length = length,
        },
    };
    ccm_cmac_start(&buf,k2_t1_generated);
}

static void k2_derivation_pre_process(k2_derivation_buf_t *param)
{
    memset(&k2_env,0,sizeof(k2_env));
    memset(param->rslt,0,sizeof(security_credentials_t));
    ccm_cmac_buf_t buf = {
        .op_type = CMAC_CALC,
        .param.cmac = {
            .k = salt_for_k2,
            .m = param->n,
            .rslt = k2_env.key_t,
            .length = GAP_KEY_LEN,
        },
    };
    ccm_cmac_start(&buf,k2_key_t_generated);   
}

void k2_derivation_start(k2_derivation_buf_t *buf,void (*cb)(k2_derivation_buf_t *,void *,uint8_t))
{
    LOG_D("%s",__func__);
    LOG_D("friend_cnt=0x%x,"
          "lpn_cnt=0x%x,"
          "friend_addr=0x%x,"
          "lpn_addr=0x%x",
          buf->friend.friend_cnt,
          buf->friend.lpn_cnt,
          buf->friend.friend_addr,
          buf->friend.lpn_addr );

    queued_async_process_start(&k2_derivation_async_process,buf,(queued_async_callback_t)cb);
}




