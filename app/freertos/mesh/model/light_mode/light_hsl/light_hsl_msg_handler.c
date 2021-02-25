#include "light_hsl_common.h"
#include "light_hsl_hue_server.h"
#include "light_hsl_saturation_server.h"
#include "light_hsl_setup_server.h"
#include "light_hsl_msg_handler.h"

#if (MESH_MODEL_LIGHT_HSL_SERVER || MESH_MODEL_LIGHT_HSL_HUE_SERVER || MESH_MODEL_LIGHT_HSL_SATURATION_SERVER || MESH_MODEL_LIGHT_HSL_SETUP_SERVER || MESH_MODEL_LIGHT_HSL_CLIENT)
msg_handler_model_t light_HSL_model_msg[Light_HSL_Opcode_MAX]=
{
#if MESH_MODEL_LIGHT_HSL_SERVER
    [Light_HSL_Get] = {light_HSL_get_rx, LIGHT_HSL_SERVER_MODEL_ID},

    [Light_HSL_Set] = {light_HSL_set_rx, LIGHT_HSL_SERVER_MODEL_ID},
    [Light_HSL_Set_Unacknowledged] = {light_HSL_set_unacknowledged_rx, LIGHT_HSL_SERVER_MODEL_ID},

    [Light_HSL_Target_Get] = {light_HSL_target_get_rx, LIGHT_HSL_SERVER_MODEL_ID},

#endif /* MESH_MODEL_LIGHT_HSL_SERVER */
#if MESH_MODEL_LIGHT_HSL_HUE_SERVER
    [Light_HSL_Hue_Get] = {light_HSL_hue_get_rx, LIGHT_HSL_HUE_SERVER_MODEL_ID},
    [Light_HSL_Hue_Set] = {light_HSL_hue_set_rx, LIGHT_HSL_HUE_SERVER_MODEL_ID},
    [Light_HSL_Hue_Set_Unacknowledged] = {light_HSL_hue_set_unacknowledged_rx, LIGHT_HSL_HUE_SERVER_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_HSL_HUE_SERVER */
#if MESH_MODEL_LIGHT_HSL_SATURATION_SERVER
    [Light_HSL_Saturation_Get] = {light_HSL_saturation_get_rx, LIGHT_HSL_SATURATION_SERVER_MODEL_ID},
    [Light_HSL_Saturation_Set] = {light_HSL_saturation_set_rx, LIGHT_HSL_SATURATION_SERVER_MODEL_ID},
    [Light_HSL_Saturation_Set_Unacknowledged] = {light_HSL_saturation_set_unacknowledged_rx, LIGHT_HSL_SATURATION_SERVER_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_HSL_SATURATION_SERVER */
#if MESH_MODEL_LIGHT_HSL_SETUP_SERVER
    [Light_HSL_Default_Get] = {light_HSL_default_get_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},

    [Light_HSL_Range_Get] = {light_HSL_Range_get_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},

    [Light_HSL_Default_Set] = {light_HSL_Default_set_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},
    [Light_HSL_Default_Set_Unacknowledged] = {light_HSL_Default_set_unacknowledged_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},

    [Light_HSL_Range_Set] = {light_HSL_Range_set_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},
    [Light_HSL_Range_Set_Unacknowledged] = {light_HSL_Range_set_unacknowledged_rx, LIGHT_HSL_SETUP_SERVER_MODEL_ID},
#endif /*MESH_MODEL_LIGHT_HSL_SETUP_SERVER */
#if MESH_MODEL_LIGHT_HSL_CLIENT
    [Light_HSL_Status] = {light_HSL_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
    [Light_HSL_Hue_Status] = {light_HSL_hue_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
    [Light_HSL_Saturation_Status] = {light_HSL_saturation_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
    [Light_HSL_Target_Status] = {light_HSL_target_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
    [Light_HSL_Default_Status] = {light_HSL_default_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
    [Light_HSL_Range_Status] = {light_HSL_Range_status_rx, LIGHT_HSL_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_LIGHT_HSL_CLIENT */
};
#endif /* MESH_MODEL_LIGHT_HSL_SERVER || MESH_MODEL_LIGHT_HSL_HUE_SERVER || MESH_MODEL_LIGHT_HSL_SATURATION_SERVER || MESH_MODEL_LIGHT_HSL_SETUP_SERVER || MESH_MODEL_LIGHT_HSL_CLIENT */
