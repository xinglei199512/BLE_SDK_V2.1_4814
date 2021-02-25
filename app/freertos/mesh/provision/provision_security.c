/**
 ****************************************************************************************
 *
 * @file   provision_security.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-20 17:23
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "provision_security.h"
#include "provision_comm.h"
#include "aes_ccm_cmac.h"
#include "gap.h"
#include "app_hwecc.h"
#include "app_hwecc_wrapper.h"
#include "security.h"
#include "log.h"
#include "stack_mem_cfg.h"
#include "osapp_utils.h"
#include "mesh_queued_msg.h"
#include "osapp_utils.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define PARAM_FOR_K1_DEV_KEY        "prdk"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 static     prov_confirmation_calc_t * g_confirmation_env =NULL;
 static uint8_t devkey_t[GAP_KEY_LEN];

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void provision_calc_confirmation_complete_callback(prov_confirmation_calc_t *ptr,void *dummy,uint8_t status);
static void provision_calc_confirmation_pre_process(prov_confirmation_calc_t *);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(provision_calc_confirmation_process, CALC_CONFIRMATION_QUEUE_LENGTH,
    prov_confirmation_calc_t , provision_calc_confirmation_pre_process, provision_calc_confirmation_complete_callback);


static void provision_check_confirmation_complete_callback(prov_confirmation_check_t *ptr,void *dummy,uint8_t status);
static void provision_check_confirmation_pre_process(prov_confirmation_check_t *);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(provision_check_confirmation_process, CHECK_CONFIRMATION_QUEUE_LENGTH,
    prov_confirmation_check_t , provision_check_confirmation_pre_process, provision_check_confirmation_complete_callback);

static void provision_session_key_calc_complete_callback(prov_session_key_calc_t *ptr,void *dummy,uint8_t status);
static void provision_session_key_calc_pre_process(prov_session_key_calc_t *);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(provision_session_key_calc_process, CALC_SESSION_KEY_QUEUE_LENGTH,
    prov_session_key_calc_t , provision_session_key_calc_pre_process, provision_session_key_calc_complete_callback);

static void provision_encrypt_data_complete_callback(prov_encrypt_data_t *ptr,void *dummy,uint8_t status);
static void provision_encrypt_data_pre_process(prov_encrypt_data_t *);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(provision_encrypt_data_process, ENCRYPT_DATA_QUEUE_LENGTH,
    prov_encrypt_data_t , provision_encrypt_data_pre_process, provision_encrypt_data_complete_callback);

static void provision_devkey_calc_complete_callback(prov_devkey_calc_t *ptr,void *dummy,uint8_t status);
static void provision_devkey_calc_pre_process(prov_devkey_calc_t *);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(provision_devkey_calc_process, DEVKEY_CALC_QUEUE_LENGTH,
    prov_devkey_calc_t , provision_devkey_calc_pre_process, provision_devkey_calc_complete_callback);
    
/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void provision_calc_confirmation_end(uint8_t status)
{
    g_confirmation_env->prov_callback(g_confirmation_env->dummy,status);
    queued_async_process_end(&provision_calc_confirmation_process,NULL,status);
}

static void provisioning_confirmation_calc_done(void)
{
     provision_calc_confirmation_end(0);
}

static void provisioning_confirmation_key_calc_done()
{
    LOG(LOG_LVL_INFO,"confirmation_key = ");
    log_hex_data(g_confirmation_env->prov_authentication->confirmation_key,           K1_LENGTH);

    aes_cmac(g_confirmation_env->prov_authentication->confirmation_key, g_confirmation_env->prov_authentication->msg_for_cfm.buf, sizeof(g_confirmation_env->prov_authentication->msg_for_cfm.buf),
        g_confirmation_env->prov_confirmation->local, provisioning_confirmation_calc_done);
}

static void provisioning_confirmation_salt_calc_done()
{
    LOG(LOG_LVL_INFO,"confirmation_salt = ");
    log_hex_data(g_confirmation_env->prov_random->field.confirmation_salt,           S1_LENGTH);

    k1_derivation(g_confirmation_env->prov_key->ecdh_secret.x,GAP_P256_KEY_LEN, g_confirmation_env->prov_random->field.confirmation_salt,
        PARAM_P_CONFIRMATION, sizeof(PARAM_P_CONFIRMATION)-1,g_confirmation_env->prov_authentication->confirmation_key, provisioning_confirmation_key_calc_done);
}


void calc_confirmation_salt(void)
{
    //reverse string data
    reverse_self(g_confirmation_env->prov_key->private,         GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->peer_public.x,   GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->peer_public.y,   GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->ecdh_secret.x,   GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->ecdh_secret.y,   GAP_P256_KEY_LEN);
    //log
    LOG(LOG_LVL_INFO,"ECDH = ");
    log_hex_data(g_confirmation_env->prov_key->ecdh_secret.x,           GAP_P256_KEY_LEN);
    LOG(LOG_LVL_INFO,"confirmation_inputs = ");
    log_hex_data(g_confirmation_env->prov_confirmation_input,     CONFIRMATION_INPUTS_BUF_SIZE);
    //calc next
    s1_salt_generation(g_confirmation_env->prov_confirmation_input,CONFIRMATION_INPUTS_BUF_SIZE, 
        g_confirmation_env->prov_random->field.confirmation_salt, provisioning_confirmation_salt_calc_done);
}
void ecdh_secret_calc_done(void *param)
{
    mesh_run(calc_confirmation_salt,portMAX_DELAY,true);
}


static void ecdh_secret_calc()
{
    //reverse string data
    reverse_self(g_confirmation_env->prov_key->private,         GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->peer_public.x,   GAP_P256_KEY_LEN);
    reverse_self(g_confirmation_env->prov_key->peer_public.y,   GAP_P256_KEY_LEN);
    //calc
    ecc_queue_t ecc_param = {
        .in = {
            .secret_key = g_confirmation_env->prov_key->private,
            .public_key[0] = g_confirmation_env->prov_key->peer_public.x,
            .public_key[1] = g_confirmation_env->prov_key->peer_public.y,
        },
        .out = {
            .key[0] = g_confirmation_env->prov_key->ecdh_secret.x,
            .key[1] = g_confirmation_env->prov_key->ecdh_secret.y,
        },
        .cb = ecdh_secret_calc_done,
        .dummy = NULL,
    };
    app_hwecc_calculate_wrapper(&ecc_param);
}



static void provision_calc_confirmation_complete_callback(prov_confirmation_calc_t *ptr,void *param,uint8_t status)
{
   
}

static void provision_calc_confirmation_pre_process(prov_confirmation_calc_t *ptr)
{
    g_confirmation_env = ptr;
    ecdh_secret_calc();
}

void provision_calc_confirmation_start(prov_confirmation_calc_t   * calc_info)
{
    queued_async_process_start(&provision_calc_confirmation_process,calc_info, NULL);
}


/*      check confirm                    */

static void provision_check_confirmation_end(void)
{
    queued_async_process_end(&provision_check_confirmation_process,NULL,0);
}

static void provision_check_confirmation_complete_callback(prov_confirmation_check_t *ptr,void *param,uint8_t status)
{
   ptr->check_callback(ptr->dummy,0);
}

static void provision_check_confirmation_pre_process(prov_confirmation_check_t *ptr)
{
    aes_cmac(ptr->confirm_key,ptr->msg,ptr->length,ptr->rst,provision_check_confirmation_end);
}

void provision_check_confirmation_start(prov_confirmation_check_t   * check_info)
{
    queued_async_process_start(&provision_check_confirmation_process,check_info, NULL);
}



/*      session key calc                    */


static void provision_session_key_calc_end(void)
{
    queued_async_process_end(&provision_session_key_calc_process,NULL,0);
}

static void provisioning_sessionkey_calc_done()
{
    prov_session_key_calc_t *ptr = (prov_session_key_calc_t *)queued_async_process_get_current(&provision_session_key_calc_process);
    LOG(LOG_LVL_INFO,"SessionKey = ");
    log_hex_data( ptr->session_key,           K1_LENGTH);

    k1_derivation(ptr->ecdh_secret->x, GAP_P256_KEY_LEN, ptr->provisioning_salt, 
        PARAM_P_SESSION_NONCE, sizeof(PARAM_P_SESSION_NONCE)-1, ptr->session_nonce,provision_session_key_calc_end);
}

static void provisioning_salt_calc_done()
{
    prov_session_key_calc_t *ptr = (prov_session_key_calc_t *)queued_async_process_get_current(&provision_session_key_calc_process);
    LOG(LOG_LVL_INFO,"ProvisioningSalt = ");
    log_hex_data(ptr->provisioning_salt,           S1_LENGTH);

    k1_derivation(ptr->ecdh_secret->x, GAP_P256_KEY_LEN,ptr->provisioning_salt,
        PARAM_P_SESSIONKEY,sizeof(PARAM_P_SESSIONKEY)-1, ptr->session_key, provisioning_sessionkey_calc_done);
}

void calc_session_key_nounce()
{
    prov_session_key_calc_t *ptr =(prov_session_key_calc_t *) queued_async_process_get_current(&provision_session_key_calc_process);
    s1_salt_generation(ptr->provisioning_salt_input,ptr->salt_input_length,ptr->provisioning_salt, provisioning_salt_calc_done);
}

static void provision_session_key_calc_complete_callback(prov_session_key_calc_t *ptr,void *param,uint8_t status)
{
   ptr->session_callback(ptr->dummy,0);
}

static void provision_session_key_calc_pre_process(prov_session_key_calc_t *ptr)
{
    calc_session_key_nounce();
}

void provision_session_key_calc_start(prov_session_key_calc_t   * session_info)
{
    queued_async_process_start(&provision_session_key_calc_process,session_info, NULL);
}

static  void provision_encrypt_data_encrypt_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    queued_async_process_end(&provision_encrypt_data_process,NULL,0);
}

static void provision_encrypt_data_complete_callback(prov_encrypt_data_t *ptr,void *param,uint8_t status)
{
   ptr->encrypt_callback(ptr->dummy,0);
}

static void provision_encrypt_data_pre_process(prov_encrypt_data_t *ptr)
{
    ccm_cmac_buf_t ccm_decrypt=
    {
        .param.ccm = {
            .key = ptr->key,
            .nonce = ptr->nonce,
            .msg = ptr->msg,
            .mic_length = ptr->mic_length,
            .msg_length = ptr->msg_length,
            .additional_data = NULL,
            .additional_data_length = 0,
            .rslt = ptr->result,
        },
        .op_type = CCM_ENCRYPT,
    };
    ccm_cmac_start(&ccm_decrypt,provision_encrypt_data_encrypt_complete);
}

void provision_encrypt_data_start(prov_encrypt_data_t   * encrypt_data_info)
{
    queued_async_process_start(&provision_encrypt_data_process,encrypt_data_info, NULL);
}

static void provision_devkey_calc_complete(void)
{
    queued_async_process_end(&provision_devkey_calc_process,NULL,0);
}

static void provision_devkey_calc_complete_callback(prov_devkey_calc_t *ptr,void *param,uint8_t status)
{
   ptr->devkey_callback(ptr->dummy,0);
}

static void k1_derivation_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    prov_devkey_calc_t *ptr =(prov_devkey_calc_t *) queued_async_process_get_current(&provision_devkey_calc_process);
    aes_cmac(devkey_t,PARAM_FOR_K1_DEV_KEY,4,ptr->dev_key,provision_devkey_calc_complete);
}

static void provision_devkey_calc_pre_process(prov_devkey_calc_t *ptr)
{
    ccm_cmac_buf_t ccm_decrypt=
    {
        .param.cmac = {
            .k = ptr->prov_salt,
            .m = ptr->ecdh,
            .length = 32,
            .rslt = devkey_t,
        },
        .op_type = CMAC_CALC,
    };
    ccm_cmac_start(&ccm_decrypt,k1_derivation_complete);
}

void provision_devkey_calc_start(prov_devkey_calc_t   * prov_devkey_info)
{
    queued_async_process_start(&provision_devkey_calc_process,prov_devkey_info, NULL);
}




