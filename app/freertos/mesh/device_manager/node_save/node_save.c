/*
 * node_save.c
 *
 *  Created on: 2018-8-20
 *      Author: jiachuang
 */

#include "osapp_config.h"
#include "node_save_common.h"
#include "node_save_macro.h"
#include "node_save.h"
#include "bxfs.h"
#include "dw_apb_ssi_typedef.h"
#include "mesh_stack_init.h"
#include "osapp_mesh.h"
#include "mesh_core_api.h"
#include "flash.h"
#include "mesh_queued_msg.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define NODE_SAVE_BASE_ADDR     0x42000
#define NODE_SAVE_SECTION_NUM   5

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum
{
    NODE_SAVE_MESH_RECOVER_MISC,
    NODE_SAVE_MESH_ELEMENT_ADDR,
    NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_NET_KEY,
    NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_APP_KEY,
    NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_DEV_KEY,
    NODE_SAVE_MESH_RECOVER_BIND_APPKEY,
    NODE_SAVE_MESH_RECOVER_SUBSCRIPTION_LIST,
    NODE_SAVE_MESH_RECOVER_PUBLICATION,
    NODE_SAVE_MESH_RECOVER_SCENE,
    NODE_SAVE_MESH_RECOVER_SCHEDULER,
    NODE_SAVE_MESH_RECOVER_RECOVER_ACTION,
}node_save_stage_t;


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static node_save_stage_t node_save_stage;








void node_save_mesh_stage_run(void)
{
    switch(node_save_stage)
    {
    case NODE_SAVE_MESH_RECOVER_MISC:
        node_recover_misc();
        node_save_process_next_stage();
        break;
    case NODE_SAVE_MESH_ELEMENT_ADDR:
        node_recover_element_uni_adddr();
        node_save_process_next_stage();
        break;
    case NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_NET_KEY:
        node_save_net_key_recover();
        //generate keys -> mesh_run
        break;
    case NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_APP_KEY:
        node_save_app_key_recover();
        node_save_process_next_stage();
        break;
    case NODE_SAVE_MESH_RECOVER_CONFIG_SERVER_DEV_KEY:
        node_save_dev_key_recover();
        node_save_process_next_stage();
        break;
    case NODE_SAVE_MESH_RECOVER_BIND_APPKEY:
        node_recover_bind_appkey();
        node_save_process_next_stage();
        break;
    case NODE_SAVE_MESH_RECOVER_SUBSCRIPTION_LIST:
        node_recover_subscription_list();
        node_save_process_next_stage();
        break;
	case NODE_SAVE_MESH_RECOVER_PUBLICATION:
	    node_recover_publish_list();
        node_save_process_next_stage();
	    break;
	case NODE_SAVE_MESH_RECOVER_SCENE:
	    node_recover_element_scene();
        node_save_process_next_stage();
	    break;
	case NODE_SAVE_MESH_RECOVER_SCHEDULER:
	    node_recover_element_scheduler();
	    node_save_stage = NODE_SAVE_MESH_RECOVER_RECOVER_ACTION;
	     //Recover complete
	    mesh_stack_init_process_next_stage();
	    break;
    default:
        LOG(LOG_LVL_WARN,"Unexcepted handle in node_save_mesh_stage_run\n");
        break;
    }
}

void node_save_process_next_stage(void)
{
    node_save_stage++;
    mesh_run(node_save_mesh_stage_run,portMAX_DELAY,false);
}

void node_save_mesh_reset(void)
{
    bxfs_dir_t dir_tmp;
    //force command will delete all sub files/dirs
    //delete misc dir
    bxfs_mkdir(&dir_tmp , (void*)ROOT_DIR , MESHDIR1_MISC);
    bxfs_del_dir(dir_tmp , true);
    //delete keys dir
    bxfs_mkdir(&dir_tmp , (void*)ROOT_DIR , MESHDIR1_KEYS);
    bxfs_del_dir(dir_tmp , true);
    //delete element dir
    for(uint8_t i=0;i<=get_element_num();i++)
    {
        bxfs_mkdir(&dir_tmp , (void*)ROOT_DIR , MESHDIR1_ELMT_MIN + i);
        bxfs_del_dir(dir_tmp , true);
    }
}

void node_save_bxfs_hardware_erase_all(void)
{
    //flash_erase(NODE_SAVE_BASE_ADDR , Sector_Erase);
    //Sometimes bxfs will write over the limit , so need erase larger.
    bxfs_earse_data();
}

void node_save_recover(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_recover\n");

    //init
//    bxfs_init(NODE_SAVE_BASE_ADDR);
    node_save_init_make_dir();

    //run
    node_save_stage = NODE_SAVE_MESH_RECOVER_MISC;
    mesh_run(node_save_mesh_stage_run,portMAX_DELAY,false);
}



