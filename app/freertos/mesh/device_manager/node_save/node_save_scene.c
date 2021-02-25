/**
 ****************************************************************************************
 *
 * @file   node_save_scene.c
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
#include "scene_server.h"
#include "generic_onoff_server.h"
#include "generic_level_server.h"
#include "light_hsl_server.h"
#include "light_ctl_server.h"
#include "light_lightness_server.h"

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
void node_save_element_scene(model_base_t *model, save_scene_value_t *scene, uint8_t scene_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_bind_appkey\n");

    //get dir and filename
    uint8_t file_name   = scene_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_scene   = MESHDIR2_ELMTX_SCENE_NUMBERLIST;

    //save to bxfs
    bxfs_write2(dir_element, dir_scene, file_name, scene, MESH_SCENE_VALUE);
}

void node_search_element_scene(uint8_t index, uint16_t scene_number, save_scene_value_t *p_scene)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_search_element_scene\n");
    uint8_t element_idx = index;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_scene = MESHDIR2_ELMTX_SCENE_NUMBERLIST;
    save_scene_value_t scene;

    dir_element = MESHDIR1_ELMT_MIN + element_idx;
    bxfs_listfile2(dir_element, dir_scene);
    LOG(3, "node_search_element_scene:%d\n", get_node_save_file_count());
    for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
        file_name = get_node_save_file_list(i);
        bxfs_read2(dir_element, dir_scene, file_name, &scene, MESH_SCENE_VALUE);
        LOG(3, "search scene_number: %x %x %x %x %x\n", 
                scene_number, scene.scene_number, scene.lightness, scene.hue, scene.saturation);
        if(scene_number == scene.scene_number) {
            memcpy(p_scene, &scene, sizeof(save_scene_value_t));
            break;
        }
    }
}

void node_recover_element_scene(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_recover_element_scene\n");
    uint8_t element_idx = 0;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_scene = MESHDIR2_ELMTX_SCENE_NUMBERLIST;
    save_scene_value_t scene;

    for(element_idx = 0; element_idx < get_element_num(); element_idx++) {
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        uint8_t first_available_scene_flag = 0;
        scene_server_t *scene_server = NULL;
        light_hsl_server_t *hsl_server = NULL;
        light_ctl_server_t *ctl_server = NULL;

        dir_element = MESHDIR1_ELMT_MIN + element_idx;

        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if(model->model_id == SCENE_SETUP_SERVER_MODEL_ID || model->model_id == SCENE_SERVER_MODEL_ID) {
                scene_server = GET_SERVER_MODEL_PTR(scene_server_t, model);
            }
            if(model->model_id == LIGHT_HSL_SERVER_MODEL_ID) {
                hsl_server = GET_SERVER_MODEL_PTR(light_hsl_server_t, model);
            }
            if(model->model_id == LIGHT_CTL_SERVER_MODEL_ID) {
                ctl_server = GET_SERVER_MODEL_PTR(light_ctl_server_t, model);
            }
            hdr = co_list_next(hdr);
        }
        bxfs_listfile2(dir_element, dir_scene);
        for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
            file_name = get_node_save_file_list(i);
            bxfs_read2(dir_element, dir_scene, file_name, &scene, MESH_SCENE_VALUE);
            LOG(3, "recover scene_number:%x %x %x %x\n", scene.scene_number, scene.lightness, scene.hue, scene.saturation);

            if(first_available_scene_flag == 0 && scene.scene_number != 0) {
                first_available_scene_flag = 1;
                scene_server->msg_format->current_scene = scene.scene_number;
                if(hsl_server) {
                    hsl_server->msg_format->present_hsl_lightness = scene.lightness;
                    hsl_server->msg_format->present_hsl_hue = scene.hue;
                    hsl_server->msg_format->present_hsl_saturation = scene.saturation;
                }else if(ctl_server) {
                    ctl_server->msg_format->present_ctl_lightness = scene.lightness;
                    ctl_server->msg_format->present_ctl_temperature = scene.hue;
                    ctl_server->msg_format->present_ctl_delta_uv = scene.saturation;
                }
            }

            if(scene_server && scene.scene_number) {
                scene_server->msg_format->scene_number[i] = scene.scene_number;
            }
        }
    }
}
void node_delete_element_scene(model_base_t *model, uint8_t scene_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"node_delete_element_scene scene_index:%d\n", scene_index);
    //get dir and filename
    uint8_t file_name   = scene_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_scene   = MESHDIR2_ELMTX_SCENE_NUMBERLIST;

    bxfs_delete2(dir_element, dir_scene, file_name);
}

