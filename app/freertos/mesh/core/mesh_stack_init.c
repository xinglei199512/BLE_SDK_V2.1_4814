/**
 ****************************************************************************************
 *
 * @file   mesh_stack_init.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-09-18 17:24
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "mesh_stack_init.h"
#include "osapp_mesh.h"
#include "security.h"
#include "mesh_env.h"
#include "node_save.h"
#include "mesh_gatt.h"
#include "node_setup.h"
//#include "upper_transport.h"
#include "mesh_user_main.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_kr_client.h"
#include "mesh_node_base.h"
#include "beacon.h"
#include "mesh_node_config.h"
#include "node_save.h"
#include "mesh_core_api.h"
#include "adv_mesh_msg_cache.h"
#include "network_msg_cache.h"
#include "mesh_reset_database.h"
#include "model_publish.h"

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


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static mesh_init_process_t mesh_init_process = MESH_INIT_START;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
 
void mesh_stack_reinit(void)
{
     mesh_init_process = MESH_INIT_START;
}

/**
 ****************************************************************************************
 * @brief   Start mesh stack init process
 * @return  void
 ****************************************************************************************
 */
void mesh_stack_init_process(void)
{
    mesh_core_params_t core_param;
    //switch to next stage
    mesh_init_process ++;
    if(provision_is_database_pending())
    {
       if(mesh_init_process == MESH_INIT_NODE_CONFIG)
       {
           mesh_init_process = MESH_INIT_AFTER_NODE_RECOVER_ADD_SVC;
       }
       else if(mesh_init_process == MESH_INIT_USER)
       {
           mesh_init_process = MESH_INIT_COMPLETE;
           provision_reset_database_finish();
       }
    }
    LOG(LOG_LVL_INFO,"mesh_stack_init_process stage = %d\n",mesh_init_process);
    //process
    switch(mesh_init_process)
    {
        case MESH_INIT_NODE_CONFIG          : ///< init node feature property , must be first.
            mesh_node_feature_config_init();
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_IV_BEACON_KEYREFRESH : ///< mesh_iv_init、mesh_sec_beacon_init、mesh_kr_client_init
            mesh_iv_init();
            mesh_beacon_init();
            mesh_kr_client_init();
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_UPPPER               : ///< upper_transport_init
            //upper_transport_init();
            adv_mesh_msg_cache_init();
            network_msg_cache_init();
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_SECURITY             : ///< security_init
            security_init(mesh_stack_init_process_next_stage);
            // not stuff
            break;
        case MESH_INIT_NODE_USER            : ///< user node-element-model tree init
            mesh_model_publish_init();
            mesh_node_setup();
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_DEBUG_ERASE_BXFS:      ///< debug to erase bxfs when role = provisioner
            mesh_core_params_get(MESH_CORE_PARAM_MESH_ROLES , &core_param);
            if(core_param.role == MESH_ROLE_CONFIG_CLIENT)
            {
                node_save_bxfs_hardware_erase_all();
            }
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_NODE_RECOVER         : ///< bxfs_init,make_dir,recover
            node_save_recover();
            // not stuff
            break;
        case MESH_INIT_AFTER_NODE_RECOVER_ADD_SVC:  ///< node recover add GATT services
            mesh_add_gatt_svc();
            // not stuff
            break;
        case MESH_INIT_START_SCAN            : ///< mesh start scan
             mesh_init_start_scan();
             //TODO
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_USER                 : ///< user init function
            mesh_user_main_init();
            mesh_stack_init_process_next_stage();
            break;
        case MESH_INIT_COMPLETE             : ///< init complete
            LOG(LOG_LVL_INFO,"Init complete!\n");
            break;
        default:
            LOG(LOG_LVL_WARN,"Unexcepted handle in mesh_stack_init_process\n");
            break;
    }
}



