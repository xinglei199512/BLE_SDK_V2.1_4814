

#include "osapp_config.h"
#include "mesh_model.h"
#include "mesh_core_api.h"
#include "access_rx_process.h"
#include "foundation_msg_handler.h"
#include "heartbeat.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "heartbeat.h"

static heartbeat_publication_server_t pub_server;
static heartbeat_subscription_server_t sub_server;

static void heartbeat_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "heartbeat_status_tx_done\n");
}

static void heartbeat_send_status(uint16_t src_addr, uint16_t dst_addr, void *msg, uint16_t msg_len, uint16_t globel_idx, uint16_t opcode, void(*cb)(void *, uint8_t))
{
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( globel_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msg_len;
    tx_param.src_addr = src_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,cb,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void config_heartbeat_subscription_send_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t from)
{
    heartbeat_subscription_status_t msg;
    msg.status = sub_server.status;
    if((sub_server.destination_addr == elmt->uni_addr && sub_server.source_addr && sub_server.source_addr == access_get_pdu_src_addr(pdu))) {
        msg.source_addr = sub_server.source_addr;
        msg.destination_addr = sub_server.destination_addr;
        if(from)
            msg.periodlog = sub_server.sub_periodlog;
        else
            msg.periodlog = 0;
        msg.countlog = heartbeat_calculate_16bit_to_8bit(sub_server.sub_count);
        msg.minhops = sub_server.minhops;
        msg.maxhops = sub_server.maxhops;
    }else {
        msg.source_addr = 0;
        msg.destination_addr = 0;
        msg.periodlog = 0;
        msg.countlog = heartbeat_calculate_16bit_to_8bit(sub_server.sub_count);
        if(msg.countlog) {
            sub_server.minhops = msg.minhops = msg.countlog;
            sub_server.maxhops = msg.maxhops = msg.countlog;
        }else {
            if(from)
                msg.minhops = 0x7f;
            else
                msg.minhops = 0x7f;
            msg.maxhops = sub_server.maxhops;
        }
    }
    LOG(3, "subcount:%x\n", sub_server.sub_count);
    LOG(3, "from:%x\n", from);
    LOG(3, "status:%x\n", msg.status);
    LOG(3, "source_addr:%x\n", msg.source_addr);
    LOG(3, "destination_addr:%x\n", msg.destination_addr);
    LOG(3, "periodlog:%x\n", msg.periodlog);
    LOG(3, "countlog:%x\n", msg.countlog);
    LOG(3, "minhops:%x\n", msg.minhops);
    LOG(3, "maxhops:%x\n", msg.maxhops);
    heartbeat_send_status(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void *)&msg, \
            sizeof(heartbeat_subscription_status_t), access_get_pdu_appkey_global_index(pdu), TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET, Config_Heartbeat_Subscription_Status), heartbeat_status_tx_done);
}


void config_heartbeat_subscription_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    heartbeat_subscription_set_t *sub_set = (heartbeat_subscription_set_t *)(access + 2);

    sub_server.status = 0x0;

    if(sub_server.destination_addr || sub_server.source_addr) {
        if(sub_server.sub_periodlog != sub_set->periodlog) {
            sub_server.sub_periodlog = sub_set->periodlog;
            if(sub_set->periodlog)
                sub_server.sub_count = 0;
        }
    }

    LOG(3,"config_heartbeat_subscription_set_rx addr:%x %x sub_count:%x\n",
            sub_server.source_addr, sub_server.destination_addr, sub_server.sub_count);

    if(sub_set->destination_addr && (sub_set->destination_addr != elmt->uni_addr)) {
        if(!(sub_set->destination_addr & 0x8000))
            sub_server.destination_addr = 0;
        return;
    }

    if(sub_set->source_addr & 0x8000)
        return;

    sub_server.source_addr = sub_set->source_addr;
    sub_server.destination_addr = sub_set->destination_addr;


    config_heartbeat_subscription_send_status(elmt, model, pdu, 1);

    heartbeat_start_subscription_timer();
}


void config_heartbeat_subscription_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
#if 0
    static int count = 0;
    sub_server.sub_count = count++;
#endif
    config_heartbeat_subscription_send_status(elmt, model, pdu, 0);
}

static void config_heartbeat_publication_send_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    heartbeat_publication_status_t msg;
    msg.status = pub_server.status;
    msg.destination_addr = pub_server.destination_addr;
    msg.countlog = heartbeat_calculate_16bit_to_8bit(pub_server.pub_count);
    msg.periodlog = pub_server.pub_periodlog;
    msg.ttl = pub_server.ttl;
    msg.features = pub_server.features;
    msg.netkeyindex = pub_server.netkeyindex;

    LOG(3, "status:%x\n", msg.status);
    LOG(3, "destination_addr:%x\n", msg.destination_addr);
    LOG(3, "countlog:%x pub_count:%x\n", msg.countlog, pub_server.pub_count);
    LOG(3, "periodlog:%x\n", msg.periodlog);
    LOG(3, "ttl:%x\n", msg.ttl);
    LOG(3, "features:%x\n", msg.features);
    LOG(3, "netkeyindex:%x\n", msg.netkeyindex);

    heartbeat_send_status(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void *)&msg, \
            sizeof(heartbeat_publication_status_t), access_get_pdu_appkey_global_index(pdu), ONE_OCTET_OPCODE_GEN(FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET, Config_Heartbeat_Publication_Status), heartbeat_status_tx_done);
}


void config_heartbeat_publication_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    heartbeat_publication_set_t *pub_set = (heartbeat_publication_set_t *)(access + 2);

    pub_server.status = 0x0;

    pub_server.destination_addr = pub_set->destination_addr;
    pub_server.pub_count = heartbeat_calculate_8bit_to_16bit(pub_set->countlog);
    pub_server.pub_periodlog = pub_set->periodlog;
    pub_server.ttl = pub_set->ttl;
    pub_server.features = pub_set->features;
    pub_server.netkeyindex = pub_set->netkeyindex;

    if(pub_server.features > 0xf)
        pub_server.features = 0;

#if 0
    //if(count++ == 1)
    //    pub_server.status = 0x4;
#endif


    LOG(3, "config_heartbeat_publication_set_rx sub_countlog:%x pub_count:%x netkeyindex:%x\n", pub_set->countlog, pub_server.pub_count, pub_server.netkeyindex);

    config_heartbeat_publication_send_status(elmt, model, pdu);

    heartbeat_start_publication_timer();
}

void config_heartbeat_publication_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    config_heartbeat_publication_send_status(elmt, model, pdu);
}

