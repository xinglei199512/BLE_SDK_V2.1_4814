/**
 ****************************************************************************************
 *
 * @file   ble_bond_errors.h
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-12-24 17:47
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
 * @addtogroup BLE_BOND_ERRORS_API
 * @ingroup BLE_API
 * @brief BLE ble_errors  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_UTILS_BLE_BOND_ERRORS_H_
#define APP_FREERTOS_UTILS_BLE_BOND_ERRORS_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/**@brief ble errors type */
typedef uint8_t ble_bond_err_t;


/*
 * DEFINES
 ****************************************************************************************
 */

/** @defgroup BLE_BOND_ERRORS_BASE Error Codes Base number definitions
* @{ */
#define BLE_BOND_ERROR_BASE_NUM                    0       ///< Global error base
/** @} */





/**
 @defgroup BLE_BOND_ERRORS Ble Bond Global Error Codes
 @{

 @brief Global Error definitions
*/
#define BLE_BOND_SUCCESS                           BLE_BOND_ERROR_BASE_NUM         ///< Successful command
#define BLE_BOND_ERROR_INTERNAL                    (BLE_BOND_ERROR_BASE_NUM + 1 )  ///< Internal Error
#define BLE_BOND_ERROR_NOT_FOUND                   (BLE_BOND_ERROR_BASE_NUM + 2 )  ///< Not found
#define BLE_BOND_ERROR_INVALID_PARAM               (BLE_BOND_ERROR_BASE_NUM + 3 )  ///< Invalid Parameter
#define BLE_BOND_ERROR_INVALID_LENGTH              (BLE_BOND_ERROR_BASE_NUM + 4 )  ///< Invalid Length
#define BLE_BOND_ERROR_NULL                        (BLE_BOND_ERROR_BASE_NUM + 5 )  ///< Null Pointer
#define BLE_BOND_ERROR_INVALID_NODE_ID             (BLE_BOND_ERROR_BASE_NUM + 6 )  ///< Invalid node id to set/get/delete
#define BLE_BOND_ERROR_NVDS_WRITE_ERR              (BLE_BOND_ERROR_BASE_NUM + 7 )  ///< save to flash error
#define BLE_BOND_ERROR_NVDS_DELETE_ERR             (BLE_BOND_ERROR_BASE_NUM + 8 )  ///< save to flash error
#define BLE_BOND_ERROR_NVDS_LENGTH_ERR             (BLE_BOND_ERROR_BASE_NUM + 9 )  ///< save to flash error
#define BLE_BOND_ERROR_DATABASE_INDEX_TAG_FULL     (BLE_BOND_ERROR_BASE_NUM + 10)  ///< database index tag is full
#define BLE_BOND_ERROR_SERVICES_FULL               (BLE_BOND_ERROR_BASE_NUM + 11)  ///< services is full

/**
 @}
*/

#endif /* APP_FREERTOS_UTILS_BLE_BOND_ERRORS_H_ */ 
/// @} BLE_BOND_ERRORS_API

