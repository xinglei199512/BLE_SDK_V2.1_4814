#ifndef OSAPP_MESH_H_
#define OSAPP_MESH_H_
#include <stdint.h>
#include <stdbool.h>
#include "gap.h"
/*
 * DEFINES
 ****************************************************************************************
 */
//#define ADV_DATA_BUF_SIZE           (GAP_ADV_DATA_LEN-0)
/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void mesh_init_start_scan(void);


void mesh_stack_init_process_next_stage(void);


#endif
