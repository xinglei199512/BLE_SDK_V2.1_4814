#ifndef BEACON_H_
#define BEACON_H_
#include <stdint.h>
#include "sdk_mesh_config.h"
#include "adv_bearer_tx.h"
#include "model_common.h"


/*
 * DEFINES
 ****************************************************************************************
 */
#define NET_BEACON_CMAC_SIZE               (8)
#define BEACON_TYPE_LENGTH                 1
#define DEVICE_UUID_LENGTH                 MESH_DEVICE_UUID_LENGTH
#define OOB_INFO_LENGTH                    2
#define URI_HASH_LENGTH                    4
#define BEACON_IV_INDEX_LENGTH             4
#define BEACON_FLAG_LENGTH                 1
#define BEACON_SEC_PKT_LENGTH             (NET_BEACON_CMAC_SIZE + NETWORK_ID_LENGTH +  BEACON_IV_INDEX_LENGTH + BEACON_FLAG_LENGTH + BEACON_TYPE_LENGTH)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 typedef struct
{
    uint8_t dev_uuid[MESH_DEVICE_UUID_LENGTH];
    uint32_t uri_hash;
    uint16_t oob_info;
}mesh_beacon_t;

typedef struct
{
    uint8_t key_refresh        : 1; /**< Key refresh procedure initiated. */
    uint8_t iv_update          : 1; /**< IV update active. */
    uint8_t _rfu               : 6; /**< Reserved for future use. */
}__attribute((packed)) net_beacon_sec_flags_t;


typedef struct
{
    net_beacon_sec_flags_t flags;                           /**< Beacon flags. */
    uint8_t                network_id[NETWORK_ID_LENGTH];   /**< Network identifier. */
    uint32_t               iv_index;                        /**< Current IV index. */
}__attribute((packed)) net_beacon_payload_t;

/**
 * Secure Network Broadcast Beacon.
 */
typedef struct
{
    net_beacon_payload_t payload;                    /**< Payload of the secure network beacon. */
    uint8_t              cmac[NET_BEACON_CMAC_SIZE]; /**< CMAC authentication value. */
}__attribute((packed)) net_beacon_t;

typedef void(*beacon_rx_callback_t)(mesh_beacon_t *beacon_para);



/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void stop_current_beacon(void);
void unprovisioned_dev_beacon_start(mesh_beacon_t *beacon_para);
void unprovisioned_dev_beacon_restart(void);
void beacon_rx(uint8_t *data,uint8_t length);
void iv_update_beacon_send_immediately(void);
void mesh_sec_beacon_rx_pkt_inc(uint8_t *network_id);
void mesh_beacon_init(void);
bool is_sec_beacon(uint8_t *data,uint8_t length);

void regisiter_beacon_rx_callback(beacon_rx_callback_t cb);
void beacon_send_to_all(void);
void beacon_send_by_net(uint8_t *network_id);

void beacon_pdu_adv_tx_start_callback(beacon_adv_tx_t *ptr);
void beacon_adv_tx_timer_callback(mesh_timer_t xTimer);
void beacon_pdu_adv_tx_start(beacon_adv_tx_t *ptr);
void config_beacon_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void config_beacon_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void config_beacon_status_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void unprovsion_beacon_start(void);
void unprovsion_beacon_stop(void);
void beacon_start(void);
void beacon_stop(void);

#endif

