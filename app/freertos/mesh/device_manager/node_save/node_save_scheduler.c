/**
 ****************************************************************************************
 *
 * @file   node_save_scheduler.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-29 15:29
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

#include "node_save_scheduler.h"
#include "node_save_scene.h"
#include "node_save.h"
#include "node_save_macro.h"
#include "node_save_model.h"
#include "node_save_common.h"
#include "mesh_node_base.h"
#include "sdk_mesh_config_pro.h"
#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "bxfs.h"
#include "mesh_core_api.h"
#include "scheduler_server.h"
#include "generic_onoff_server.h"
#include "generic_level_server.h"
#include "light_hsl_server.h"
#include "light_ctl_server.h"
#include "light_lightness_server.h"
#include "scene_server.h"

/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void node_save_element_scheduler(model_base_t *model, save_scheduler_value_t *scheduler, uint8_t scheduler_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_element_scheduler\n");

    //get dir and filename
    uint8_t file_name   = scheduler_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_scheduler   = MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST;

    //save to bxfs
    bxfs_write2(dir_element, dir_scheduler, file_name, scheduler, MESH_SCHEDULER_VALUE);
}

void node_search_element_scheduler_from_index(uint8_t elem_index, uint8_t scheduler_index, save_scheduler_value_t *p_scheduler)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_search_element_scheduler\n");
    uint8_t element_idx = elem_index;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_scheduler = MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST;
    save_scheduler_value_t scheduler;

    dir_element = MESHDIR1_ELMT_MIN + element_idx;
    bxfs_listfile2(dir_element, dir_scheduler);
    LOG(3, "node_search_element_scheduler_from_index:%d\n", get_node_save_file_count());
    for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
        file_name = get_node_save_file_list(i);
        bxfs_read2(dir_element, dir_scheduler, file_name, &scheduler, MESH_SCHEDULER_VALUE);
        LOG(3, "search scheduler_index: %x %x %x %x %x\n", 
                scheduler_index, scheduler.index, scheduler.year, scheduler.mouth, scheduler.day);
        if(scheduler_index == scheduler.index) {
            memcpy(p_scheduler, &scheduler, sizeof(save_scheduler_value_t));
            break;
        }
    }
}

void node_search_element_scheduler_from_scene_number(uint8_t elem_index, uint16_t scene_number, save_scheduler_value_t *p_scheduler)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_search_element_scheduler\n");
    uint8_t element_idx = elem_index;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_scheduler = MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST;
    save_scheduler_value_t scheduler;

    dir_element = MESHDIR1_ELMT_MIN + element_idx;
    bxfs_listfile2(dir_element, dir_scheduler);
    LOG(3, "node_search_element_scheduler_from_scene_number:%d\n", get_node_save_file_count());
    for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
        file_name = get_node_save_file_list(i);
        bxfs_read2(dir_element, dir_scheduler, file_name, &scheduler, MESH_SCHEDULER_VALUE);
        LOG(3, "search scheduler_index: %x %x %x %x %x\n", 
                scene_number, scheduler.index, scheduler.year, scheduler.mouth, scheduler.scene_number);
        if(scene_number == scheduler.scene_number) {
            memcpy(p_scheduler, &scheduler, sizeof(save_scheduler_value_t));
            break;
        }
    }
}

void node_recover_element_scheduler(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_recover_element_scheduler\n");
    uint8_t element_idx = 0;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_scheduler = MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST;
    save_scheduler_value_t scheduler;

    for(element_idx = 0; element_idx < get_element_num(); element_idx++) {
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        uint8_t first_available_scheduler_flag = 0;
        scheduler_server_t *scheduler_server = NULL;

        dir_element = MESHDIR1_ELMT_MIN + element_idx;

        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if(model->model_id == SCHEDULER_SETUP_SERVER_MODEL_ID || model->model_id == SCHEDULER_SERVER_MODEL_ID) {
                scheduler_server = GET_SERVER_MODEL_PTR(scheduler_server_t, model);
                break;
            }
            hdr = co_list_next(hdr);
        }
        bxfs_listfile2(dir_element, dir_scheduler);
        for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
            file_name = get_node_save_file_list(i);
            bxfs_read2(dir_element, dir_scheduler, file_name, &scheduler, MESH_SCHEDULER_VALUE);
            LOG(3, "recover scene_number:%x %x %x\n", 
                    scheduler.scene_number, scheduler.index, scheduler.year);

            if(first_available_scheduler_flag == 0 && scheduler.scene_number != 0) {
                first_available_scheduler_flag = 1;
                if(scheduler_server)
                    memcpy(&scheduler_server->msg_format->action_set, &scheduler, sizeof(save_scheduler_value_t));
            }

            if(scheduler_server)
                scheduler_server->msg_format->schedules |= 1 << scheduler.index;
        }
    }
}
void node_delete_element_scheduler(model_base_t *model, uint8_t scheduler_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"node_delete_element_scheduler scheduler_index:%d\n", scheduler_index);
    //get dir and filename
    uint8_t file_name   = scheduler_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_scheduler   = MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST;

    bxfs_delete2(dir_element, dir_scheduler, file_name);
}

