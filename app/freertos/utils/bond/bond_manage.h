/**
 ****************************************************************************************
 *
 * @file   bond_manage.h
 *
 * @brief  ble bond manage apis.
 *
 * @author  Hui Chen
 * @date    2018-12-24 10:09
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
 * @addtogroup BLE_BOND_MANAGE_API
 * @ingroup BLE_API
 * @brief BLE bond_manage  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_BLE_BOND_MANAGE_H_
#define APP_FREERTOS_BLE_BOND_MANAGE_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "bx_config.h"
#include "ble_bond_errors.h"
#include "gapm_task.h"
#include "gap.h"
#include "gapc_task.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bond_save.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

#ifndef BOND_MG_MAX_CONNECT_NUM
#define BOND_MG_MAX_CONNECT_NUM             BLE_USER_CONN_NUM  //Number of simultaneous links
#endif /*BOND_MG_MAX_CONNECT_NUM*/

#define BOND_MG_INVALID_ID                  BOND_NODE_UNALLOC_ID

//'000000'~'999999'
#define BOND_MG_APP_KEY_LEN                 6
#define BOND_MG_INVALID_PASSWORD            0xffffff
/*
 * ENUMERATIONS
 ****************************************************************************************
 */
//pair result
enum
{
    /// Pairing Failed information
    BOND_MG_PAIRING_FAILED = 0x00,
    /// Pairing Finished information
    BOND_MG_PAIRING_SUCCEED = 0x01,
};
//bond result
enum
{
    /// no bond  information
    BOND_MG_NO_BOND = 0x00,
    /// have bond information
    BOND_MG_BONDED = 0x01,
};
//bond  Numeric Comparison password
enum
{
    /// Numeric Comparison password is wrong
    BOND_MG_ACTION_NC_WRONG = 0x00,
    /// Numeric Comparison password is right
    BOND_MG_ACTION_NC_RIGHT = 0x01,
};
//resolve rpa result
enum
{
    /// resolve private address resolve failed
    BOND_MG_RPA_RESOLVE_FAILED = 0x00,
    /// resolve private address resolve success
    BOND_MG_RPA_RESOLVE_SUCCEED = 0x01,
};
/// bond manage pair mode config
typedef enum
{
    /// Pairing is not allowed.
    BOND_MG_PAIR_MODE_NO_PAIR = 0x00,
    /// Wait for a pairing request or slave security request,not auto send.
    BOND_MG_PAIR_MODE_WAIT_FOR_REQ = 0x01,
    /// When the link connected, don't wait, the bond manage auto initiate a pairing request or slave security request.
    BOND_MG_PAIR_MODE_INITIATE = 0x02,
}bond_manage_pair_mode_t;

/// bond manage event type
typedef enum
{
    //flash recover complete,ble connected.
    BOND_MG_EVT_CONNECTED     = 0,
    // display password num
    BOND_MG_EVT_DISPLAY_PW,
    // Numeric Comparison password//dispaly and input
    BOND_MG_EVT_NC_PW,
    // input password num
    BOND_MG_EVT_INPUT_PW,
    //Legacy oob info
    BOND_MG_EVT_LEGACY_OOB,
    //pairing result
    BOND_MG_EVT_PAIR_RESULT,
    //encrypt result
    BOND_MG_EVT_ENCRYPT_RESULT,

    BOND_MG_EVT_SECURITY_REQ,

    BOND_MG_EVT_DISCONNECTED,
}bond_manage_evt_t;
/// bond manage event type
typedef enum
{
    //input password num
    BOND_MG_ACTION_KEY_INPUT_PW = 0,
    //oob info
    BOND_MG_ACTION_OOB_PW,
    //set static password
    BOND_MG_ACTION_STATIC_PW,
    // Numeric Comparison password
    BOND_MG_ACTION_NC_PW,
}bond_manage_action_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/* @brief  bond manage event typedef */
///bond manage event connected struct
typedef struct
{
    ///bond evt result
    bool            result;//bond result
    bond_node_id_t  bond_id;
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_connected_t;
typedef struct
{
    uint8_t reason;
    bond_node_id_t bond_id;
    uint8_t conn_idx;
}bond_mg_evt_param_disconnected_t;
///bond manage event display_pw struct
typedef struct
{
    uint32_t        key;//display password
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_display_pw_t;
///bond manage event  Numeric Comparison password struct
typedef struct
{
    uint32_t        key;//Numeric Comparison password
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_nc_pw_t;
///bond manage event input_pw struct
typedef struct
{
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_input_pw_t;
///bond manage event pair result struct
typedef struct
{
    uint8_t            success;
    union{
        uint8_t auth;
        uint8_t reason;
    }u;
    bond_node_id_t  bond_id;
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_pair_result_t;
///bond manage event encrypt result struct
typedef struct
{
    uint8_t         auth_level;///// Authentication Requirements @ref enum gap_auth
    bond_node_id_t  bond_id;
    uint8_t         conn_idx;//connect index
}bond_mg_evt_param_encrypt_result_t;
///bond manage event pair result struct
typedef struct
{
    /// Key value MSB -> LSB
    uint8_t     key[GAP_KEY_LEN];
    uint8_t     conn_idx;//connect index
}bond_mg_evt_param_legacy_oob_t;

typedef struct
{
    struct gapc_pairing *p_pairing_feat;
    uint8_t *p_accept;
    uint8_t conn_idx;
    enum gap_role role;
}bond_mg_evt_param_security_req_t;

///bond manage event param struct
typedef union
{
    bond_mg_evt_param_connected_t       connected;
    bond_mg_evt_param_disconnected_t disconnected;
    bond_mg_evt_param_security_req_t    security_req;
    bond_mg_evt_param_display_pw_t      display_pw;
    bond_mg_evt_param_nc_pw_t           nc_pw;
    bond_mg_evt_param_input_pw_t        input_pw;
    bond_mg_evt_param_pair_result_t     pair_result;
    bond_mg_evt_param_encrypt_result_t  encrypt_result;
    bond_mg_evt_param_legacy_oob_t      legacy_oob;
}bond_manage_evt_param_t;

/* @brief  bond manage action typedef */
///bond manage event param struct
typedef struct
{
    uint32_t    key;//input key
    uint8_t     conn_idx;//connect index
}bond_mg_action_key_input_pw_t;
///bond manage event param struct
typedef struct
{
    bool        result;//
    uint8_t     conn_idx;//connect index
}bond_mg_action_nc_pw_t;
///bond manage action param struct
typedef union
{
    bond_mg_action_key_input_pw_t   input_key;
    bond_mg_action_nc_pw_t          nc_right;
    uint32_t                        static_password;
}bond_manage_action_param_t;

/* @brief bond manage function callback typedef */
/**
 ****************************************************************************************
 * @brief   Callback function to fill irk list of random resolve private address.
 *
 * @param[in]  irk          The pointer of irk list.
 *
 * @return    None.
 ****************************************************************************************
 */
typedef void (*fill_rpa_irk_list_cb_t)(struct gap_sec_key *irk);
/**
 ****************************************************************************************
 * @brief   Callback function to fill irk list of random resolve private address.
 *
 * @param[out] status       The result of resolve random resolve private address.
 *                          BOND_MG_PAIRING_FAILED = 0x00, Pairing Failed information
 *                          BOND_MG_PAIRING_SUCCEED = 0x01, Pairing Finished information
 * @param[out]  param        The pointer of the param of the result.
 *
 * @return    None.
 ****************************************************************************************
 */
typedef void (*reslove_result_cb_t)(bool status,struct gapm_addr_solved_ind const *param);
/**
 ****************************************************************************************
 * @brief   Callback function to send the event of bond manage
 *
 * @param[out] evt          bond manage event type.
 * @param[out] p_param      The pointer of bond manage event param.
 *
 * @return    None.
 ****************************************************************************************
 */
typedef void (*bond_manage_evt_cb_t)(bond_manage_evt_t evt,bond_manage_evt_param_t *p_param);


/* @brief  bond manage config data typedef */
///bond manage init device config data struct
typedef struct
{
    ///Callback function to send the event of bond manage
    bond_manage_evt_cb_t        evt;
    ///bond manage pair mode config
    bond_manage_pair_mode_t     pair_mode;
    /// Pairing Features (request = GAPC_PAIRING_RSP)
    struct gapc_pairing         pairing_feat;
}bond_manage_dev_cfg_t;
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief   Func bond_manage_init
 *
 * @param[in] dev_addr   The pointer of bond manage init device config data.
 *
 * @return    The result of bond manage config param init.
 ****************************************************************************************
 */
ble_bond_err_t bond_manage_init(bond_manage_dev_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief   Func bond_manage_random_resolve_private_addr
 *
 * @param[in] dev_addr   bd address to resolve irk key.
 * @param[in] key_cnt    irk key count.
 * @param[in] fill_cb    Callback function to fill random private addr irk.
 * @param[in] result_cb  Callback function to send the rusult of resolving random private addr
 *
 * @return    None.
 ****************************************************************************************
 */
void bond_manage_random_resolve_private_addr(bd_addr_t dev_addr,uint8_t key_cnt,fill_rpa_irk_list_cb_t fill_cb,reslove_result_cb_t result_cb);

/**
 ****************************************************************************************
 * @brief   Func bond_manage_connect_index_to_bond_id
 *
 * @param[in] conn_index  The connect handle index .
 *
 * @return    bond_node_id The bond id,may be BOND_MG_INVALID_ID.
 ****************************************************************************************
 */
bond_node_id_t bond_manage_connect_index_to_bond_id(uint8_t conn_index);

enum gap_lk_sec_lvl bond_manage_get_conn_sec_lvl(uint8_t conn_index);


/**
 ****************************************************************************************
 * @brief   Func bond_manage_action_set
 *
 * @param[in] action    The application send action to bond manage.
 * @param[in] p_param   The pointer of bond manage action  param.
 *
 * @return    The result of bond manage action set.
 ****************************************************************************************
 */
ble_bond_err_t bond_manage_action_set(bond_manage_action_t action,bond_manage_action_param_t *p_param);

/**
 ****************************************************************************************
 * @brief   Func bond_manage [gap slave role] send req msg to[gap master role]
 *
 * @param[in] conn_index  The connect handle index .
 *
 ****************************************************************************************
 */
void bond_manage_slave_req_security(uint8_t conn_index);

void bond_manage_master_req_security(uint8_t conn_index);

/**
 ****************************************************************************************
 * @brief   Func bond_manage initiate security procedure
 *
 * @param[in] conn_index  The connect handle index .
 *
 ****************************************************************************************
 */
void bond_manage_initiate_security_procedure(uint8_t conn_idx);

/**
 ****************************************************************************************
 * @brief     If this index is connected
 * @param[in] conn_idx    connection index
 * @return    If this index is connected
 ****************************************************************************************
 */
bool bond_manage_is_connected(uint8_t conn_idx);

enum gap_role bond_manage_get_connection_role(uint8_t conn_idx);

struct gap_bdaddr *bond_manage_get_peer_addr(uint8_t conn_idx);

#endif /* APP_FREERTOS_BLE_BOND_MANAGE_H_ */
/// @} BLE_BOND_MANAGE_API

