
#ifndef PROVISION_SERVICE_ADV_INTERFACES_H_
#define PROVISION_SERVICE_ADV_INTERFACES_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "log.h"
#include "adv_bearer_tx.h"
#include "beacon.h"
#include "gap.h"
#include "co_endian.h"


uint8_t provisioning_service_advertising_data[29] = {0x00}; //user define 

/*************************************************************************************
 * 
 * Interfaces provided to  customer 
 */

uint8_t provisioning_service_advertising_start(void (* callback)(uint8_t, uint8_t, uint8_t));
uint8_t provisioning_service_advertising_stop(void);

#endif /* PROVISION_SERVICE_ADV_INTERFACES_H_ */
