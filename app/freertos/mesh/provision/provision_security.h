/**
 ****************************************************************************************
 *
 * @file   provision_security.h
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

/**
 ****************************************************************************************
 * @addtogroup MESH_provision_security_API Mesh provision_security API
 * @ingroup MESH_API
 * @brief Mesh provision_security  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_PROVISION_SECURITY_H_
#define APP_FREERTOS_MESH_PROVISION_PROVISION_SECURITY_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "provision_comm.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define PARAM_P_CONFIRMATION    "prck"
#define PARAM_P_SESSIONKEY      "prsk"
#define PARAM_P_SESSION_NONCE   "prsn"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef void (*prov_calc_confirmation_callback)(void* dummy, uint8_t result);

typedef struct
{
    provision_key_t   *prov_key;
    provision_authentication_t  *prov_authentication;
    provision_confirmation_t    *prov_confirmation;
    provision_random_t          *prov_random;
    uint8_t * prov_confirmation_input;
    prov_calc_confirmation_callback prov_callback;
    void *  dummy;
}prov_confirmation_calc_t;

typedef void (*prov_check_confirmation_callback)(void* dummy, uint8_t result);

typedef struct
{
    uint8_t *confirm_key;
    uint8_t *msg;
    uint8_t length;
    uint8_t *rst;
    prov_check_confirmation_callback check_callback;
    void *  dummy;
}prov_confirmation_check_t;

typedef void (*prov_session_key_calc_callback)(void* dummy, uint8_t result);

typedef struct
{
    uint8_t *provisioning_salt_input;
    uint8_t  salt_input_length;
    uint8_t *provisioning_salt;
    public_key_t *ecdh_secret;
    uint8_t *session_key;
    uint8_t *session_nonce;
    prov_session_key_calc_callback session_callback;
    void *  dummy;
}prov_session_key_calc_t;


typedef void (*prov_encrypt_data_callback)(void* dummy, uint8_t result);

typedef struct
{
    uint8_t *key;
    uint8_t *nonce;
    uint8_t *result;
    uint8_t *msg;
    uint8_t  msg_length;
    uint8_t  mic_length;
    prov_encrypt_data_callback encrypt_callback;
    void *  dummy;
}prov_encrypt_data_t;


typedef void (*prov_devkey_calc_callback)(void* dummy, uint8_t result);

typedef struct
{
    uint8_t *ecdh;
    uint8_t *prov_salt;
    uint8_t *dev_key;
    prov_devkey_calc_callback devkey_callback;
    void *  dummy;
}prov_devkey_calc_t;

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void provision_check_confirmation_start(prov_confirmation_check_t   * check_info);
void provision_session_key_calc_start(prov_session_key_calc_t   * session_info);
void provision_calc_confirmation_start(prov_confirmation_calc_t   * calc_info);
void provision_encrypt_data_start(prov_encrypt_data_t   * encrypt_data_info);
void provision_devkey_calc_start(prov_devkey_calc_t   * prov_devkey_info);
#endif /* APP_FREERTOS_MESH_PROVISION_PROVISION_SECURITY_H_ */ 
/// @} MESH_provision_security_API

