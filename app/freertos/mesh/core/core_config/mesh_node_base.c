/**
 ****************************************************************************************
 *
 * @file   mesh_node_base.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-09-20 17:14
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
#include "mesh_node_base.h"
#include "sdk_mesh_config.h"
#include "mesh_env.h"
#include "mesh_core_api.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/** initialization config server/client model*/
//init a config server
#define INIT_CONFIG_SERVER(model_name )   mesh_model_init(&model_name.model.base,CONFIGURATION_SERVER_MODEL_ID,true,0,0);
//init a config client
#define INIT_CONFIG_CLIENTR(model_name )  mesh_model_init(&model_name.model.base,CONFIGURATION_CLIENT_MODEL_ID,true,0,0);



/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static config_client_model_t config_client;



/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void mesh_node_init(void);

/**
 ****************************************************************************************
 * @brief   Init mesh node
 * @return  void
 ****************************************************************************************
 */
static void mesh_node_init(void)
{
    uint8_t i=0;
    mesh_elmt_t *elmt_buf = get_mesh_elmt();
    //init element list
    for(i=0;i<get_element_num();i++)
    {
        co_list_init(&elmt_buf[i].model_list);
    }
}
/**
 ****************************************************************************************
 * @brief   Init relationship between mesh element and model
 * @return  void
 ****************************************************************************************
 */
void mesh_element_init(uint8_t elmt_idx,model_base_t *model)
{
    mesh_elmt_t *elmt = get_mesh_elmt();
    model->elmt = &elmt[elmt_idx];
    co_list_push_back(&elmt[elmt_idx].model_list,&model->hdr);
}
/**
 ****************************************************************************************
 * @brief   Init mesh element and config server model
 * @return  void
 ****************************************************************************************
 */
void mesh_app_system_init_server(void)
{
    //1.init node
    mesh_node_init();

    //2.init model
    config_server_init();
    //INIT_CONFIG_CLIENTR(config_client);

    //3.init element
    //mesh_element_init(0,&config_client.model.base);
}

/**
 ****************************************************************************************
 * @brief   Init mesh element and config client model
 * @return  void
 ****************************************************************************************
 */
void mesh_app_system_init_client(void)
{
    //1.init node
    mesh_node_init();

    //2.init config client model
    INIT_CONFIG_CLIENTR(config_client);

    //3.init element
    mesh_element_init(0,&config_client.model.base);
}


/**
 ****************************************************************************************
 * @brief   Get the pointer of config client.
 * @return  The pointer of config client.
 ****************************************************************************************
 */
config_client_model_t* get_config_client(void)
{
    return &config_client;
}


