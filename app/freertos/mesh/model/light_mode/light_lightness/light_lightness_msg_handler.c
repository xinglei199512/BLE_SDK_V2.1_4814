
#include "light_lightness_common.h"
#include "light_lightness_setup_server.h"
#include "light_lightness_msg_handler.h"

#if (MESH_MODEL_LIGHT_LIGHTNESS_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_CLIENT)
msg_handler_model_t light_lightness_model_msg[Light_Lightness_Opcode_MAX]=
{
#if MESH_MODEL_LIGHT_LIGHTNESS_SERVER
    [Light_Lightness_Get] = {light_lightness_get_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},
    [Light_Lightness_Set] = {light_lightness_set_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},
    [Light_Lightness_Set_Unacknowledged] = {light_lightness_set_unacknowledged_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},

    [Light_Lightness_Linear_Get] = {light_lightness_Linear_get_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},
    [Light_Lightness_Linear_Set] = {light_lightness_Linear_set_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},
    [Light_Lightness_Linear_Set_Unacknowledged] = {light_lightness_Linear_set_unacknowledged_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},

    [Light_Lightness_Last_Get] = {light_lightness_Last_get_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},

    [Light_Lightness_Default_Get] = {light_lightness_Default_get_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},

    [Light_Lightness_Range_Get] = {light_lightness_Range_get_rx, LIGHT_LIGHTNESS_SERVER_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_LIGHTNESS_SERVER */
#if MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER
    [Light_Lightness_Default_Set] = {light_lightness_Default_set_rx, LIGHT_LIGHTNESS_SETUP_SERVER_MODEL_ID},
    [Light_Lightness_Default_Set_Unacknowledged] = {light_lightness_Default_set_unacknowledged_rx, LIGHT_LIGHTNESS_SETUP_SERVER_MODEL_ID},
    [Light_Lightness_Range_Set] = {light_lightness_Range_set_rx, LIGHT_LIGHTNESS_SETUP_SERVER_MODEL_ID},
    [Light_Lightness_Range_Set_Unacknowledged] = {light_lightness_Range_set_unacknowledged_rx, LIGHT_LIGHTNESS_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER */
#if MESH_MODEL_LIGHT_LIGHTNESS_CLIENT
    [Light_Lightness_Status] = {light_lightness_status_rx, LIGHT_LIGHTNESS_CLIENT_MODEL_ID},
    [Light_Lightness_Linear_Status] = {light_lightness_Linear_status_rx, LIGHT_LIGHTNESS_CLIENT_MODEL_ID},
    [Light_Lightness_Default_Status] = {light_lightness_Default_status_rx, LIGHT_LIGHTNESS_CLIENT_MODEL_ID},
    [Light_Lightness_Last_Status] = {light_lightness_Last_status_rx, LIGHT_LIGHTNESS_CLIENT_MODEL_ID},
    [Light_Lightness_Range_Status] = {light_lightness_Range_status_rx, LIGHT_LIGHTNESS_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_LIGHTNESS_CLIENT */
};
#endif /* MESH_MODEL_LIGHT_LIGHTNESS_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_CLIENT */
