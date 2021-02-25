#ifndef VIRT_ADDR_MNGT_H_
#define VIRT_ADDR_MNGT_H_
#include <stdint.h>
#include <stdbool.h>
#include "mesh_ble_time.h"
#include "mesh_errors.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define VIRT_ADDR_SET_BUF_SIZE                  5
#define LABEL_UUID_SIZE                         16
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    uint8_t label_uuid[LABEL_UUID_SIZE];
    uint16_t virt_addr;
}virt_addr_mngt_t;

typedef struct
{   
    ble_txrx_time_t rx_time;
    uint32_t model_id;
    uint16_t elmt_addr;
    uint16_t netkey_global_idx;
    uint16_t opcode;
    uint16_t src_addr;
    bool sig_model;
    uint16_t appkey_idx : 12,
             credential_flag : 1,
             rfu:3;
    uint8_t ttl;
    uint8_t period;
    uint8_t retransmit;
}access_pdu_param_t;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void virt_addr_search(uint16_t virt_addr,void (*candidate_add)(virt_addr_mngt_t *virt_addr));
virt_addr_mngt_t* virt_addr_add(uint8_t *label_uuid, uint8_t *status);
void virt_addr_salt_init(void);
uint8_t * virt_addr_get_salt(void);
err_t free_virt_addr_buf( virt_addr_mngt_t * ptr);

#endif
