
#ifndef APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_POWER_ONOFF_H_
#define APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_POWER_ONOFF_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "mesh_model.h"

enum {
    POWER_ONOFF_GENERIC_ONOFF = 0,
    POWER_ONOFF_GENERIC_POWER_ONOFF = 1,
    POWER_ONOFF_LIGHT_LIGHTNESS = 2,
    POWER_ONOFF_LIGHT_CTL = 3,
    POWER_ONOFF_LIGHT_HSL = 4
};

typedef struct {
    uint8_t type     :4;
    uint8_t onpowerup:4;
    uint8_t onoff;
    uint16_t lightness;
    uint16_t hue_or_temperature;
    uint16_t saturation_or_delta_uv;
}__attribute((packed))save_power_onoff_value_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void node_save_element_onpowerup(model_base_t *model , save_power_onoff_value_t *scene, uint8_t index);
void node_recover_element_onpowerup(void);
void node_delete_element_onpowerup(model_base_t *model , uint8_t index);
int node_search_element_onpowerup(uint8_t index, save_power_onoff_value_t *p_scene);


#endif /* APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCENE_H_ */ 
/// @} MESH_node_save_scene_API
