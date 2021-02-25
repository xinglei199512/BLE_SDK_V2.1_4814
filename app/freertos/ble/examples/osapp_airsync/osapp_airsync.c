#define LOG_TAG        "osapp.air_sync"
#define LOG_LVL        LVL_DBG
#include "osapp_config.h"
#include "osapp_svc_manager.h"
#include <stdlib.h>
#include "osapp_utils.h"
#include "bond_manage.h"
#include "bx_log.h"
#include "epb.h"
#include "epb_mmbp.h"
#include "crc32.h"
#include "co_endian.h"
#include "md5.h"
#include "aes.h"



#define APP_ADV_CHMAP                            0x7
#define APP_ADV_INT                              32
#define AIR_SYNC_MAX_MTU                         247
#define BLE_WECHAT_MAX_DATA_LEN                  (AIR_SYNC_MAX_MTU - 3)

#define ATT_DECL_CHAR_ARRAY                      {0x03,0x28}
#define ATT_DESC_CLIENT_CHAR_CFG_ARRAY           {0x02,0x29}
#define WECHAT_SERVICE_ARRAY                     {0xe7, 0xfe}
#define WECHAT_WRITE_CHARACTERISTICS_ARRAY       {0xc7, 0xfe}
#define WECHAT_INDICATE_CHARACTERISTICS_ARRAY    {0xc8, 0xfe}
#define WECHAT_READ_CHARACTERISTICS_ARRAY        {0xc9, 0xfe}

#define CHALLENAGE_LENGTH                        4
#define CHUNK                                    20 //data frame size

#define PROTO_VERSION                            0x010004
#define AUTH_PROTO                               1
#define MAC_ADDRESS_LENGTH                       6
#define CMD_AUTH                                 1
#define CMD_INIT                                 2
#define CMD_SENDDAT                              3

#define WECHAT_SVC_ADV_NAME                      "AIRSYNC"
#define DEVICE_ID                                "wechatbleid"
#define DEVICE_TYPE                              "wechatbletype"
#define SEND_HELLO_WECHAT                        "wechat,airsync"
#define COMPANY_IDENTIFIER                       0x2233
#define DEVICE_KEY                               {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}


//#define EAM_MD5_NO_ENRYPT                        1  //auth method
//#define EAM_MD5_ENRYPT                           1
#define EAM_MAC_NO_ENCRYPT                       2


#ifdef EAM_MAC_NO_ENCRYPT
    #define AUTH_METHOD EAM_MAC_NO_ENCRYPT
    #define MD5_TYPE_AND_ID_LENGTH 0
    #define CIPHER_TEXT_LENGTH 0
#endif

#ifdef EAM_MD5_ENRYPT
    #define AUTH_METHOD EAM_MD5_ENRYPT
    #define MD5_TYPE_AND_ID_LENGTH 16
    #define CIPHER_TEXT_LENGTH 16
#endif

#ifdef EAM_MD5_NO_ENRYPT
    #define AUTH_METHOD EAM_MD5_NO_ENRYPT
    #define MD5_TYPE_AND_ID_LENGTH 16
    #define CIPHER_TEXT_LENGTH 0
#endif


enum wechat_svc_att_db_handles
{
    WECHAT_SVC_IDX_IND_CHAR,
    WECHAT_SVC_IDX_IND_VAL,
    WECHAT_SVC_IDX_IND_CFG,
    WECHAT_SVC_IDX_WT_CHAR,
    WECHAT_SVC_IDX_WT_VAL,
    WECHAT_SVC_IDX_RD_CHAR,
    WECHAT_SVC_IDX_RD_VAL,
    WECHAT_SVC_ATT_NUM,
};

typedef enum
{
    err_unpack_auth_resp = 0x9990,
    err_unpack_init_resp = 0x9991,
    err_unpack_send_data_resp = 0x9992,
    err_unpack_ctlcmd_resp = 0x9993,
    err_unpack_recv_data_push = 0x9994,
    err_unpack_switch_switch_view = 0x9995,
    err_npack_switch_background = 0x9996,
    err_unpack_err_decode = 0x9997,
}unpack_error_code_t;


typedef struct
{
    uint16_t manuf_id;
    uint8_t local_addr[6];

}adv_manuf_data_t;

typedef struct
{
    uint8_t *data;
    uint16_t len;
    uint16_t offset;
} rcv_data_info_t;

typedef struct 
{
    int cmd;
    CString send_msg;
} cmd_info_t;

typedef struct 
{
    unsigned short send_data_seq;
    unsigned short push_data_seq;
    unsigned short seq; 
}pkt_seq_t;


typedef struct
{
    unsigned char b_magic_number;
    unsigned char b_ver;
    unsigned short n_length;
    unsigned short n_cmd_id;
    unsigned short n_seq;
} bp_fix_head_t;


static rcv_data_info_t g_rcv_data = {NULL, 0, 0};
static osapp_utils_param_t local_addr;
static uint8_t conn_hdl;
static pkt_seq_t pkt_seq = {0,0,0};
static uint8_t challeange[CHALLENAGE_LENGTH] = {0x11,0x22,0x33,0x44}; 

#if defined EAM_MD5_NO_ENRYPT || EAM_MD5_ENRYPT
static uint8_t md5_type_and_id[16];
#endif

#if defined EAM_MD5_ENRYPT
static const uint8_t aes_key_init[16] = DEVICE_KEY;
static uint8_t session_key[32];
#endif

struct gattm_svc_desc const wechat_svc_desc ={
        .start_hdl = 0,
        .task_id = TASK_ID_AHI,
        .perm = PERM(SVC_MI,DISABLE)|PERM(SVC_EKS,DISABLE)|\
            PERM(SVC_AUTH,NO_AUTH)|PERM(SVC_UUID_LEN,UUID_16),PERM_VAL(SVC_SECONDARY,0),
        .nb_att = WECHAT_SVC_ATT_NUM,
        .uuid = WECHAT_SERVICE_ARRAY,
};

struct gattm_att_desc const wechat_svc_att_db[WECHAT_SVC_ATT_NUM] = {
            [WECHAT_SVC_IDX_IND_CHAR] = {
                .uuid = ATT_DECL_CHAR_ARRAY,
                .perm = PERM(RD,ENABLE),
                .max_len = 0,
                .ext_perm= PERM(UUID_LEN,UUID_16),
            },
            [WECHAT_SVC_IDX_IND_VAL] = {
                .uuid = WECHAT_INDICATE_CHARACTERISTICS_ARRAY,
                .perm = PERM(IND,ENABLE),
                .max_len = BLE_WECHAT_MAX_DATA_LEN,
                .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
            },
            [WECHAT_SVC_IDX_IND_CFG] = {
                .uuid = ATT_DESC_CLIENT_CHAR_CFG_ARRAY,
                .perm = PERM(RD,ENABLE)|PERM(WRITE_REQ,ENABLE),
                .max_len = 0,
                .ext_perm = PERM(UUID_LEN,UUID_16),
            },
            [WECHAT_SVC_IDX_WT_CHAR] = {
                .uuid = ATT_DECL_CHAR_ARRAY,
                .perm = PERM(RD,ENABLE),
                .max_len = 0,
                .ext_perm= PERM(UUID_LEN,UUID_16),
            },
            [WECHAT_SVC_IDX_WT_VAL] = {

                .uuid = WECHAT_WRITE_CHARACTERISTICS_ARRAY,
                .perm = PERM(WRITE_REQ,ENABLE)|PERM(WRITE_COMMAND,ENABLE)|PERM(WP,NO_AUTH),
                .max_len = BLE_WECHAT_MAX_DATA_LEN,
                .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
            },
            [WECHAT_SVC_IDX_RD_CHAR] = {
                .uuid = ATT_DECL_CHAR_ARRAY,
                .perm = PERM(RD,ENABLE),
                .max_len = 0,
                .ext_perm= PERM(UUID_LEN,UUID_16),
            },
            [WECHAT_SVC_IDX_RD_VAL] = {
                .uuid = WECHAT_READ_CHARACTERISTICS_ARRAY,
                .perm = PERM(RD,ENABLE)|PERM(RP,NO_AUTH),
                .max_len = BLE_WECHAT_MAX_DATA_LEN,
                .ext_perm = PERM(UUID_LEN,UUID_16)|PERM(RI,ENABLE),
            },
};

static void air_sync_read_req_ind(osapp_svc_helper_t const *,ke_task_id_t const,uint16_t);
static void air_sync_write_req_ind(osapp_svc_helper_t const *,ke_task_id_t const,uint16_t,uint16_t,uint16_t,uint8_t const*);
static void cmd_proces_2_stream(cmd_info_t *info, uint8_t **r_data, uint32_t *r_len);

osapp_svc_helper_t air_sync_svc_helper = 
{
    .svc_desc = &wechat_svc_desc,
    .att_desc = wechat_svc_att_db,
    .att_num = WECHAT_SVC_ATT_NUM,
    .read = air_sync_read_req_ind,
    .write = air_sync_write_req_ind,
};

static void air_sync_read_req_ind(osapp_svc_helper_t const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx)
{
    LOG_I("read att_idx:%d",att_idx);
}

static void osapp_send_indication(uint8_t* dat, uint16_t len)
{
    ke_task_id_t dest_id = KE_BUILD_ID(TASK_ID_GATTC, conn_hdl);
    static uint16_t notify_seq_num = 0;
    struct gattc_send_evt_cmd *cmd= AHI_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,dest_id, gattc_send_evt_cmd, len);
    cmd->operation = GATTC_INDICATE;
    cmd->seq_num = notify_seq_num++;
    cmd->handle = osapp_get_att_handle_helper(&air_sync_svc_helper,WECHAT_SVC_IDX_IND_VAL);
    cmd->length = len;
    memcpy(cmd->value, dat, cmd->length);
    os_ahi_msg_send(cmd, portMAX_DELAY);
}

static void osapp_send_indication_wrap(uint8_t* dat, uint32_t len)
{
    LOG_I("send ind wrap len = %d", len);
    for(uint8_t i = 0; i < (len / CHUNK); i++)
    {
        osapp_send_indication(dat + CHUNK * i, CHUNK);
    }
}

static uint32_t chunk_padding(uint32_t len)
{
    return ((len / CHUNK != 0) || (len < CHUNK)) ? (len / CHUNK) * CHUNK + CHUNK : len;
}

void send_auth_cmd_pack(void)
{
    uint8_t* data_pack;
    uint32_t len_pack;
    cmd_info_t cmd_type = 
    {
        .cmd = CMD_AUTH,
    };
    cmd_proces_2_stream(&cmd_type, &data_pack, &len_pack);
    osapp_send_indication_wrap(data_pack, len_pack);
    vPortFree(data_pack);
}

void send_init_cmd_pack(void)
{
    uint8_t* data_pack;
    uint32_t len_pack;
    cmd_info_t cmd_type = 
    {
        .cmd = CMD_INIT,
    };
    cmd_proces_2_stream(&cmd_type, &data_pack, &len_pack);
    osapp_send_indication_wrap(data_pack, len_pack);
    vPortFree(data_pack);
}

void send_dat_cmd_pack(const char* dat)
{
    uint8_t* data_pack;
    uint32_t len_pack;
    cmd_info_t cmd_type = 
    {
        .cmd = CMD_SENDDAT,
        .send_msg = 
        {
            .len = strlen(dat)+1,
            .str = dat,
        },
    };
    cmd_proces_2_stream(&cmd_type, &data_pack, &len_pack);
    osapp_send_indication_wrap(data_pack, len_pack);
    vPortFree(data_pack);
}

#if defined EAM_MD5_ENRYPT
static void cmd_auth_encrpyt_init(uint8_t* p_cipher_text)
{
    uint8_t deviceid[] = DEVICE_ID;
    static uint32_t seq = 0x00000001;
    uint32_t ran = 0x11223344;
    ran = co_htonl(ran);
    seq = co_htonl(seq);

    uint8_t id_len = strlen(DEVICE_ID);
    uint8_t* data = pvPortMalloc(id_len+8);
    BX_ASSERT(data);
    
    memcpy(data,deviceid,id_len);
    memcpy(data+id_len,(uint8_t*)&ran,4);
    memcpy(data+id_len+4,(uint8_t*)&seq,4);
    uint32_t crc = crc32(0, data, id_len+8);

    crc = co_htonl(crc);
    memset(data,0x00,id_len+8);
    memcpy(data,(uint8_t*)&ran,4);
    memcpy(data+4,(uint8_t*)&seq,4);
    memcpy(data+8,(uint8_t*)&crc,4);    
    AES_Init(aes_key_init);
    AES_Encrypt_PKCS7 (data, p_cipher_text, 12, aes_key_init);
    vPortFree(data);
    seq++;
}

static void decrypt_recv_ptk(uint8_t*      data_raw, uint32_t len_raw)
{
    uint32_t length = len_raw- sizeof(bp_fix_head_t);
    uint8_t *p = pvPortMalloc (length);
    BX_ASSERT(p);
    AES_Init(session_key);
    AES_Decrypt(p,data_raw+sizeof(bp_fix_head_t),len_raw- sizeof(bp_fix_head_t),session_key);

    uint8_t temp;
    temp = p[length - 1];
    len_raw = len_raw - temp;
    memcpy(data_raw + sizeof(bp_fix_head_t), p ,length -temp);
    vPortFree(p);
}

static void get_session_key(const uint8_t* data, int len)
{
    if(len)
    {
        LOG_I("get session key:");
        AES_Init(aes_key_init);
        AES_Decrypt(session_key, data, len,aes_key_init);
        for(uint8_t i = 0;i<16;i++)
        {
            LOG_RAW("0x%02x ",session_key[i]);    
        }
        LOG_RAW("\r\n");
    }
}
#endif

static void set_send_ptk_fix_head(bp_fix_head_t* p_head, uint16_t len, uint16_t cmd_id, uint16_t seq)
{
    p_head->b_magic_number = 0xfe;
    p_head->b_ver = 1;
    p_head->n_length = len;
    p_head->n_cmd_id = cmd_id;
    p_head->n_seq = seq;
}

static void cmd_auth_2_stream(uint8_t **r_data, uint32_t *r_len)
{
    base_request bas_req;
    uint32_t len_padding;
    bp_fix_head_t fix_head;

    #if defined EAM_MD5_ENRYPT
    uint8_t cipher_text[16];
    cmd_auth_encrpyt_init(cipher_text);
    auth_request auth_req = {&bas_req, true,{md5_type_and_id, MD5_TYPE_AND_ID_LENGTH}, PROTO_VERSION, AUTH_PROTO, (em_auth_method)AUTH_METHOD, true ,{cipher_text, CIPHER_TEXT_LENGTH}, false, {NULL, 0}, false, {NULL, 0}, false, {NULL, 0},true,{DEVICE_ID,sizeof(DEVICE_ID)}};
    #endif

    #if defined EAM_MAC_NO_ENCRYPT
    auth_request auth_req = {&bas_req, false,{NULL, 0}, PROTO_VERSION, AUTH_PROTO, (em_auth_method)AUTH_METHOD, false,{NULL, 0}, true, {local_addr.addr.addr.addr, MAC_ADDRESS_LENGTH}, false, {NULL, 0}, false, {NULL, 0},true,{DEVICE_ID,sizeof(DEVICE_ID)}};
    #endif

    #if defined EAM_MD5_NO_ENRYPT
    auth_request auth_req = {&bas_req, true,{md5_type_and_id, MD5_TYPE_AND_ID_LENGTH}, PROTO_VERSION, (em_auth_method)AUTH_PROTO, (em_auth_method)AUTH_METHOD, false ,{NULL, 0}, false, {NULL, 0}, false, {NULL, 0}, false, {NULL, 0},true,{DEVICE_ID,sizeof(DEVICE_ID)}};
    #endif

    *r_len = epb_auth_request_pack_size(&auth_req) + sizeof(bp_fix_head_t);
    len_padding = chunk_padding(*r_len);
    *r_data = (uint8_t *)pvPortMalloc(len_padding);
    BX_ASSERT(*r_data);
    memset(*r_data, 0, len_padding);
    if(epb_pack_auth_request(&auth_req, *r_data+sizeof(bp_fix_head_t), *r_len-sizeof(bp_fix_head_t))<0)
    {
        BX_ASSERT(0);
    }
    set_send_ptk_fix_head(&fix_head, co_htons(*r_len), co_htons(ECI_req_auth), co_htons(pkt_seq.seq));
    memcpy(*r_data, &fix_head, sizeof(bp_fix_head_t));
    *r_len = len_padding;
}

static void cmd_init_2_stream(uint8_t **r_data, uint32_t *r_len)
{
    base_request bas_req;
    uint32_t len_padding;
    bp_fix_head_t fix_head;

    init_request init_req = {&bas_req,false, {NULL, 0},true, {challeange, CHALLENAGE_LENGTH}};
    *r_len = epb_init_request_pack_size(&init_req) + sizeof(bp_fix_head_t);

    #if defined EAM_MD5_ENRYPT
    uint8_t length = *r_len;
    uint8_t *p = pvPortMalloc(AES_get_length( *r_len-sizeof(bp_fix_head_t)));
    BX_ASSERT(p);
    *r_len = AES_get_length(*r_len-sizeof(bp_fix_head_t))+sizeof(bp_fix_head_t);
    #endif

    len_padding = chunk_padding(*r_len);
    *r_data = (uint8_t *)pvPortMalloc(len_padding);
    BX_ASSERT(*r_data);
    memset(*r_data, 0, len_padding);
    if(epb_pack_init_request(&init_req, *r_data+sizeof(bp_fix_head_t), *r_len-sizeof(bp_fix_head_t))<0)
    {
        BX_ASSERT(0);
    }
    
    #if defined EAM_MD5_ENRYPT
    AES_Init(session_key);
    AES_Encrypt_PKCS7(*r_data+sizeof(bp_fix_head_t),p,length-sizeof(bp_fix_head_t),session_key);//原始数据长度
    memcpy(*r_data + sizeof(bp_fix_head_t), p, *r_len-sizeof(bp_fix_head_t));
    vPortFree(p);
    #endif

    set_send_ptk_fix_head(&fix_head, co_htons(*r_len), co_htons(ECI_req_init), co_htons(pkt_seq.seq));
    memcpy(*r_data, &fix_head, sizeof(bp_fix_head_t));
    *r_len = len_padding;
}

static void cmd_send_dat_2_stram(cmd_info_t *info, uint8_t **r_data, uint32_t *r_len)
{
    base_request bas_req;
    uint32_t len_padding;
    bp_fix_head_t fix_head;

    send_data_request send_dat_req = {&bas_req, {(uint8_t*)info->send_msg.str, info->send_msg.len}, false, (EmDeviceDataType)NULL};
    *r_len = epb_send_data_request_pack_size(&send_dat_req) + sizeof(bp_fix_head_t);
    #if defined EAM_MD5_ENRYPT
    uint16_t length = *r_len;
    uint8_t *p = pvPortMalloc(AES_get_length( *r_len-sizeof(bp_fix_head_t)));
    BX_ASSERT(p);
    *r_len = AES_get_length(*r_len-sizeof(bp_fix_head_t))+sizeof(bp_fix_head_t);
    #endif
    
    len_padding = chunk_padding(*r_len);
    *r_data = (uint8_t *)pvPortMalloc(len_padding);
    BX_ASSERT(*r_data);
    memset(*r_data, 0, len_padding);
    if(epb_pack_send_data_request(&send_dat_req, *r_data+sizeof(bp_fix_head_t), *r_len-sizeof(bp_fix_head_t))<0)
    {
        #if defined EAM_MD5_ENRYPT
        vPortFree(p);
        #endif
        
        BX_ASSERT(0);
    }
    
    #if defined EAM_MD5_ENRYPT
    //encrypt body
    AES_Init(session_key);
    AES_Encrypt_PKCS7(*r_data+sizeof(bp_fix_head_t),p,length-sizeof(bp_fix_head_t),session_key);//原始数据长度
    memcpy(*r_data + sizeof(bp_fix_head_t), p, *r_len-sizeof(bp_fix_head_t));
    vPortFree(p); 
    #endif
    
    set_send_ptk_fix_head(&fix_head, co_htons(*r_len), co_htons(ECI_req_sendData), co_htons(pkt_seq.seq));
    memcpy(*r_data, &fix_head, sizeof(bp_fix_head_t));
    *r_len = len_padding;
    pkt_seq.send_data_seq++;
}



static void parse_resp_auth_ptk(uint8_t *data_raw, uint32_t len_raw)
{
    auth_response* auth_resp;
    auth_resp = epb_unpack_auth_response(data_raw+sizeof(bp_fix_head_t),len_raw-sizeof(bp_fix_head_t));
    BX_ASSERT(auth_resp);
    BX_ASSERT(auth_resp->base_response);
    if(auth_resp->base_response->err_code != 0)
    {
        LOG_I("resp auth error code:0x%x", auth_resp->base_response->err_code);
        epb_unpack_auth_response_free(auth_resp);
        BX_ASSERT(0);
    }

    #if defined EAM_MD5_ENRYPT// get sessionkey
    get_session_key(auth_resp->aes_session_key.data, auth_resp->aes_session_key.len);
    #endif
    
    send_init_cmd_pack();
    epb_unpack_auth_response_free(auth_resp);
}

static void parse_resp_init_ptk(uint8_t *data_raw, uint32_t len_raw)
{    
    #if defined EAM_MD5_ENRYPT
    decrypt_recv_ptk(data_raw, len_raw);
    #endif

    init_response *init_resp = epb_unpack_init_response(data_raw+sizeof(bp_fix_head_t), len_raw-sizeof(bp_fix_head_t));
    BX_ASSERT(init_resp);
    BX_ASSERT(init_resp->base_response);
    BX_ASSERT(!init_resp->base_response->err_code);
    BX_ASSERT(init_resp->has_challeange_answer);
    if(crc32(0,challeange,CHALLENAGE_LENGTH) == init_resp->challeange_answer)
    {
        LOG_I("crc32 ok");
        send_dat_cmd_pack(SEND_HELLO_WECHAT);
    }
}

static void parse_resp_send_dat_ptk(uint8_t *data_raw, uint32_t len_raw)
{
    #if defined EAM_MD5_ENRYPT        
    decrypt_recv_ptk(data_raw, len_raw);
    #endif    

    send_data_response *send_data_resp;
    send_data_resp = epb_unpack_send_data_response(data_raw+sizeof(bp_fix_head_t),len_raw-sizeof(bp_fix_head_t));
    BX_ASSERT(send_data_resp);
    if(send_data_resp->base_response->err_code)
    {
        epb_unpack_send_data_response_free(send_data_resp);
        LOG_I("send dat error, error code: 0x%x", send_data_resp->base_response->err_code);
        BX_ASSERT(!send_data_resp->base_response->err_code);
    }
    epb_unpack_send_data_response_free(send_data_resp);
}

static int parse_push_recev_dat_ptk(uint8_t *data_raw, uint32_t len_raw)
{
    #if defined EAM_MD5_ENRYPT
    decrypt_recv_ptk(data_raw, len_raw);
    #endif
    
    recv_data_push *dat_push;
    dat_push = epb_unpack_recv_data_push(data_raw+sizeof(bp_fix_head_t), len_raw-sizeof(bp_fix_head_t));
    if(!dat_push)
    {
        LOG_I("unpack the dat_push err!");
        return err_unpack_recv_data_push;
    }
    LOG_I("unpack the dat_push successfully!");
    LOG_I("dat_push->data.len: 0x%x",dat_push->data.len);
    LOG_I("dat_push->data.data:");
    const uint8_t *d = dat_push->data.data;
    for(uint8_t i=0;i<dat_push->data.len;++i)
    {
        LOG_I(" 0x%x",d[i]);
    }
    if(dat_push->has_type)
    {
        LOG_I("dat_push has type!");
        LOG_I("type: %d",dat_push->type);
    }
    epb_unpack_recv_data_push_free(dat_push);
    pkt_seq.push_data_seq++;

    return 0;
}

static int parse_push_switch_view_ptk(uint8_t *data_raw, uint32_t len_raw)
{
    LOG_I("Received switch_switch_view");
    #if defined EAM_MD5_ENRYPT
    decrypt_recv_ptk(data_raw, len_raw);
    #endif
    
    switch_switch_view *switch_view;
    switch_view = epb_unpack_switch_switch_view(data_raw+sizeof(bp_fix_head_t),len_raw-sizeof(bp_fix_head_t));
    if(!switch_view)
    {
        LOG_I("unpack the switchview_push err!");
        return err_unpack_switch_switch_view;
    }
    else
    {
        LOG_I("ECI_push_switchView switch_view_op = %d", switch_view->switch_view_op);
        LOG_I("ECI_push_switchView view_id = %d", switch_view->view_id);
    }
    epb_unpack_switch_switch_view_free(switch_view);

    return 0;
}

static int parse_push_switch_backgnd_ptk(uint8_t *data_raw, uint32_t len_raw)
{
    LOG_I("Received switch_backgroud_push");
    #if defined EAM_MD5_ENRYPT
    decrypt_recv_ptk(data_raw, len_raw);
    #endif
    
    switch_backgroud_push *switch_background = epb_unpack_switch_backgroud_push(data_raw+sizeof(bp_fix_head_t),len_raw-sizeof(bp_fix_head_t));
    if(!switch_background)
    {
        LOG_I("unpack the switchbackground_push err!");
        return err_npack_switch_background;
    }    
    else
    {
        LOG_I("ECI_push_switchBackgroud switch_background_op = %d", switch_background->switch_background_op);
    }
    epb_unpack_switch_backgroud_push_free(switch_background);

    return 0;
}

static void cmd_proces_2_stream(cmd_info_t *info, uint8_t **r_data, uint32_t *r_len)
{
    pkt_seq.seq++;
    switch (info->cmd)
    {
        case CMD_AUTH:
            cmd_auth_2_stream(r_data, r_len);
        break;
        case CMD_INIT:
            cmd_init_2_stream(r_data, r_len);
        break;
        case CMD_SENDDAT:
            cmd_send_dat_2_stram(info, r_data, r_len);
        break;
        default:
            LOG_W("und cmd!!!");
        break;
    }
}


int parse_recev_pkt(uint8_t *data_raw, uint32_t len_raw)
{
    bp_fix_head_t *fix_head = (bp_fix_head_t *)data_raw;
    LOG_I("CMDID: %d", co_htons(fix_head->n_cmd_id));
    int ret = 0;
    switch(co_htons(fix_head->n_cmd_id))
    {
        case ECI_resp_auth:
            parse_resp_auth_ptk(data_raw, len_raw);
        break;
        case ECI_resp_init:
            parse_resp_init_ptk(data_raw, len_raw);
        break;
        case ECI_resp_sendData:
            parse_resp_send_dat_ptk(data_raw, len_raw);
        break;
        case ECI_push_recvData:
            ret = parse_push_recev_dat_ptk(data_raw, len_raw);
        break;
        case ECI_push_switchView:
            ret = parse_push_switch_view_ptk(data_raw, len_raw);
        break;
        case ECI_push_switchBackgroud:
            ret = parse_push_switch_backgnd_ptk(data_raw, len_raw);
        break;
        default:
            LOG_I("no response type");
        break;
    }
    vPortFree(data_raw);// free recev pack
    return ret;
}

static void recev_integrt_raw_ptk(uint16_t length,uint8_t const*value)
{
    int chunk_size = 0;
    if (g_rcv_data.len == 0) 
    {
        bp_fix_head_t *fix_head = (bp_fix_head_t *) value;
        g_rcv_data.len = co_htons(fix_head->n_length);
        g_rcv_data.offset = 0;
        g_rcv_data.data = (uint8_t *)pvPortMalloc(g_rcv_data.len );
    }
    chunk_size = g_rcv_data.len - g_rcv_data.offset;
    chunk_size = chunk_size < length ? chunk_size : length;
    memcpy(g_rcv_data.data+g_rcv_data.offset, value, chunk_size);
    g_rcv_data.offset += chunk_size;
    if (g_rcv_data.len <= g_rcv_data.offset) 
    {
        parse_recev_pkt(g_rcv_data.data, g_rcv_data.len);
        g_rcv_data.len = 0;
        g_rcv_data.offset = 0;
    }    
}

static void air_sync_write_req_ind(osapp_svc_helper_t const *svc_helper,ke_task_id_t const src_id,uint16_t att_idx,uint16_t offset,uint16_t length,uint8_t const*value)
{
//    LOG_I("write att_idx:%d",att_idx);
    struct gattc_write_cfm *cfm = AHI_MSG_ALLOC(GATTC_WRITE_CFM,src_id,gattc_write_cfm);
    cfm->status = ATT_ERR_NO_ERROR;
    cfm->handle = osapp_get_att_handle_helper(svc_helper,att_idx);
    osapp_ahi_msg_send(cfm, sizeof(struct gattc_write_cfm),portMAX_DELAY); 

    if(att_idx == WECHAT_SVC_IDX_IND_CFG)
    {
        LOG_I("master enable cccd");
        send_auth_cmd_pack();
    }
    if(att_idx == WECHAT_SVC_IDX_WT_VAL)
    {
        recev_integrt_raw_ptk(length,value);
    }
}

static void osapp_start_advertising()
{
    adv_manuf_data_t manuf_dat = 
    {
        .manuf_id = COMPANY_IDENTIFIER,
    };
    memcpy(manuf_dat.local_addr, local_addr.addr.addr.addr, 6);
    uint8_t adv_svc_uuid[2] = WECHAT_SERVICE_ARRAY;
    struct gapm_start_advertise_cmd *cmd = AHI_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,TASK_ID_GAPM, gapm_start_advertise_cmd);
    cmd->op.addr_src    = GAPM_STATIC_ADDR;
    cmd->channel_map    = APP_ADV_CHMAP;
    cmd->intv_min = APP_ADV_INT;
    cmd->intv_max = APP_ADV_INT;
    cmd->op.code        = GAPM_ADV_UNDIRECT;
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
            // Flag value is set by the GAP
    cmd->info.host.adv_data_len = ADV_DATA_PACK(cmd->info.host.adv_data,3, \
        GAP_AD_TYPE_COMPLETE_NAME,WECHAT_SVC_ADV_NAME,sizeof(WECHAT_SVC_ADV_NAME), \
        GAP_AD_TYPE_MORE_16_BIT_UUID, adv_svc_uuid, ATT_UUID_16_LEN, \
        GAP_AD_TYPE_MANU_SPECIFIC_DATA, (uint8_t*)&manuf_dat, sizeof(adv_manuf_data_t));
    cmd->info.host.scan_rsp_data_len  = 0;
    os_ahi_msg_send(cmd,portMAX_DELAY);   
}

static void add_svc_callback(uint8_t status,osapp_svc_helper_t *svc_helper)
{
    osapp_start_advertising();
}

static void osapp_gapm_cmp_evt_handler(ke_msg_id_t const msgid, struct gapm_cmp_evt const * param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_cmp_evt const *cmp_evt = param;
    switch(cmp_evt->operation)
    {
    case GAPM_SET_DEV_CONFIG:
        BX_ASSERT(cmp_evt->status==GAP_ERR_NO_ERROR);
        osapp_utils_get_params(OSAPP_UTILS_TYPE_GET_BD_ADDR,&local_addr);
        reverse_self( local_addr.addr.addr.addr, 6);
        osapp_add_svc_req_helper(&air_sync_svc_helper,1,add_svc_callback);
        break;
    case GAPM_ADV_UNDIRECT:
        LOG_I("adv status:%d",cmp_evt->status);
        break;
    default:
        LOG_W("gapm_cmp_evt operation:0x%x",cmp_evt->operation);
        break;
            
    }
}


static void osapp_gattc_cmp_evt_handler(ke_msg_id_t const msgid,struct gattc_cmp_evt const *param,ke_task_id_t const dest_id,ke_task_id_t src_id)
{
    switch(param->operation)
    {
        case GATTC_NOTIFY:
            if(param->status == ATT_ERR_NO_ERROR)
            {
                LOG_I("notification done");
            }else
            {
                LOG_I("noti status:%d",param->status);
            }
            break;
        case GATTC_INDICATE:        
            break;
        default:
            LOG_I("op:%d,seq:%d,status:%d",param->operation,param->seq_num,param->status);
            break;
    }
}



static void bond_manage_evt_cb(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param)
{
    LOG_I("bond_manage_evt_cb evt: %d",evt);

        switch(evt)
        {
            case BOND_MG_EVT_CONNECTED :
                {
                    conn_hdl = p_param->connected.conn_idx;
                    LOG_I("BOND_MG_EVT_CONNECTED conn_hdl=%d", conn_hdl);
                }
                break;
            case BOND_MG_EVT_DISCONNECTED:
                {
                    memset(&pkt_seq, 0, sizeof(pkt_seq));
                    LOG_I("disconn:%d",p_param->disconnected.reason);
                    osapp_start_advertising();
                }
                break;
            default:break;
        }
}

static void air_sync_bond_manage_init(void)
{
    bond_manage_dev_cfg_t cfg=
    {
          .evt = bond_manage_evt_cb,
          .pair_mode = BOND_MG_PAIR_MODE_NO_PAIR,
          .pairing_feat = {
              /// IO capabilities (@see gap_io_cap)
              .iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
              /// OOB information (@see gap_oob)
              .oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
              /// Authentication (@see gap_auth)
              /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
              /// and 'Secure Connections'.
              .auth = GAP_AUTH_REQ_NO_MITM_NO_BOND,
              /// Encryption key size (7 to 16)
              .key_size = 16,
              ///Initiator key distribution (@see gap_kdist)
              .ikey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY ,
              ///Responder key distribution (@see gap_kdist)
              .rkey_dist = GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY ,
              /// Device security requirements (minimum security level). (@see gap_sec_req)
              .sec_req = GAP_NO_SEC,
          },
    };
    
    ble_bond_err_t err = bond_manage_init(&cfg);
    BX_ASSERT(err==BLE_BOND_SUCCESS);

}


static osapp_msg_handler_table_t const handler_table[]=
{
    {GAPM_CMP_EVT,(osapp_msg_handler_t)osapp_gapm_cmp_evt_handler},
    {GATTC_CMP_EVT,(osapp_msg_handler_t)osapp_gattc_cmp_evt_handler},
};
osapp_msg_handler_info_t handler_info = HANDLER_ARRAY_INFO(handler_table);

int32_t air_sync_get_md5(void)
{
    int32_t error_code = 0;
    
#if defined EAM_MD5_NO_ENRYPT || EAM_MD5_ENRYPT
    char device_type[] = DEVICE_TYPE;
    char device_id[] = DEVICE_ID;
    char argv[sizeof(DEVICE_TYPE) + sizeof(DEVICE_ID) - 1];
    memcpy(argv,device_type,sizeof(DEVICE_TYPE));
    memcpy(argv + sizeof(DEVICE_TYPE)-1,device_id,sizeof(DEVICE_ID));
    error_code = md5(argv, md5_type_and_id);
    LOG_I("MD5:");
    for(uint8_t i = 0; i < 16; i++)
        LOG_RAW(" %02x", md5_type_and_id[i]);
    LOG_RAW("\r\n");
#endif

    return error_code;
}

void user_init()
{    
    osapp_utils_set_dev_mtu(AIR_SYNC_MAX_MTU);
    osapp_utils_set_dev_init(GAP_ROLE_PERIPHERAL,GAPM_CFG_ADDR_PRIVATE);
    air_sync_bond_manage_init();
    osapp_svc_manager_init();
    ahi_handler_register(&handler_info);
    air_sync_get_md5();

}

