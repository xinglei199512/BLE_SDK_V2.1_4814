/**
 ****************************************************************************************
 *
 * @file   provision_comm.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:31
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_provision_comm_API Mesh provision_comm API
 * @ingroup MESH_API
 * @brief Mesh provision_comm  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_PROVISION_COMM_H_
#define APP_FREERTOS_MESH_PROVISION_PROVISION_COMM_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "security.h"
#include "gap.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define BEACON_UUID_LEN                                        16
#define INVITE_PDU_PARAMS_LEN                                  1
#define CAPABILITIES_PDU_PARAMS_LEN                            11
#define START_PDU_PARAMS_LEN                                   5
#define PUBLIC_KEY_PDU_PARAMS_LEN                              64
#define CONFIRMATION_PDU_PARAMS_LEN                            MAC_LENGTH
#define RANDOM_PDU_PARAMS_LEN                                  16
#define INVITE_PDU_OFFSET_IN_CONFIRMATION_INPUTS               0
#define CAPABILITIES_PDU_OFFSET_INCONFIRMATION_INPUTS          (INVITE_PDU_OFFSET_IN_CONFIRMATION_INPUTS + INVITE_PDU_PARAMS_LEN)
#define START_PDU_OFFSET_IN_CONFIRMATION_INPUTS                (CAPABILITIES_PDU_OFFSET_INCONFIRMATION_INPUTS + CAPABILITIES_PDU_PARAMS_LEN)
#define PROVISIONER_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS   (START_PDU_OFFSET_IN_CONFIRMATION_INPUTS + START_PDU_PARAMS_LEN)
#define DEVICE_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS        (PROVISIONER_PUBLIC_KEY_OFFSET_IN_CONFIRMATION_INPUTS + PUBLIC_KEY_PDU_PARAMS_LEN)
#define CONFIRMATION_INPUTS_BUF_SIZE                           (INVITE_PDU_PARAMS_LEN + CAPABILITIES_PDU_PARAMS_LEN + START_PDU_PARAMS_LEN + PUBLIC_KEY_PDU_PARAMS_LEN*2)
#define ENCRYPTED_PROVISIONING_DATA_LEN                        25
#define PROVISIONING_DATA_MIC_LEN                              8
#define DATA_PDU_PARAMS_LEN                                    (ENCRYPTED_PROVISIONING_DATA_LEN + PROVISIONING_DATA_MIC_LEN)
#define AUTHVALUE_LEN                                           16
#define PARAMS_OFFSET_IN_PROVISIONING_PDU                      1
#define PRIVATE_KEY_LENGTH                                     32


#define  ARRAY_TABLE_SIZE(table)  sizeof(table)/sizeof(table[0])

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum 
{
    Error_Code_RFU = 0,
    Invalid_PDU,
    Invalid_Format,
    Unexpected_PDU,
    Confirmation_Failed,
    Out_of_Resources,
    Decryption_Failed,
    Unexptected_Error,
    Cannot_Assign_Addresses,
}Provisioning_Error_Codes;

typedef enum 
{
    PROVISIOON_SUCCESS = 0,
    PROVISIOON_TIMEOUT,
    PROVISIOON_FAIL,
}Provisioning_Close_Codes;
 
typedef enum {
    Provisioning_Invite = 0,
    Provisioning_Capabilities ,
    Provisioning_Start,
    Provisioning_Public_Key,
    Provisioning_Input_Complete,
    Provisioning_Confirmation,
    Provisioning_Random,
    Provisioning_Data,
    Provisioning_Complete,
    Provisioning_Failed,
    Provisioning_PDU_Type_Max,
}Provisioning_PDU_Type;



typedef enum {
    BINARY_AUTH = 0,
    NUMERIC_AUTH = 1,
    ALPHANUMERIC_AUTH = 2,
}Auth_Data_Type;

enum Authentication_Method{
    No_OOB_Authentication = 0,
    Static_OOB_Authentication,
    Output_OOB_Authentication,
    Input_OOB_Authentication,
};

enum Public_key_Method{
    No_OOB_Publickey = 0,
    Using_OOB_Publickey,
};

enum Mesh_Algorithm_Method
{
    PROV_ALGORITHM_FIPS_P256,   
};


enum Auth_Output_Action{
    BLINK = 0,
    BEEP,
    VIBRATE,
    OUTPUT_NUMERIC,
    OUTPUT_ALPHANUMERIC,
};

enum Auth_Inputput_Action{
    PUSH = 0,
    TWIST,
    INPUT_NUMERIC,
    INPUT_ALPHANUMERIC,
};

typedef enum
{
    /*  capabilities exchange*/
    IDLE_STATUS,
    INVITE_TX_STATUS,
    INVITE_TX_CANCEL_STATUS,
    WAIT_INVITE_ACK_STATUS,
    CAPABILITIES_TX_STATUS,
    CAPABILITIES_TX_CANCEL_STATUS,
    WAIT_CAPABILITIES_ACK_STATUS,
    WAIT_CAPABILITIES_STATUS,
    WAIT_INPUT_START_STATUS,
    START_TX_STATUS,
    START_TX_CANCEL_STATUS,
    WAIT_START_ACK_STATUS,
    WAIT_START_STATUS , /* 12  */
    START_CHECK_STATUS,
    
    /*  key and auth exchange*/
    CAPABILITIES_EXCHANGE_FINISH_STATUS,
    WAIT_PEER_PUBLIC_KEY_STATUS,
    WAIT_OOB_PUBLICKEY_STATUS,
    PUBLIC_KEY_TX_STATUS,
    PUBLIC_KEY_TX_CANCEL_STATUS,
    PUBLIC_KEY_CHECK_STATUS,
    WAIT_PUBLIC_KEY_ACK_STATUS,
    WAIT_AUTH_INFO_STATUS,
    INPUT_COMPELE_TX_STATUS,
    INPUT_COMPELE_TX_CANCEL_STATUS,
    WAIT_INPUT_COMPELE_ACK_STATUS,
    WAIT_INPUT_COMPELE_STATUS,   /*23 */
    /*  auth and data decrypt*/

    WAIT_PEER_CONFIRMATION_STATUS,
    CONFIRM_CALC_STATUS,
    CONFIRMATION_TX_STATUS,
    CONFIRMATION_TX_CANCEL_STATUS,
    WAIT_CONFIRMATION_ACK_STATUS,
    WAIT_PEER_RANDOM_STATUS,
    WAIT_RANDOM_ACK_STATUS,
    CONFIRMATION_CHECK_STATUS,
    RANDOM_TX_STATUS,
    RANDOM_TX_CANCEL_STATUS,
    CALC_SESSION_STATUS,
    CALC_SESSION_CANCEL_STATUS,
    WAITING_PROVISION_DATA_STATUS,
    DECRY_PROVISION_DATA_STATUS,
    ENCRYPT_PROVISIONING_DATA_STATUS,
    ENCRYPT_PROVISIONING_CANCEL_STATUS,
    PROVISIONING_DATA_TX_STATUS,
    PROVISIONING_DATA_TX_CANCEL_STATUS,
    WAIT_PROVISIONING_DATA_ACK_STATUS,
    WAIT_PEER_COMPLETE_STATUS,
    COMPLETE_TX_STATUS,
    COMPLETE_TX_CANCEL_STATUS,
    WAIT_COMPLETE_ACK_STATUS,
    PROVISION_DONE_STATUS,
    WAIT_TX_CANCEL_STATUS,
    WAIT_ACK_CANCEL_STATUS,
    WAIT_ASYNC_CALC_STATUS,
    UNPROV_DEVKEY_CALC_STATUS,
    LINK_CLOSE_TX_STATUS,
    FAIL_PDU_TX_STATUS,
    WAIT_RX_ACK_STATUS,
    WAIT_CLOSE_LINK_STATUS,
    INVALID_STATUS,
} prov_fsm_state_t;


typedef enum
{
    Timeout,
    Tx_Normal_Cancel,
    Tx_Timeout_Cancel,
    Tx_Success,
    Tx_Fail,
    Ack_Finish,
    Link_Close_Evt,
    Link_Close_Tx_Finish,
    Provisioning_Fail_Rx,
    Start_Provision,
    Provisioning_Capablities_Rx,
    Provisioning_Invite_Rx,
    Provisioning_Start_Rx,
    Provisioning_Public_Key_Rx,  /* 11 */
    Provisioning_Input_Complete_Rx,
    Provisioning_Confirmation_Rx,
    Start_Pdu_Check_Pass,
    Start_Pdu_Check_Fail,
    Public_Key_Check_Pass,
    Public_Key_Check_Fail,
    User_Input_Complete,
    Confirmation_Calc_Done,
    Provisioning_Random_Rx,
    Confirmation_Check_Pass,
    Confirmation_Check_Fail,
    Provisioning_Data_Rx,
    Session_Key_Calc_Done,
    Decrypt_Success,
    Decrypt_Fail,
    Provision_Fail,
    Encrypt_Data_done,
    Devkey_Calc_done,
    Provisioning_Complete_Rx,
    Invalid_Evt
} prov_fsm_evt_t;



typedef struct
{
    uint8_t *data;
    uint16_t len;
}prov_pkt_in_t;

typedef struct
{
    Provisioning_Close_Codes error;
}prov_link_close_t;

typedef enum {
    Tx_Success_Finish = 0,
    Tx_Fail_Finish,
    Tx_Cancel_Finish
}Tx_Reason;

typedef struct{
    uint8_t network_key[MESH_NETWORK_KEY_LENGTH];
    uint16_t key_index;
    uint8_t flags;
    uint32_t current_iv_index;
    uint16_t unicast_addr;
}provision_data_t;


typedef struct{
    uint8_t algorithm;
    uint8_t public_key;
    uint8_t auth_method;
    uint8_t auth_action;
    uint8_t auth_size;
}provision_start_t;

typedef struct{
    uint8_t attation;
}provision_invite_t;

typedef struct
{
    uint8_t pdu[CONFIRMATION_PDU_PARAMS_LEN];
    uint8_t length;
} provision_confirmation_cache_t;


typedef struct{
    uint8_t confirmation_key[K1_LENGTH];
    union{
        uint8_t buf[RANDOM_PDU_PARAMS_LEN + AUTHVALUE_LEN];
        struct{
            uint8_t random[RANDOM_PDU_PARAMS_LEN];
            uint8_t auth_value[AUTHVALUE_LEN];
        }detail;
    }msg_for_cfm;
    uint8_t confirmation_rslt[CONFIRMATION_PDU_PARAMS_LEN];
}provision_authentication_t;

typedef union{
    uint8_t provisioning_salt_input[S1_LENGTH + 2*RANDOM_PDU_PARAMS_LEN];
    struct{
        uint8_t confirmation_salt[S1_LENGTH];
        uint8_t random_provisioner[RANDOM_PDU_PARAMS_LEN];
        uint8_t random_device[RANDOM_PDU_PARAMS_LEN];
    }field;    
}provision_random_t;

typedef struct{
    uint8_t provisioning_salt[S1_LENGTH];
    uint8_t session_key[K1_LENGTH];
    uint8_t session_nonce[K1_LENGTH];
}session_info_t;

typedef struct{
    uint16_t algorithms;
    uint16_t output_oob_action;
    uint16_t input_oob_action;
    uint8_t elements_num;
    uint8_t public_key_type;
    uint8_t static_oob_type;
    uint8_t output_oob_size;
    uint8_t input_oob_size;
}provision_capabilities_t;

typedef struct{
    uint8_t local[CONFIRMATION_PDU_PARAMS_LEN];
    uint8_t peer[CONFIRMATION_PDU_PARAMS_LEN];
}provision_confirmation_t;

typedef struct{
    uint8_t packed_data[DATA_PDU_PARAMS_LEN];
    uint8_t encrypted_data[DATA_PDU_PARAMS_LEN];
}provision_encrypted_data_t;

#define OOB_PUBLIC_KEY_MAX_CAPABILITIES 2
#define OOB_AUTH_MAX_CAPABILITIES       3


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 
typedef struct
{
    public_key_t peer_public;
    public_key_t local_public;
    uint8_t private[PRIVATE_KEY_LENGTH];
    public_key_t ecdh_secret;
}provision_key_t;

typedef struct
{
    public_key_t local_public;
    uint8_t private[PRIVATE_KEY_LENGTH];
}provisioner_key_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

uint8_t *provisioning_pdu_build(uint8_t type, uint8_t *p_length);
Auth_Data_Type provision_get_auth_data_type(uint8_t auth_method, uint8_t         auth_action);
void provision_generate_auth(uint8_t * data, uint8_t size, Auth_Data_Type data_type);
Provisioning_PDU_Type provision_pkt_analysis(uint8_t * pkt, uint16_t length);
void provision_print_evt(prov_fsm_evt_t evt);
void provision_print_state(prov_fsm_state_t state);


#endif /* APP_FREERTOS_MESH_PROVISION_PROVISION_COMM_H_ */ 
/// @} MESH_provision_comm_API

