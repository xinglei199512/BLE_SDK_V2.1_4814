/**
 ****************************************************************************************
 *
 * @file   node_save_onpowerup.c
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

#include "node_save_onpowerup.h"
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
#include "generic_onoff_server.h"
#include "light_hsl_server.h"
#include "light_ctl_server.h"
#include "light_lightness_server.h"
#include "generic_power_onoff_server.h"

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
void node_save_element_onpowerup(model_base_t *model, save_power_onoff_value_t *scene, uint8_t index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_bind_appkey\n");

    //get dir and filename
    uint8_t file_name   = index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_onpowerup   = MESHDIR2_ELMTX_POWER_ONOFF_NUMBERLIST;

    LOG(3, "save scene:%x %x %x %x %x %x\n", scene->type, scene->onpowerup, scene->onoff, scene->lightness, scene->hue_or_temperature, scene->saturation_or_delta_uv);
    //save to bxfs
    bxfs_write2(dir_element, dir_onpowerup, file_name, scene, MESH_POWER_ONOFF_VALUE);
}

int node_search_element_onpowerup(uint8_t index, save_power_onoff_value_t *p_scene)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_search_element_onpowerup\n");
    uint8_t element_idx = index;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_onpowerup = MESHDIR2_ELMTX_POWER_ONOFF_NUMBERLIST;
    save_power_onoff_value_t scene;

    dir_element = MESHDIR1_ELMT_MIN + element_idx;
    bxfs_listfile2(dir_element, dir_onpowerup);
    LOG(3, "node_search_element_onpowerup:%d\n", get_node_save_file_count());
    if(get_node_save_file_count()) {
        file_name = get_node_save_file_list(0);
        bxfs_read2(dir_element, dir_onpowerup, file_name, &scene, MESH_POWER_ONOFF_VALUE);
        LOG(3, "search onpowerup:%x %x %x %x\n", 
                scene.onpowerup, scene.lightness, scene.hue_or_temperature, scene.saturation_or_delta_uv);
        memcpy(p_scene, &scene, sizeof(save_power_onoff_value_t));
        return 0;
    }
    return -1;
}
void node_recover_element_onpowerup(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO, "NODESAVE:node_recover_element_onpowerup\n");
    uint8_t element_idx = 0;
    uint8_t file_name;
    uint8_t dir_element = 0;
    uint8_t dir_onpowerup = MESHDIR2_ELMTX_POWER_ONOFF_NUMBERLIST;
    save_power_onoff_value_t scene;

    for(element_idx = 0; element_idx < get_element_num(); element_idx++) {
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        generic_power_onoff_server_t *onpowerup_server = NULL;
        light_hsl_server_t *hsl_server = NULL;
        light_ctl_server_t *ctl_server = NULL;
        generic_onoff_server_t *onoff_server = NULL;
        light_lightness_server_t *lightness_server = NULL;

        dir_element = MESHDIR1_ELMT_MIN + element_idx;

        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if(model->model_id == GENERIC_POWER_ONOFF_SERVER_MODEL_ID || model->model_id == GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL_ID) {
                onpowerup_server = GET_SERVER_MODEL_PTR(generic_power_onoff_server_t, model);
            }
            else if(model->model_id == LIGHT_HSL_SERVER_MODEL_ID) {
                hsl_server = GET_SERVER_MODEL_PTR(light_hsl_server_t, model);
            }
            else if(model->model_id == LIGHT_CTL_SERVER_MODEL_ID) {
                ctl_server = GET_SERVER_MODEL_PTR(light_ctl_server_t, model);
            }
            else if(model->model_id == LIGHT_LIGHTNESS_SERVER_MODEL_ID) {
                lightness_server = GET_SERVER_MODEL_PTR(light_lightness_server_t, model);
            }
            else if(model->model_id == GENERIC_ONOFF_SERVER_MODEL_ID) {
                onoff_server = GET_SERVER_MODEL_PTR(generic_onoff_server_t, model);
            }

            hdr = co_list_next(hdr);
        }
        bxfs_listfile2(dir_element, dir_onpowerup);
        for(uint8_t i = 0; i < get_node_save_file_count(); i++) {
            file_name = get_node_save_file_list(i);
            bxfs_read2(dir_element, dir_onpowerup, file_name, &scene, MESH_POWER_ONOFF_VALUE);
            LOG(3, "recover scene:%x %x %x %x %x %x\n", scene.type, scene.onpowerup, scene.onoff, scene.lightness, scene.hue_or_temperature, scene.saturation_or_delta_uv);

            onpowerup_server->msg_format->onpowerup = scene.onpowerup;

            if(scene.type == POWER_ONOFF_LIGHT_HSL) {
            	if(hsl_server){
            		hsl_server->msg_format->target_hsl_lightness = hsl_server->msg_format->default_hsl_lightness = hsl_server->msg_format->present_hsl_lightness = scene.lightness;
            		hsl_server->msg_format->target_hsl_hue = hsl_server->msg_format->default_hsl_hue = hsl_server->msg_format->present_hsl_hue = scene.hue_or_temperature;
            		hsl_server->msg_format->target_hsl_saturation = hsl_server->msg_format->default_hsl_saturation = hsl_server->msg_format->present_hsl_saturation = scene.saturation_or_delta_uv;
            	}

                if(onoff_server) {
                    onoff_server->msg_format.target_onoff = onoff_server->msg_format.present_onoff = scene.onoff;
                }

                if(lightness_server && scene.lightness) {
                    lightness_server->msg_format->lightness_last = lightness_server->msg_format->lightness_default = scene.lightness;
                }
            }

            if(scene.type == POWER_ONOFF_LIGHT_CTL && ctl_server) {
                ctl_server->msg_format->target_ctl_lightness = ctl_server->msg_format->default_ctl_lightness = ctl_server->msg_format->present_ctl_lightness = scene.lightness;
                ctl_server->msg_format->target_ctl_temperature = ctl_server->msg_format->default_ctl_temperature = ctl_server->msg_format->present_ctl_temperature = scene.hue_or_temperature;
                ctl_server->msg_format->target_ctl_delta_uv = ctl_server->msg_format->default_ctl_delta_uv = ctl_server->msg_format->present_ctl_delta_uv = scene.saturation_or_delta_uv;

                if(onoff_server && onoff_server->msg_format.present_onoff != scene.onoff)
                    onoff_server->msg_format.target_onoff = onoff_server->msg_format.present_onoff = scene.onoff;
            }
        }
    }
}
void node_delete_element_onpowerup(model_base_t *model, uint8_t index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"node_delete_element_onpowerup index:%d\n", index);
    //get dir and filename
    uint8_t file_name   = index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_onpowerup   = MESHDIR2_ELMTX_POWER_ONOFF_NUMBERLIST;

    bxfs_delete2(dir_element, dir_onpowerup, file_name);
}

