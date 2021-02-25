#ifndef ACCESS_PDU_DECRYPT_H_
#define ACCESS_PDU_DECRYPT_H_
#include "upper_pdu.h"

typedef struct
{
    access_pdu_cryptic_param_t cryptic;
}access_pdu_decrypt_callback_param_t;

void access_pdu_decrypt_start(upper_pdu_rx_t *pdu,uint8_t *rslt_buf,void (*callback)(uint8_t,access_pdu_decrypt_callback_param_t *));
void access_pdu_virt_addr_decrypt(void);


#endif
