/**
 ****************************************************************************************
 *
 * @file   mesh_dm_error.h
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-10-17 14:40
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
 * @addtogroup MESH_DM_ERRORS_API
 * @ingroup  MESH_ERRORS_API
 * @brief defines for BLE MESH ERRORS API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_DEVICE_MANAGER_MESH_DM_ERROR_H_
#define FREERTOS_APP_MESH_DEVICE_MANAGER_MESH_DM_ERROR_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "sdk_mesh_config_pro.h"

/**
  @defgroup MESH_DM_ERRORS Mesh Device manager Global Error Codes
  @{

  @brief Global Error definitions
*/

/** @defgroup MESH_DM_ERRORS_BASE Error Codes Base number definitions
 * @{ */
#define MESH_DM_ERROR_BASE_NUM                    MESH_DM_ERROR_ALLOC       ///< Global error base
/** @} */

#define MESH_DM_SUCCESS                           (MESH_CORE_ERROR_ALLOC + 0)  ///< Successful command
#define MESH_DM_ERROR_INVALID_ADDR                (MESH_DM_ERROR_BASE_NUM + 1)  ///< Invalid_Address
#define MESH_DM_ERROR_INVALID_MODEL               (MESH_DM_ERROR_BASE_NUM + 2)  ///< Invalid_Model
#define MESH_DM_ERROR_INVALID_APPKEY_INDEX        (MESH_DM_ERROR_BASE_NUM + 3)  ///< Invalid_AppKey_Index
#define MESH_DM_ERROR_INVALID_NETKEY_INDEX        (MESH_DM_ERROR_BASE_NUM + 4)  ///< Invalid_NetKey_Index
#define MESH_DM_ERROR_INSUFFICIENT_RESOURCES      (MESH_DM_ERROR_BASE_NUM + 5)  ///< Insufficient_Resources
#define MESH_DM_ERROR_KEY_INDEX_ALREADY_STORED    (MESH_DM_ERROR_BASE_NUM + 6)  ///< Key_Index_Already_Stored

/**
  @}
*/
#endif /* FREERTOS_APP_MESH_DEVICE_MANAGER_MESH_DM_ERROR_H_ */ 
/// @} MESH_DM_ERRORS_API

