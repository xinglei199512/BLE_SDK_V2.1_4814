#include "osapp_config.h"
#include <stdint.h>
#include <string.h>
//#include "osapp_mesh.h"
#include "security.h"
#include "bx_ring_queue.h"
#include "co_math.h"
#include "log.h"
#include "proxy_s.h"
#include "osapp_utils.h"
#include "aes_128.h"
#include "aes_ccm_cmac.h"
#include "k2_derivation.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define PARAM_FOR_K2_SALT           "smk2"
#define PARAM_FOR_K3_SALT           "smk3"
#define PARAM_FOR_K3_P              "id64"
#define PARAM_FOR_K4_SALT           "smk4"
#define PARAM_FOR_K4_P              "id6"
#define PARAM_FOR_K1_DEV_KEY        "prdk"
#define PARAM_IDENTITY_KEY_SALT     "nkik"
#define PARAM_BEACON_KEY_SALT       "nkbk"
#define PARAM_K1_P                  "id128"
#define PARAM_K3_P                  "id64"
/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct{
    uint8_t key_t[GAP_KEY_LEN];
    void (*cb)();
    uint8_t *result;
    uint8_t *p;
    uint8_t p_len;
}k1_derivation_env;
typedef struct{
    uint8_t *n;
    uint8_t t[GAP_KEY_LEN];
    uint8_t p[5];
    void (*cb)();
    uint8_t salt[GAP_KEY_LEN];
    uint8_t res[GAP_KEY_LEN];
    uint8_t *result;
}k3_derivation_env_t;
typedef struct{
    uint8_t *n;
    uint8_t t[GAP_KEY_LEN];
    uint8_t p[4];
    void (*cb)();
    uint8_t salt[GAP_KEY_LEN];
    uint8_t res[GAP_KEY_LEN];
    uint8_t *result;
}k4_derivation_env_t;

//netkey derivation to all keys
typedef struct
{
    net_key_box_t keys;
    net_key_box_t *goal;
    void (*callback)();
    uint8_t salts[GAP_KEY_LEN];
    uint8_t identity_key_salt_str[4];
    uint8_t beacon_key_salt_str[4];
    uint8_t k1_p_value[6];
    uint8_t k3_p_value[5];
} generate_net_keys_t;
//gatt  proxy_node_identity_hash_generation -> hash 
struct{
    proxy_node_identity_hash_t in;
    uint8_t out[PROXY_NODE_IDENTITY_HASH_AES_ECB_LEN];
    void (*cb)(const uint8_t *p_hash,const uint8_t *p_random);
}l_node_identity_hash;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static uint8_t const s1_key[GAP_KEY_LEN] = {0,};
static k3_derivation_env_t k3_derivation_env;
static k4_derivation_env_t k4_derivation_env;
//netkey derivation to all keys
static generate_net_keys_t net_keys;
//init call back
static void (*security_init_cb)(void) = NULL;
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

bool  security_is_busy(void)
{
    return false;
}


void aes_cmac(uint8_t const*k,uint8_t const*m,uint8_t length,uint8_t *result,void (*cb)())
{
    ccm_cmac_buf_t ccm_encrypt=
    {
        .param.cmac = {
            .k = k,
            .m = m,
            .length = length,
            .rslt = result,
        },
        .op_type = CMAC_CALC,
    };
    ccm_cmac_start(&ccm_encrypt,(void (*)(ccm_cmac_buf_t *,void *,uint8_t))cb);
}


void s1_salt_generation(uint8_t *m,uint8_t length,uint8_t *result, void (*cb)())
{
    aes_cmac(s1_key, m, length, result, cb);
}

static void k1_derivation_complete()
{
    aes_cmac(k1_derivation_env.key_t,k1_derivation_env.p,k1_derivation_env.p_len,k1_derivation_env.result,k1_derivation_env.cb);
}

void k1_derivation(uint8_t *n,uint8_t n_len,uint8_t *salt,uint8_t *p,uint8_t p_len,uint8_t *result,void (*cb)())
{
    memset(&k1_derivation_env,0,sizeof(k1_derivation_env));
    k1_derivation_env.cb = cb;
    k1_derivation_env.result = result;
    k1_derivation_env.p = p;
    k1_derivation_env.p_len = p_len;
    aes_cmac(salt,n,n_len,k1_derivation_env.key_t,k1_derivation_complete);
}


static void salt_for_k2_generated()
{
    LOG(LOG_LVL_INFO,"salt for k2 generated\n");
    if(security_init_cb)
    {
        security_init_cb();
        security_init_cb = NULL;
    }
}

static void salt_for_k2_init()
{
    s1_salt_generation(PARAM_FOR_K2_SALT,sizeof(PARAM_FOR_K2_SALT)-1, salt_for_k2,salt_for_k2_generated);
}

/*************K3*******************/
void k3_gen_done(void)
{
    memcpy(k3_derivation_env.result , k3_derivation_env.res + 8, 8);
    k3_derivation_env.cb();
}
void k3_t_gen_done(void)
{
    aes_cmac(k3_derivation_env.t , k3_derivation_env.p , 5 , k3_derivation_env.res , k3_gen_done);
}
void k3_gen_salt_done(void)
{
    aes_cmac(k3_derivation_env.salt , k3_derivation_env.n , GAP_KEY_LEN , k3_derivation_env.t , k3_t_gen_done);
}
void k3_derivation(uint8_t *n , uint8_t *result , void (*cb)())
{
    //fill msg
    memcpy(k3_derivation_env.p,PARAM_FOR_K3_P,4);
    k3_derivation_env.p[4] = 0x01;
    k3_derivation_env.n = n;
    k3_derivation_env.cb = cb;
    k3_derivation_env.result =result;
    s1_salt_generation(PARAM_FOR_K3_SALT , 4 , k3_derivation_env.salt , k3_gen_salt_done);
}
/*************K3*******************/


/*************K4*******************/
void k4_gen_done(void)
{
    *k4_derivation_env.result = k4_derivation_env.res[15] & 0x3F;
    k4_derivation_env.cb();
}
void k4_t_gen_done(void)
{
    aes_cmac(k4_derivation_env.t , k4_derivation_env.p , 4 , k4_derivation_env.res , k4_gen_done);
}
void k4_gen_salt_done(void)
{
    aes_cmac(k4_derivation_env.salt , k4_derivation_env.n , GAP_KEY_LEN , k4_derivation_env.t , k4_t_gen_done);
}
void k4_derivation(uint8_t *n , uint8_t *result , void (*cb)())
{
    //fill msg
    memcpy(k4_derivation_env.p,PARAM_FOR_K4_P,3);
    k4_derivation_env.p[3] = 0x01;
    k4_derivation_env.n = n;
    k4_derivation_env.cb = cb;
    k4_derivation_env.result =result;
    s1_salt_generation(PARAM_FOR_K4_SALT , 4 , k4_derivation_env.salt , k4_gen_salt_done);
}
/*************K4*******************/



/*********netkey derivation to all keys ***********/
void all_keys_done(void)
{
    *net_keys.goal = net_keys.keys;
    net_keys.callback();
}
void nid_enc_pri_key_cb(k2_derivation_buf_t *param,void *dummy,uint8_t status)
{
    //network id
    k3_derivation(net_keys.keys.netkey , net_keys.keys.network_id , all_keys_done);
}
void beacon_key_cb(void)
{
    //nid enc pri key 
    k2_derivation_buf_t buf;
    buf.n = net_keys.keys.netkey;
    buf.rslt = &net_keys.keys.master;
    buf.master = true;
    k2_derivation_start(&buf , nid_enc_pri_key_cb);
}
void beacon_key_salt_cb(void)
{
    //generate beacon key (k1)
    k1_derivation(net_keys.keys.netkey,MESH_KEY_LENGTH,net_keys.salts,net_keys.k1_p_value,6,net_keys.keys.beacon_key,beacon_key_cb);
}
void identity_key_cb(void)
{
    //generate salt(s1) for beacon key
    s1_salt_generation(net_keys.beacon_key_salt_str,4,net_keys.salts,beacon_key_salt_cb);
}
void identity_key_salt_cb(void)
{
    //generate identity key (k1)
    k1_derivation(net_keys.keys.netkey,MESH_KEY_LENGTH,net_keys.salts,net_keys.k1_p_value,6,net_keys.keys.identity_key,identity_key_cb);
}
void net_keys_init(void)
{
    memset(&net_keys , 0 , sizeof(net_keys));
    memcpy(net_keys.identity_key_salt_str , PARAM_IDENTITY_KEY_SALT , 4);
    memcpy(net_keys.beacon_key_salt_str   , PARAM_BEACON_KEY_SALT , 4);
    memcpy(net_keys.k1_p_value , PARAM_K1_P , 5);
    memcpy(net_keys.k3_p_value , PARAM_K3_P , 4);
    net_keys.k1_p_value [5] = 0x01;
    net_keys.k3_p_value [4] = 0x01;
}

void generte_all_net_keys(net_key_box_t * your_keys , void (*cb)())
{
    net_keys_init();
    net_keys.goal = your_keys;
    net_keys.callback = cb;
    //netkey copy to local
    memcpy(net_keys.keys.netkey , net_keys.goal->netkey , MESH_KEY_LENGTH);
    //generate salt(s1) for identity key
    s1_salt_generation(net_keys.identity_key_salt_str,4,net_keys.salts,identity_key_salt_cb);
}
/*********netkey derivation to all keys ***********/




/*********ECDHSecret + ProvisioningSalt  ->  Device Key ***********/
void ecdh_prov_salt_to_devkey(uint8_t* ecdh , uint8_t* prov_salt , uint8_t* dev_key , void(*cb)())
{
    k1_derivation(ecdh , 32 , prov_salt , PARAM_FOR_K1_DEV_KEY , 4 , dev_key , cb);
}

//=========gatt  proxy_node_identity_hash_generation -> hash ======<<<<<<<<<
static void proxy_node_identity_hash_generation_done(void)
{
    LOG(LOG_LVL_INFO,"proxy_node_identity_hash_generation_done \n");
    if(l_node_identity_hash.cb != 0)
    {
        l_node_identity_hash.cb(&l_node_identity_hash.out[8],&l_node_identity_hash.in.metadata.params.random[0]);
    }
}
void proxy_node_identity_hash_generation(const uint8_t * p_plain,void (*cb)(const uint8_t *p_hash,const uint8_t *p_random))
{
    //1. save data
    memcpy((proxy_node_identity_hash_t *)&l_node_identity_hash.in,(proxy_node_identity_hash_t *)p_plain,sizeof(proxy_node_identity_hash_t));
    //2. save cb
    l_node_identity_hash.cb = cb;
    //3. encrypt
    aes_128_param_t aes_128 = 
    {
            .key = l_node_identity_hash.in.p_identiykey,
            .plain = &l_node_identity_hash.in.metadata.p_data[0],
            .encrypted = &l_node_identity_hash.out[0],
    };
    aes_128_start(&aes_128,(void (*)(aes_128_param_t *,void *,uint8_t))proxy_node_identity_hash_generation_done);

}
//=========gatt  proxy_node_identity_hash_generation -> hash ======>>>>>>>>>>


void security_init(void (*cb)())
{
    security_init_cb = cb;
    salt_for_k2_init();
    virt_addr_salt_init();
}

