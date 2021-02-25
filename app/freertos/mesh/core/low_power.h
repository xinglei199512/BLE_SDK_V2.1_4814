#ifndef LOW_POWER_H_
#define LOW_POWER_H_
#include "upper_pdu.h"
#include "friendship.h"


#define LOW_POWER_SEG_TX_TIMER_FACTOR 3


void low_power_init(void);
void friend_request_tx_done_callback(void *pdu,uint8_t status);
void friend_request_param_set(uint32_t poll_timeout,friend_req_criteria_t criteria,uint8_t receive_delay);
void friend_request_tx(void);
void lpn_friend_clear_tx(void);
void friend_poll_tx(void);
void friend_update_rx_handler(upper_pdu_rx_t *ptr);
void friend_offer_judge(upper_pdu_rx_t *ptr);
void friend_offer_rx_handler(upper_pdu_rx_t *ptr);
void friend_subscription_list_confirm_rx_handler(upper_pdu_rx_t *ptr);
void lpn_schedule_at_pkt_rx(bool is_update_pkt);
bool low_power_mode_status_get(void);
uint32_t low_power_poll_interval(void);
uint16_t  low_power_friend_addr(void);

void set_lp_netkey_global_idx(uint16_t global_idx);
void set_lp_default_netkey_global_idx(void);
void low_power_toggle_fsn(void);
security_credentials_t *lpn_tx_net_security_get(void);
err_t low_power_get_friend_tx_credentials_by_idx(mesh_global_idx_t netkey_idx, security_credentials_t ** pp_security_credentials);
void friend_subscription_list_add_tx(uint8_t transaction_number,uint16_t *address_list , uint8_t count);
void friend_subscription_list_remove_tx(uint8_t transaction_number,uint16_t *address_list , uint8_t count);
void low_power_generate_credentials(void* dm_handle);
bool lpn_is_friend_security(security_credentials_t * p_security_credentials);

uint32_t get_lp_polltimeout_value(void);



#endif
