#ifndef SECURITY_H_
#define SECURITY_H_
#include <stdint.h>
#include "sdk_mesh_definitions.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define MAC_LENGTH                          16
#define S1_LENGTH                           MAC_LENGTH
#define K1_LENGTH                           MAC_LENGTH

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 //BASIC FUNCTION
void aes_cmac(uint8_t const*k,uint8_t const*m,uint8_t length,uint8_t *result,void (*cb)());

//GENERATION KEYS
void s1_salt_generation(uint8_t *m,uint8_t length,uint8_t *result, void (*cb)());
void k1_derivation(uint8_t *n,uint8_t n_len,uint8_t *salt,uint8_t *p,uint8_t p_len,uint8_t *result,void (*cb)());

void k3_derivation(uint8_t *n , uint8_t *result , void (*cb)());
void k4_derivation(uint8_t *n , uint8_t *result , void (*cb)());

//TOOLBOX KEYS
void security_init(void (*cb)());
void generte_all_net_keys(net_key_box_t * your_keys , void (*cb)());
void ecdh_prov_salt_to_devkey(uint8_t* ecdh , uint8_t* prov_salt , uint8_t* dev_key , void(*cb)());
extern void proxy_node_identity_hash_generation(const uint8_t * p_plain,void (*cb)(const uint8_t *p_hash,const uint8_t *p_random));
bool  security_is_busy(void);

#endif


