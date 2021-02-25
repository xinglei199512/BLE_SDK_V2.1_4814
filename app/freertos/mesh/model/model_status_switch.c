/**
 ****************************************************************************************
 *
 * @file   model_status_switch.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-11 09:34
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

#include "model_status_switch.h"

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

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
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
void status_switch_onoff_to_level(generic_onoff_server_t *onoff_server, generic_level_server_t *level_server, uint16_t lightness_last)
{
    if(onoff_server && level_server) {
        if(onoff_server->msg_format.present_onoff == 1) {
            level_server->msg_format.present_level = lightness_last - 32768;
        }else {
            level_server->msg_format.present_level = 0x8000;
        }
    }
}

void status_switch_onoff_to_lightness(generic_onoff_server_t *onoff_server, light_lightness_server_t *lightness_server)
{
    if(onoff_server && lightness_server) {
        if(onoff_server->msg_format.present_onoff == 1) {
            lightness_server->msg_format->present_lightness_actual = lightness_server->msg_format->lightness_last;
        }else {
            lightness_server->msg_format->present_lightness_actual = 0;
        }
        lightness_server->msg_format->present_lightness_linear = (lightness_server->msg_format->present_lightness_actual * lightness_server->msg_format->present_lightness_actual)/65535;
    }
}
void status_switch_onoff_to_ctl(generic_onoff_server_t *onoff_server, light_ctl_server_t *ctl_server, uint16_t lightness_last)
{
    if(onoff_server && ctl_server) {
        if(onoff_server->msg_format.present_onoff == 1) { ctl_server->msg_format->present_ctl_lightness = lightness_last;
        }else {
            ctl_server->msg_format->present_ctl_lightness = 0;
        }
    }
}
void status_switch_onoff_to_hsl(generic_onoff_server_t *onoff_server, light_hsl_server_t *hsl_server, uint16_t lightness_last)
{
    if(onoff_server && hsl_server) {
        if(onoff_server->msg_format.present_onoff == 1) {
            hsl_server->msg_format->present_hsl_lightness = lightness_last;
        }else {
            hsl_server->msg_format->present_hsl_lightness = 0;
        }
    }
}

void status_switch_level_to_onoff(generic_level_server_t *level_server, generic_onoff_server_t *onoff_server)
{
    if(level_server && onoff_server) {
        if(level_server->msg_format.present_level == 0x8000) {
            onoff_server->msg_format.present_onoff = 0;
        }else {
            onoff_server->msg_format.present_onoff = 1;
        }
    }
}
void status_switch_level_to_lightness(generic_level_server_t *level_server, light_lightness_server_t *lightness_server)
{
    if(level_server && lightness_server) {
        lightness_server->msg_format->present_lightness_actual = level_server->msg_format.present_level + 32768;
        lightness_server->msg_format->present_lightness_linear = (lightness_server->msg_format->present_lightness_actual * lightness_server->msg_format->present_lightness_actual)/65535;

        if(lightness_server->msg_format->present_lightness_actual)
            lightness_server->msg_format->lightness_last = lightness_server->msg_format->present_lightness_actual;
    }
}
void status_switch_level_to_ctl(generic_level_server_t *level_server, light_ctl_server_t *ctl_server)
{
    if(level_server && ctl_server) {
        ctl_server->msg_format->present_ctl_lightness = level_server->msg_format.present_level + 32768;
#if 0
        ctl_server->msg_format->present_ctl_temperature = T_MIN + (level_server->msg_format.present_level + 32768) * (T_MAX - T_MIN)/65535;
#endif
    }
}
void status_switch_level_to_hsl(generic_level_server_t *level_server, light_hsl_server_t *hsl_server)
{
    if(level_server && hsl_server) {
        hsl_server->msg_format->present_hsl_lightness = level_server->msg_format.present_level + 32768;
#if 0
        hsl_server->msg_format->present_hsl_hue = level_server->msg_format.present_level + 32768;
        hsl_server->msg_format->present_hsl_saturation = level_server->msg_format.present_level + 32768;
#endif
    }
}

void status_switch_lightness_to_onoff(light_lightness_server_t *lightness_server, generic_onoff_server_t *onoff_server)
{
    if(lightness_server && onoff_server) {
        onoff_server->msg_format.present_onoff = lightness_server->msg_format->present_lightness_actual > 0 ? 1 : 0;
    }
}

void status_switch_lightness_to_level(light_lightness_server_t *lightness_server, generic_level_server_t *level_server)
{
    if(lightness_server && level_server) {
        level_server->msg_format.present_level = lightness_server->msg_format->present_lightness_actual - 32768;
    }
}
void status_switch_lightness_to_ctl(light_lightness_server_t *lightness_server, light_ctl_server_t *ctl_server)
{
    if(lightness_server && ctl_server) {
        ctl_server->msg_format->present_ctl_lightness = lightness_server->msg_format->present_lightness_actual;
    }
}
void status_switch_lightness_to_hsl(light_lightness_server_t *lightness_server, light_hsl_server_t *hsl_server)
{
    if(lightness_server && hsl_server) {
        hsl_server->msg_format->present_hsl_lightness = lightness_server->msg_format->present_lightness_actual;
    }
}


void status_switch_ctl_to_onoff(light_ctl_server_t *ctl_server, generic_onoff_server_t *onoff_server)
{
    if(ctl_server && onoff_server) {
        onoff_server->msg_format.present_onoff = ctl_server->msg_format->present_ctl_lightness > 0 ? 1 : 0;
    }
}
void status_switch_ctl_to_level(light_ctl_server_t *ctl_server, generic_level_server_t *level_server)
{
    if(ctl_server && level_server) {
        level_server->msg_format.present_level = ctl_server->msg_format->present_ctl_lightness - 32768;
    }
}
void status_switch_ctl_to_lightness(light_ctl_server_t *ctl_server, light_lightness_server_t *lightness_server)
{
    if(ctl_server && lightness_server) {
        lightness_server->msg_format->present_lightness_actual = ctl_server->msg_format->present_ctl_lightness;
        lightness_server->msg_format->present_lightness_linear = (lightness_server->msg_format->present_lightness_actual * lightness_server->msg_format->present_lightness_actual)/65535;

        if(lightness_server->msg_format->present_lightness_actual)
            lightness_server->msg_format->lightness_last = lightness_server->msg_format->present_lightness_actual;
    }
}
void status_switch_ctl_to_hsl(light_ctl_server_t *ctl_server, light_hsl_server_t *hsl_server)
{
    if(ctl_server && hsl_server) {
        hsl_server->msg_format->present_hsl_lightness = ctl_server->msg_format->present_ctl_lightness;
    }
}

void status_switch_hsl_to_onoff(light_hsl_server_t *hsl_server, generic_onoff_server_t *onoff_server)
{
    if(hsl_server && onoff_server) {
        onoff_server->msg_format.present_onoff = hsl_server->msg_format->present_hsl_lightness > 0 ? 1 : 0;
    }
}
void status_switch_hsl_to_level(light_hsl_server_t *hsl_server, generic_level_server_t *level_server)
{
    if(hsl_server && level_server) {
        level_server->msg_format.present_level = hsl_server->msg_format->present_hsl_lightness - 32768;
    }
}
void status_switch_hsl_to_lightness(light_hsl_server_t *hsl_server, light_lightness_server_t *lightness_server)
{
    if(hsl_server && lightness_server) {
        lightness_server->msg_format->present_lightness_actual = hsl_server->msg_format->present_hsl_lightness;
        lightness_server->msg_format->present_lightness_linear = (lightness_server->msg_format->present_lightness_actual * lightness_server->msg_format->present_lightness_actual)/65535;

        if(lightness_server->msg_format->present_lightness_actual)
            lightness_server->msg_format->lightness_last = lightness_server->msg_format->present_lightness_actual;
    }
}
void status_switch_hsl_to_ctl(light_hsl_server_t *hsl_server, light_ctl_server_t *ctl_server)
{
    if(hsl_server && ctl_server) {
        ctl_server->msg_format->present_ctl_lightness = hsl_server->msg_format->present_hsl_lightness;
    }
}
