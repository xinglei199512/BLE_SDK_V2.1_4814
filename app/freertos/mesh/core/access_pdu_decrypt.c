
#include "access_pdu_decrypt.h"
#include "candidate.h"
#include "aes_ccm_cmac.h"
#include "appkey_devkey_decrypt.h"
#include "mesh_queued_msg.h"

static upper_pdu_rx_t *input_pdu;
static uint8_t *output_buf;
static void (*access_decrypt_cb)(uint8_t,access_pdu_decrypt_callback_param_t *);
static access_pdu_decrypt_callback_param_t access_pdu_decrypt_cb_param;

static void access_pdu_decrypt_end(uint8_t status)
{
    access_decrypt_cb(status,&access_pdu_decrypt_cb_param);
}

static void appkey_devkey_common_addr_decrypt_complete(uint8_t status,app_key_t *appkey,app_key_box_t *appkey_box)
{
    access_pdu_decrypt_cb_param.cryptic.appkey = appkey;
    access_pdu_decrypt_cb_param.cryptic.appkey_box = appkey_box;
    access_pdu_decrypt_cb_param.cryptic.virt_addr = NULL;
    access_pdu_decrypt_end(status);
}

static void virt_addr_decrypt_complete(uint8_t status,app_key_t *appkey,app_key_box_t *appkey_box)
{
    if(status == AUTH_FAILED)
    {
        virt_addr_candidate_remove();
        access_pdu_virt_addr_decrypt();
    }else
    {
        access_pdu_decrypt_cb_param.cryptic.appkey = appkey;
        access_pdu_decrypt_cb_param.cryptic.appkey_box = appkey_box;
        access_pdu_decrypt_cb_param.cryptic.virt_addr = virt_addr_candidate_current_pick();
        virt_addr_candidate_remove_all();
        access_pdu_decrypt_end(status);
    }
}

static void access_decrypt_fail(void *param)
{
    access_decrypt_cb(AUTH_FAILED,&access_pdu_decrypt_cb_param);
}

void access_pdu_virt_addr_decrypt(void)
{
    virt_addr_mngt_t *virt_addr = virt_addr_candidate_current_pick();
    if(virt_addr)
    {
        appkey_decrypt_start(input_pdu,output_buf,virt_addr,virt_addr_decrypt_complete);
    }else
    {
        mesh_queued_msg_send(access_decrypt_fail,NULL);
    }
}

static void access_pdu_decrypt_pre_process()
{
    if(input_pdu->head.access.akf)
    {
        if(IS_VIRTUAL_ADDR(input_pdu->dst_addr))
        {
            virt_addr_search(input_pdu->dst_addr,virt_addr_candidate_add);
            access_pdu_virt_addr_decrypt();
        }else
        {
            appkey_decrypt_start(input_pdu,output_buf,NULL,appkey_devkey_common_addr_decrypt_complete);
        }
    }else
    {
        devkey_decrypt_start(input_pdu,output_buf,appkey_devkey_common_addr_decrypt_complete);
    }
}

void access_pdu_decrypt_start(upper_pdu_rx_t *pdu,uint8_t *rslt_buf,void (*callback)(uint8_t,access_pdu_decrypt_callback_param_t *))
{
    input_pdu = pdu;
    output_buf = rslt_buf;
    access_decrypt_cb = callback;
    access_pdu_decrypt_pre_process();
}



