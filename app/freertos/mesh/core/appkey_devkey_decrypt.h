#ifndef APPKEY_DEVKEY_DECRYPT_H_
#define APPKEY_DEVKEY_DECRYPT_H_
#include "upper_pdu.h"
#include "virt_addr_mngt.h"


void appkey_decrypt_start(upper_pdu_rx_t *upper_pdu,uint8_t *decrypted_buf,virt_addr_mngt_t *virt_addr,void (*decrypt_callback)(uint8_t,app_key_t *,app_key_box_t *));

void devkey_decrypt_start(upper_pdu_rx_t *upper_pdu,uint8_t *decrypted_buf,void (*decrypt_callback)(uint8_t,app_key_t *,app_key_box_t *));
void access_pdu_appkey_decrypt(void);

#endif
