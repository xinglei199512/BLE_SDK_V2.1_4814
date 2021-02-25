/*
 * node_save_misc.c
 *
 *  Created on: 2018-8-22
 *      Author: jiachuang
 */

#include "osapp_config.h"
#include "node_save.h"
#include "node_save_macro.h"
#include "node_save_misc.h"
#include "node_save_common.h"
#include "bxfs.h"
#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_core_api.h"

/***************misc*****************/

void node_save_misc_is_provisioner(void)
{
    mesh_core_params_t param;
    uint8_t value;
    mesh_core_params_get(MESH_CORE_PARAM_IS_PROVISIONER , &param);
    value = param.is_provisioner;
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_misc_is_provisioner\n");
    bxfs_write1(MESHDIR1_MISC , MESHFILE1_MISC_IS_PROVISIONER ,        &value       , MESH_SIZE_IS_PROVISIONER);
}

void node_save_misc_is_provisioned(void)
{
    mesh_core_params_t param;
    uint8_t value;
    mesh_core_params_get(MESH_CORE_PARAM_IS_PROVISIONED , &param);
    value = param.is_provisioned;
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_misc_is_provisioned\n");
    bxfs_write1(MESHDIR1_MISC , MESHFILE1_MISC_IS_PROVISIONED ,        &value       , MESH_SIZE_IS_PROVISIONED);
}

void node_save_misc_sequence_number(void)
{
    uint32_t save_seq;
    if((iv_update_get_seq_num() % 500) == 0)
    {
        save_seq = iv_update_get_seq_num() + 500;
        if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_misc_sequence_number\n");
        bxfs_write1(MESHDIR1_MISC , MESHFILE1_MISC_SEQUENCE_NUMBER ,             &save_seq            , MESH_SIZE_SEQUENCE_NUMBER);
        node_save_write_through();
    }
    else
    {
        if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"Only seq%%100 == 0 will be saved.\n");
    }
}

void node_save_misc_iv_index(void)
{
    uint32_t iv_index= 0;
    iv_index = mesh_beacon_iv_index_get();
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_misc_iv_index\n");
    bxfs_write1(MESHDIR1_MISC , MESHFILE1_MISC_IV_INDEX ,                    &iv_index                   , MESH_SIZE_IV_INDEX);
}


void node_recover_misc(void)
{
    uint32_t iv_index= 0;
    uint32_t seq_temp= 0;
    uint8_t is_provisioner = 0;
    uint8_t is_provisioned = 0;
    mesh_core_params_t param;
    //read value
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_misc:is_provisioner\n");
    bxfs_read1(MESHDIR1_MISC , MESHFILE1_MISC_IS_PROVISIONER ,        &is_provisioner       , MESH_SIZE_IS_PROVISIONER);
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_misc:is_provisioned\n");
    bxfs_read1(MESHDIR1_MISC , MESHFILE1_MISC_IS_PROVISIONED ,        &is_provisioned       , MESH_SIZE_IS_PROVISIONED);
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_misc:sequence_number\n");
    bxfs_read1(MESHDIR1_MISC , MESHFILE1_MISC_SEQUENCE_NUMBER ,             &seq_temp                   , MESH_SIZE_SEQUENCE_NUMBER);
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_misc:iv_index\n");
    bxfs_read1(MESHDIR1_MISC , MESHFILE1_MISC_IV_INDEX ,                    &iv_index                   , MESH_SIZE_IV_INDEX);
    //set to system
    param.is_provisioner = is_provisioner;
    mesh_core_params_set(MESH_CORE_PARAM_IS_PROVISIONER , &param);
    param.is_provisioned = is_provisioned;
    mesh_core_params_set(MESH_CORE_PARAM_IS_PROVISIONED , &param);
    mesh_beacon_iv_index_set(iv_index);
    iv_update_set_seq_num(seq_temp);
}

/***************misc*****************/



/***************element*****************/

void node_save_element_uni_adddr(void)
{
    for(uint8_t i=0;i<get_element_num();i++)
    {
        if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_element%d_uni_adddr\n",i);
        bxfs_write1(MESHDIR1_ELMT_MIN + i , MESHFILE1_ELMT_UNI_ADDR , &get_mesh_elmt()[i].uni_addr , MESH_SIZE_UNI_ADDR);
    }
}

void node_recover_element_uni_adddr(void)
{
    for(uint8_t i=0;i<get_element_num();i++)
    {
        if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_element%d_uni_adddr\n",i);
        bxfs_read1(MESHDIR1_ELMT_MIN + i , MESHFILE1_ELMT_UNI_ADDR , &get_mesh_elmt()[i].uni_addr , MESH_SIZE_UNI_ADDR);
    }
}

/***************element*****************/



