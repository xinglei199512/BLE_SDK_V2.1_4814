#ifndef MODEL_RX_H_
#define MODEL_RX_H_
#include "upper_pdu.h"
#include "access_pdu_decrypt.h"

void deliver_to_model(upper_pdu_rx_t* uppdu,void * access_data,access_pdu_decrypt_callback_param_t*param);



#endif
