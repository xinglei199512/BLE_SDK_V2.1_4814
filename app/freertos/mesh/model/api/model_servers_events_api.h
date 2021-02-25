#ifndef FREERTOS_APP_MESH_MODEL_MODEL_EVENTS_API_H_
#define FREERTOS_APP_MESH_MODEL_MODEL_EVENTS_API_H_
/**
 ****************************************************************************************
 * @addtogroup MESH_MODEL_MODEL_SERVER_EVENTS
 * @ingroup  MESH_MODEL_API
 * @brief defines for BLE MESH MODEL API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**Enum of mesh onoff state define */
/**Mesh model event type. */
typedef enum
{
    ONOFF_MODEL_EVT_SET,
    ONOFF_MODEL_EVT_GET,
}mesh_onoff_model_evt_type_t;

typedef enum
{
    POWER_ONOFF_MODEL_EVT_SET,
    POWER_ONOFF_MODEL_EVT_GET,
}mesh_onpowerup_model_evt_type_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/* for level */
typedef enum
{
    LEVEL_MODEL_EVT_SET,
    LEVEL_MODEL_EVT_GET,
}mesh_level_model_evt_type_t;

/* for light_lightness */
typedef enum
{
    LIGHTNESS_LINEAR_EVT_SET,
    LIGHTNESS_LINEAR_EVT_GET,
    LIGHTNESS_ACTUAL_EVT_SET,
    LIGHTNESS_ACTUAL_EVT_GET,
    LIGHTNESS_DEFAULT_EVT_SET,
    LIGHTNESS_DEFAULT_EVT_GET,
}mesh_light_lightness_model_evt_type_t;

/* for light_ctl */
typedef enum
{
     LIGHT_CTL_SET,
     LIGHT_CTL_GET,
     LIGHT_CTL_TEMPERATURE_SET,
     LIGHT_CTL_TEMPERATURE_GET,
     LIGHT_CTL_DEFAULT_SET,
     LIGHT_CTL_DEFAULT_GET,
}mesh_light_ctl_model_evt_type_t;

/* for light_hsl */
typedef enum
{
     LIGHT_HSL_SET,
     LIGHT_HSL_GET,
     LIGHT_HSL_HUE_SET,
     LIGHT_HSL_HUE_GET,
     LIGHT_HSL_SATURATION_SET,
     LIGHT_HSL_SATURATION_GET,
     LIGHT_HSL_DEFAULT_SET,
     LIGHT_HSL_DEFAULT_GET,
}mesh_light_hsl_model_evt_type_t;

/* for time */
typedef enum
{
     TIME_TIME_EVT_SET,
     TIME_TIME_EVT_GET,
     TIME_TIME_ROLE_EVT_SET,
     TIME_TIME_ROLE_EVT_GET,
     TIME_TIME_ZONE_EVT_SET,
     TIME_TIME_ZONE_EVT_GET,
}mesh_time_model_evt_type_t;

/* for scene */
typedef enum
{
     TIME_SCENE_EVT_STORE,
     TIME_SCENE_EVT_RECALL,
     TIME_SCENE_EVT_GET,
     TIME_SCENE_EVT_REGISTER_GET,
     TIME_SCENE_EVT_DELETE,
}mesh_scene_model_evt_type_t;

/* for scheduler */
typedef enum
{
     TIME_SCHEDULER_EVT_GET,
     TIME_SCHEDULER_EVT_ACTION_SET,
     TIME_SCHEDULER_EVT_ACTION_GET,
}mesh_scheduler_model_evt_type_t;
typedef struct
{
    /** Target state. */
    uint16_t target_value;
} mesh_model_evt_set_t;

typedef struct
{
  uint16_t present_value;
} mesh_model_evt_get_t;
typedef struct
{
    /** Target state. */
    uint16_t target_value;
    uint8_t repeat_flag;
} mesh_light_scene_model_evt_set_t;
typedef struct
{
    /** Target state. */
    uint16_t target_value;
    uint8_t repeat_flag;
} mesh_light_scheduler_model_evt_set_t;

typedef struct
{
    /** Target state. */
    uint16_t target_ctl_lightness;
    uint16_t target_ctl_temperature;
    uint16_t target_ctl_delta_uv;
} mesh_light_ctl_model_evt_set_t;

typedef struct
{
    uint16_t present_ctl_lightness;
    uint16_t present_ctl_temperature;
    uint16_t present_ctl_delta_uv;
} mesh_light_ctl_model_evt_get_t;

typedef struct
{
    /** Target state. */
    uint16_t target_hsl_lightness;
    uint16_t target_hsl_hue;
    uint16_t target_hsl_saturation;
} mesh_light_hsl_model_evt_set_t;

typedef struct
{
    uint16_t present_hsl_lightness;
    uint16_t present_hsl_hue;
    uint16_t present_hsl_saturation;
} mesh_light_hsl_model_evt_get_t;
/** Mesh models event structure. */
typedef struct
{
    /** Type of event. */
    union
    {
        mesh_onoff_model_evt_type_t onoff_type;
        mesh_level_model_evt_type_t level_type;
        mesh_onpowerup_model_evt_type_t onpowerup_type;
        mesh_scene_model_evt_type_t scene_type;
        mesh_time_model_evt_type_t time_type;
        mesh_scheduler_model_evt_type_t scheduler_type;
        mesh_light_lightness_model_evt_type_t lightness_type;
        mesh_light_ctl_model_evt_type_t ctl_type;
        mesh_light_hsl_model_evt_type_t hsl_type;
    }type;
    /** Union of event parameters. */
    union
    {
        mesh_model_evt_set_t model_value_set;
        mesh_model_evt_get_t model_value_get;
        mesh_light_ctl_model_evt_set_t model_ctl_set;
        mesh_light_ctl_model_evt_get_t model_ctl_get;
        mesh_light_hsl_model_evt_set_t model_hsl_set;
        mesh_light_hsl_model_evt_get_t model_hsl_get;
        mesh_light_scene_model_evt_set_t model_scene_set;
        mesh_light_scheduler_model_evt_set_t model_scheduler_set;
    } params;
} mesh_model_evt_t;


/**
 * Mesh model event callback type.
 * @param[in] p_evt Event pointer from the configuration server.
 */
typedef void (*mesh_model_evt_cb_t)(const mesh_model_evt_t * p_evt);



/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/// @} MESH_MODEL_MODEL_SERVER_EVENTS
#endif /* FREERTOS_APP_MESH_MODEL_MODEL_EVENTS_API_H_ */
