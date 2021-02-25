#include <stddef.h>
#include <string.h>
#include "aes_ccm_cmac.h"
#include "queued_async_framework.h"
#include "bx_ring_queue.h"
#include "gap.h"
#include "aes_128.h"
#include "co_math.h"
#include "stack_mem_cfg.h"

static struct{
    uint8_t in[MAX_BLOCK_NUM][AES_BLOCK_SIZE];
    uint8_t out[MAX_BLOCK_NUM][AES_BLOCK_SIZE];
    uint8_t block_num;
}block_env;

static struct{
    uint8_t subkey1[GAP_KEY_LEN];
    uint8_t subkey2[GAP_KEY_LEN];
}cmac_env;
static uint8_t cmac_constant[GAP_KEY_LEN];



static void start_calc_mic(aes_128_param_t *aes_128,void *dummy,uint8_t status);
static void ccm_cmac_pre_process(ccm_cmac_buf_t *);
QUEUED_ASYNC_PROCESS_SPECIFIC_CALLBACK(ccm_cmac_async_process, CCM_CMAC_BUF_QUEUE_LENGTH,
    ccm_cmac_buf_t, ccm_cmac_pre_process);
    
static void block_xor(uint8_t *in1,uint8_t *in2,uint8_t *out,uint8_t block_size,bool in2_reverse)
{
    uint8_t i;
    for(i = 0;i<block_size;++i)
    {
        *out = *in1 ^ *in2;
        if(in2_reverse)
        {
            --in2;
        }else{
            ++in2;
        }
        ++in1;
        ++out;
    }
}

static void build_ccm_block_b(aes_ccm_param_t *param,uint8_t *plain_msg)
{
    memset(&block_env,0,sizeof(block_env));
    uint8_t a_data = param->additional_data_length ? 1 : 0;
    uint8_t m = (param->mic_length-2)/2;
    uint8_t l = MSG_LENGTH_FIELD_SIZE -1;
    block_env.in[0][0] = 64*a_data + 8*m+ l;
    memcpy(&block_env.in[0][1],param->nonce,NONCE_LENGTH);
    block_env.in[0][14] = 0;
    block_env.in[0][15] = param->msg_length;
    uint8_t blk_idx = 1;
    if(a_data)
    {
        block_env.in[1][0] = 0;
        block_env.in[1][1] = param->additional_data_length;
        uint8_t blk_limit_for_a;
        uint8_t a_data_length_in_b1;
        int8_t remained_length = param->additional_data_length - (AES_BLOCK_SIZE - ADDITIONAL_DATA_LENGTH_FIELD_SIZE);
        if(remained_length <= 0)
        {
            blk_limit_for_a = 1;
            a_data_length_in_b1 = param->additional_data_length;
        }else{
            blk_limit_for_a = remained_length/AES_BLOCK_SIZE  + 1;
            a_data_length_in_b1 = (AES_BLOCK_SIZE - ADDITIONAL_DATA_LENGTH_FIELD_SIZE);
        }
        memcpy(&block_env.in[1][2],param->additional_data,a_data_length_in_b1);
        ++blk_idx;
        uint8_t *a_data_ptr =(uint8_t *) param->additional_data + a_data_length_in_b1;
        while(blk_idx<blk_limit_for_a)
        {
            memcpy(&block_env.in[blk_idx][0],a_data_ptr,AES_BLOCK_SIZE);
            a_data_ptr += AES_BLOCK_SIZE;
            ++blk_idx;
        }
        if(remained_length > 0)
        {
            memcpy(&block_env.in[blk_idx][0],a_data_ptr,remained_length % AES_BLOCK_SIZE);
            ++blk_idx;
        }
    }
    uint8_t *msg_ptr = plain_msg;
    uint8_t blk_limit = param->msg_length /AES_BLOCK_SIZE + blk_idx;
    while(blk_idx<blk_limit)
    {
        memcpy(&block_env.in[blk_idx][0],msg_ptr,AES_BLOCK_SIZE);
        msg_ptr += AES_BLOCK_SIZE;
        ++blk_idx;
    }
    uint8_t tailing_length = param->msg_length%AES_BLOCK_SIZE;
    if(tailing_length > 0)
    {
        memcpy(&block_env.in[blk_idx][0],msg_ptr,tailing_length);
        ++blk_idx;
    }
    block_env.block_num = blk_idx;
}

static void build_ccm_block_c_prototype(aes_ccm_param_t *param)
{
    memset(block_env.in[0],0,sizeof(block_env.in[0]));
    uint8_t l = MSG_LENGTH_FIELD_SIZE -1;
    block_env.in[0][0] = l;
    memcpy(&block_env.in[0][1],param->nonce,NONCE_LENGTH);
    block_env.block_num = CEILING(param->msg_length,AES_BLOCK_SIZE) + 1;
}

static uint8_t *build_ccm_block_c(uint8_t cnt)
{
    block_env.in[0][15]=cnt;
    return &block_env.in[0][0];
}

static void msg_xor_block_s(uint8_t *msg,uint8_t msg_length,uint8_t *out)
{
    uint8_t (*msg_ptr)[AES_BLOCK_SIZE] = (void *)msg;
    uint8_t (*dst_ptr)[AES_BLOCK_SIZE] = (void *)out;
    uint8_t i;
    for(i=0;i<block_env.block_num - 2;++i)
    {
        block_xor((uint8_t *)msg_ptr[i],&block_env.out[i+1][0],(uint8_t *)dst_ptr[i],AES_BLOCK_SIZE,false);
    }
    uint8_t tailing_length = msg_length % AES_BLOCK_SIZE ? msg_length % AES_BLOCK_SIZE : AES_BLOCK_SIZE;
    block_xor((uint8_t *)msg_ptr[i],&block_env.out[i+1][0],(uint8_t *)dst_ptr[i],tailing_length,false);
}

static void mic_xor_block_s0(uint8_t *mic,uint8_t mic_length,uint8_t *out)
{
    block_xor(mic,&block_env.out[0][0],out,mic_length,false);
}

static void generate_encrypted_data(aes_ccm_param_t *param)
{
    msg_xor_block_s((uint8_t *)param->msg,param->msg_length,param->rslt);
    uint8_t *mic_ptr = param->rslt + param->msg_length;
    mic_xor_block_s0(mic_ptr,param->mic_length,mic_ptr);
    queued_async_process_end(&ccm_cmac_async_process,NULL,0);
}

static void auth_mic(aes_ccm_param_t *param,uint8_t *plain_msg)
{
    build_ccm_block_b(param,plain_msg);
    aes_128_param_t aes_128 = 
    {
            .key = param->key,
            .plain = &block_env.in[0][0],
            .encrypted = &block_env.out[0][0],
    };
    aes_128_start(&aes_128,start_calc_mic);
}

static void generate_decrypted_data(aes_ccm_param_t *param)
{
    uint8_t *msg_ptr =(uint8_t *) param->msg;
    uint8_t msg_length =param->msg_length;
    uint8_t *dst_ptr = param->rslt;
    uint8_t mic_length = param->mic_length;
    msg_xor_block_s(msg_ptr,msg_length,dst_ptr);
    mic_xor_block_s0(msg_ptr+msg_length,mic_length,dst_ptr+msg_length);
    auth_mic(param,dst_ptr);
}

static void start_calc_key_stream_blocks(aes_128_param_t *aes_base,void *dummy, uint8_t status)
{
    static uint8_t i = 0;
    ccm_cmac_buf_t *ccm_buf = queued_async_process_get_current(&ccm_cmac_async_process);
    if(i < block_env.block_num)
    {
        aes_128_param_t aes_128 = 
        {
                .key = ccm_buf->param.ccm.key,
                .plain = build_ccm_block_c(i),
                .encrypted = &block_env.out[i][0],
        };
        aes_128_start(&aes_128,start_calc_key_stream_blocks);
        ++i;
    }else{
        i = 0;
        if(ccm_buf->op_type == CCM_ENCRYPT)
        {
            generate_encrypted_data(&ccm_buf->param.ccm);
        }else if(ccm_buf->op_type == CCM_DECRYPT)
        {
            generate_decrypted_data(&ccm_buf->param.ccm);
        }else
        {
            BX_ASSERT(0);
        }
    }
}


static void calc_key_blocks(aes_ccm_param_t *param)
{
    build_ccm_block_c_prototype(param);
    start_calc_key_stream_blocks(NULL,NULL,0);
}

static void mic_calc_done(ccm_cmac_buf_t *buf)
{
    uint8_t j;
    uint8_t mic_length = buf->param.ccm.mic_length;
    uint8_t *dst = buf->param.ccm.rslt  + buf->param.ccm.msg_length;
    uint8_t *src = &block_env.out[block_env.block_num-1][0];
    switch(buf->op_type)
    {
    case CCM_ENCRYPT:
        for(j = 0;j<mic_length;++j)
        {
            *dst++= *src++;
        }
        calc_key_blocks(&buf->param.ccm);
    break;
    case CCM_DECRYPT:
        {
            uint8_t auth_rslt = DECRYPT_SUCCESSFUL;
            for(j=0;j<mic_length;++j)
            {
                if(*dst++!=*src++)
                {
                    auth_rslt = AUTH_FAILED;
                    break;
                }
            }
            queued_async_process_end(&ccm_cmac_async_process,NULL,auth_rslt);
        }
    break;
    case CMAC_CALC:
        memcpy(buf->param.cmac.rslt,src,AES_BLOCK_SIZE);
        queued_async_process_end(&ccm_cmac_async_process,NULL,0);
    break;
    default:
        BX_ASSERT(0);
    break;
    }
}

static void start_calc_mic(aes_128_param_t *aes_128,void *dummy,uint8_t status)
{
    static uint8_t i = 1;
    ccm_cmac_buf_t *ccm_buf = queued_async_process_get_current(&ccm_cmac_async_process);
    if(i<block_env.block_num)
    {
        block_xor(&block_env.in[i][0],&block_env.out[i-1][0],&block_env.in[i][0],AES_BLOCK_SIZE,false);
        uint8_t const *key = ccm_buf->op_type == CMAC_CALC ? ccm_buf->param.cmac.k : ccm_buf->param.ccm.key;
        aes_128_param_t aes_128 = 
        {
                .key = key,
                .plain = &block_env.in[i][0],
                .encrypted = &block_env.out[i][0],
        };
        aes_128_start(&aes_128,start_calc_mic);
        ++i;
    }else
    {
        i = 1;
        mic_calc_done(ccm_buf);
    }
}

static void make_const_zero()
{
    memset(cmac_constant,0,sizeof(cmac_constant));
}

static void make_const_rb()
{
    cmac_constant[GAP_KEY_LEN - 1] = 0x87;
}

static void build_cmac_block_m(aes_cmac_param_t *param)
{
    memset(&block_env,0,sizeof(block_env));
    uint8_t i;
    uint8_t const *msg = param->m;
    for(i =0;i<CEILING(param->length,AES_BLOCK_SIZE) -1;++i)
    {
        memcpy(&block_env.in[i][0],msg,AES_BLOCK_SIZE);
        msg+=AES_BLOCK_SIZE;
    }
    memcpy(&block_env.in[i][0],msg,param->m + param->length -msg);
    block_env.block_num = i + 1;
    uint8_t *dst = &block_env.in[i][0];
    uint8_t *subkey = param->length % AES_BLOCK_SIZE ? cmac_env.subkey2 : cmac_env.subkey1;
    if(param->length % AES_BLOCK_SIZE)
    {
        block_env.in[i][param->length  % AES_BLOCK_SIZE] = 0x80;
    }
    block_xor(dst,subkey,dst,AES_BLOCK_SIZE,false);
}

static uint8_t left_shift_1bit(uint8_t *data,uint8_t num_of_byte)
{
    uint8_t tmp = 0;
    uint8_t carry = 0;
    data += num_of_byte - 1;
    while(num_of_byte --)
    {
        tmp = *data;
        *data <<= 1;
        *data |=  carry;
        carry = tmp >> 7;
        data --;
    }
    return carry;
}

static void finish_subkey_generation(aes_128_param_t *aes_128,void *dummy,uint8_t status)
{
    make_const_rb();
    if(left_shift_1bit(cmac_env.subkey1,GAP_KEY_LEN))
    {
        block_xor(cmac_env.subkey1, cmac_constant, cmac_env.subkey1, GAP_KEY_LEN,false);
    }
    memcpy(cmac_env.subkey2,cmac_env.subkey1,GAP_KEY_LEN);
    if(left_shift_1bit(cmac_env.subkey2,GAP_KEY_LEN))
    {
        block_xor(cmac_env.subkey2,cmac_constant,cmac_env.subkey2,GAP_KEY_LEN,false);
    }
    ccm_cmac_buf_t *buf = queued_async_process_get_current(&ccm_cmac_async_process);
    build_cmac_block_m(&buf->param.cmac);
    aes_128_param_t aes_128_param = 
    {
            .key = buf->param.cmac.k,
            .plain = &block_env.in[0][0],
            .encrypted = &block_env.out[0][0],
    };
    aes_128_start(&aes_128_param,start_calc_mic);
}

static void generate_subkey(aes_cmac_param_t *param)
{
    make_const_zero();
    aes_128_param_t aes_128 = 
    {
            .key = param->k,
            .plain = cmac_constant,
            .encrypted = cmac_env.subkey1,
    };
    aes_128_start(&aes_128,finish_subkey_generation);
}

static void ccm_cmac_pre_process(ccm_cmac_buf_t *buf)
{
    switch(buf->op_type)
    {
    case CCM_ENCRYPT:
        auth_mic(&buf->param.ccm,(uint8_t *)buf->param.ccm.msg);
    break;
    case CCM_DECRYPT:
        calc_key_blocks(&buf->param.ccm);
    break;
    case CMAC_CALC:
        generate_subkey(&buf->param.cmac);
    break;
    default:
        BX_ASSERT(0);
    break;
    }
}

void ccm_cmac_start(ccm_cmac_buf_t *buf,void (*callback)(ccm_cmac_buf_t *,void *,uint8_t))
{
    queued_async_process_start(&ccm_cmac_async_process,buf, (queued_async_callback_t)callback);
}
