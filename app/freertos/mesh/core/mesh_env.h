#ifndef MESH_ENV_H_
#define MESH_ENV_H_
#include <stdint.h>
#include "sdk_mesh_config.h"
#include "osapp_mesh.h"
#include "mesh_model.h"
//#include "bearer.h"
#include "network_keys_dm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


void init_elmt_addr(uint16_t addr);
//void mesh_node_init(void);
//void mesh_element_init(uint8_t elmt_idx,model_base_t *model);
void mesh_model_init(model_base_t *model,uint32_t model_id,bool sig_model,uint8_t bound_key_buf_size,app_key_t **bound_key_buf);
void model_publish_subscribe_bind(model_base_t *model,model_publish_state_t *publish,mesh_addr_t *subscription_list,uint8_t subscription_list_size,model_publish_timeout_cb_t publish_period_cb);

mesh_elmt_t *get_elmt_by_uni_addr(uint16_t uni_addr);
dm_netkey_handle_t get_netkey_by_dst_addr(uint16_t addr);

model_base_t *get_model_inst_from_elmt(mesh_elmt_t *elmt,uint32_t model_id,bool sig_model);
bool appkey_model_check(model_base_t *model_inst,app_key_t *appkey);

uint8_t ivi_nid_gen(uint32_t iv_index,key_ptr_t *netkey);
uint8_t get_nid(key_ptr_t *netkey);
uint8_t get_ttl(mesh_elmt_t *elmt);
uint8_t *get_netkey(key_ptr_t *ptr);

uint8_t get_aid(key_ptr_t *ptr);
uint8_t *get_appkey(key_ptr_t *ptr);
uint8_t *get_encryption_key(key_ptr_t *ptr);
uint8_t *get_privacy_key(key_ptr_t * ptr);
uint32_t get_local_iv_index(void);//TODO: delete it,no use

uint8_t * get_devkey(uint16_t src_addr, uint16_t dst_addr);
uint8_t * get_peer_devkey(uint16_t src_addr, uint16_t dst_addr);

bool is_lpn_node(void);



//void config_add_devkey(uint8_t * devkey , uint16_t addr);



//void bind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status);
//void unbind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status);
//


/**
 ****************************************************************************************
 * @brief The mesh node get primary element addr.
 *     (the unicast address of its primary element)
 *
 * @return uni_addr  primary element unicast addr.
 *
 ****************************************************************************************
 */
extern uint16_t mesh_node_get_primary_element_addr(void);


dm_netkey_handle_t  mesh_sec_beacon_get_netlist(dm_netkey_pos_t index);
err_t mesh_sec_beacon_get_net_index(uint8_t *network_id,dm_netkey_pos_t *p_index);

dm_netkey_handle_t mesh_sec_beacon_get_netlist_by_netid(uint8_t *network_id);
void  mesh_free(void * ptr);  
void* mesh_alloc(size_t xWantedSize);

void set_is_provisioned(bool value);
bool get_is_provisioned(void);

/*

transport_ctrl_pdu_rx_t *alloc_transport_ctrl_pdu_rx_buf(void);
void free_transport_ctrl_pdu_rx_buf(transport_ctrl_pdu_rx_t *ptr);
lower_pdu_rx_t *alloc_lower_pdu_rx_buf(void);
void free_lower_pdu_rx_buf(lower_pdu_rx_t *ptr);
*/

#endif

