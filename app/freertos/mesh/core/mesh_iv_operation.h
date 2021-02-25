
#ifndef MESH_IV_OPERATION_H_
#define MESH_IV_OPERATION_H_


#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "sdk_mesh_definitions.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/** Limit for silence recover. */
#define MESH_IV_RECOVERY_LIMIT                  42
#define MESH_OK                                 0
#define MESH_FAIL                               1

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void mesh_beacon_iv_index_set(uint32_t val);
uint32_t iv_update_get_seq_num(void);
void iv_update_set_seq_num(uint32_t value);
void mesh_beacon_iv_kr_flag_set(bool flag);



#endif
