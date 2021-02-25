/**
 ****************************************************************************************
 *
 * @file   mesh_app.c
 *
 * @brief  .
 *
 * @author  liuzy
 * @date    2018-09-25 17:20
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
//mesh platform
#include <math.h>
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "mesh_core_api.h"
//app
#include "node_setup.h"
#include "mesh_app_hal.h"
//tools
#include "co_utils.h"
#include <stdbool.h>
#include "string.h"
//feature
#include "node_save.h"
//model
#include "config_server_events_api.h"
#include "model_servers_events_api.h"
#include "config_server.h"
#include "generic_onoff_server.h"
#include "generic_transition_client.h"
#include "generic_power_onoff_server.h"
#include "generic_power_onoff_common.h"
#include "mesh_env.h"
#include "mesh_node_base.h"
#include "proxy_s.h"
#include "tmall_model_server.h"
#include "tmall_model_client.h"
#include "light_lightness_server.h"
#include "light_lightness_setup_server.h"
#include "light_ctl_client.h"
#include "light_ctl_common.h"
#include "light_ctl_setup_server.h"
#include "light_ctl_temperature_server.h"
#include "light_hsl_server.h"
#include "light_hsl_common.h"
#include "light_hsl_setup_server.h"
#include "light_hsl_hue_server.h"
#include "light_hsl_saturation_server.h"
#include "model_status_switch.h"
#include "generic_level_server.h"
#include "scheduler_server.h"
#include "scheduler_client.h"
#include "scene_server.h"
#include "scene_client.h"
#include "time_server.h"
#include "node_save_onpowerup.h"
#include "unprov_device_intf.h"
#include "node_save_common.h"
#include "node_save_onoffcount.h"
#include "node_save_common.h"
#include "tmall_model_common.h"
#include "tmall_model_client.h"
#include "tmall_model_server.h"
//hal
#include "mesh_app_hal.h"
#include "health_server.h"
#include "health_common.h"
#include "config_server_pub_sub.h"
#include "mesh_sched.h"

#include "config_server_pub_sub.h"
#include "foundation_common.h"
#include "co_endian.h"
#define ONOFF_NETWORK_ACCESS_STATE_COUNT 3
#define APP_KEY_MAX 3
#define SUB_ADDR_MAX 4
mesh_addr_t sub_addr_list[SUB_ADDR_MAX];
/*test*/
extern uint16_t dst_addrs;
extern bool success_respond;


extern TimerHandle_t xAutoReloadTimer1;

static uint16_t save_current_lightness = 0;
static uint16_t save_current_temperature = 0;

static light_lightness_msg_format_t lightness_msg_format;
static light_hsl_msg_format_t hsl_msg_format;
static scene_server_msg_format_t scene_msg;
static generic_power_onoff_msg_format_t onpowerup_msg[2];
static light_ctl_msg_format_t ctl_msg_format;

DEF_SCENE_SERVER_MODEL(scene_server_0, APP_KEY_MAX);
DEF_SCENE_SETUP_SERVER_MODEL(scene_setup_server_0, APP_KEY_MAX);

DEF_GENERIC_POWER_ONOFF_SERVER_MODEL(generic_onpowerup_server_0, APP_KEY_MAX);
DEF_GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL(generic_onpowerup_setup_server_0, APP_KEY_MAX);

DEF_TMALL_MODEL_SERVER_MODEL(tmall_model_server_0, APP_KEY_MAX);
DEF_TMALL_MODEL_CLIENT_MODEL(tmall_model_client_0, APP_KEY_MAX);

DEF_LIGHT_HSL_SERVER_MODEL(light_hsl_server_0, APP_KEY_MAX);
DEF_LIGHT_HSL_SETUP_SERVER_MODEL(light_hsl_setup_server_0, APP_KEY_MAX);
DEF_LIGHT_HSL_HUE_SERVER_MODEL(light_hsl_hue_server_0, APP_KEY_MAX);
DEF_LIGHT_HSL_SATURATION_SERVER_MODEL(light_hsl_saturation_server_0, APP_KEY_MAX);

DEF_GENERIC_ONOFF_SERVER_MODEL(generic_onoff_server_0, APP_KEY_MAX);

DEF_GENERIC_TRANSITION_SERVER_MODEL(generic_transition_server_0, APP_KEY_MAX);

DEF_HEALTH_SERVER_MODEL(health_server_0, APP_KEY_MAX);

DEF_LIGHT_CTL_SERVER_MODEL(light_ctl_server_0, APP_KEY_MAX);
DEF_LIGHT_CTL_TEMPERATURE_SERVER_MODEL(light_ctl_temperature_server_0, APP_KEY_MAX);
DEF_LIGHT_CTL_SETUP_SERVER_MODEL(light_ctl_setup_server_0, APP_KEY_MAX);

DEF_LIGHT_LIGHTNESS_SERVER_MODEL(light_lightness_server_0, APP_KEY_MAX);
DEF_LIGHT_LIGHTNESS_SETUP_SERVER_MODEL(light_lightness_setup_server_0, APP_KEY_MAX);

DEF_GENERIC_LEVEL_SERVER_MODEL(generic_level_server_0, APP_KEY_MAX);
/*test*/
uint8_t tids=1;
uint8_t tid3=1;
int num=1;
void user_scan_stop_callback(void)
{
    LOG(3,"user_scan_stop_callback\n"); 
}
void user_generic_onoff_status_publish(uint8_t present_onoff,uint8_t target_onoff)
{
	LOG(3,"user_generic_onoff_status_publish\n");
	
	  generic_onoff_server_0.msg_format.present_onoff =present_onoff;	
	  generic_onoff_server_0.msg_format.target_onoff =target_onoff;
	  generic_onoff_server_0.model.base.publish->addr.addr.addr=dst_addrs;
	
    generic_onoff_status_publish(&generic_onoff_server_0);
}

void user_generic_level_status_publish(void)
{
	  generic_level_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    generic_level_status_publish(&generic_level_server_0);
}
void user_light_lightness_status_publish(void)
{
	  light_lightness_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    light_lightness_status_publish(&light_lightness_server_0);
}
void user_light_hsl_status_publish(void)
{
	  light_hsl_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    light_hsl_status_publish(&light_hsl_server_0);
}
void user_time_scene_status_publish(void)
{
	  scene_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    time_scene_status_publish(&scene_server_0);
}
void user_light_hsl_hue_status_publish(void)
{
	  light_hsl_hue_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    light_hsl_hue_status_publish(&light_hsl_hue_server_0);
}
void user_light_hsl_saturation_status_publish(void)
{
	  light_hsl_saturation_server_0.model.base.publish->addr.addr.virt->virt_addr=dst_addrs;
    light_hsl_saturation_status_publish(&light_hsl_saturation_server_0);
}
void tmall_test_indication_key0(void)
{
  LOG(3,"tmall_test_indication_key0\n");
	custom_tmall_indication_key0_t msg;
  uint16_t  dst_addr2=0xf000;
	msg.attr_type=0x0005;
	uint32_t opcode=THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID_1, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Indication);
	custom_tmall_msg_publish(&tmall_model_client_0,&msg,sizeof(msg),dst_addr2,opcode);
}
void tmall_test_indication_onoff(bool onoff)
{
  LOG(3,"tmall_test_indication_onoff :%x\n",onoff);
	custom_tmall_indication_onoff_t msg;
  uint16_t  dst_addr2=0xf000;
	msg.tid=tid3++;
	msg.attr_type=0x0100;//attr_type :onoff
	msg.onoff=onoff;
	uint32_t opcode=THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID_1, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Indication);
	custom_tmall_msg_publish(&tmall_model_client_0,&msg,sizeof(msg),dst_addr2,opcode);
}
void tmall_test_time(void)
{
	LOG(3,"tmall_test_time \n");
	custom_tmall_time_t msg;
 
	msg.tid=tids++;
	msg.attr_type=0xf01f;
	uint32_t opcode=THREE_OCTETS_OPCODE_GEN(TMALL_MODEL_OPCODE_OFFSET, TMALL_MODEL_OPCODE_COMPANY_ID_1, TMALL_MODEL_OPCODE_COMPANY_ID_SHIFT, Vendor_Message_Attr_Time);
	custom_tmall_msg_publish(&tmall_model_client_0,&msg,sizeof(msg),dst_addrs,opcode);

}
void model_status_publish(void)
{
    //user_generic_onoff_status_publish();
    user_generic_level_status_publish();
    user_light_lightness_status_publish();
    user_light_hsl_status_publish();
    user_light_hsl_hue_status_publish();
    user_light_hsl_saturation_status_publish();
}

void ctl_onpowerup_save_current_scene(uint8_t element_idx, uint8_t type)
{
    save_power_onoff_value_t value;
    node_delete_element_onpowerup(&generic_onpowerup_server_0.model.base, 0xfe);

    value.type = type;
    value.onpowerup = generic_onpowerup_server_0.msg_format->onpowerup = 2;
    value.onoff = generic_onoff_server_0.msg_format.target_onoff = 1;
    value.lightness = light_ctl_server_0.msg_format->default_ctl_lightness;
    if(generic_onpowerup_server_0.msg_format->onpowerup == 2) {
        //value.lightness = light_lightness_server_0.msg_format->target_lightness_actual;
        value.lightness = light_ctl_server_0.msg_format->target_ctl_lightness;
        value.hue_or_temperature = light_ctl_server_0.msg_format->target_ctl_temperature;
        value.saturation_or_delta_uv = light_ctl_server_0.msg_format->target_ctl_delta_uv;
    }else {
        //value.lightness = light_lightness_server_0.msg_format->lightness_default;
        value.lightness = light_ctl_server_0.msg_format->default_ctl_lightness;
        value.hue_or_temperature = light_ctl_server_0.msg_format->default_ctl_temperature;
        value.saturation_or_delta_uv = light_ctl_server_0.msg_format->default_ctl_delta_uv;
    }
    node_save_element_onpowerup(&generic_onpowerup_server_0.model.base, &value, 0xfe);
    node_save_write_through();
}

void hsl_onpowerup_save_current_scene(uint8_t element_idx, uint8_t type)
{
    save_power_onoff_value_t value;

    node_delete_element_onpowerup(&generic_onpowerup_server_0.model.base, 0xff);

    value.type = type;
    value.onpowerup = generic_onpowerup_server_0.msg_format->onpowerup = 2;
    value.onoff = generic_onoff_server_0.msg_format.target_onoff = 0;
    value.lightness = light_hsl_server_0.msg_format->default_hsl_lightness;
    if(generic_onpowerup_server_0.msg_format->onpowerup == 2) {
        //value.lightness = light_lightness_server_0.msg_format->target_lightness_actual;
        value.lightness = light_hsl_server_0.msg_format->target_hsl_lightness;
        value.hue_or_temperature = light_hsl_server_0.msg_format->target_hsl_hue;
        value.saturation_or_delta_uv = light_hsl_server_0.msg_format->target_hsl_saturation;
    }else {
        //value.lightness = light_lightness_server_0.msg_format->lightness_default;
        value.lightness = light_hsl_server_0.msg_format->default_hsl_lightness;
        value.hue_or_temperature = light_hsl_server_0.msg_format->default_hsl_hue;
        value.saturation_or_delta_uv = light_hsl_server_0.msg_format->default_hsl_saturation;
    }
    node_save_element_onpowerup(&generic_onpowerup_server_0.model.base, &value, 0xff);
    node_save_write_through();
}

static void hsl_processing_and_state_bound(light_hsl_server_t *hsl_server)
{
    light_RGB light;


    HSL2RGB(hsl_server->msg_format->present_hsl_hue/65535.0, 
            hsl_server->msg_format->present_hsl_saturation/65535.0,
            hsl_server->msg_format->present_hsl_lightness/65535.0,
            &light);

    LOG(LOG_LVL_INFO,"hsl_processing R:0x%x G:0x%x B:0x%x\n", \
            (uint16_t)(light.R), (uint16_t)(light.G), (uint16_t)(light.B));

    hsl_server->state_bound->bound_cb(hsl_server->state_bound->led_r, (uint16_t)(light.R));
    hsl_server->state_bound->bound_cb(hsl_server->state_bound->led_g, (uint16_t)(light.G));
    hsl_server->state_bound->bound_cb(hsl_server->state_bound->led_b, (uint16_t)(light.B));
}

static void ctl_processing_and_state_bound(light_ctl_server_t *ctl_server)
{
    float temperature_warm;
    uint16_t lightness;

    //temperature_warm = (ctl_server->msg_format->present_ctl_temperature);
    temperature_warm = ((((float)ctl_server->msg_format->present_ctl_temperature) - T_MIN) * 0xffff) / (T_MAX - T_MIN);
    lightness = ctl_server->msg_format->present_ctl_lightness;

    //LOG(3, "ctl_processing_and_state_bound:%x %x %x\n", ctl_server->msg_format->present_ctl_temperature, (uint16_t )(temperature_warm), lightness);
    if(lightness) {
        ctl_server->state_bound->bound_cb(ctl_server->state_bound->led_r, (uint16_t)temperature_warm);
        ctl_server->state_bound->bound_cb(ctl_server->state_bound->led_g, (uint16_t)lightness);
    }else {
        ctl_server->state_bound->bound_cb(ctl_server->state_bound->led_g, 0);
        ctl_server->state_bound->bound_cb(ctl_server->state_bound->led_r, 0);
    }
}

void user_onoff_0_evt_cb(const mesh_model_evt_t * p_evt)
{
	LOG(LOG_LVL_INFO,"dst_addrs:%x\n",dst_addrs);
    switch(p_evt->type.onoff_type)
    {
        case ONOFF_MODEL_EVT_SET:
            LOG(LOG_LVL_INFO,"user_onoff_0_evt_cb:%d %d!!!\n",generic_onoff_server_0.msg_format.present_onoff, p_evt->params.model_value_set.target_value);

            if(generic_onoff_server_0.msg_format.present_onoff != p_evt->params.model_value_set.target_value) {
                generic_onoff_server_0.msg_format.present_onoff = (uint8_t)p_evt->params.model_value_set.target_value;
#if 0
                if(generic_onoff_server_0.msg_format.present_onoff && light_hsl_server_0.msg_format->target_hsl_lightness == 0) {
                    status_switch_level_to_onoff(&generic_level_server_0, &generic_onoff_server_0);
                    status_switch_level_to_lightness(&generic_level_server_0, &light_lightness_server_0);
                    status_switch_level_to_hsl(&generic_level_server_0, &light_hsl_server_0);
                }else {
                    status_switch_onoff_to_level(&generic_onoff_server_0, &generic_level_server_0, light_hsl_server_0.msg_format->target_hsl_lightness);
                    status_switch_onoff_to_lightness(&generic_onoff_server_0, &light_lightness_server_0);
                    status_switch_onoff_to_hsl(&generic_onoff_server_0, &light_hsl_server_0, light_hsl_server_0.msg_format->target_hsl_lightness);
                }

                if(generic_onoff_server_0.msg_format.present_onoff != 0)
                    light_hsl_server_0.msg_format->target_hsl_lightness = light_hsl_server_0.msg_format->present_hsl_lightness;

                hsl_processing_and_state_bound(&light_hsl_server_0);
#endif

                if(generic_onoff_server_0.msg_format.present_onoff && save_current_lightness == 0)
                {
                    light_ctl_server_0.msg_format->present_ctl_lightness = 0xffff;
                    light_ctl_server_0.msg_format->present_ctl_temperature = (T_MAX+T_MIN)/2;
                } else {
                    status_switch_onoff_to_ctl(&generic_onoff_server_0, &light_ctl_server_0, save_current_lightness);
                    light_ctl_server_0.msg_format->present_ctl_temperature = save_current_temperature;
                }

                if(generic_onoff_server_0.msg_format.present_onoff != 0)
                {
                    save_current_temperature = light_ctl_server_0.msg_format->target_ctl_temperature = light_ctl_server_0.msg_format->present_ctl_temperature;
                    save_current_lightness = light_ctl_server_0.msg_format->target_ctl_lightness = light_ctl_server_0.msg_format->present_ctl_lightness;
                }

                ctl_processing_and_state_bound(&light_ctl_server_0);

            }
            //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
            //ctl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_CTL);
            break;
        case ONOFF_MODEL_EVT_GET:
            LOG(LOG_LVL_INFO,"ONOFF_MODEL_EVT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    }
}

void user_level_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3,"user_level_0_evt_cb:%x\n", p_evt->type.level_type);
    switch(p_evt->type.level_type)
    {
        case LEVEL_MODEL_EVT_SET:
            LOG(LOG_LVL_INFO,"!!!LED1= %x %x!!!\n", generic_level_server_0.msg_format.present_level, p_evt->params.model_value_set.target_value);
            if(generic_onoff_server_0.msg_format.present_onoff == 0)
                return;
            if(generic_level_server_0.msg_format.present_level != p_evt->params.model_value_set.target_value) {
                generic_level_server_0.msg_format.present_level = p_evt->params.model_value_set.target_value;
                status_switch_level_to_onoff(&generic_level_server_0, &generic_onoff_server_0);
                status_switch_level_to_lightness(&generic_level_server_0, &light_lightness_server_0);
                status_switch_level_to_ctl(&generic_level_server_0, &light_ctl_server_0);
                ctl_processing_and_state_bound(&light_ctl_server_0);
            }
            break;
        case LEVEL_MODEL_EVT_GET:
            LOG(LOG_LVL_INFO,"LEVEL_MODEL_EVT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    } 
}

void user_lightness_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "%s type:%x\n", __func__, p_evt->type.lightness_type);
    uint16_t lightness = p_evt->params.model_value_set.target_value;
    switch(p_evt->type.lightness_type)
    {
        case LIGHTNESS_ACTUAL_EVT_SET:
            LOG(LOG_LVL_INFO,"!!!LIGHTNESS_ACTUAL_EVT_SET:%x %x!!! onoff:%d\n", lightness, light_lightness_server_0.msg_format->present_lightness_actual, generic_onoff_server_0.msg_format.present_onoff);
            if(generic_onoff_server_0.msg_format.present_onoff == 0)
                return;
            if(light_lightness_server_0.msg_format->present_lightness_actual != lightness) {
                light_lightness_server_0.msg_format->present_lightness_actual = lightness;
                light_lightness_server_0.msg_format->present_lightness_linear = (lightness * lightness)/65535;
                if(light_lightness_server_0.msg_format->present_lightness_actual)
                    light_lightness_server_0.msg_format->lightness_last = light_lightness_server_0.msg_format->present_lightness_actual;
                status_switch_lightness_to_onoff(&light_lightness_server_0, &generic_onoff_server_0);
                status_switch_lightness_to_level(&light_lightness_server_0, &generic_level_server_0);
#if 0
                status_switch_lightness_to_hsl(&light_lightness_server_0, &light_hsl_server_0);
                hsl_processing_and_state_bound(&light_hsl_server_0);
#endif

                status_switch_lightness_to_ctl(&light_lightness_server_0, &light_ctl_server_0);
                ctl_processing_and_state_bound(&light_ctl_server_0);
            }
            generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
#if 0
            light_hsl_server_0.msg_format->target_hsl_lightness = light_lightness_server_0.msg_format->target_lightness_actual;
            //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
#endif
            save_current_lightness = light_ctl_server_0.msg_format->target_ctl_lightness = light_lightness_server_0.msg_format->target_lightness_actual;
            //ctl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_CTL);
            break;
        case LIGHTNESS_ACTUAL_EVT_GET:
            LOG(LOG_LVL_INFO,"LIGHTNESS_ACTUAL_EVT_GET!!!\n");
            break;
        case LIGHTNESS_LINEAR_EVT_SET:
            if(light_lightness_server_0.msg_format->present_lightness_linear != lightness) {
                light_lightness_server_0.msg_format->present_lightness_linear = lightness;
                light_lightness_server_0.msg_format->present_lightness_actual = (uint16_t)sqrt(lightness * 65535);
                if(lightness && (lightness % 512 == 0))
                    light_lightness_server_0.msg_format->present_lightness_actual += 1;
                if(light_lightness_server_0.msg_format->present_lightness_actual)
                    light_lightness_server_0.msg_format->lightness_last = light_lightness_server_0.msg_format->present_lightness_actual;
                status_switch_lightness_to_onoff(&light_lightness_server_0, &generic_onoff_server_0);
                status_switch_lightness_to_level(&light_lightness_server_0, &generic_level_server_0);
                status_switch_lightness_to_hsl(&light_lightness_server_0, &light_hsl_server_0);
                hsl_processing_and_state_bound(&light_hsl_server_0);

                status_switch_lightness_to_ctl(&light_lightness_server_0, &light_ctl_server_0);
                ctl_processing_and_state_bound(&light_ctl_server_0);
            }
            break;
        case LIGHTNESS_LINEAR_EVT_GET:
            LOG(LOG_LVL_INFO,"LIGHTNESS_LINEAR_EVT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    }
}

void user_lightness_setup_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "%s type:%x\n", __func__, p_evt->type.lightness_type);
    uint16_t lightness = p_evt->params.model_value_set.target_value;
    switch(p_evt->type.hsl_type) {
        case LIGHTNESS_DEFAULT_EVT_SET:
            LOG(LOG_LVL_INFO,"LIGHTNESS_DEFAULT_EVT_SET lightness:%x\n", lightness);
            if(lightness) 
                light_lightness_server_0.msg_format->lightness_last = lightness;
            light_hsl_server_0.msg_format->default_hsl_lightness = light_lightness_server_0.msg_format->lightness_default;
            //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
            break;
        case LIGHTNESS_DEFAULT_EVT_GET:
            LOG(LOG_LVL_INFO,"LIGHTNESS_DEFAULT_EVT_GET\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    }
}

void user_hsl_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    uint16_t lightness = p_evt->params.model_hsl_set.target_hsl_lightness;
    uint16_t hue = p_evt->params.model_hsl_set.target_hsl_hue;
    uint16_t saturation = p_evt->params.model_hsl_set.target_hsl_saturation;
    LOG(3, "%s type:%x value:%x %x %x\n", __func__, p_evt->type.hsl_type, lightness, hue, saturation);
    if(generic_onoff_server_0.msg_format.present_onoff == 0)
        return;
    switch(p_evt->type.hsl_type)
    {
        case LIGHT_HSL_SET:
                LOG(LOG_LVL_INFO,"LIGHT_HSL_SET!!!\n");
                light_hsl_server_0.msg_format->present_hsl_lightness = lightness;
                light_hsl_server_0.msg_format->present_hsl_hue = hue;
                light_hsl_server_0.msg_format->present_hsl_saturation = saturation;

                generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
                //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
                hsl_processing_and_state_bound(&light_hsl_server_0);
            break;
        case LIGHT_HSL_GET:
                LOG(LOG_LVL_INFO,"LIGHT_HSL_GET!!!\n");
            break;
        case LIGHT_HSL_HUE_SET:
                LOG(LOG_LVL_INFO,"LIGHT_HSL_HUE_SET!!!\n");
                light_hsl_server_0.msg_format->present_hsl_hue = hue;
                generic_level_server_0.msg_format.present_level = hue - 32768;

                generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
                //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
                hsl_processing_and_state_bound(&light_hsl_server_0);
            break;
        case LIGHT_HSL_HUE_GET:
            LOG(LOG_LVL_INFO,"LIGHT_HSL_HUE_GET!!!\n");
            break;
        case LIGHT_HSL_SATURATION_SET:
                LOG(LOG_LVL_INFO,"LIGHT_HSL_SATURATION_SET!!!\n");
                //light_hsl_server_0.msg_format->present_hsl_lightness = lightness;
                //light_hsl_server_0.msg_format->present_hsl_hue = hue;
                light_hsl_server_0.msg_format->present_hsl_saturation = saturation;

                generic_level_server_0.msg_format.present_level = saturation - 32768;

                generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
                //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
                hsl_processing_and_state_bound(&light_hsl_server_0);
            break;
        case LIGHT_HSL_SATURATION_GET:
            LOG(LOG_LVL_INFO,"LIGHT_HSL_SATURATION_GET!!!\n");
            break;
        case LIGHT_HSL_DEFAULT_SET:
            light_lightness_server_0.msg_format->lightness_default = light_hsl_server_0.msg_format->default_hsl_lightness;
            //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
            LOG(LOG_LVL_INFO,"LIGHT_HSL_DEFAULT_SET!!!\n");
            break;
        case LIGHT_HSL_DEFAULT_GET:
            LOG(LOG_LVL_INFO,"LIGHT_HSL_DEFAULT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    } 
}

void user_hsl_setup_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "%s\n", __func__);
}

void user_ctl_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    //LOG(3, "%s type:%x\n", __func__, p_evt->type.ctl_type);
    uint16_t lightness = p_evt->params.model_ctl_set.target_ctl_lightness;
    uint16_t temperature = p_evt->params.model_ctl_set.target_ctl_temperature;
    uint16_t delta_uv = p_evt->params.model_ctl_set.target_ctl_delta_uv;
    if(generic_onoff_server_0.msg_format.present_onoff == 0)
        return;
    switch(p_evt->type.ctl_type)
    {
        case LIGHT_CTL_SET:
            LOG(LOG_LVL_INFO,"!!!LIGHT_CTL_SET=%x!!!\n", lightness);
                //light_ctl_server_0.msg_format->present_ctl_lightness = lightness;
           
                if(temperature < T_MIN)
                    temperature = T_MIN;
                else if(temperature > T_MAX)
                    temperature = T_MAX;

                if(light_ctl_server_0.msg_format->present_ctl_temperature == temperature)
                    return;

                light_ctl_server_0.msg_format->present_ctl_temperature = temperature;

                light_ctl_server_0.msg_format->present_ctl_delta_uv = delta_uv;

                save_current_temperature = light_ctl_server_0.msg_format->present_ctl_temperature;
                //status_switch_ctl_to_onoff(&light_ctl_server_0, &generic_onoff_server_0);

                generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
                //ctl_onpowerup_save_current_scene(1, POWER_ONOFF_LIGHT_CTL);
                ctl_processing_and_state_bound(&light_ctl_server_0);
            break;
        case LIGHT_CTL_GET:
            LOG(LOG_LVL_INFO,"LIGHT_CTL_GET!!!\n");
            break;
        case LIGHT_CTL_TEMPERATURE_SET:
                if(temperature < T_MIN)
                    light_ctl_server_0.msg_format->present_ctl_temperature = T_MIN;
                else if(temperature > T_MAX)
                    light_ctl_server_0.msg_format->present_ctl_temperature = T_MAX;
                else
                    light_ctl_server_0.msg_format->present_ctl_temperature = temperature;

                light_ctl_server_0.msg_format->present_ctl_delta_uv = delta_uv;

                generic_level_server_0.msg_format.present_level = ((light_ctl_server_0.msg_format->present_ctl_temperature - T_MIN) * 65535) / (T_MAX - T_MIN) - 32768;
                LOG(LOG_LVL_INFO,"LIGHT_CTL_TEMPERATURE_SET!!! level:%d temperature:%d\n", generic_level_server_0.msg_format.present_level, light_ctl_server_0.msg_format->present_ctl_temperature);

                status_switch_ctl_to_onoff(&light_ctl_server_0, &generic_onoff_server_0);

                generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
                //ctl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_CTL);
                ctl_processing_and_state_bound(&light_ctl_server_0);
            break;
        case LIGHT_CTL_TEMPERATURE_GET:
            LOG(LOG_LVL_INFO,"LIGHT_CTL_TEMPERATURE_GET!!!\n");
            break;
        case LIGHT_CTL_DEFAULT_SET:
            light_lightness_server_0.msg_format->lightness_default = light_ctl_server_0.msg_format->default_ctl_lightness;
            //ctl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_CTL);
            break;
        case LIGHT_CTL_DEFAULT_GET:
            LOG(LOG_LVL_INFO,"LIGHT_CTL_DEFAULT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    } 
}
void user_ctl_setup_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "%s\n", __func__);
}

void user_tmall_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "user_tmall_0_evt_cb:%x %x %x %x\n", 
            tmall_model_server_0.msg_format[0].attr_type, tmall_model_server_0.msg_format[0].value[0], tmall_model_server_0.msg_format[0].value[1], tmall_model_server_0.msg_format[0].value[2]);
    if(generic_onoff_server_0.msg_format.present_onoff == 0)
        return;
    if(tmall_model_server_0.msg_format[0].attr_type == 0x0123) {
        light_hsl_server_0.msg_format->target_hsl_lightness = light_hsl_server_0.msg_format->present_hsl_lightness = tmall_model_server_0.msg_format[0].value[0];
        light_hsl_server_0.msg_format->target_hsl_hue = light_hsl_server_0.msg_format->present_hsl_hue = tmall_model_server_0.msg_format[0].value[1];
        light_hsl_server_0.msg_format->target_hsl_saturation = light_hsl_server_0.msg_format->present_hsl_saturation = tmall_model_server_0.msg_format[0].value[2];

        status_switch_hsl_to_onoff(&light_hsl_server_0, &generic_onoff_server_0);
        generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;

        hsl_processing_and_state_bound(&light_hsl_server_0);
        //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
    }
}

void user_health_attention_0_evt_cb(const mesh_model_evt_t *p_evt)
{
    uint16_t attention_value = p_evt->params.model_value_set.target_value;
    LOG(3, "user_health_attention_0_evt_cb attention:%x\n", attention_value);
    if(attention_value) {

    }else {

    }
}

void user_onpowerup_0_evt_cb(const mesh_model_evt_t * p_evt)
{
    LOG(3, "user_onpowerup_0_evt_cb type:%x\n", p_evt->type.onpowerup_type);
    switch(p_evt->type.onpowerup_type)
    {
        case POWER_ONOFF_MODEL_EVT_SET: {
                LOG(LOG_LVL_INFO,"user_onpowerup_0_evt_cb %d %d!!!\n", generic_onpowerup_server_0.msg_format->onpowerup, p_evt->params.model_value_set.target_value);
                //hsl_onpowerup_save_current_scene(0, POWER_ONOFF_LIGHT_HSL);
            }
            break;
        case POWER_ONOFF_MODEL_EVT_GET:
            LOG(LOG_LVL_INFO,"POWER_ONOFF_MODEL_EVT_GET!!!\n");
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    }
}

static void user_node_save_scene(scene_server_t *scene_server, light_hsl_server_t *hsl_server, uint8_t scene_index, uint8_t delete_flag)
{
    save_scene_value_t scene;
    scene.scene_number = scene_server->msg_format->current_scene;
    scene.lightness = hsl_server->msg_format->present_hsl_lightness;
    scene.hue = hsl_server->msg_format->present_hsl_hue;
    scene.saturation = hsl_server->msg_format->present_hsl_saturation;
    LOG(LOG_LVL_INFO,"value:%d %x %x %x %x\n", 
            scene_index, scene.scene_number, scene.lightness, scene.hue, scene.saturation);

    if(delete_flag)
        node_delete_element_scene(&scene_server->model.base, scene_index);

    node_save_element_scene(&scene_server->model.base, &scene, scene_index);
}

void user_scene_0_evt_cb(const mesh_model_evt_t *p_evt)
{
    LOG(3,"user_scene_0_evt_cb:%x\n", p_evt->type.scene_type);
    switch(p_evt->type.scene_type)
    {
        case TIME_SCENE_EVT_STORE: 
            {
                save_scene_value_t scene;
                scene.scene_number = scene_setup_server_0.msg_format->current_scene;
                scene.lightness = light_hsl_server_0.msg_format->present_hsl_lightness;
                scene.hue = light_hsl_server_0.msg_format->present_hsl_hue;
                scene.saturation = light_hsl_server_0.msg_format->present_hsl_saturation;
                LOG(LOG_LVL_INFO,"TIME_SCENE_EVT_STORE value:%d %x %x %x %x\n", 
                        p_evt->params.model_scene_set.target_value, scene.scene_number, scene.lightness, scene.hue, scene.saturation);
                node_save_element_scene(&scene_setup_server_0.model.base, &scene, p_evt->params.model_scene_set.target_value);

                user_node_save_scene(&scene_setup_server_0, &light_hsl_server_0, (uint8_t)p_evt->params.model_value_set.target_value, p_evt->params.model_scene_set.repeat_flag);
            }
            break;
        case TIME_SCENE_EVT_RECALL:
            {
                scene_server_0.msg_format->current_scene = p_evt->params.model_scene_set.target_value;
                save_scene_value_t scene ;
                node_search_element_scene(0, scene_server_0.msg_format->current_scene, &scene);
                LOG(LOG_LVL_INFO,"TIME_SCENE_EVT_RECALL!!! %x %x %x %x\n",
                        scene.scene_number, scene.lightness, scene.hue, scene.saturation);
                if(scene.scene_number) {
                    light_hsl_server_0.msg_format->present_hsl_lightness = scene.lightness;
                    light_hsl_server_0.msg_format->present_hsl_hue = scene.hue;
                    light_hsl_server_0.msg_format->present_hsl_saturation = scene.saturation;

                    status_switch_hsl_to_onoff(&light_hsl_server_0, &generic_onoff_server_0);
                    status_switch_hsl_to_level(&light_hsl_server_0, &generic_level_server_0);
                    status_switch_hsl_to_lightness(&light_hsl_server_0, &light_lightness_server_0);

                    hsl_processing_and_state_bound(&light_hsl_server_0);
                }
            }
            break;
        case TIME_SCENE_EVT_GET:
            LOG(LOG_LVL_INFO,"TIME_SCENE_EVT_GET!!!\n");
            break;
        case TIME_SCENE_EVT_REGISTER_GET:
            LOG(LOG_LVL_INFO,"TIME_SCENE_EVT_REGISTER_GET!!!\n");
            break;
        case TIME_SCENE_EVT_DELETE:
            {
                node_delete_element_scene(&scene_setup_server_0.model.base, p_evt->params.model_scene_set.target_value);
                LOG(LOG_LVL_INFO,"TIME_SCENE_EVT_DELETE!!! value:%d\n", p_evt->params.model_scene_set.target_value);
            }
            break;
        default:
            LOG(LOG_LVL_INFO,"EVT NOT FOUND!!!\n");
            break;
    } 
}

void user_config_server_evt_cb(config_server_evt_type_t type, config_server_evt_param_t*p_param)
{
    LOG(LOG_LVL_INFO , "user_config_server_evt_cb=%d\n",type);

    switch(type)
    {
        case CONFIG_SERVER_EVT_RELAY_SET:
        {
            break;
        }
        case CONFIG_SERVER_EVT_APPKEY_ADD:
        {
            uint8_t status = 0;
            LOG(3, "user_config_server_evt_cb %p %p\n", p_param, p_param->appkey_add.appkey_handle);
            uint16_t global_idx = p_param->appkey_add.appkey_handle->global_idx;
            LOG(3, "user_config_server_evt_cb global_idx:%x\n", global_idx);
#if 1
            bind_appkey_to_model(&scene_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&scene_setup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&generic_onpowerup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&generic_onpowerup_setup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&tmall_model_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&tmall_model_client_0.model.base, global_idx, &status);
            bind_appkey_to_model(&generic_onoff_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&generic_level_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_ctl_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_ctl_setup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_ctl_temperature_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_lightness_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_lightness_setup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_hsl_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_hsl_setup_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_hsl_hue_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&light_hsl_saturation_server_0.model.base, global_idx, &status);
            bind_appkey_to_model(&health_server_0.model.base, global_idx, &status);
							
							xTimerStart(xAutoReloadTimer1,0);
#endif
        }
        break;
        case CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_ADD:
        {
#if 1
            config_model_subscription_add(scene_server_0.model.base.elmt, &scene_server_0.model.base, 0xc000);
            config_model_subscription_add(scene_setup_server_0.model.base.elmt, &scene_setup_server_0.model.base, 0xc000);
            config_model_subscription_add(generic_onpowerup_server_0.model.base.elmt, &generic_onpowerup_server_0.model.base, 0xc000);
            config_model_subscription_add(generic_onpowerup_setup_server_0.model.base.elmt, &generic_onpowerup_setup_server_0.model.base, 0xc000);
            config_model_subscription_add(tmall_model_server_0.model.base.elmt, &tmall_model_server_0.model.base, 0xc000);
            //config_model_subscription_add(tmall_model_client_0.model.base.elmt, &tmall_model_client_0.model.base, 0xc000);
            config_model_subscription_add(generic_onoff_server_0.model.base.elmt, &generic_onoff_server_0.model.base, 0xc000);
            config_model_subscription_add(generic_level_server_0.model.base.elmt, &generic_level_server_0.model.base, 0xc000);
            config_model_subscription_add(light_ctl_server_0.model.base.elmt, &light_ctl_server_0.model.base, 0xc000);
            config_model_subscription_add(light_ctl_setup_server_0.model.base.elmt, &light_ctl_setup_server_0.model.base, 0xc000);
            config_model_subscription_add(light_ctl_temperature_server_0.model.base.elmt, &light_ctl_temperature_server_0.model.base, 0xc000);
            config_model_subscription_add(light_lightness_server_0.model.base.elmt, &light_lightness_server_0.model.base, 0xc000);
            config_model_subscription_add(light_lightness_setup_server_0.model.base.elmt, &light_lightness_setup_server_0.model.base, 0xc000);
            config_model_subscription_add(light_hsl_server_0.model.base.elmt, &light_hsl_server_0.model.base, 0xc000);
            config_model_subscription_add(light_hsl_setup_server_0.model.base.elmt, &light_hsl_setup_server_0.model.base, 0xc000);
            config_model_subscription_add(light_hsl_hue_server_0.model.base.elmt, &light_hsl_hue_server_0.model.base, 0xc000);
            config_model_subscription_add(light_hsl_saturation_server_0.model.base.elmt, &light_hsl_saturation_server_0.model.base, 0xc000);
            config_model_subscription_add(health_server_0.model.base.elmt, &health_server_0.model.base, 0xc000);
#endif
            break;
        }
				case CONFIG_SERVER_EVT_NODE_RESET:
				{
        LOG(LOG_LVL_INFO , "CONFIG_SERVER_EVT_NODE_RESET\n");
				node_delete_mesh_dir();
				}
				break;
        default:break;
    }
}

void mesh_app_server_init(void)
{
    memset(&scene_server_0, 0, sizeof(scene_server_t));
    memset(&scene_setup_server_0, 0, sizeof(scene_server_t));

    memset(&generic_onoff_server_0, 0, sizeof(generic_onoff_server_t)); 
    memset(&generic_onpowerup_server_0, 0, sizeof(generic_power_onoff_server_t)); 
    memset(&generic_onpowerup_setup_server_0, 0, sizeof(generic_power_onoff_setup_server_t)); 

    memset(&generic_transition_server_0, 0, sizeof(generic_transition_server_t));

    memset(&health_server_0, 0, sizeof(health_server_t));
    memset(&generic_level_server_0, 0, sizeof(generic_level_server_t));
    memset(&light_hsl_server_0, 0, sizeof(light_hsl_server_t));
    memset(&light_hsl_setup_server_0, 0, sizeof(light_hsl_setup_server_t));
    memset(&light_hsl_hue_server_0, 0, sizeof(light_hsl_hue_server_t));
    memset(&light_hsl_saturation_server_0, 0, sizeof(light_hsl_saturation_server_t));
    memset(&light_lightness_server_0, 0, sizeof(light_lightness_server_t));
    memset(&light_lightness_setup_server_0, 0, sizeof(light_lightness_setup_server_t));
    memset(&light_ctl_server_0, 0, sizeof(light_ctl_server_t));
    memset(&light_ctl_setup_server_0, 0, sizeof(light_ctl_setup_server_t));

    //1.init model
    INIT_SCENE_SERVER_MODEL(scene_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_scene_0_evt_cb);
    INIT_SCENE_SETUP_SERVER_MODEL(scene_setup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_scene_0_evt_cb);
    INIT_GENERIC_POWER_ONOFF_SERVER_MODEL(generic_onpowerup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_onpowerup_0_evt_cb);
    INIT_GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL(generic_onpowerup_setup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_onpowerup_0_evt_cb);


    INIT_CUSTOM_TMALL_SERVER_MODEL(tmall_model_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_tmall_0_evt_cb);
    INIT_CUSTOM_TMALL_CLIENT_MODEL(tmall_model_client_0, APP_KEY_MAX, 0, NULL, 0);

    INIT_LIGHT_HSL_SERVER_MODEL(light_hsl_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_hsl_0_evt_cb);
    INIT_LIGHT_HSL_SETUP_SERVER_MODEL(light_hsl_setup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_hsl_0_evt_cb);
    INIT_LIGHT_HSL_HUE_SERVER_MODEL(light_hsl_hue_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_hsl_0_evt_cb);
    INIT_LIGHT_HSL_SATURATION_SERVER_MODEL(light_hsl_saturation_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_hsl_0_evt_cb);

    INIT_LIGHT_CTL_SERVER_MODEL(light_ctl_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_ctl_0_evt_cb);
    INIT_LIGHT_CTL_TEMPERATURE_SERVER_MODEL(light_ctl_temperature_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_ctl_0_evt_cb);
    INIT_LIGHT_CTL_SETUP_SERVER_MODEL(light_ctl_setup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_ctl_setup_0_evt_cb);

    INIT_LIGHT_LIGHTNESS_SERVER_MODEL(light_lightness_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_lightness_0_evt_cb);
    INIT_LIGHT_LIGHTNESS_SETUP_SERVER_MODEL(light_lightness_setup_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_lightness_setup_0_evt_cb);

    INIT_GENERIC_TRANSITION_SERVER_MODEL(generic_transition_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, NULL);

    INIT_GENERIC_LEVEL_SERVER_MODEL(generic_level_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_level_0_evt_cb);

    INIT_GENERIC_ONOFF_SERVER_MODEL(generic_onoff_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_onoff_0_evt_cb);

    INIT_HEALTH_SERVER_MODEL(health_server_0, APP_KEY_MAX, 0, sub_addr_list, SUB_ADDR_MAX, user_health_attention_0_evt_cb);

    //3.set initial state
    generic_onpowerup_server_0.msg_format = generic_onpowerup_setup_server_0.msg_format = &onpowerup_msg[0];
    light_lightness_server_0.msg_format = light_lightness_setup_server_0.msg_format = &lightness_msg_format;
    light_hsl_server_0.msg_format = light_hsl_setup_server_0.msg_format = light_hsl_hue_server_0.msg_format = light_hsl_saturation_server_0.msg_format = &hsl_msg_format;
    scene_server_0.msg_format = scene_setup_server_0.msg_format = &scene_msg;
    light_ctl_server_0.msg_format = light_ctl_setup_server_0.msg_format = light_ctl_temperature_server_0.msg_format = &ctl_msg_format;

    health_server_0.company_id = MESH_PARAM_CID;

    generic_onpowerup_server_0.msg_format->onpowerup = 2;

    generic_level_server_0.msg_format.present_level = 0x8000;
    generic_onoff_server_0.msg_format.present_onoff = (generic_level_server_0.msg_format.present_level == 0x8000) ? 0 : 1;
 
    generic_level_server_0.msg_format.delta_last_tid = -1;
    light_lightness_server_0.msg_format->present_lightness_actual = generic_level_server_0.msg_format.present_level + 32768;
    light_lightness_server_0.msg_format->present_lightness_linear = (light_lightness_server_0.msg_format->present_lightness_actual * light_lightness_server_0.msg_format->present_lightness_actual)/65535;
    light_lightness_server_0.msg_format->lightness_last = light_lightness_server_0.msg_format->present_lightness_actual;
    light_lightness_setup_server_0.msg_format->lightness_range_min = 0x2000;
    light_lightness_setup_server_0.msg_format->lightness_range_max = 0xffff;
    light_lightness_setup_server_0.msg_format->status_code = 0;
    light_lightness_setup_server_0.msg_format->lightness_default = light_lightness_server_0.msg_format->present_lightness_actual;
    light_hsl_server_0.msg_format->present_hsl_lightness = light_lightness_server_0.msg_format->present_lightness_actual;
    light_hsl_server_0.msg_format->present_hsl_hue = light_lightness_server_0.msg_format->present_lightness_actual;
    light_hsl_server_0.msg_format->present_hsl_saturation = light_lightness_server_0.msg_format->present_lightness_actual;

    light_hsl_hue_server_0.msg_format->hue_range_min = 0;
    light_hsl_hue_server_0.msg_format->hue_range_max = 0xffff;
    light_hsl_saturation_server_0.msg_format->saturation_range_min = 0;
    light_hsl_saturation_server_0.msg_format->saturation_range_max = 0xffff;

    LOG(3, "powerup:%x lightness:%x\n", generic_onpowerup_server_0.msg_format->onpowerup, light_lightness_server_0.msg_format->present_lightness_actual);
    light_ctl_server_0.msg_format->present_ctl_lightness = light_lightness_server_0.msg_format->present_lightness_actual;
    light_ctl_server_0.msg_format->present_ctl_temperature = (T_MAX+T_MIN)/2;
    light_ctl_server_0.msg_format->present_ctl_delta_uv = 0;
    light_ctl_server_0.msg_format->status_code = 0;
    light_ctl_server_0.msg_format->range_min = T_MIN;
    light_ctl_server_0.msg_format->range_max = T_MAX;

    model_common_state_bound_field_set(0, MODEL_STATE_BOUND_LIGHT_LIGHTNESS, hal_set_hsl_led);
    model_common_state_bound_leds_num_set(0, BX_DONGLE_LED1_R, BX_DONGLE_LED1_G, BX_DONGLE_LED1_B);

    model_common_state_bound_field_set(1, MODEL_STATE_BOUND_LIGHT_CTL, hal_set_hsl_led);
    model_common_state_bound_leds_num_set(1, BX_DONGLE_LED2_R, BX_DONGLE_LED2_G, BX_DONGLE_LED2_B);

    generic_onoff_server_0.state_bound 
        = generic_onpowerup_server_0.state_bound 
        = generic_level_server_0.state_bound 
        = light_lightness_server_0.state_bound 
        = light_hsl_server_0.state_bound 
        = model_common_state_bound_get_from_element_id(0);

    generic_onoff_server_0.delay_trans_timer
        = generic_level_server_0.delay_trans_timer
        = light_lightness_server_0.delay_trans_timer
        = light_hsl_server_0.delay_trans_timer
        = light_hsl_saturation_server_0.delay_trans_timer
        = light_hsl_hue_server_0.delay_trans_timer
        = scene_server_0.delay_trans_timer 
        = scene_setup_server_0.delay_trans_timer 
        = model_common_delay_trans_timer_get_from_element_id(0);


    light_ctl_server_0.state_bound = model_common_state_bound_get_from_element_id(1);
    light_ctl_server_0.delay_trans_timer = model_common_delay_trans_timer_get_from_element_id(1);

    //4.Register config server event callbacks
    regisite_config_server_evt_cb(user_config_server_evt_cb);
}

void user_reset_timer_callback(mesh_timer_t xTimer)
{
    node_save_system_onoffcount(0);
    node_save_write_through();
    LOG(3, "user_reset_timer_callback onoffcount 0\n");

    delete_reset_timer();
}

static void scan_stop_then_delete_mesh_dir(void)
{
    node_delete_mesh_dir();
}


void user_model_parameter_init(void)
{
    uint8_t onoffcount = node_recover_system_onoffcount();
    node_recover_element_onpowerup();

    LOG(3, "%s onpowerup:%d onoffcount:%d\n", __func__, generic_onpowerup_server_0.msg_format->onpowerup, onoffcount);
#if 0
    if(generic_onpowerup_server_0.msg_format->onpowerup == 0) {
        generic_onoff_server_0.msg_format.present_onoff = 0x0;
        status_switch_onoff_to_level(&generic_onoff_server_0, &generic_level_server_0, light_lightness_server_0.msg_format->lightness_last);
        status_switch_onoff_to_lightness(&generic_onoff_server_0, &light_lightness_server_0);
        status_switch_onoff_to_hsl(&generic_onoff_server_0, &light_hsl_server_0, light_lightness_server_0.msg_format->lightness_last);
    }else if(generic_onpowerup_server_0.msg_format->onpowerup == 1){
        if(light_hsl_server_0.msg_format->present_hsl_lightness) {
            status_switch_hsl_to_onoff(&light_hsl_server_0, &generic_onoff_server_0);
            status_switch_hsl_to_level(&light_hsl_server_0, &generic_level_server_0);
            status_switch_hsl_to_lightness(&light_hsl_server_0, &light_lightness_server_0);
        }else {
            generic_onoff_server_0.msg_format.present_onoff = 0x1;
            status_switch_onoff_to_level(&generic_onoff_server_0, &generic_level_server_0, light_lightness_server_0.msg_format->lightness_last);
            status_switch_onoff_to_lightness(&generic_onoff_server_0, &light_lightness_server_0);
            status_switch_onoff_to_hsl(&generic_onoff_server_0, &light_hsl_server_0, light_lightness_server_0.msg_format->lightness_last);
        }
    }else {
    	generic_onoff_server_0.msg_format.target_onoff = generic_onoff_server_0.msg_format.present_onoff;
        status_switch_onoff_to_level(&generic_onoff_server_0, &generic_level_server_0, light_lightness_server_0.msg_format->lightness_last);
        status_switch_onoff_to_lightness(&generic_onoff_server_0, &light_lightness_server_0);
        status_switch_onoff_to_hsl(&generic_onoff_server_0, &light_hsl_server_0, light_lightness_server_0.msg_format->lightness_last);
    }

    if(generic_onoff_server_0.msg_format.present_onoff == 0) {
        light_hsl_server_0.msg_format->target_hsl_lightness = light_hsl_server_0.msg_format->present_hsl_lightness = 0xffff;
    }

    hsl_processing_and_state_bound(&light_hsl_server_0);
#endif

    status_switch_onoff_to_ctl(&generic_onoff_server_0, &light_ctl_server_0, light_ctl_server_0.msg_format->present_ctl_lightness);
    if(generic_onoff_server_0.msg_format.present_onoff == 0) {
        light_ctl_server_0.msg_format->target_ctl_lightness = light_ctl_server_0.msg_format->present_ctl_lightness = 0xffff;
        light_ctl_server_0.msg_format->target_ctl_temperature = light_ctl_server_0.msg_format->present_ctl_temperature = (T_MAX+T_MIN)/2;
    }
    ctl_processing_and_state_bound(&light_ctl_server_0);

    if(generic_onoff_server_0.msg_format.present_onoff == 0) {
        generic_onoff_server_0.msg_format.present_onoff = 1;
    }
    status_switch_ctl_to_lightness(&light_ctl_server_0, &light_lightness_server_0);
    model_status_publish();

    if(onoffcount == 0xf0) {
        node_save_system_onoffcount(0);
        node_save_write_through();
        beacon_start();
        user_onoff_timer_init();
    }else if(onoffcount >= ONOFF_NETWORK_ACCESS_STATE_COUNT - 1) {
        node_save_system_onoffcount(0xf0);
        node_save_write_through();
        mesh_sched_stop_scan(scan_stop_then_delete_mesh_dir);
        //mesh_core_system_set(MESH_CORE_SYSTEM_ALL_RESET);
    }else {
        node_save_system_onoffcount(++onoffcount);
        node_save_write_through();
        if(!get_is_provisioned())
            beacon_start();
        user_reset_timer_init();
    }

}

//init user models
void mesh_node_setup(void)
{
    //system init node-element-model tree
    mesh_core_params_t param;
    param.role = MESH_ROLE_CONFIG_SERVER;
    mesh_core_params_set(MESH_CORE_PARAM_MESH_ROLES , &param);
    //init user model list
    mesh_app_server_init();
}

