/**
 ****************************************************************************************
 *
 * @file   unit_test_config.h
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-12-10 14:28
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_unit_test_config_API Mesh unit_test_config API
 * @ingroup MESH_API
 * @brief Mesh unit_test_config  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_UNIT_TEST_UNIT_TEST_CONFIG_H_
#define APP_FREERTOS_MESH_UNIT_TEST_UNIT_TEST_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
//#define UNIT_TEST_SECURITY_AES_128
//#define UNIT_TEST_SECURITY_AES_CCM_ENCRYPT
#define UNIT_TEST_SECURITY_AES_CCM_DECRYPT
//#define UNIT_TEST_SECURITY_AES_CMAC_BASIC

//#define UNIT_TEST_SECURITY_AES_CMAC_K1
//#define UNIT_TEST_SECURITY_AES_CMAC_K2_MASTER
//#define UNIT_TEST_SECURITY_AES_CMAC_K2_FRIEND
//#define UNIT_TEST_SECURITY_AES_CMAC_K4
//#define UNIT_TEST_SECURITY_AES_CMAC_NETKEY_GENERATE_ALL_KEYS



/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */



#endif /* APP_FREERTOS_MESH_UNIT_TEST_UNIT_TEST_CONFIG_H_ */ 
/// @} MESH_unit_test_config_API

