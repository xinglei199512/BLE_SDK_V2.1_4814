/**
 ****************************************************************************************
 *
 * @file   mesh_stack_init.h
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-09-18 17:25
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
 * @addtogroup MESH_mesh_stack_init_API Mesh mesh_stack_init API
 * @ingroup MESH_API
 * @brief Mesh mesh_stack_init  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_MESH_STACK_INIT_H_
#define FREERTOS_APP_MESH_MESH_STACK_INIT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "sdk_mesh_config.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum
{
    MESH_INIT_START,                                ///< start of init
    MESH_INIT_NODE_CONFIG,                          ///< init node feature property , must be first.
    MESH_INIT_IV_BEACON_KEYREFRESH,                 ///< mesh_iv_init、mesh_sec_beacon_init、mesh_kr_client_init
    MESH_INIT_UPPPER,                               ///< upper_transport_init
    MESH_INIT_SECURITY,                             ///< security_init
    MESH_INIT_NODE_USER,                            ///< user node-element-model tree init
    MESH_INIT_DEBUG_ERASE_BXFS,                     ///< debug to erase bxfs when role = provisioner
    MESH_INIT_NODE_RECOVER,                         ///< bxfs_init,make_dir,recover
    MESH_INIT_AFTER_NODE_RECOVER_ADD_SVC,           ///< node recover add GATT services
    MESH_INIT_START_SCAN,                           ///< mesh start scan
    MESH_INIT_USER,                                 ///< user init function
    MESH_INIT_COMPLETE,                             ///< init complete
}mesh_init_process_t;

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

/**
 ****************************************************************************************
 * @brief   Start mesh stack init process
 * @return  void
 ****************************************************************************************
 */
void mesh_stack_init_process(void);


#endif /* FREERTOS_APP_MESH_MESH_STACK_INIT_H_ */ 
/// @} MESH_mesh_stack_init_API

