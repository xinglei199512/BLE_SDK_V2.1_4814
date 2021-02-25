#ifndef NETWORK_PDU_ENCRYPT_H_
#define NETWORK_PDU_ENCRYPT_H_
#include "network_pdu.h"
//#include "sdk_mesh_definitions.h"

void network_pdu_encrypt_start(network_tx_data_t *src,uint32_t iv_index,security_credentials_t *security,
    void (*callback)(network_pdu_packet_u *),uint8_t pkt_type,network_pdu_packet_u *rslt_buf);


#endif
