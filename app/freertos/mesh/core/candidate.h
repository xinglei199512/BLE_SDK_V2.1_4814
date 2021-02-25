#ifndef CANDIDATE_H_
#define CANDIDATE_H_

#include "bx_ring_queue.h"
#include "sdk_mesh_definitions.h"

void candidate_add(ring_queue_t *rq,void *ptr);

void *candidate_current_pick(ring_queue_t *rq);

void candidate_current_remove(ring_queue_t *rq);

void candidate_remove_all(ring_queue_t *rq);

void netkey_candidate_add(net_key_t *netkey,security_credentials_t* keybox);

security_credentials_t *netkey_candidate_current_key_box_pick(void);

net_key_t *netkey_candidate_current_key_env_pick(void);

void netkey_candidate_remove(void);

void netkey_candidate_remove_all(void);

void appkey_candidate_add(app_key_t *appkey,app_key_box_t *keybox);

app_key_box_t *appkey_candidate_current_key_box_pick(void);

app_key_t *appkey_candidate_current_key_env_pick(void);

void appkey_candidate_remove(void);

void appkey_candidate_remove_all(void);

void virt_addr_candidate_add(virt_addr_mngt_t * virt_addr);

virt_addr_mngt_t *virt_addr_candidate_current_pick(void);

void virt_addr_candidate_remove(void);

void virt_addr_candidate_remove_all(void);

#endif
