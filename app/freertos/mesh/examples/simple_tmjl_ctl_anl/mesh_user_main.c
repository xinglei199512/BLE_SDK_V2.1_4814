/**
 ****************************************************************************************
 *
 * @file   mesh_user_main.c
 *
 * @brief  .
 *
 * @author  liuzy
 * @date    2018-09-18 17:23
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
#include "mesh_user_main.h"
#include "sdk_mesh_config.h"
#include "simple_ctl_anl_s.h"
//#include "provisioner_cfg_c.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

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

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   user init function
 * @return  void
 ****************************************************************************************
 */
void mesh_user_main_init(void)
{
    ///user data init
    simple_hsl_ctl_server_init();

    LOG(LOG_LVL_INFO,"mesh_user_main_init\n");
}


