#include "foundation_msg_handler.h"
#include "generic_onoff_msg_handler.h"
#include "generic_level_msg_handler.h"
#include "generic_power_onoff_msg_handler.h"
#include "generic_transition_msg_handler.h"
#include "light_lightness_msg_handler.h"
#include "light_ctl_msg_handler.h"
#include "light_hsl_msg_handler.h"
//#include "tc_model_msg_handler.h"

#include "time_msg_handler.h"
#include "vendor_models_msg_handler.h"
#include "co_endian.h"

static msg_map_t one_octet_msg_map[]=
{
    {fd_model_msg_one_opcode,             FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET, One_Octet_Opcode_Msg_Num_Max},
//    {sensor_model_msg_one_octet,          SENSOR_ONE_OCTET_OPCODE_OFFSET,            Sensor_One_Octet_Opcode_Max},
#if (MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_MODEL_CLIENT || MESH_MODEL_SCENE_MODEL_CLIENT || MESH_MODEL_SCHEDULER_CLIENT || MESH_MODEL_SCHEDULER_SETUP_SERVER)
    {time_and_scene_one_octet_model_msg,  TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET,    Time_And_Scene_One_Opcode_MAX},
#endif /* MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_MODEL_CLIENT || MESH_MODEL_SCENE_MODEL_CLIENT || MESH_MODEL_SCHEDULER_CLIENT || MESH_MODEL_SCHEDULER_SETUP_SERVER */
};

static msg_map_t two_octets_msg_map[]=
{
    {fd_model_msg_two_opcode,                   FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET,    Two_Octets_Opcode_Msg_Num_Max},
#if (MESH_MODEL_GENERIC_ONOFF_SERVER || MESH_MODEL_GENERIC_ONOFF_CLIENT)
    {generic_onoff_model_msg,                   GENERIC_ONOFF_OPCODE_OFFSET,                   Generic_OnOff_Opcode_Max},
#endif /* (MESH_MODEL_GENERIC_ONOFF_SERVER || MESH_MODEL_GENERIC_ONOFF_CLIENT) */
#if (MESH_MODEL_GENERIC_LEVEL_SERVER || MESH_MODEL_GENERIC_LEVEL_CLIENT)
    {generic_level_model_msg,                   GENERIC_LEVEL_OPCODE_OFFSET,                   Generic_Level_Opcode_Max},
#endif /* (MESH_MODEL_GENERIC_LEVEL_SERVER || MESH_MODEL_GENERIC_LEVEL_CLIENT) */
#if (MESH_MODEL_GENERIC_TRANSITION_SERVER || MESH_MODEL_GENERIC_TRANSITION_CLIENT)
    {generic_default_transition_time_model_msg, GENERIC_DEFAULT_TRANSITION_TIME_OPCODE_OFFSET, Generic_Default_Transition_Time_Opcode_Max},
#endif /* MESH_MODEL_GENERIC_TRANSITION_SERVER || MESH_MODEL_GENERIC_TRANSITION_CLIENT */
#if (MESH_MODEL_GENERIC_POWER_ONOFF_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT)
    {generic_power_onoff_model_msg,             GENERIC_POWER_ONOFF_OPCODE_OFFSET,             Generic_OnPowerUp_Opcode_Max},
#endif /* MESH_MODEL_GENERIC_POWER_ONOFF_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER || MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT */
#if (MESH_MODEL_LIGHT_LIGHTNESS_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_CLIENT)
    {light_lightness_model_msg,                 LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET,      Light_Lightness_Opcode_MAX},
#endif /* MESH_MODEL_LIGHT_LIGHTNESS_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER || MESH_MODEL_LIGHT_LIGHTNESS_CLIENT */
#if (MESH_MODEL_LIGHT_CTL_SERVER || MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER || MESH_MODEL_LIGHT_CTL_SETUP_SERVER || MESH_MODEL_LIGHT_CTL_CLIENT)
    {light_CTL_model_msg,                       LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET,            Light_CTL_Opcode_MAX},
#endif /* MESH_MODEL_LIGHT_CTL_SERVER || MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER || MESH_MODEL_LIGHT_CTL_SETUP_SERVER || MESH_MODEL_LIGHT_CTL_CLIENT */
#if (MESH_MODEL_LIGHT_HSL_SERVER || MESH_MODEL_LIGHT_HSL_HUE_SERVER || MESH_MODEL_LIGHT_HSL_SATURATION_SERVER || MESH_MODEL_LIGHT_HSL_SETUP_SERVER || MESH_MODEL_LIGHT_HSL_CLIENT)
    {light_HSL_model_msg,                       LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET,            Light_HSL_Opcode_MAX},
#endif /* MESH_MODEL_LIGHT_HSL_SERVER || MESH_MODEL_LIGHT_HSL_HUE_SERVER || MESH_MODEL_LIGHT_HSL_SATURATION_SERVER || MESH_MODEL_LIGHT_HSL_SETUP_SERVER || MESH_MODEL_LIGHT_HSL_CLIENT */
#if (MESH_MODEL_TIME_SERVER || MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_CLIENT || MESH_MODEL_SCENE_SERVER || MESH_MODEL_SCENE_SETUP_SERVER || MESH_MODEL_SCENE_CLIENT || MESH_MODEL_SCHEDULER_SERVER || MESH_MODEL_SCHEDULER_CLIENT)
    {time_and_scene_two_octet_model_msg,        TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET,       Time_And_Scene_Two_Opcode_MAX},
#endif /* MESH_MODEL_TIME_SERVER || MESH_MODEL_TIME_SETUP_SERVER || MESH_MODEL_TIME_CLIENT || MESH_MODEL_SCENE_SERVER || MESH_MODEL_SCENE_SETUP_SERVER || MESH_MODEL_SCENE_CLIENT || MESH_MODEL_SCHEDULER_SERVER || MESH_MODEL_SCHEDULER_CLIENT */
#if (MESH_MODEL_SCENE_SETUP_SERVER)
    {scene_setup_two_octet_model_msg,           SCENE_SETUP_TWO_OCTETS_OPCODE_OFFSET,          Scene_Setup_Two_Opcode_MAX},
#endif /* MESH_MODEL_SCENE_SETUP_SERVER */
//    {sensor_model_msg_two_octets,               SENSOR_TWO_OCTETS_OPCODE_OFFSET,               Sensor_Two_Octets_Opcode_Max},
};

#if (MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT||MESH_MODEL_TC_MODEL_SERVER||MESH_MODEL_TC_MODEL_CLIENT)
static msg_map_t three_octets_msg_map[]=
{
	#if (MESH_MODEL_TC_MODEL_SERVER || MESH_MODEL_TC_MODEL_CLIENT)
    { .ptr = tc_model_msg,
            .u.three_opcode.offset = TC_MODEL_OPCODE_OFFSET,
            .u.three_opcode.num = TC_Message_MAX,
            .u.three_opcode.company_id = TC_MODEL_OPCODE_COMPANY_ID,
    },
#endif /* MESH_MODEL_TC_MODEL_SERVER || MESH_MODEL_TC_MODEL_CLIENT */

#if (MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT)
    { .ptr = tmall_model_msg,
            .u.three_opcode.offset = TMALL_MODEL_OPCODE_OFFSET,
            .u.three_opcode.num = Vendor_Message_MAX,
            .u.three_opcode.company_id = TMALL_MODEL_OPCODE_COMPANY_ID,
    },
#endif /* MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT */
};
#endif /* MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT||MESH_MODEL_TC_MODEL_SERVER||MESH_MODEL_TC_MODEL_CLIENT */

static msg_map_array_t sig_msg_map_entry[3]=
{
    [0] = {one_octet_msg_map,   ARRAY_LEN(one_octet_msg_map)},
    [1] = {two_octets_msg_map,  ARRAY_LEN(two_octets_msg_map)},
#if (MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT||MESH_MODEL_TC_MODEL_SERVER||MESH_MODEL_TMALL_MODEL_CLIENT)
    [2] = {three_octets_msg_map,  ARRAY_LEN(three_octets_msg_map)},
#endif /* MESH_MODEL_TMALL_MODEL_SERVER || MESH_MODEL_TMALL_MODEL_CLIENT */
};


msg_handler_model_t *mesh_sig_msg_handler_search(uint8_t *access_payload )
{
    msg_map_array_t *msg_map;
    uint32_t opcode_offset;
    uint8_t hdl_index = 0;
    uint8_t byte_opcode = ONE_BYPE_OPCODE;	
    if((access_payload[0] & 0x80) == 0)
    {	
        opcode_offset = access_payload[0] & 0x7f;
        byte_opcode = ONE_BYPE_OPCODE;
    }else if((access_payload[0] & 0xc0) == 0x80)
    {
        opcode_offset = ((access_payload[0] & 0x3f)<<8)  | access_payload[1];
        byte_opcode = TWO_BYPE_OPCODE;
    }else {
        opcode_offset = ((access_payload[0] & 0x3f)<<16)  | ((access_payload[1] & 0xff) << 8) | access_payload[2];
        byte_opcode = THREE_BYPE_OPCODE;
    }

    msg_map = &sig_msg_map_entry[byte_opcode];

    LOG(3,"mesh_sig_msg_handler_search:opcode_offset=0x%x bype_opcode:%d\n", opcode_offset, byte_opcode);
    uint16_t i;
    for(i=0;i<msg_map->size;++i)
    {
        if(byte_opcode == THREE_BYPE_OPCODE) {
					  uint16_t company_id = co_bswap16((uint16_t)opcode_offset);
//           uint16_t company_id = (uint16_t)opcode_offset;
            uint8_t three_opcode = (opcode_offset & 0xff0000) >> 16;
					
            if(company_id == msg_map->msg_map[i].u.three_opcode.company_id 
                    && three_opcode >= msg_map->msg_map[i].u.three_opcode.offset
                    && three_opcode < (msg_map->msg_map[i].u.three_opcode.num + msg_map->msg_map[i].u.three_opcode.offset)) {
                hdl_index = three_opcode - msg_map->msg_map[i].u.three_opcode.offset;
                break;
            }
        }else {
            if(opcode_offset >= msg_map->msg_map[i].u.one_two_opcode.offset && opcode_offset < (msg_map->msg_map[i].u.one_two_opcode.num + msg_map->msg_map[i].u.one_two_opcode.offset)) {
                hdl_index = opcode_offset - msg_map->msg_map[i].u.one_two_opcode.offset;
                break;
            }
        }
    }

    LOG(3,"mesh_sig_msg_handler_search: i=%d hdl_index=0x%x\n", i, hdl_index);
    if(i<msg_map->size)
    {
        return &msg_map->msg_map[i].ptr[hdl_index];
    }else
    {
        return NULL;
    }
}
