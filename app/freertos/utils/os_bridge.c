 
#include "os_bridge.h"
#include <stdbool.h>
#include "osapp_config.h"
#include "swint.h"

typedef struct __attribute__((packed))
{
    uint8_t type;
    ke_msg_id_t id;
    ke_task_id_t dest_id;
    ke_task_id_t src_id;
    uint16_t param_len;
    uint8_t param[];
}virtual_port_tx_buf_t;

struct virtual_port_channel
{
    /// call back function pointer
    void (*callback) (void*,uint8_t);
    //call back function parameter
    void *dummy;
};



struct virtual_port_env_tag
{
    struct virtual_port_channel rx;
    struct virtual_port_channel tx;
};

struct rx_state_t
{
    uint8_t *rx_bufptr;
    uint8_t *pheader;
    uint16_t rx_index;
    uint16_t rx_total_len;
};

static struct virtual_port_env_tag virtual_port_env;
static struct rx_state_t virtual_port_rx_stat;

static void ble_msg_rx_cb()
{
    void (*callback)(void *,uint8_t) = virtual_port_env.rx.callback;
    void *data = virtual_port_env.rx.dummy;
    virtual_port_env.rx.callback = NULL;
    virtual_port_env.rx.dummy = NULL;
    callback(data,RWIP_EIF_STATUS_OK);
    msg2blestack_from_ISR();
}

static void ble_msg_tx_cb()
{
    void (*callback)(void *,uint8_t) = virtual_port_env.tx.callback;
    void *data = virtual_port_env.tx.dummy;
    virtual_port_env.tx.callback = NULL;
    virtual_port_env.tx.dummy = NULL;
    callback(data,RWIP_EIF_STATUS_OK);
    msg2blestack_from_ISR();
}

void ble_msg_start_recv(uint8_t type,ble_rx_msg_t *msg)
{
    virtual_port_rx_stat.rx_bufptr[0] = type;
    virtual_port_rx_stat.rx_total_len = msg->param_len + sizeof(ble_rx_msg_t) - sizeof(uint32_t);
    virtual_port_rx_stat.pheader = (uint8_t *)msg;
    virtual_port_rx_stat.rx_index = 0;
    swint_req(ble_msg_rx_cb);
}

static void virtual_port_read(uint8_t *bufptr, uint32_t size, void (*callback) (void*,uint8_t), void *dummy)
{
    virtual_port_env.rx.callback = callback;
    virtual_port_env.rx.dummy = dummy;
    virtual_port_rx_stat.rx_bufptr = bufptr;
    if(virtual_port_rx_stat.pheader)
    {
        memcpy(bufptr,&virtual_port_rx_stat.pheader[virtual_port_rx_stat.rx_index],size);
        virtual_port_rx_stat.rx_index += size;
        if(virtual_port_rx_stat.rx_index == virtual_port_rx_stat.rx_total_len)
        {
            vPortFree(virtual_port_rx_stat.pheader);
            virtual_port_rx_stat.pheader = NULL;
        }
        swint_req(ble_msg_rx_cb);
    }
}


static void virtual_port_write(uint8_t *bufptr, uint32_t size, void (*callback) (void*,uint8_t), void *dummy)
{
    virtual_port_env.tx.callback = callback;
    virtual_port_env.tx.dummy = dummy;
    virtual_port_tx_buf_t *buf = (virtual_port_tx_buf_t *)bufptr;
    switch(buf->type)
    {
    case AHI_KE_MSG_TYPE:
    {
        uint8_t *param;
        if(buf->param_len)
        {
            param = pvPortMalloc(buf->param_len);
            BX_ASSERT(param);
            memcpy(param,buf->param,buf->param_len);
        }else
        {
            param = NULL;
        }
        bool sent = app_queue_ahi_rsp_send(buf->id,buf->src_id,buf->param_len,param,0);
        BX_ASSERT(sent);
    }
    break;
    default:
    
    break;
    }
    swint_req(ble_msg_tx_cb);
}

static void virtual_port_flow_on(void)
{

}


static bool virtual_port_flow_off(void)
{
    return true;
}

// Creation of virtual external interface api
static const struct rwip_eif_api virtual_port_api =
{
    virtual_port_read,
    virtual_port_write,
    virtual_port_flow_on,
    virtual_port_flow_off,
};
const struct rwip_eif_api *os_get_eif_api()
{
    return &virtual_port_api;
}


