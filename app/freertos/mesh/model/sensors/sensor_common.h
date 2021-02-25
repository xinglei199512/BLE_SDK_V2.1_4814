/**
 ****************************************************************************************
 *
 * @file   sensor_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2019-02-25 11:06
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_sensor_common_API Mesh sensor_common API
 * @ingroup MESH_API
 * @brief Mesh sensor_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

#define SENSOR_NUM 1
#define DATA_LEN 2

typedef struct {
    uint16_t property_id;
}__attribute((packed))sensor_descriptor_t;

typedef struct {
    uint16_t descriptor[SENSOR_NUM];
}__attribute((packed))sensor_descriptor_status_t;

typedef struct {
    uint16_t property_id;
    uint8_t  fast_cadence_period_divisor:7,
             status_trigger_type:1;
    uint8_t status_trigger_delta_down[DATA_LEN];
    uint8_t status_trigger_delta_up[DATA_LEN];
    uint8_t status_min_interval;
    uint8_t fast_cadence_low[DATA_LEN];
    uint8_t fast_cadence_high[DATA_LEN];
}__attribute((packed))sensor_cadence_set_t;

typedef sensor_cadence_set_t sensor_cadence_status_t;


typedef struct {
    uint16_t sensor_property_id;
}__attribute((packed))sensor_settings_get_t;

typedef struct {
    uint16_t sensor_property_id;
    uint16_t sensor_setting_property_ids[DATA_LEN];
}__attribute((packed))sensor_settings_status_t;

typedef struct {
    uint16_t sensor_property_id;
    uint16_t sensor_setting_property_id;
}__attribute((packed))sensor_setting_get_t;

typedef struct {
    uint16_t sensor_property_id;
    uint16_t sensor_setting_property_id;
    uint8_t sensor_setting_raw[DATA_LEN];
}__attribute((packed))sensor_setting_set_t;

typedef struct {
    uint16_t sensor_property_id;
    uint16_t sensor_setting_property_id;
    uint8_t sensor_setting_access;
    uint8_t sensor_setting_raw[DATA_LEN];
}__attribute((packed))sensor_setting_status_t;

typedef struct {
    uint16_t sensor_property_id;
    uint16_t sensor_setting_property_id;
}__attribute((packed))sensor_setting_status_default_t;


typedef struct {
    uint16_t property_id;
}__attribute((packed))sensor_get_t;

typedef struct {
    uint8_t marshalled_sensor_data;
}__attribute((packed))sensor_status_t;

typedef struct {
    uint16_t property_id;
    uint8_t raw_value_x[DATA_LEN];
}__attribute((packed))sensor_column_get_t;

typedef sensor_column_get_t sensor_column_status_default_t;

typedef struct {
    uint16_t property_id;
    uint8_t raw_value_x[DATA_LEN];
    uint8_t column_width[DATA_LEN];
}__attribute((packed))sensor_column_status_t;

typedef struct {
    uint16_t property_id;
    uint8_t raw_value_x1[DATA_LEN];
    uint8_t raw_value_x2[DATA_LEN];
}__attribute((packed))sensor_series_get_default_t;

typedef struct {
    uint16_t property_id;
}__attribute((packed))sensor_series_get_t;

typedef struct {
    uint16_t property_id;
    uint8_t raw_value_x[DATA_LEN];
    uint8_t column_width[DATA_LEN];
    uint8_t raw_value_y[DATA_LEN];
}__attribute((packed))sensor_series_status_t;

typedef sensor_series_get_t sensor_series_status_default_t;
#endif /* APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_COMMON_H_ */ 
/// @} MESH_sensor_common_API

