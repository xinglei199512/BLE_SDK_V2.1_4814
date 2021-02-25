
#include "time_common.h"
#include "time_msg_handler.h"
#include "time_server.h"
#include "scheduler_server.h"
#include "scene_server.h"

#if (MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_MODEL_CLIENT || MESH_MODEL_SCENE_MODEL_CLIENT || MESH_MODEL_SCHEDULER_CLIENT || MESH_MODEL_SCHEDULER_SETUP_SERVER)
msg_handler_model_t time_and_scene_one_octet_model_msg[Time_And_Scene_One_Opcode_MAX]=
{
#if MESH_MODEL_TIME_SETUP_SERVER
    [Time_Set] = {time_set_rx, TIME_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_TIME_SETUP_SERVER */
#if MESH_MODEL_TIME_MODEL_CLIENT
    [Time_Status] = {time_status_rx, TIME_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_TIME_MODEL_CLIENT */
#if MESH_MODEL_SCENE_MODEL_CLIENT
    [Scene_Status] = {scene_status_rx, SCENE_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_SCENE_MODEL_CLIENT */
#if MESH_MODEL_SCHEDULER_CLIENT
    [Scheduler_Action_Status] = {scheduler_action_status_rx, SCHEDULER_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_SCHEDULER_CLIENT */
#if MESH_MODEL_SCHEDULER_SETUP_SERVER
    [Scheduler_Action_Set] = {scheduler_action_set_rx, SCHEDULER_SETUP_SERVER_MODEL_ID},
    [Scheduler_Action_Set_Unacknowledged] = {scheduler_action_set_unacknowledged_rx, SCHEDULER_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_SCHEDULER_SETUP_SERVER */
};
#endif /* MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_MODEL_CLIENT || MESH_MODEL_SCENE_MODEL_CLIENT || MESH_MODEL_SCHEDULER_CLIENT || MESH_MODEL_SCHEDULER_SETUP_SERVER */

#if (MESH_MODEL_TIME_SERVER || MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_CLIENT || MESH_MODEL_SCENE_SERVER || MESH_MODEL_SCENE_SETUP_SERVER || MESH_MODEL_SCENE_CLIENT || MESH_MODEL_SCHEDULER_SERVER || MESH_MODEL_SCHEDULER_CLIENT)
msg_handler_model_t time_and_scene_two_octet_model_msg[Time_And_Scene_Two_Opcode_MAX]=
{
#if MESH_MODEL_TIME_SERVER
    [Time_Get] = {time_get_rx, TIME_SERVER_MODEL_ID},
    [Time_Zone_Get] = {time_zone_get_rx, TIME_SERVER_MODEL_ID},
    [TAI_UTC_Delta_Get] = {tai_utc_delta_get_rx, TIME_SERVER_MODEL_ID},
#endif /*MESH_MODEL_TIME_SERVER */
#if MESH_MODEL_TIME_SETUP_SERVER
    [Time_Role_Get] = {time_role_get_rx, TIME_SETUP_SERVER_MODEL_ID},
    [Time_Role_Set] = {time_role_set_rx, TIME_SETUP_SERVER_MODEL_ID},
    [Time_Zone_Set] = {time_zone_set_rx, TIME_SETUP_SERVER_MODEL_ID},
    [TAI_UTC_Delta_Set] = {tai_utc_delta_set_rx, TIME_SETUP_SERVER_MODEL_ID},
#endif /*MESH_MODEL_TIME_SETUP_SERVER */
#if MESH_MODEL_TIME_CLIENT
    [Time_Zone_Status] = {time_zone_status_rx, TIME_CLIENT_MODEL_ID},
    [Time_Role_Status] = {time_role_status_rx, TIME_CLIENT_MODEL_ID},
    [TAI_UTC_Delta_Status] = {tai_utc_delta_status_rx, TIME_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_TIME_CLIENT */
#if MESH_MODEL_SCENE_SERVER
    [Scene_Get] = {scene_get_rx, SCENE_SERVER_MODEL_ID},
    [Scene_Recall] = {scene_recall_rx, SCENE_SERVER_MODEL_ID},
    [Scene_Recall_Unacknowledged] = {scene_recall_unacknowledged_rx, SCENE_SERVER_MODEL_ID},
    [Scene_Register_Get] = {scene_register_get_rx, SCENE_SERVER_MODEL_ID},
#endif /* MESH_MODEL_SCENE_SERVER */
#if MESH_MODEL_SCENE_SETUP_SERVER
    [Scene_Store] = {scene_store_rx, SCENE_SETUP_SERVER_MODEL_ID},
    [Scene_Store_Unacknowledged] = {scene_store_unacknowledged_rx, SCENE_SETUP_SERVER_MODEL_ID},
#endif /* MESH_MODEL_SCENE_SETUP_SERVER */
#if MESH_MODEL_SCENE_CLIENT
    [Scene_Register_Status] = {scene_register_status_rx, SCENE_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_SCENE_CLIENT */
#if MESH_MODEL_SCHEDULER_SERVER
    [Scheduler_Action_Get] = {scheduler_action_get_rx, SCHEDULER_SERVER_MODEL_ID},
    [Scheduler_Get] = {scheduler_get_rx, SCHEDULER_SERVER_MODEL_ID},
#endif /* MESH_MODEL_SCHEDULER_SERVER */
#if MESH_MODEL_SCHEDULER_CLIENT
    [Scheduler_Status] = {scheduler_Status_rx, SCHEDULER_CLIENT_MODEL_ID},
#endif /* MESH_MODEL_SCHEDULER_CLIENT */
};
#endif /*MESH_MODEL_TIME_SERVER || MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_CLIENT || MESH_MODEL_SCENE_SERVER || MESH_MODEL_SCENE_SETUP_SERVER || MESH_MODEL_SCENE_CLIENT || MESH_MODEL_SCHEDULER_SERVER || MESH_MODEL_SCHEDULER_CLIENT */

#if (MESH_MODEL_SCENE_SETUP_SERVER)
msg_handler_model_t scene_setup_two_octet_model_msg[Scene_Setup_Two_Opcode_MAX]=
{
#if MESH_MODEL_SCENE_SETUP_SERVER
    [Scene_Delete] = {scene_delete_rx, SCENE_SETUP_SERVER_MODEL_ID},
    [Scene_Delete_Unacknowledged] = {scene_delete_unacknowledged_rx, SCENE_SETUP_SERVER_MODEL_ID},
#endif
};
#endif /* MESH_MODEL_SCENE_SETUP_SERVER */
