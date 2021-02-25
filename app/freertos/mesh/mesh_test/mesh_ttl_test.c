
#include "access_rx_process.h"
#include "lower_rx_process.h"
#include "mesh_core_api.h"
#include "mesh_gatt.h"
#include "co_endian.h"
#include "mesh_env.h"

#if MESH_TOOLS_TTL_TEST
bool  network_rx_ttl_check(network_pdu_rx_t *pdu,network_pdu_packet_u *ptr)
{
    uint8_t  bd_addr[GAP_BD_ADDR_LEN];
    mesh_core_params_t core_param;
    core_param.mac_address = bd_addr;
    mesh_core_params_get(MESH_CORE_PARAM_MAC_ADDRESS,&core_param);
    uint8_t ttl_tx  = ACCESS_DEFAULT_TTL - bd_addr[0];
    uint8_t ttl_rx  = ACCESS_DEFAULT_TTL - bd_addr[1] + bd_addr[0];
    uint8_t dir = 0;
    uint16_t  self_addr = mesh_node_get_primary_element_addr();
    uint16_t  src_addr = co_bswap16(ptr->pkt.src_be);
    if(pdu->from != ADV_BEARER_RX)
    {
        return true;
    }
    if(self_addr > src_addr || src_addr == 0x7ff) // phone default addr
    {
       dir = 0;
    }
    else
    {
       dir = 1;
    }
    if(ptr->pkt.ttl ==  ttl_tx && dir == 0)
    {
        return true;
    }
    if(ptr->pkt.ttl ==  ttl_rx && dir == 1)
    {
        return true;
    }
    LOG(LOG_LVL_INFO,"discard src:%d tt: %d\n",src_addr,ptr->pkt.ttl);
    return false;
}
#endif


