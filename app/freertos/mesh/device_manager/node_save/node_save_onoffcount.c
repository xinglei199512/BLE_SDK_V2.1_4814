/**
 ****************************************************************************************
 *
 * @file   node_save_onoffcount.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-29 15:29
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

#include "node_save.h"
#include "node_save_macro.h"
#include "node_save_model.h"
#include "node_save_common.h"
#include "mesh_node_base.h"
#include "sdk_mesh_config_pro.h"
#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "bxfs.h"
#include "mesh_core_api.h"
#include "node_save_onoffcount.h"

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
void node_save_system_onoffcount(uint8_t count)
{
    LOG(3, "node_save_system_onoffcount:%x\n", count);
    //save to bxfs
    bxfs_write1(MESHDIR1_ONOFF_COUNT, MESHFILE1_ONOFF_POWER_COUNT, &count, MESH_ONPOWER_ONOFF_COUNT);
}

uint8_t node_recover_system_onoffcount(void)
{
    uint8_t count = 0;
    LOG(3, "1node_recover_system_onoffcount:%x\n", count);
    bxfs_read1(MESHDIR1_ONOFF_COUNT, MESHFILE1_ONOFF_POWER_COUNT, &count, MESH_ONPOWER_ONOFF_COUNT);
    LOG(3, "2node_recover_system_onoffcount:%x\n", count);

    return count;
}

