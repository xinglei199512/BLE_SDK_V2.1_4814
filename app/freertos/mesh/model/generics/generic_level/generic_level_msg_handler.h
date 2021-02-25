#ifndef GENERIC_LEVEL_MSG_HANDLER__H
#define GENERIC_LEVEL_MSG_HANDLER__H
#include "generic_level_server.h"
#include "generic_level_client.h"
#include "model_msg.h"

#define GENERIC_LEVEL_SERVER_MODEL_ID 0x1002
#define GENERIC_LEVEL_CLIENT_MODEL_ID 0x1003
#define GENERIC_LEVEL_OPCODE_OFFSET 0x205
enum Generic_Level_Two_Octets_Opcode
{
    Generic_Level_Get,
    Generic_Level_Set,
    Generic_Level_Set_Unacknowledged,
    Generic_Level_Status,
    Generic_Delta_Set,
    Generic_Delta_Set_Unacknowledged,
    Generic_Move_Set,
    Generic_Move_Set_Unacknowledged,
    Generic_Level_Opcode_Max,
};

extern msg_handler_model_t generic_level_model_msg[Generic_Level_Opcode_Max];
#endif /* GENERIC_LEVEL_MSG_HANDLER__H */
