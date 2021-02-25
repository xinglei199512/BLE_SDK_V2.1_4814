/*
 * node_save_common.c
 *
 *  Created on: 2018��8��22��
 *      Author: jiachuang
 */

#include "osapp_config.h"
#include "node_save_macro.h"
#include "node_save_common.h"
#include "node_save.h"
#include "osapp_utils.h"
#include "bxfs.h"
#include "mesh_core_api.h"
#include "mesh_sched.h"
#include "config_server.h"
#include "config_client.h"

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static uint16_t node_save_file_list[NODE_SAVE_DIR_LIST_NUM];
static uint16_t node_save_file_count = 0;

  
/***************bxfs debug*****************/
uint8_t node_save_log_status[BXFS_TMP_BUF_OVERFLOW+1][40] =
{
        "BXFS_NO_ERROR                     ",
        "BXFS_RECORD_DATA_CORRUPTED        ",
        "BXFS_INSUFFICIENT_NODE_BUF        ",
        "BXFS_INSUFFICIENT_RECORD_DATA_BUF ",
        "BXFS_DIR_IDX_OVERFLOW             ",
        "BXFS_PARENT_DIR_NOT_FOUND         ",
        "BXFS_DIR_NOT_EXISTED              ",
        "BXFS_DIR_NOT_EMPTY                ",
        "BXFS_DIR_KEY_ALREADY_EXISTED      ",
        "BXFS_RECORD_KEY_ALREADY_EXISTED   ",
        "BXFS_RECORD_KEY_NOT_FOUND         ",
        "BXFS_TMP_BUF_OVERFLOW             ",
};

static void log_mkdir_status(uint8_t status)
{
    if(NODE_SAVE_LOG_ENABLE)
    {
        LOG(3,"bxfs:status = %s\n",node_save_log_status[status]);
    }
}

/***************bxfs debug*****************/



/***************list file interface*****************/

uint8_t get_node_save_file_count(void)
{
    return node_save_file_count;
}

uint8_t get_node_save_file_list(uint8_t position)
{
    return node_save_file_list[position];
}


/***************list file interface*****************/

/***************bxfs interface*****************/

uint16_t node_save_write_through(void)
{
    return bxfs_write_through();
}

/* Depth = 1 */
void bxfs_write1(uint8_t dir1 ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 1;
    //bxfs
    status = bxfs_hierarchy_write(&param , dir1);
    //log
    LOG(3,"NS:SAVE1:%d->%d:%x,%d\n",dir1,file,data,length);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_read1(uint8_t dir1 ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 1;
    //bxfs
    status = bxfs_hierarchy_read(&param , dir1);
    //log
    LOG(3,"NS:READ1:%d->%d:%x,%d\n",dir1,file,data,length);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_delete1(uint8_t dir1 , uint8_t file)
{
    bxfs_hierarchy_del_record_t param;
    uint8_t status;
    //init value
    param.record_key = file;
    param.dir_depth  = 1;
    //bxfs
    status = bxfs_hierarchy_del_record(&param , dir1);
    //log
    LOG(3,"NS:DELETE1:%d\\%d\n",dir1,file);
    log_mkdir_status(status);
}

/* Depth = 2 */
void bxfs_write2(uint8_t dir1 , uint8_t dir2  ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 2;
    //bxfs
    status = bxfs_hierarchy_write(&param , dir1 , dir2);
    //log
    LOG(3,"NS:SAVE2:%d\\%d->%d:%x,%d\n",dir1,dir2,file,data,length);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_read2(uint8_t dir1 , uint8_t dir2 ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 2;
    //bxfs
    status = bxfs_hierarchy_read(&param , dir1 , dir2);
    //log
    LOG(3,"NS:READ2:%d\\%d->%d:%x,%d\n",dir1,dir2,file,data,length);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_delete2(uint8_t dir1 , uint8_t dir2 , uint8_t file)
{
    bxfs_hierarchy_del_record_t param;
    uint8_t status;
    //init value
    param.record_key = file;
    param.dir_depth  = 2;
    //bxfs
    status = bxfs_hierarchy_del_record(&param , dir1 , dir2);
    //log
    LOG(3,"NS:DELETE2:%d\\%d\\%d\n",dir1,dir2,file);
    log_mkdir_status(status);
}

/* Depth = 3 */
void bxfs_write3(uint8_t dir1 , uint8_t dir2 , uint8_t dir3 ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 3;
    //bxfs
    status = bxfs_hierarchy_write(&param , dir1 , dir2 , dir3);
    //log
    LOG(3,"NS:SAVE3:%d\\%d\\%d->%d:%x,%d status:%d\n",dir1,dir2,dir3,file,data,length, status);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_read3(uint8_t dir1 , uint8_t dir2 , uint8_t dir3 ,                uint8_t file , void * data , uint16_t length)
{
    bxfs_hierarchy_rw_t param;
    uint8_t status;
    //init value
    param.data   = data;
    param.length = length;
    param.record_key = file;
    param.dir_depth  = 3;
    //bxfs
    status = bxfs_hierarchy_read(&param , dir1 , dir2 , dir3);
    //log
    LOG(3,"NS:READ3:%d\\%d\\%d->%d:%x,%d\n",dir1,dir2,dir3,file,data,length);
    log_hex_data(data,length);
    log_mkdir_status(status);
}
void bxfs_delete3(uint8_t dir1 , uint8_t dir2 , uint8_t dir3 , uint8_t file)
{
    bxfs_hierarchy_del_record_t param;
    uint8_t status;
    //init value
    param.record_key = file;
    param.dir_depth  = 3;
    //bxfs
    status = bxfs_hierarchy_del_record(&param , dir1 ,dir2,dir3);
    //log
    LOG(3,"NS:DELETE3:%d\\%d\\%d\\%d\n",dir1,dir2,dir3,file);
    log_mkdir_status(status);
}

/* List File */
void bxfs_listfile3(uint8_t dir1 , uint8_t dir2 , uint8_t dir3)
{
    bxfs_hierarchy_record_list_t param;
    uint8_t status;
    //init value
    param.dir_depth = 3;
    param.list = node_save_file_list;
    param.num  = NODE_SAVE_DIR_LIST_NUM;
    //bxfs
    status = bxfs_hierarchy_record_list_get(&param , dir1 , dir2 , dir3);
    node_save_file_count = param.num;
    if(status)
        node_save_file_count = 0;
    //log
    LOG(3,"NS:LIST3:%d\\%d\\%d\n",dir1,dir2,dir3);
    log_mkdir_status(status);
    LOG(3,"bxfs:count = %d\n",node_save_file_count);
}

void bxfs_listfile2(uint8_t dir1 , uint8_t dir2)
{
    bxfs_hierarchy_record_list_t param;
    uint8_t status;
    //init value
    param.dir_depth = 2;
    param.list = node_save_file_list;
    param.num  = NODE_SAVE_DIR_LIST_NUM;
    //bxfs
    status = bxfs_hierarchy_record_list_get(&param , dir1 , dir2);
    node_save_file_count = param.num;

    if(status)
        node_save_file_count = 0;
    //log
    LOG(3,"NS:LIST2:%d\\%d\n",dir1,dir2);
    log_mkdir_status(status);
    LOG(3,"bxfs:count = %d stauts:%d\n",node_save_file_count, status);
}

/* Make dir */
void node_save_mkdir(bxfs_dir_t *dir_to_make,bxfs_dir_t upper_dir,uint8_t dir_key)
{
    uint8_t status;
    status = bxfs_mkdir(dir_to_make,upper_dir,dir_key);
    log_mkdir_status(status);
}

/***************bxfs interface*****************/

/***************bxfs delete ****************/
void node_delete_mesh_dir(void)
{
    bxfs_dir_t dir_temp;
    int i = 0;

    /* 1 delete misc dir */
    bxfs_get_child_dir_by_name((void*)ROOT_DIR, MESHDIR1_MISC, &dir_temp);
    bxfs_del_dir(dir_temp, true);

    /* 2 delete keys dir */
    bxfs_get_child_dir_by_name((void*)ROOT_DIR, MESHDIR1_KEYS, &dir_temp);
    bxfs_del_dir(dir_temp, true);
    
//    bxfs_print_dir_tree();

    /* 3 delete elem dir */
    for(i=0;i<get_element_num();i++)
    {
        bxfs_get_child_dir_by_name((void*)ROOT_DIR, MESHDIR1_ELMT_MIN + i, &dir_temp);
        bxfs_del_dir(dir_temp, true);
//        bxfs_print_dir_tree();
    }

    node_save_write_through();

    /* 4 check dir_id_max */
    if(bxfs_get_dir_id_max() > 0xFDE8)
        node_save_bxfs_hardware_erase_all();

    /* 5 reset platform */
    platform_reset(0);
}


/***************bxfs mkdir*****************/

void node_save_init_make_dir(void)
{
    bxfs_dir_t dir_onoffcount, dir_misc, dir_keys, dir_elmt, dir_model, dir_tmp, dir_scene, dir_scheduler;
    uint8_t i = 0,model_index = 0;

    //mask onoff count dir
    {
        node_save_mkdir(&dir_onoffcount, (void*)ROOT_DIR, MESHDIR1_ONOFF_COUNT);
    }
    //make misc dir
    {
        //make dir: \misc
        node_save_mkdir(&dir_misc, (void*)ROOT_DIR, MESHDIR1_MISC);
    }
    //make keys dir
    {
        //make dir: \keys
        node_save_mkdir(&dir_keys, (void*)ROOT_DIR, MESHDIR1_KEYS);
        //make sub dir
        node_save_mkdir(&dir_tmp, dir_keys, MESHDIR2_KEYS_NETKEYLIST);
        node_save_mkdir(&dir_tmp, dir_keys, MESHDIR2_KEYS_APPKEYLIST);
        node_save_mkdir(&dir_tmp, dir_keys, MESHDIR2_KEYS_DEVKEYLIST);
    }
    //make element and sub dir
    for(i=0;i<get_element_num();i++)
    {
        //make dir: \elementX
        node_save_mkdir(&dir_elmt, (void*)ROOT_DIR, MESHDIR1_ELMT_MIN + i);
        //make dir: scene
        node_save_mkdir(&dir_scene, dir_elmt, MESHDIR2_ELMTX_SCENE_NUMBERLIST);
        //make dir: scheduler
        node_save_mkdir(&dir_scheduler, dir_elmt, MESHDIR2_ELMTX_SCHEDULER_NUMBERLIST);
        //make element sub dir
        model_index = 0;
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[i].model_list);
        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            //make model dir
            model_index ++;
            node_save_mkdir(&dir_model, dir_elmt, model_index);
            //make model sub dir
            if((model->model_id == CONFIGURATION_SERVER_MODEL_ID) || (model->model_id == CONFIGURATION_CLIENT_MODEL_ID))//config model
            {
                ;
            }
            else//usual model
            {
                node_save_mkdir(&dir_tmp, dir_model, MESHDIR3_ELMTX_MODELX_SUBSCRIPTION_LIST);
                node_save_mkdir(&dir_tmp, dir_model, MESHDIR3_ELMTX_MODELX_BOUNDKEY_LIST);
                node_save_mkdir(&dir_tmp, dir_model, MESHDIR3_ELMTX_MODELX_PUBLICATION);
            }
            hdr = co_list_next(hdr);
        }
    }
}


/***************bxfs mkdir*****************/






/***************get dir*****************/


uint8_t get_element_save_dir(mesh_elmt_t *elmt)
{
    uint8_t i=0;
    for(i=0;i<get_element_num();i++)
    {
        if(elmt == &get_mesh_elmt()[i])
        {
            return MESHDIR1_ELMT_MIN + i;
        }
    }
    return MESH_SAVE_ERROR;
}

uint8_t get_model_dir_index_in_its_element(model_base_t *model)
{
    mesh_elmt_t *elmt = model->elmt;
    uint8_t element_index = 1;
    struct co_list_hdr *hdr = co_list_pick(&elmt->model_list);

    while(hdr)
    {
        model_base_t *current_model = CONTAINER_OF(hdr, model_base_t, hdr);
        if(current_model == model)
        {
            return element_index;
        }
        element_index ++;
        hdr = co_list_next(hdr);
    }
    return MESH_SAVE_ERROR;
}

/***************get dir*****************/







/***************reset node save*****************/


/***************reset node save*****************/


