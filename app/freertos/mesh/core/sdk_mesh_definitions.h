/**
 ****************************************************************************************
 *
 * @file   sdk_mesh_definitions.h
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-09-15 15:02
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_SDK_DEFINES Mesh SDK data struck and define definitions
 * @ingroup MESH_API
 * @brief Mesh SDK data struck and define definitions
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_CORE_SDK_MESH_DEFINITIONS_H_
#define FREERTOS_APP_MESH_CORE_SDK_MESH_DEFINITIONS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "sdk_mesh_config.h"
//#include "bearer.h"
#include "co_list.h"
#include "virt_addr_mngt.h"
#include "FreeRTOS.h"
#include "timers.h"

/*
 * MACROS
 ****************************************************************************************
 */
#define IS_GROUP_ADDR(addr)                     ((addr) & 0xc000)
#define IS_VIRTUAL_ADDR(addr)                   (((addr)&0x8000)&&((~addr)&0x4000))
#define IS_UNICAST_ADDR(addr)                   (!((addr)&0x8000)&&(addr))
#define IS_UNASSIGNED_ADDR(addr)                (!(addr))
/*
 * DEFINES
 ****************************************************************************************
 */
#define MESH_PROVISIONING_AD_TYPE               0x29
#define MESH_MESSAGE_AD_TYPE                    0x2A
#define MESH_BEACON_AD_TYPE                     0x2B
#define MESH_DEVICE_UUID_LENGTH                 16
#define MESH_KEY_LENGTH                         16
#define MESH_NETWORK_KEY_LENGTH                 MESH_KEY_LENGTH
#define NETWORK_ID_LENGTH                       8
#define MESH_SEC_BEACON_LENGTH                  22

#define PROXY_PDU_LENGTH_MAX                    66


/** Unassigned address. */
#define MESH_ADDR_UNASSIGNED                  (0x0000)
/** All-proxies fixed group address. */
#define MESH_ALL_PROXIES_ADDR                 (0xFFFC)
/** All-friends fixed group address. */
#define MESH_ALL_FRIENDS_ADDR                 (0xFFFD)
/** All-relays fixed group address. */
#define MESH_ALL_RELAYS_ADDR                  (0xFFFE)
/** All-nodes fixed group address. */
#define MESH_ALL_NODES_ADDR                   (0xFFFF)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */
enum Beacon_Type{
    Unprovisioned_Device_Beacon = 0,
    Secure_Network_Beacon = 1,
};
enum Nonce_Type{
    Network_Nonce = 0,
    Application_Nonce,
    Device_Nonce,
    Proxy_Nonce,
};
typedef enum
{
    /** Key refresh phase 0. Indicates normal device operation. */
    MESH_KEY_REFRESH_PHASE_0,
    /** Key refresh phase 1. Old keys are used for packet transmission, but new keys can be used to receive messages. */
    MESH_KEY_REFRESH_PHASE_1,
    /** Key refresh phase 2. New keys are used for packet transmission, but old keys can be used to receive messages. */
    MESH_KEY_REFRESH_PHASE_2,
    /** Key refresh phase 3. Used to complete a key refresh procedure and transition back to phase 0. */
    MESH_KEY_REFRESH_PHASE_3,
    MESH_KEY_REFRESH_PHASE_INVALID,
} mesh_key_refresh_phase_t;
//network key array index
typedef enum
{
    KEY_PRIMARY = 0,
    KEY_UPDATED = 1,
}key_box_array_idx_t;
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef uint16_t mesh_global_idx_t;//mesh global index typedef

typedef struct
{
    uint8_t privacy_key[MESH_KEY_LENGTH];
    uint8_t encryption_key[MESH_KEY_LENGTH];
    uint8_t nid;
}security_credentials_t;

typedef struct
{
    uint8_t netkey[MESH_KEY_LENGTH];
    uint8_t identity_key[MESH_KEY_LENGTH];
    uint8_t beacon_key[MESH_KEY_LENGTH];
    uint8_t network_id[NETWORK_ID_LENGTH];
    security_credentials_t master;
}net_key_box_t;
typedef struct
{
     net_key_box_t key[2];
     struct co_list friend;
     uint16_t global_idx;
     mesh_key_refresh_phase_t  key_refresh_phase;
     uint8_t  primary_used; 
}net_key_t;
/** node save device key structure */
typedef struct
{
     uint8_t netkey[2][MESH_KEY_LENGTH];
     uint16_t global_idx;
     mesh_key_refresh_phase_t  key_refresh_phase;
     uint8_t  primary_used;
}net_key_nodesave_t;

typedef struct
{
    uint8_t appkey[MESH_KEY_LENGTH];
    uint8_t aid;
}app_key_box_t;
typedef struct
{
    net_key_t *bound_netkey;
    app_key_box_t key[2];
    uint16_t global_idx;//mesh need(0-1024)
    uint8_t primary_used;
}app_key_t;
/** node save device key structure */
typedef struct
{
    app_key_t src_app_key;
    uint16_t netkey_global_idx;
    uint8_t primary_used;
}app_key_nodesave_t;

typedef struct
{
    net_key_t *pool[DEVKEY_BOUND_NETKEY_MAX_NUM];
    uint8_t update_ack[DEVKEY_BOUND_NETKEY_MAX_NUM];
}dev_key_bound_netkey_pool_t;
typedef struct
{
    dev_key_bound_netkey_pool_t bound_netkey;
    uint8_t  key[MESH_KEY_LENGTH];
    uint16_t addr;
//    bool is_used;
}dev_key_t;
/** node save device key structure */
typedef struct
{
    dev_key_t src_dev_key;
    uint16_t net_key_global_index_pool[DEVKEY_BOUND_NETKEY_MAX_NUM];
}dev_key_nodesave_t;

typedef struct{
    union{
        app_key_t *app;
        net_key_t *net;
    }key;
    uint8_t idx;//decrpt select(0-1)
}key_ptr_t;

/**
 * @brief Callback function type for Mesh network pdu decrypt.
 *
 * @param[in] p_pdu       Pointer to the network pdu database.
 * @param[in] length      The length of the decrypt data.
 * @param[in] conn_index  gatt connect index.
 *
 */
//typedef void (*network_pdu_decrypt_cb_t)(const void * p_pdu, uint16_t length,uint16_t conn_index);



/** Mesh GATT Network packet structure. */
typedef struct
{
    uint16_t connection_index; /**< Proxy connection index the packet was received from. */
} mesh_gatt_network_rx_t;

/** Mesh Scanner Network packet structure. */
typedef struct
{
    int8_t rssi; /**< RSSI value of the received packet. */
} mesh_scanner_network_rx_t;

/** Mesh decrypt Network packet structure. */
typedef struct
{
    uint16_t conn_index;
} mesh_decrypt_network_rx_t;

/** Mesh encrypt Network packet structure. */
typedef struct
{
    uint16_t conn_index;
} mesh_encrypt_network_tx_t;



//===================gatt network===========================>>>>>>>>>>>>>>>


/*
typedef  struct
{
    union{
        mesh_tx_buf_t *tx;
        network_rx_buf_t *rx;
    }buf;
    uint32_t iv_index;
    uint32_t ctl_ttl_seq_num;
    uint16_t src_addr;
    uint16_t dst_addr;
    key_ptr_t netkey;
    uint8_t network_raw_data[BEARER_BUF_SIZE];
    uint8_t ivi_nid;
    uint8_t encrypted_length;
    uint8_t mic_length;
    bool is_used;
    union{
    mesh_rx_params_t rx_params; 
    mesh_tx_params_t tx_params;
    }io_param;
}network_pdu_base_t;




struct upper_pdu_base_s;//Structure declaration
typedef struct
{

    struct upper_pdu_base_s *upper;
    mesh_timer_t segment_tx;
    uint32_t seg_acked;
    uint8_t SegN;
    uint8_t SegO;
    uint8_t retransmit_cnt;
    uint8_t cancel_current_transmit;
}segmentation_mngt_t;
typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t head:7,
                   seg:1;
    }field;
}lower_transport_header_t;
struct upper_pdu_base_s
{
    union{
        reassembly_mngt_t *reassembly;
        segmentation_mngt_t *segmentation;
    }mngt;
    uint8_t *upper;
    uint32_t seq_auth_lsb: 24,
                    ttl_for_tx: 7,
                    ctl: 1;
    uint32_t iv_index;
    key_ptr_t netkey;
    uint16_t total_length;
    uint16_t src_addr;
    uint16_t dst_addr;
    lower_transport_header_t header;
    uint8_t mic_length;
    bool is_used;
};

typedef struct
{
    upper_pdu_rx_t* uppdu;
    void * access_payload;
    access_pdu_decrypt_callback_param_t* param;
}access_pdu_base_t;


typedef upper_pdu_base_t transport_ctrl_pdu_rx_t;
typedef access_pdu_base_t access_pdu_rx_t;

*/
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


#endif /* FREERTOS_APP_MESH_CORE_SDK_MESH_DEFINITIONS_H_ */ 
/// @} MESH_SDK_DEFINES

