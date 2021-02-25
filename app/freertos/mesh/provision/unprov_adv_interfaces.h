
#ifndef UNPROV_ADV_INTERFACES_H_
#define UNPROV_ADV_INTERFACES_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "log.h"
#include "adv_bearer_tx.h"
#include "beacon.h"
#include "gap.h"
#include "co_endian.h"


uint8_t unprov_beacon_data[22] = {0x00}; //user define 


/*************************************************************************************
 * 
 * Interfaces provided to  customer 
 */

uint8_t unprov_beacon_start(void (* callback)(uint8_t, uint8_t, uint8_t));
uint8_t unprov_beacon_stop(void);



#endif /* UNPROV_ADV_INTERFACES_H_ */
