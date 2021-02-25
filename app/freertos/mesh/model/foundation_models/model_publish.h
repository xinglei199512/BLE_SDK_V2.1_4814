/**
 ****************************************************************************************
 *
 * @file   model_publish.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2018-11-02 09:32
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_model_publish_API Mesh model_publish API
 * @ingroup MESH_API
 * @brief Mesh model_publish  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_MODEL_FOUNDATION_MODELS_MODEL_PUBLISH_H_
#define FREERTOS_APP_MESH_MODEL_FOUNDATION_MODELS_MODEL_PUBLISH_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define MESH_PUBLISH_COUNTER_TICK   100               // unit: ms



/*
 * ENUMERATIONS
 ****************************************************************************************
 */

typedef enum
{
    /** Step resolution: 100ms / step. */
    MODEL_PUBLISH_RESOLUTION_100MS = 0,
    /** Step resolution: 1s / step. */
    MODEL_PUBLISH_RESOLUTION_1S    = 1,
    /** Step resolution: 10s / step. */
    MODEL_PUBLISH_RESOLUTION_10S   = 2,
    /** Step resolution: 10min / step. */
    MODEL_PUBLISH_RESOLUTION_10MIN = 3,
    /** Maximum publish resolution. */
    MODEL_PUBLISH_RESOLUTION_MAX = MODEL_PUBLISH_RESOLUTION_10MIN
} model_publish_resolution_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
 typedef uint32_t publish_timestamp_t;

 typedef struct _model_publish_list_t
 {
     model_publish_state_t * p_pubstate;
     uint32_t target;
     void * cookie;
     model_publish_timeout_cb_t cb;
     struct _model_publish_list_t * p_next;
 }model_publish_list_t;



/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void mesh_model_publish_init(void);
void model_publish_period_set(model_publish_state_t * p_pubstate, model_publish_timeout_cb_t cb, void * cookie);



#endif /* FREERTOS_APP_MESH_MODEL_FOUNDATION_MODELS_MODEL_PUBLISH_H_ */ 
/// @} MESH_model_publish_API

