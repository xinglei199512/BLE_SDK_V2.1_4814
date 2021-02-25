#include "osapp_config.h"

#include <stdint.h>
#include <stdarg.h>

#include "sdk_mesh_definitions.h"
#include "mesh_env.h"
#include "beacon.h"
#include "co_endian.h"
#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_model.h"

#include "mesh_kr_comm.h"
#include "mesh_kr_client.h"
#include "mesh_kr_server.h"






uint8_t mesh_kr_Get_tx_key_index(net_key_t *netkey)
{
     if(netkey->key_refresh_phase != MESH_KEY_REFRESH_PHASE_2)
     {
         return netkey->primary_used;
     }
     return (1-netkey->primary_used);
}


uint8_t mesh_kr_Get_beacon_key_index(net_key_t *netkey)
{
   return netkey->primary_used;
}

uint8_t mesh_kr_Get_beacon_key_newindex(net_key_t *netkey)
{
   return (1-netkey->primary_used);
}



bool bit_get(const uint8_t * p_bitfield, uint32_t bit)
{
    return !!(p_bitfield[bit / BIT_BLOCK_SIZE] & BIT_MASK(bit));
}

void bit_set(uint8_t * p_bitfield, uint32_t bit)
{
    p_bitfield[bit / BIT_BLOCK_SIZE] |= BIT_MASK(bit);
}

void bit_clear(uint8_t * p_bitfield, uint32_t bit)
{
    p_bitfield[bit / BIT_BLOCK_SIZE] &= ~(BIT_MASK(bit));
}


uint8_t mesh_get_key_refresh(net_key_t *netkey)
{
    if(netkey->key_refresh_phase == MESH_KEY_REFRESH_PHASE_2)
    {
         return 1 ;
    }
    return 0;
}








