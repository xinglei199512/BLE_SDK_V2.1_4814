/**
****************************************************************************************
*
* @file co_utils.c
*
* @brief Common Utility functions
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup  CO_UTILS
* @{
****************************************************************************************
*/

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"      // SW configuration

#include <string.h>           // for mem* functions
#include "co_bt.h"            // common bt definitions
#include "co_utils.h"         // common utility definitions



/*
 * DEFINES
 ****************************************************************************************
 */

///Default BT address (if none defined in NVDS)
#define BT_DEFAULT_BDADDR  {{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB}}

/// Minimum time to do a kernel timer action
#define CO_MIN_NB_SLOT_DUR       32

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */


/*
 * CONSTANT DEFINITIONS
 ****************************************************************************************
 */

/// Number of '1' bits in values from 0 to 15, used to fasten bit counting
const unsigned char one_bits[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};

/// SCA to PPM
const uint16_t co_sca2ppm[] =
{
    [SCA_500PPM] = 500,
    [SCA_250PPM] = 250,
    [SCA_150PPM] = 150,
    [SCA_100PPM] = 100,
    [SCA_75PPM] = 75,
    [SCA_50PPM] = 50,
    [SCA_30PPM] = 30,
    [SCA_20PPM] = 20
};

/// NULL BD address
const struct bd_addr co_null_bdaddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/// Default BD address
const struct bd_addr co_default_bdaddr = BT_DEFAULT_BDADDR;


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#if RW_DEBUG
void co_bytes_to_string(char* dest, uint8_t* src, uint8_t nb_bytes)
{
    for(int i = 0 ; i < nb_bytes ; i++)
    {
        char digit;
        uint8_t byte = *(src + nb_bytes - 1 - i);

        // MSbs
        digit = (byte & 0xF0) >> 4;
        digit += (digit < 10) ? 48:55;
        *(dest+2*i) = (char)digit;

        // LSbs
        digit = (byte & 0x0F) >> 0;
        digit += (digit < 10) ? 48:55;
        *(dest+2*i+1) = (char)digit;
    }
}
#endif //RW_DEBUG

bool co_bdaddr_compare(struct bd_addr const *bd_address1,
                          struct bd_addr const *bd_address2)
{

    for (uint8_t idx=0; idx < BD_ADDR_LEN; idx++)
    {
        /// checks if the addresses are similar
        if (bd_address1->addr[idx] != bd_address2->addr[idx])
        {
           return (false);
        }
    }
    return(true);
}

#if (BT_EMB_PRESENT)
uint32_t co_slot_to_duration(uint16_t slot_cnt)
{
    uint32_t duration = 0;
    /* Avoid Slot number < 32                                                           */
    if (slot_cnt < CO_MIN_NB_SLOT_DUR)
    {
        slot_cnt = CO_MIN_NB_SLOT_DUR;
    }
    /* Duration contains a number of Slot and a tick is 10 ms                           */
    duration = slot_cnt >> 4;
    return (duration);
}

uint8_t co_nb_good_channels(const struct chnl_map* map)
{
    uint8_t nb_good_channels = 0;

    // Count number of good channels
    for(int i = (CHNL_MAP_LEN-1) ; i >= 0 ; i--)
    {
        uint8_t byte = map->map[i];
        nb_good_channels += NB_ONE_BITS(byte);
    }

    return nb_good_channels;
}
#endif //BT_EMB_PRESENT

/// @} CO_UTILS
